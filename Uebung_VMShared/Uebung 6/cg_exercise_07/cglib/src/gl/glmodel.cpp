#include <cglib/core/glheaders.h>
#include <cglib/gl/glmodel.h>
#include <cglib/core/obj_mesh.h>
#include <cglib/core/image.h>
#include <cglib/core/assert.h>
#include <cglib/core/glmstream.h>

#include <iostream>
#include <map>
#include <unordered_map>

struct GLTextureList
{
	std::map<std::string, unsigned int> textureIDs;

	GLTextureList() :
		textureIDs() {}

	unsigned int getGLTexture(const std::string& filename, float gamma = 2.2f);
};

unsigned int GLTextureList::
getGLTexture(const std::string& filename, float gamma)
{
	const auto& i = textureIDs.find(filename);

	// already loaded?
	if (i != textureIDs.end())
		return i->second;

	auto tex = loadGLTexture(filename, gamma);
	textureIDs[filename] = tex;

	return tex;
}

unsigned int
loadGLTexture(const std::string& filename, float gamma)
{
	GLint internalFormat = GL_RGBA;
	if (gamma == 2.2f) {
		internalFormat = GL_SRGB_ALPHA;
		gamma = 1.0f;
	}

	Image img;
	img.load(filename, gamma);

	unsigned int glTex;
	glGenTextures(1, &glTex);
	glBindTexture(GL_TEXTURE_2D, glTex);
	glTexImage2D(GL_TEXTURE_2D, 0, internalFormat, img.getWidth(), img.getHeight(), 0, GL_RGBA, GL_FLOAT, img.getPixels());
	glGenerateMipmap(GL_TEXTURE_2D);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glBindTexture(GL_TEXTURE_2D, 0);

	return glTex;
}

struct Edge
{
	glm::vec3 v[2];
	glm::uvec2 idx;
};

static inline bool
operator==(const Edge &e1, const Edge &e2)
{
	return (all(equal(e1.v[0], e2.v[0])) && all(equal(e1.v[1], e2.v[1])))
		|| (all(equal(e1.v[0], e2.v[1])) && all(equal(e1.v[1], e2.v[0])));
}

static inline size_t
hash_vec3(const glm::vec3 &v)
{
	std::hash<float> hasher;
	size_t seed = hasher(v.x);
	seed ^= hasher(v.y) + 0x9e3779b9 + (seed << 6) + (seed >> 2);
	seed ^= hasher(v.z) + 0x9e3779b9 + (seed << 6) + (seed >> 2);
	return seed;
}

static inline size_t
hash_edge(const Edge &e)
{
	return hash_vec3(e.v[0]) ^ hash_vec3(e.v[1]);
}

