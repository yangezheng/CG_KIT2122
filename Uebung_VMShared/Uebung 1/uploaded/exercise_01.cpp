#include <cglib/colors/exercise.h>
#include <cglib/colors/convert.h>
#include <cglib/colors/cmf.h>

#include <cglib/core/glheaders.h>
#include <cglib/core/glmstream.h>

#include <cglib/core/assert.h>
#include <iostream>

/*
 * Draw the given vertices directly as GL_TRIANGLES.
 * For each vertex, also set the corresponding color.
 */
void draw_triangles(
	std::vector<glm::vec3> const& vertices,
	std::vector<glm::vec3> const& colors)
{
	cg_assert(vertices.size() == colors.size());
	cg_assert(vertices.size() % 3 == 0);

	glBegin(GL_TRIANGLES);
		for (int i = 0; i <= vertices.size(); i ++){
			glColor3f(colors[i].x,colors[i].y,colors[i].z);
			glVertex3f(vertices[i].x,vertices[i].y,vertices[i].z);
		}
	glEnd();


}

/*
 * Generate a regular grid of resolution N*N (2*N*N triangles) in the xy-plane (z=0).
 * Store the grid in vertex-index form.
 *
 * The vertices of the triangles should be in counter clock-wise order.
 *
 * The grid must fill exactly the square [0, 1]x[0, 1], and must
 * be generated as an Indexed Face Set (Shared Vertex representation).
 * 
 * The first vertex should be at position (0,0,0) and the last
 * vertex at position (1,1,0)
 *
 * An example for N = 3:
 *
 *   ^
 *   |  ----------
 *   |  |\ |\ |\ |
 *   |  | \| \| \|
 *   |  ----------
 *   |  |\ |\ |\ |
 * y |  | \| \| \|
 *   |  ----------
 *   |  |\ |\ |\ |
 *   |  | \| \| \|
 *   |  ----------
 *   |
 *   |-------------->
 *          x
 *
 */
void generate_grid(
	std::uint32_t N,
	std::vector<glm::vec3>* vertices,
	std::vector<glm::uvec3>* indices)
{
	cg_assert(N >= 1);
	cg_assert(vertices);
	cg_assert(indices);

	vertices->clear();
	indices->clear();

	for (int row = 0; row <= N; row++) {
        for (int col = 0; col <= N; col++) {
            vertices->push_back(glm::vec3((float)col / N , (float)row / N , 0.0));
			indices->push_back(glm::uvec3(col, col+1, col+(N*(1+row))));
			indices->push_back(glm::uvec3(col+(N*(1+row)), col+1, col+(N*(1+row)+1)));
		}
    }

}

/*
 * Draw the given vertices as indexed GL_TRIANGLES using glDrawElements.
 * For each vertex, also set the corresponding color.
 *
 * Don't forget to enable the correct client states. After drawing
 * the triangles, you need to disable the client states again.
 */
void draw_indexed_triangles(
	std::vector<glm::vec3>  const& vertices,
	std::vector<glm::vec3>  const& colors,
	std::vector<glm::uvec3> const& indices)
{
	cg_assert(vertices.size() == colors.size());
	
	// Enabling Arrays
	glEnableClientState(GL_VERTEX_ARRAY);
	glEnableClientState(GL_COLOR_ARRAY);
	
	// Specifying Data for the Arrays
	glVertexPointer(3, GL_FLOAT, 0, vertices.data());
	glColorPointer(3,GL_FLOAT, 0, colors.data());
	
	// Dereference a Sequence of Array Elements
	glDrawElements(GL_TRIANGLES, indices.size() *3, GL_UNSIGNED_INT, indices.data());
	
	// Closing Arrays
	glDisableClientState(GL_VERTEX_ARRAY);
	glDisableClientState(GL_COLOR_ARRAY);
}

/*
 * Generate a triangle strip with N segments (2*N triangles)
 * in the xy plane (z=0).
 *
 * The vertices of the triangles should be in counter clock-wise order.
 *
 * The triangle strip must fill exactly the square [0, 1]x[0, 1].
 *
 * The first vertex should be at position (0,1,0) and the last
 * vertex at position (1,0,0)
 *
 * An example for N = 3:
 *
 *   ^
 *   |  ----------
 *   |  | /| /| /|
 * y |  |/ |/ |/ |
 *   |  ----------
 *   |
 *   |-------------->
 *           x
 *
 */
void generate_strip(
	std::uint32_t N,
	std::vector<glm::vec3>* vertices)
{
	cg_assert(N >= 1);
	cg_assert(vertices);

	vertices->clear();

    for (int col = 0; col <= N; col++) {
        vertices->push_back(glm::vec3((float)col / N , 1.0 , 0.0));
		vertices->push_back(glm::vec3((float)col / N , 0.0 , 0.0));
    }

}

/*
 * Draw the given vertices as a triangle strip.
 * For each vertex, also set the corresponding color.
 */
void draw_triangle_strip(
	std::vector<glm::vec3> const& vertices,
	std::vector<glm::vec3> const& colors)
{
	cg_assert(vertices.size() == colors.size());

	std::vector<glm::uvec3>* indices;
	for (int i = 0; i <= (vertices.size()-2)/2; i++){
		indices->push_back(glm::uvec3(i,i+1, i+2));
		indices->push_back(glm::uvec3(i+2,i+1, i+3));
	}
	draw_indexed_triangles(vertices, colors, *indices);

}

/*
 * Integrate the given piecewise linear function 
 * using trapezoidal integration.
 *
 * The function is given at points
 *     x[0], ..., x[N]
 * and its corresponding values are
 *     y[0], ..., y[N]
 */
float integrate_trapezoidal(
	std::vector<float> const& x,
	std::vector<float> const& y)
{
	cg_assert(x.size() == y.size());
	cg_assert(x.size() > 1);

	return 0.f;
}

/*
 * Convert the given spectrum to RGB using your
 * implementation of integrate_trapezoidal(...)
 *
 * The color matching functions and the wavelengths
 * for which they are given can be found in
 *     cglib/colors/cmf.h
 * and
 *     cglib/src/colors/cmf.cpp
 *
 * The wavelengths corresponding to the spectral values 
 * given in spectrum are defined in cmf::wavelengths
 */
glm::vec3 spectrum_to_rgb(std::vector<float> const& spectrum)
{
	cg_assert(spectrum.size() == cmf::wavelengths.size());

	return glm::vec3(0.f);
}
// CG_REVISION 058df19c2a3171c31e1c8d1972576f6014c87d14
