#include <cglib/core/glheaders.h>
#include <cglib/gl/prefilter_envmap.h>
#include <glm/glm.hpp>
#include <cglib/core/image.h>
#include <cglib/rt/texture.h>

#define PI static_cast<float>(M_PI)

glm::vec2
lonlat_coordf_from_direction(glm::vec3 dir, glm::ivec2 size)
{
	return glm::vec2(
		0.5f + 0.5f * atan2(dir.z, dir.x) / PI,
		0.5f + 0.5f * asin(dir.y) / (PI / 2.0f)
	);
}

glm::ivec2
lonlat_coord_from_direction(glm::vec3 dir, glm::ivec2 size)
{
	auto fc = lonlat_coordf_from_direction(dir, size);
	return glm::clamp(glm::ivec2(fc * glm::vec2(size)), glm::ivec2(0), size);
}

glm::vec3
direction_from_lonlat_coord(glm::ivec2 c, glm::ivec2 size)
{
	auto fc = (glm::vec2(c) + glm::vec2(.5f)) / glm::vec2(size);
	float lon = (2.0f * fc.x - 1.0f) * PI;
	float lat = (2.0f * fc.y - 1.0f) * (PI / 2.0f);
	float r = cos(lat);
	return glm::vec3( r * cos(lon), sin(lat), r * sin(lon) );
}


float
solid_angle_from_lonlat_coord(glm::ivec2 c, glm::ivec2 size)
{
	auto fc = (glm::vec2(c) + glm::vec2(.5f)) / glm::vec2(size);
	
	float lon = (2.0f * fc.x - 1.0f) * PI;
	float lond = 2.0f * PI / float(size.x);
	float lat = (2.0f * fc.y - 1.0f) * (PI / 2.0f);
	float latd = 2.0f * PI / float(2 * size.y);

	float cosLon = cos(lon), cosLat = cos(lat);
	float sinLon = sin(lon), sinLat = sin(lat);

	float r = cosLat;
	float rd = -sinLat;

	return glm::length(glm::cross(
			lond * glm::vec3( r * -sinLon, 0.0f, r * cosLon ),
			latd * glm::vec3( rd * cosLon, cosLat, rd * sinLon )
		));
}

std::shared_ptr<Image>
prefilter_environment_diffuse(Image const& img)
{
	auto size = glm::ivec2(img.getWidth(), img.getHeight());
	auto filtered = std::make_shared<Image>(size.x, size.y);

	for (glm::ivec2 c(0); c.y < size.y; ++c.y) {
		for (c.x = 0; c.x < size.x; ++c.x) {
			// TODO: compute normal direction
			auto normal = direction_from_lonlat_coord(c, size);

			glm::vec4 color = glm::vec4(0.0f);

			// TODO: collect samples
			for (glm::ivec2 d(0); d.y < size.y; ++d.y) {
				for (d.x = 0; d.x < size.x; ++d.x) {
					// TODO: compute incident direction
					auto lightdir = direction_from_lonlat_coord(d, size);

					// TODO: compute weight
					float weight = glm::max(glm::dot(normal, lightdir), 0.0f) * solid_angle_from_lonlat_coord(d, size);

					// TODO: accumulate samples
					color += weight * img.getPixel(d.x, d.y);
				}
			}

			filtered->setPixel(c.x, c.y, color);
		}
	}

	return filtered;
}

std::shared_ptr<Image>
prefilter_environment_specular(Image const& img, float n)
{
	auto size = glm::ivec2(img.getWidth(), img.getHeight());
	auto filtered = std::make_shared<Image>(size.x, size.y);

	for (glm::ivec2 c(0); c.y < size.y; ++c.y) {
		for (c.x = 0; c.x < size.x; ++c.x) {
			// TODO: compute reflection direction
			auto reflectiondir = direction_from_lonlat_coord(c, size);

			glm::vec4 color = glm::vec4(0.0f);

			// TODO: collect samples
			for (glm::ivec2 d(0); d.y < size.y; ++d.y) {
				for (d.x = 0; d.x < size.x; ++d.x) {
					// TODO: compute incident direction
					auto lightdir = direction_from_lonlat_coord(d, size);

					// TODO: compute weight
					float weight = pow(glm::max(glm::dot(reflectiondir, lightdir), 0.0f), n) * solid_angle_from_lonlat_coord(d, size);

					// TODO: accumulate samples
					color += weight * img.getPixel(d.x, d.y);
				}
			}

			filtered->setPixel(c.x, c.y, color);
		}
	}

	return filtered;
}

void
resample_to_cubemap(ImageTexture const& lonlatmap)
{
	auto& lonlatimg = *lonlatmap.get_mip_levels()[0];
	auto lonlatsize = glm::ivec2(lonlatimg.getWidth(), lonlatimg.getHeight());
	
	int cubesize = lonlatsize.y / 2;
	std::vector<glm::vec4> cubeface(cubesize * cubesize);

	auto&& resample_to_face = [&](GLenum face, glm::vec3 facenrm, glm::vec3 axis1, glm::vec3 axis2)
	{
		for (glm::ivec2 c(0); c.y < cubesize; ++c.y) {
			for (c.x = 0; c.x < cubesize; ++c.x) {
				// compute direction from cube face px coords
				auto cf = (glm::vec2(c) + glm::vec2(.5f)) / float(cubesize) * 2.0f - glm::vec2(1.0f);
				auto dir = glm::normalize(facenrm + cf.x * axis1 + cf.y * axis2);

				// compute lonlat px coordinate
				auto llfc = lonlat_coordf_from_direction(dir, lonlatsize);
				cubeface[c.x + c.y * cubesize] = lonlatmap.evaluate_bilinear(0, llfc);
			}
		}

		glTexImage2D(face, 0, GL_RGBA, cubesize, cubesize, 0, GL_RGBA, GL_FLOAT, cubeface.data());
	};
	resample_to_face(GL_TEXTURE_CUBE_MAP_POSITIVE_X, glm::vec3(+1.0f, 0.0f, 0.0f), glm::vec3(0.0f, 0.0f, -1.0f), glm::vec3(0.0f, -1.0f, 0.0f));
	resample_to_face(GL_TEXTURE_CUBE_MAP_NEGATIVE_X, glm::vec3(-1.0f, 0.0f, 0.0f), glm::vec3(0.0f, 0.0f, +1.0f), glm::vec3(0.0f, -1.0f, 0.0f));
	resample_to_face(GL_TEXTURE_CUBE_MAP_POSITIVE_Y, glm::vec3(0.0f, +1.0f, 0.0f), glm::vec3(+1.0f, 0.0f, 0.0f), glm::vec3(0.0f, 0.0f, +1.0f));
	resample_to_face(GL_TEXTURE_CUBE_MAP_NEGATIVE_Y, glm::vec3(0.0f, -1.0f, 0.0f), glm::vec3(+1.0f, 0.0f, 0.0f), glm::vec3(0.0f, 0.0f, -1.0f));
	resample_to_face(GL_TEXTURE_CUBE_MAP_POSITIVE_Z, glm::vec3(0.0f, 0.0f, +1.0f), glm::vec3(+1.0f, 0.0f, 0.0f), glm::vec3(0.0f, -1.0f, 0.0f));
	resample_to_face(GL_TEXTURE_CUBE_MAP_NEGATIVE_Z, glm::vec3(0.0f, 0.0f, -1.0f), glm::vec3(-1.0f, 0.0f, 0.0f), glm::vec3(0.0f, -1.0f, 0.0f));
}