GLObjModel::
GLObjModel(const std::string &path, int _flags)
	: flags(_flags)
{
	OBJFile obj_file(false);
	bool result = obj_file.loadFile(path);
	if(!result) {
		std::cerr << "could not load model \"" << path << "\"!" << std::endl;
		return;
	}
	glGenVertexArrays(1, &vao);
	glGenBuffers(1, &buf_vertex);
	glGenBuffers(1, &buf_normal);
	glGenBuffers(1, &buf_texcoord);
	glGenBuffers(1, &buf_index);

	std::vector<glm::vec3> vertices, normals;
	std::vector<glm::vec2> tex_coords;
	std::vector<uint32_t> indices;

	auto hash_func = [](const glm::uvec3 &v) -> size_t {
			std::hash<uint32_t> hasher;
			size_t seed = hasher(v.x);
			seed ^= hasher(v.y) + 0x9e3779b9 + (seed << 6) + (seed >> 2);
			seed ^= hasher(v.z) + 0x9e3779b9 + (seed << 6) + (seed >> 2);
			return seed;
	};

	std::unordered_map<glm::uvec3, uint32_t, decltype(hash_func)&> vertex_index_map(
			100000, hash_func
			);

	GLTextureList textures;

	for(unsigned i = 0; i < obj_file.getModelCount(); i++) {
		const auto &model = obj_file.getModel(i);
		vertex_index_map.clear();

		const auto &modelSurfaces = model->getSurfaces();
		for(const auto &surf: modelSurfaces) {

			GLObjModelSurface surfaceInfo;
			// save indices offset
			surfaceInfo.startIndex = indices.size();

			// get texture
      auto get_texture = [&surf, &textures](const std::string& name) {
			const auto iter = surf->material->additionalInfo.find(name);
			  if (iter != surf->material->additionalInfo.end())
			  {
			  	return textures.getGLTexture(iter->second);
			  }
        return 0U;
      };
      surfaceInfo.gl_texKd = get_texture("map_Kd");
      if (!surfaceInfo.gl_texKd) surfaceInfo.gl_texKd = textures.getGLTexture("assets/empty.png");
      surfaceInfo.gl_texKs = get_texture("map_Ks");
      surfaceInfo.gl_texBump = get_texture("map_Bump");

			surfaceInfo.kd = surf->material->diffuse;
			surfaceInfo.ks = surf->material->specular;

			for(unsigned j = 0; j < surf->vertexIndices.size(); j++) {
				for(int k = 0; k < 3; k++) {
					glm::uvec3 idx(~0);
					idx[0] = surf->vertexIndices[j][k];
					if(surf->normalIndices.size() > 0) {
						idx[1] = surf->normalIndices.at(j)[k];
					}
					if(surf->texcoordIndices.size() > 0) {
						idx[2] = surf->texcoordIndices.at(j)[k];
					}

					uint32_t vertex_idx;
					auto it = vertex_index_map.find(idx);
					if(it != vertex_index_map.end()) {
						vertex_idx = it->second;
					}
					else {
						vertices.push_back(model->getVertices().at(idx[0]));
						if(surf->normalIndices.size() > 0) {
							normals.push_back(model->getNormals().at(idx[1]));
						}
						if(surf->texcoordIndices.size() > 0) {
							tex_coords.push_back(model->getTexcoords().at(idx[2]));
						}
						vertex_idx = vertices.size() - 1;
						vertex_index_map[idx] = vertex_idx;
					}
					indices.push_back(vertex_idx);
				}
			}

			// save end index and store to surface list
			surfaceInfo.endIndex = indices.size();
			modelSurfaceRange.push_back(surfaceInfo);
		}
	}

	glBindVertexArray(vao);
	glBindBuffer(GL_ARRAY_BUFFER, buf_vertex);
	glBufferData(GL_ARRAY_BUFFER, sizeof(glm::vec3) * vertices.size(), vertices.data(), GL_STATIC_DRAW);
	glEnableVertexAttribArray(0);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, 0);

	if(normals.size() > 0) {
		glBindBuffer(GL_ARRAY_BUFFER, buf_normal);
		glBufferData(GL_ARRAY_BUFFER, sizeof(glm::vec3) * normals.size(), normals.data(), GL_STATIC_DRAW);
		glEnableVertexAttribArray(1);
		glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 0, 0);
	}

	if(tex_coords.size() > 0) {
		glBindBuffer(GL_ARRAY_BUFFER, buf_texcoord);
		glBufferData(GL_ARRAY_BUFFER, sizeof(glm::vec2) * tex_coords.size(), tex_coords.data(), GL_STATIC_DRAW);
		glEnableVertexAttribArray(2);
		glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, 0, 0);
	}


	bool adjacency = flags & ADJACENCY;
	if(adjacency) {
		std::unordered_map<Edge, Edge, decltype(hash_edge)&> edge_table(100000, hash_edge);
		//std::unordered_map<glm::uvec2, glm::uvec2, decltype(hf)&> edge_table(100000, hf);

		//edge_table[glm::uvec2(1337, 42)] = glm::uvec2(10, 10);
		//cg_assert(edge_table.count(glm::uvec2(42, 1337)) > 0);

		for(size_t i = 0; i < indices.size(); i += 3) {
			glm::uvec3 idx = glm::uvec3(indices[i], indices[i + 1], indices[i + 2]);

			for(int j = 0; j < 3; j++) {
				//glm::uvec2 e = glm::uvec2(idx[j % 3], idx[(j + 1) % 3]);
				Edge e;
				e.v[0] = vertices.at(idx[(j + 0) % 3]);
				e.v[1] = vertices.at(idx[(j + 1) % 3]);
				e.idx = glm::uvec2(~0);

				//std::cout << e.v[0] << " " << e.v[1] << std::endl;

				if(edge_table.count(e) == 0) {
					edge_table[e] = e;
				}
				else {
					//std::cout << "found edge" << std::endl;
				}
				//std::cout << all(equal(edge_table[e].v[0], vertices[idx[j % 3]])) << std::endl;
				edge_table[e].idx[all(equal(edge_table[e].v[0], vertices[idx[j % 3]]))] = idx[(j + 2) % 3];
			}
		}

		std::vector<uint32_t> indices_adj;
		indices_adj.reserve(indices.size() * 3);
		
		for(size_t i = 0; i < indices.size(); i += 3) {
			glm::uvec3 idx = glm::uvec3(indices[i], indices[i + 1], indices[i + 2]);

			for(int j = 0; j < 3; j++) {
				//glm::uvec2 e = glm::uvec2(idx[j % 3], idx[(j + 1) % 3]);
				//glm::uvec2 ee = glm::uvec2(std::min(e.x, e.y), std::max(e.x, e.y));

				Edge e;
				e.v[0] = vertices[idx[(j + 0) % 3]];
				e.v[1] = vertices[idx[(j + 1) % 3]];

				cg_assert(edge_table.count(e) > 0);
				//auto adj = edge_table[ee];


				indices_adj.push_back(idx[j % 3]);
				auto adj = edge_table[e].idx[!all(equal(edge_table[e].v[0], vertices[idx[j % 3]]))];
				// valid adjacency information found
				if(adj != ~0u) {
					indices_adj.push_back(adj);
				}
				else {
					indices_adj.push_back(idx[j % 3]);
				}
			}
		}

		indices = indices_adj;
	}



	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, buf_index);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, indices.size() * sizeof(uint32_t), indices.data(), GL_STATIC_DRAW);

	glBindVertexArray(0);


}

