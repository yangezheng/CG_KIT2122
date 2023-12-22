#include <cglib/rt/intersection.h>
#include <cglib/rt/object.h>
#include <cglib/rt/ray.h>
#include <cglib/rt/raytracing_context.h>
#include <cglib/rt/render_data.h>
#include <cglib/rt/renderer.h>
#include <cglib/rt/texture.h>
#include <cglib/rt/texture_mapping.h>
#include <cglib/core/thread_local_data.h>

#include <cglib/core/image.h>
#include <cglib/core/glmstream.h>
#include <cglib/core/assert.h>

#include <algorithm>


// -----------------------------------------------------------------------------

/*
 * Evaluates a texture for the given uv-coordinate without filtering.
 *
 * This method transformes the uv-coordinate into a st-coordinate and
 * rounds down to integer pixel values.
 *
 * The parameter level in [0, mip_levels.size()-1] is the miplevel of
 * the texture that is to be evaluated.
 */
glm::vec4 ImageTexture::
evaluate_nearest(int level, glm::vec2 const& uv) const
{
	cg_assert(level >= 0 && level < static_cast<int>(mip_levels.size()));
	cg_assert(mip_levels[level]);

	// TODO: compute the st-coordinates for the given uv-coordinates and mipmap level
	int s = 0;
	int t = 0;

	int width = this->mip_levels.at(level)->getWidth();
	int height = this->mip_levels.at(level)->getHeight();

	float u = glm::dot(glm::vec2(1.f, 0.f), uv);
	float v = glm::dot(glm::vec2(0.f, 1.f), uv);

	s = width * u;
	t = height * v;


	// get the value of pixel (s, t) of miplevel level
	return get_texel(level, s, t);
}

// -----------------------------------------------------------------------------

/*
 * Implement clamping here.
 *
 * The input "val" may be arbitrary, including negative and very large positive values.
 * The method shall always return a value in [0, size).
 * Out-of-bounds inputs must be clamped to the nearest boundary.
 */
int ImageTexture::
wrap_clamp(int val, int size)
{
	cg_assert(size > 0);

	if (val < 0) return 0;
	if (val >= size) return (size-1);

	return val;
}

/*
 * Implement repeating here.
 *
 * The input "val" may be arbitrary, including negative and very large positive values.
 * The method shall always return a value in [0, size).
 * Out-of-bounds inputs must be mapped back into [0, size) so that 
 * the texture repeats infinitely.
 */
int ImageTexture::
wrap_repeat(int val, int size)
{
	cg_assert(size > 0);

	while (val < 0) {
		val = val + size;
	}
	
	while (val >= size) {
		val = val - size; 
	}

	return val;
}


// -----------------------------------------------------------------------------


/*
 * Implement bilinear filtering here.
 *
 * Use mip_levels[level] as the image to filter.
 * The input uv coordinates may be arbitrary, including values outside [0, 1).
 *
 * Callers of this method must guarantee that level is valid, and
 * that mip_levels[level] is properly initialized. Compare the assertions.
 *
 * The color of a texel is to be interpreted as the color at the texel center.
 */
glm::vec4 ImageTexture::
evaluate_bilinear(int level, glm::vec2 const& uv) const
{
	cg_assert(level >= 0 && level < static_cast<int>(mip_levels.size()));
	cg_assert(mip_levels[level]);
	
	float x;
	float y;

	float width = this->mip_levels.at(level)->getWidth();
	float height = this->mip_levels.at(level)->getHeight();

	float u = glm::dot(glm::vec2(1.f, 0.f), uv);
	float v = glm::dot(glm::vec2(0.f, 1.f), uv);

	x = width * u;
	y = height * v;

	// t1 position
	int t1x = x;
	int t1y = y;
	t1y = y - t1y > 0 ? t1y+1 : t1y;
	
	// t2 position
	int t2x = x;
	int t2y = y;
	t2y = y - t2y > 0 ? t2y+1 : t2y;
	t2x = x - t2x > 0 ? t2x+1 : t2x;

	// t3 position
	int t3x = x;
	int t3y = y;

	// t4 position
	int t4x = x;
	int t4y = y;
	t4x = x - t4x > 0 ? t4x+1 : t4x;

	// a and b
	float a =  x - t3x;
	float oneMinusA = 1 - a;
	float b = t2y - y;
	float oneMinusB = 1 - b;

	// get texels' color
	glm::vec4 t1 = get_texel(level, t1x, t1y);
	glm::vec4 t2 = get_texel(level, t2x, t2y);
	glm::vec4 t3 = get_texel(level, t3x, t3y);
	glm::vec4 t4 = get_texel(level, t4x, t4y);
	
	glm::vec4 color = t1 * oneMinusA * oneMinusB + t2 * a * oneMinusB + t3 * oneMinusA * b + t4 * a * b;

	return color;
}

