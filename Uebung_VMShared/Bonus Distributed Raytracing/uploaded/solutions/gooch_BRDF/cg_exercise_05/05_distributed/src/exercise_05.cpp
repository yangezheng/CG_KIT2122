#include <cglib/rt/renderer.h>

#include <cglib/rt/epsilon.h>
#include <cglib/rt/intersection.h>
#include <cglib/rt/object.h>
#include <cglib/rt/light.h>
#include <cglib/rt/ray.h>
#include <cglib/rt/raytracing_context.h>
#include <cglib/rt/render_data.h>
#include <cglib/rt/scene.h>
#include <cglib/core/glmstream.h>
#include <exception>
#include <stdexcept>

#include <cglib/core/thread_local_data.h>

/*
 * Creates a random sample on a unit disk
 *
 * Parameters:
 * - u1: random number [0,1)
 * - u2: random number [0,1)
 */
glm::vec2
uniform_sample_disk(float u1, float u2)
{
	// TODO DOF: implement uniform sampling on a unit disk here
	float r = glm::sqrt(u1);
	glm::vec2 p = glm::vec2(r*glm::cos(2*M_PI*u2), r*glm::sin(2*M_PI*u2));
	return p;
}

/*
 * Creates a random sample on a unit sphere
 *
 * Parameters:
 * - u1: random number [0,1)
 * - u2: random number [0,1)
 */
glm::vec3
uniform_sample_sphere(float u1, float u2)
{
	// TODO AmbientOcclusion/IndirectIllumination: 
	// implement uniform sampling on a unit sphere
	float h = 1.0f - 2*u1;
	float r = glm::sqrt(1.0f - glm::pow(h,2));
	glm::vec3 d = glm::vec3(r * glm::cos(2*M_PI*u2),h,r * glm::sin(2*M_PI*u2));

	return d;
}

/*
 * Creates a random sample on a hemisphere with
 * normal direction N
 *
 * Parameters:
 * - data: RenderData for access to random number generator
 * - N: main direction of the hemisphere
 */
glm::vec3
uniform_sample_hemisphere(RenderData & data, glm::vec3 const& N)
{
	// TODO AmbientOcclusion/IndirectIllumination: 
	// implement uniform sampling on a unit hemisphere.
	// data.tld->rand() creates uniform [0, 1] random numbers
	// TIP: use uniform_sample_sphere 

	glm::vec3 d = uniform_sample_sphere(data.tld->rand(),data.tld->rand());
	// if the angle of two vector is greater than 90, then dot product is a minus number
	while (glm::dot(d,N) < 0){
		d = uniform_sample_sphere(data.tld->rand(),data.tld->rand());
	}
	return d;
}

// returns a random uniformly distributed point on the light source
glm::vec3 AreaLight::uniform_sample_point(float x0, float x1) const
{
	// TODO SoftShadow: Use the two random numbers to uniformly sample
	// a point on the area light
	return this->position + x0 * this->tangent + x1 * this->bitangent;
}



glm::vec3 trace_recursive_with_lens(RenderData & data, Ray const& ray, int depth)
{
	glm::vec3 contribution(0.0f);

	if (data.context.params.dof && data.context.params.dof_rays > 0)
	{
		// TODO DOF: compute point on focus plane that is common to all rays of this pixel
		// get some free lines of code
		glm::vec3 camera_direction = data.context.get_active_scene()->camera->get_direction();
		glm::vec3 camera_position = data.context.get_active_scene()->camera->get_position(data.camera_mode);
		float focal_length = data.context.params.focal_length;
		glm::mat4 inverse_view_matrix = data.context.get_active_scene()->camera->get_inverse_view_matrix(data.camera_mode);
		float radius = data.context.params.lens_radius;

		// 1. step: get the focal plane
		// the camera direction is perpendicular to focal plane
		// the distance between camera position to the intersection point is focal_length
		// the focal plane's normal is the camera_direction, since it's perpendicular to each other
		glm::vec3 focal_plane_center = camera_position + glm::normalize(camera_direction)*focal_length;
		glm::vec3 focal_plane_normal = glm::normalize(camera_direction);
		//get focal point
		float t;  // parametric distance from ray origin to focal_point
		if (intersect_plane(ray.origin,ray.direction,focal_plane_center,focal_plane_normal,&t) == false){
			return glm::vec3(0.f);
		}
		glm::vec3 focal_point = ray.origin + (glm::normalize(ray.direction) * t);

		// TODO DOF: sample random points on the lens
		for ( int i = 0; i < data.context.params.dof_rays; i++){
			glm::vec4 sample_point_camera_space = glm::vec4(uniform_sample_disk(data.tld->rand(),data.tld->rand()) * radius, 0.f, 1.f);
			glm::vec3 new_ray_position =  inverse_view_matrix * sample_point_camera_space;
			glm::vec3 new_ray_direction = glm::normalize(focal_point - new_ray_position);

			// TODO DOF: generate ray from the sampled point on the
			// lens through the common point in the focus plane
			Ray new_ray = Ray(new_ray_position,new_ray_direction);

			// TODO DOF: start ray tracing with the new lens ray
			contribution += trace_recursive(data, new_ray, depth);
		}
		// TODO DOF: compute average contribution of all lens rays
		contribution /= float(data.context.params.dof_rays);
	}
	else
		contribution = trace_recursive(data, ray, depth);
	return contribution;
}

