#include <cglib/gl/renderer.h>

#include <glm/gtx/transform.hpp>
#include <glm/gtc/matrix_inverse.hpp>

glm::mat4
compute_view_projection_light(
		const glm::vec3 &light_position,  // position of the light source in world space
		const glm::vec3 &light_direction, // light view direction
		float near_clip_plane,            // distance to the near clip plane
		float far_clip_plane,             // distance to the far clip plane
		float field_of_view,              // field of view in radians
		const glm::vec3 &up_vector)       // vector pointing upwards in world space
{
	// compute the View Projection Matrix for the light source
	glm::mat4 view_matrix = glm::lookAt(light_position,light_direction,up_vector);
	glm::mat4 projection_matrix = glm::perspective(field_of_view,1.0f,near_clip_plane,far_clip_plane);
	return projection_matrix * view_matrix;

}

void
initialize_alpha_blending()
{
	// TODO: Set up the blend equation and blend function for 
	// alpha blending.
	glEnable(GL_BLEND);
	glBlendEquation(GL_FUNC_ADD);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_CONSTANT_ALPHA);
}

void
initialize_additive_blending()
{
	// TODO: Set up the blend equation and blend function for 
	// additive blending.
	glEnable(GL_BLEND);
	glBlendEquation(GL_FUNC_ADD);
	glBlendFunc(GL_ONE,GL_ONE);
}

void
initialize_premultiplied_blending()
{
	// TODO: Set up the blend equation and blend function for 
	// blending with premultiplied alpha.
	glEnable(GL_BLEND);
	glBlendEquation(GL_FUNC_ADD);
	glBlendFunc(GL_ONE,GL_ONE_MINUS_CONSTANT_ALPHA);
}

// CG_REVISION bec57fb16d91551176a88d64c4775a0b7642b469