// -----------------------------------------------------------------------------

/*
 * This method creates a mipmap hierarchy for
 * the texture.
 *
 * This is done by iteratively reducing the
 * dimenison of a mipmap level and averaging
 * pixel values until the size of a mipmap
 * level is [1, 1].
 *
 * The original data of the texture is stored
 * in mip_levels[0].
 *
 * You can allocale memory for a new mipmap level 
 * with dimensions (size_x, size_y) using
 *		mip_levels.emplace_back(new Image(size_x, size_y));
 */
void ImageTexture::
create_mipmap()
{
	/* this are the dimensions of the original texture/image */
	int size_x = mip_levels[0]->getWidth();
	int size_y = mip_levels[0]->getHeight();

	cg_assert("must be power of two" && !(size_x & (size_x - 1)));
	cg_assert("must be power of two" && !(size_y & (size_y - 1)));

	int lastMip = 0;

	while (size_x > 1 && size_y > 1){
		size_x = size_x / 2;
		size_y = size_y / 2;
		Image tempImg = Image(size_x, size_y);
		mip_levels.emplace_back(new Image(size_x,size_y));
/* 		for(int i =1; i <= size_x ; i++ ){
			for (int j=1;j<= size_y; j++ ){
				glm::vec4 t1 = mip_levels[lastMip]->getPixel(i*2,j*2);
				glm::vec4 t2 = mip_levels[lastMip]->getPixel(i*2-1,j*2);
				glm::vec4 t3 = mip_levels[lastMip]->getPixel(i*2,j*2-1);
				glm::vec4 t4 = mip_levels[lastMip]->getPixel(i*2-1,j*2-1);
				glm::vec4 color = (t1+t2+t3+t4)*0.25f;
				tempImg.setPixel(i,j,color);
			}
		}
		lastMip ++;
		mip_levels[lastMip].reset(&tempImg); */
	}

}

/*
 * Compute the dimensions of the pixel footprint's AABB in uv-space.
 *
 * First intersect the four rays through the pixel corners with
 * the tangent plane at the given intersection.
 *
 * Then the given code computes uv-coordinates for these
 * intersection points.
 *
 * Finally use the uv-coordinates and compute the AABB in
 * uv-space. 
 *
 * Return width (du) and height (dv) of the AABB.
 *
 */
glm::vec2 Object::
compute_uv_aabb_size(const Ray rays[4], Intersection const& isect)
{
	// TODO: compute intersection positions
	glm::vec3 intersection_positions[4] = {
		isect.position, isect.position, isect.position, isect.position
	};

	for (int i = 0; i < 4; ++i) {
		// todo: compute intersection positions using a ray->plane
		// intersection
	}

	// compute uv coordinates from intersection positions
	glm::vec2 intersection_uvs[4];
	get_intersection_uvs(intersection_positions, isect, intersection_uvs);

	// TODO: compute dudv = length of sides of AABB in uv space
	return glm::vec2(0.0);
}