GLObjModel::
~GLObjModel()
{
	glDeleteVertexArrays(1, &vao);
	glDeleteBuffers(1, &buf_vertex);
	glDeleteBuffers(1, &buf_normal);
	glDeleteBuffers(1, &buf_texcoord);
	glDeleteBuffers(1, &buf_index);
}

void GLObjModel::
draw()
{
	bool adjacency = flags & ADJACENCY;
	glBindVertexArray(vao);

	// draw each surface with texture
	for (const GLObjModelSurface& faceData : modelSurfaceRange)
	{
    if (faceData.gl_texBump) 
    {
      glActiveTexture(GL_TEXTURE2);
  	  glBindTexture(GL_TEXTURE_2D, faceData.gl_texBump);
    }
    if (faceData.gl_texKs) 
    {
      glActiveTexture(GL_TEXTURE1);
  	  glBindTexture(GL_TEXTURE_2D, faceData.gl_texKs);
    }
    if (faceData.gl_texKd) 
    {
      glActiveTexture(GL_TEXTURE0);
		  glBindTexture(GL_TEXTURE_2D, faceData.gl_texKd);
    }

		unsigned int numIndices = faceData.endIndex - faceData.startIndex;
		if(!adjacency) {
			glDrawElements(GL_TRIANGLES, numIndices, GL_UNSIGNED_INT, (char*)0 + sizeof(unsigned int) * faceData.startIndex);
		}
		else {
			glDrawElements(GL_TRIANGLES_ADJACENCY, numIndices * 2, GL_UNSIGNED_INT, (char*)0 + sizeof(unsigned int) * faceData.startIndex * 2);
		}
	}

	glBindVertexArray(0);
}

void GLObjModel::
draw(int surface_range_idx) 
{
  if (surface_range_idx >= modelSurfaceRange.size() || surface_range_idx < 0) return;

	bool adjacency = flags & ADJACENCY;
	glBindVertexArray(vao);

  auto& faceData = modelSurfaceRange[surface_range_idx];

  if (faceData.gl_texBump) 
  {
    glActiveTexture(GL_TEXTURE2);
	  glBindTexture(GL_TEXTURE_2D, faceData.gl_texBump);
  }
  if (faceData.gl_texKs) 
  {
    glActiveTexture(GL_TEXTURE1);
	  glBindTexture(GL_TEXTURE_2D, faceData.gl_texKs);
  }
  if (faceData.gl_texKd) 
  {
    glActiveTexture(GL_TEXTURE0);
	  glBindTexture(GL_TEXTURE_2D, faceData.gl_texKd);
  }

	unsigned int numIndices = faceData.endIndex - faceData.startIndex;
	if(!adjacency) {
		glDrawElements(GL_TRIANGLES, numIndices, GL_UNSIGNED_INT, (char*)0 + sizeof(unsigned int) * faceData.startIndex);
	}
	else {
		glDrawElements(GL_TRIANGLES_ADJACENCY, numIndices * 2, GL_UNSIGNED_INT, (char*)0 + sizeof(unsigned int) * faceData.startIndex * 2);
	}

	glBindVertexArray(0);
}


int GLObjModel::
index_count() const
{
	int vc = 0;
	for(auto &i: modelSurfaceRange) {
		vc += (i.endIndex - i.startIndex);
	}
	return vc;
}