float evaluate_ambient_occlusion(
	RenderData &data,           // class containing raytracing information
	glm::vec3 const& P,         // world space position
	glm::vec3 const& N)         // normal at the position (already normalized)
{
	// TODO AmbientOcclusion: compute ambient occlusion
	float ambient_occlusion = 0.f;
	for (int i = 0; i < data.context.params.ao_rays; i++) {
		glm::vec3 random_direciton = glm::normalize(uniform_sample_hemisphere(data,N));
		float distance = max_unobstructed_distance(data,P,random_direciton);
		float v = distance == FLT_MAX ? 1 : 1.f - (1.f / (1 + glm::pow(data.context.params.half_ao_radius,-2) * glm::pow(distance,2)));
		ambient_occlusion += v * glm::dot(random_direciton,N);
	}
	ambient_occlusion = ambient_occlusion / data.context.params.ao_rays * 2;

	return ambient_occlusion;
}

glm::vec3 evaluate_illumination_from_light(
	RenderData &data,           // class containing raytracing information
	MaterialSample const& mat,  // the material at position
	Light const& light,         // the light source
	glm::vec3 const& LP,        // a point on the light source
	glm::vec3 const& P,         // world space position
	glm::vec3 const& N,         // normal at the position (already normalized)
	glm::vec3 const& V)         // view vector (already normalized)
{
	glm::vec3 L = LP - P;                       // direction to the light
	const float dist2 = glm::dot(L, L);         // compute squared distance to light point
	L /= sqrt(dist2);                           // normalize direction

	float visibility = 1.f;
	if (data.context.params.shadows) {
		if (!visible(data, P, LP)) {
			visibility = 0.f;
		}
	}	

	auto incomingLight = visibility * light.getEmission(-L) / dist2;
	return evaluate_gooch_BRDF(data, mat, L, N, V) * incomingLight;
}


glm::vec3 evaluate_illumination(
	RenderData &data,           // class containing raytracing information
	MaterialSample const& mat,  // the material at position
	glm::vec3 const& P,         // world space position
	glm::vec3 const& N,         // normal at the position (already normalized)
	glm::vec3 const& V,         // view vector (already normalized)
	int depth)                  // the current recursion depth
{
	glm::vec3 direct_illumination(0.f);
	if (!data.context.params.disable_direct || depth > 1)
	{
		if (data.context.params.soft_shadow)
		{
			for (auto& light : data.context.get_active_scene()->area_lights)
			{
				glm::vec3 contribution_of_one_light = glm::vec3(0.f);
				// TODO SoftShadow: sample point on light source for soft shadows
				for (int i = 0; i < data.context.params.shadow_rays; i ++) {
					const glm::vec3 LP = light->uniform_sample_point(data.tld->rand(), data.tld->rand());
					float visibility = visible(data,P,LP) ? 1.f : 0.f;
					glm::vec3 light_direction = glm::normalize(LP - P);
					glm::vec3 BRDF = evaluate_phong_BRDF(data,mat,light_direction,N,V);
					glm::vec3 emission = light->getEmission(-light_direction);
					contribution_of_one_light += BRDF*emission*visibility/glm::distance(P,LP);
				}
				direct_illumination += light->get_area() / data.context.params.shadow_rays * contribution_of_one_light;
				(void) light; // prevent unused warning
			}
		}
		else {
			for (auto& light : data.context.get_active_scene()->lights) {
				const glm::vec3 LP = light->getPosition();
				direct_illumination += evaluate_illumination_from_light(
						data, mat, *light, LP, P, N, V);
			}
			direct_illumination /= data.context.get_active_scene()->lights.size();
		}
	}

	

	glm::vec3 indirect_illumination(0.f);
	if (data.context.params.indirect) 
	{
		// TODO IndirectIllumination: compute indirect illumination
		glm::vec3 sum = glm::vec3(0.f);
		for (int i =0; i < data.context.params.indirect_rays; i++){
			const glm::vec3 sample_point = uniform_sample_hemisphere(data,N);
			Ray new_ray = Ray(P,glm::normalize(sample_point));
			glm::vec3 ray_contribution = trace_recursive(data,new_ray,depth+1);
			glm::vec3 light_direction = glm::normalize(sample_point);
			glm::vec3 BRDF = evaluate_phong_BRDF(data,mat,light_direction,N,V);
			sum += ray_contribution * BRDF;
		}
		indirect_illumination = sum * (float)(2.f * M_PI / data.context.params.indirect_rays);
	}

	return direct_illumination + indirect_illumination;
}

// CG_REVISION bec57fb16d91551176a88d64c4775a0b7642b469