/*
 * Implement trilinear filtering at a given uv-position.
 *
 * Transform the AABB dimensions dudv in st-space and
 * take the maximal coordinate as the 1D footprint size T.
 *
 * Determine mipmap levels i and i+1 such that
 *		texel_size(i) <= T <= texel_size(i+1)
 *
 *	Hint: use std::log2(T) for that.
 *
 *	Perform bilinear filtering in both mipmap levels and
 *	linearly interpolate the resulting values.
 *
 */
glm::vec4 ImageTexture::
evaluate_trilinear(glm::vec2 const& uv, glm::vec2 const& dudv) const
{
	return glm::vec4(0.f);
}

// -----------------------------------------------------------------------------

/*
 * Transform the given direction d using the matrix transform.
 *
 * The output direction must be normalized, even if d is not.
 */
glm::vec3 transform_direction(glm::mat4 const& transform, glm::vec3 const& d)
{
	glm::vec3 tempd = glm::normalize(d);
	glm::vec4 temp(tempd[0],tempd[1],tempd[2],0.f);
	temp = transform * temp;
	glm::vec3 temp1(temp[0],temp[1],temp[2]);
	return glm::normalize(temp1);
}

/*
 * Transform the given position p using the matrix transform.
 */
glm::vec3 transform_position(glm::mat4 const& transform, glm::vec3 const& p)
{
	glm::vec3 tempp = glm::normalize(p);
	glm::vec4 temp(tempp[0],tempp[1],tempp[2],1.f);
	temp = transform * temp;
	glm::vec3 temp1(temp[0],temp[1],temp[2]);
	return glm::normalize(temp1);
}

/*
 * Intersect with the ray, but do so in object space.
 *
 * First, transform ray into object space. Use the methods you have
 * implemented for this.
 * Then, intersect the object with the transformed ray.
 * Finally, make sure you transform the intersection back into world space.
 *
 * isect is guaranteed to be a valid pointer.
 * The method shall return true if an intersection was found and false otherwise.
 *
 * isect must be filled properly if an intersection was found.
 */
bool Object::
intersect(Ray const& ray, Intersection* isect) const
{
	cg_assert(isect);

	if (RaytracingContext::get_active()->params.transform_objects) {
		// TODO: transform ray, intersect object, transform intersection
		// information back
	}
	return geo->intersect(ray, isect);
}


/*
 * Transform a direction from tangent space to object space.
 *
 * Tangent space is a right-handed coordinate system where
 * the tangent is your thumb, the normal is the index finger, and
 * the bitangent is the middle finger.
 *
 * normal, tangent, and bitangent are given in object space.
 * Build a matrix that rotates d from tangent space into object space.
 * Then, transform d with this matrix to obtain the result.
 * 
 * You may assume that normal, tangent, and bitangent are normalized
 * to length 1.
 *
 * The output vector must be normalized to length 1, even if d is not.
 */
glm::vec3 transform_direction_to_object_space(
	glm::vec3 const& d, 
	glm::vec3 const& normal, 
	glm::vec3 const& tangent, 
	glm::vec3 const& bitangent)
{
	cg_assert(std::fabs(glm::length(normal)    - 1.0f) < 1e-4f);
	cg_assert(std::fabs(glm::length(tangent)   - 1.0f) < 1e-4f);
	cg_assert(std::fabs(glm::length(bitangent) - 1.0f) < 1e-4f);
	return d;
}

// -----------------------------------------------------------------------------

// Optional bonus assignment! The fourier scene uses evaluate_bilinear(..)
// for displaying amplitude, phase and result images so you need a working
// solution for the bilinear filtering!
//
// Spectrum and reconstruction are 2D memory blocks of the same size [sx, sy]
// in row major layout, i.e one row of size sx after the other. 
//
// Spectrum contains the complex fourier-coefficients \hat{x}_{kl}.
//
// Reconstruct the original grayscale image using fourier transformation!
void DiscreteFourier2D::reconstruct(
	int M, int N,
	std::complex<float> const* spectrum,
	std::complex<float>	* reconstruction)
{
}
// CG_REVISION 30dc06794809ebd993cc23fa03a37b35c2d244d5
