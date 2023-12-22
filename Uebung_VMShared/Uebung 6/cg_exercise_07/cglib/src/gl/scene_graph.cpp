#include <cglib/gl/scene_graph.h>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/matrix_inverse.hpp>

	#define MAYBE_REFERENCE_FUNCTION(name) name

std::shared_ptr<SceneGraphNode>
MAYBE_REFERENCE_FUNCTION(buildSphereFlakeSceneGraph)(const std::shared_ptr<GLObjModel>& model, float sphere_model_radius, float size_factor, int number_of_remaining_recursions)
{
	// TODO: create root SceneGraphNode and set model
	auto root = std::make_shared<SceneGraphNode>();
	root->model = model;

	// TODO: only create children if there are remaining recursions
	if (number_of_remaining_recursions <= 0)
		return root;

	float child_distance_from_center = sphere_model_radius + sphere_model_radius * size_factor;

	glm::mat4 child_scale_mat = glm::scale(glm::mat4(1.f),glm::vec3(size_factor));
	glm::mat4 child_translation_mat = glm::translate(glm::mat4(1.f),glm::vec3(0, child_distance_from_center, 0));
	
	glm::mat4 child_rotation_matrices[5];
	// top
	child_rotation_matrices[0] = glm::mat4(1.f);
	// left
	child_rotation_matrices[1] = glm::rotate(glm::mat4(1.f), float(M_PI) / 2.f, glm::vec3(0, 0, 1));
	// right
	child_rotation_matrices[2] = glm::rotate(glm::mat4(1.f), float(M_PI) / 2.f, glm::vec3(0, 0, -1));
	// front
	child_rotation_matrices[3] = glm::rotate(glm::mat4(1.f), float(M_PI) / 2.f, glm::vec3(1, 0, 0));
	// back
	child_rotation_matrices[4] = glm::rotate(glm::mat4(1.f), float(M_PI) / 2.f, glm::vec3(-1, 0, 0));

	// TODO: create 5 child nodes
	for (int i = 0; i < 5; ++i)
	{
		// TODO: create sphere flake sub graph for each child
		auto subgraph = MAYBE_REFERENCE_FUNCTION(buildSphereFlakeSceneGraph)(model, sphere_model_radius, size_factor, number_of_remaining_recursions - 1);

		// TODO: compute transformation matrix from the child node to the parent node (this function call's root node)
		subgraph->node_to_parent = child_rotation_matrices[i] * child_translation_mat * child_scale_mat;
		subgraph->parent_to_node = glm::inverse(subgraph->node_to_parent);

		// TODO: add each subgraph to the children of this function call's root node
		root->children.push_back(subgraph);
	}

	return root;
}

void
SceneGraphNode::
MAYBE_REFERENCE_FUNCTION(collectTransformedModels)(std::vector<TransformedModel>& transformed_models, const glm::mat4& parent_to_world, const glm::mat4& world_to_parent) const
{
	// TODO: compute node_to_world and world_to_node transformation matrices for this node
	auto node_to_world = parent_to_world * node_to_parent;
	auto world_to_node = parent_to_node * world_to_parent;

	// TODO: add this node's model to the list of transformed models
	transformed_models.push_back( TransformedModel(node_to_world, world_to_node, model.get()) );

	// TODO: recursively transform and add the models of all children (subgraphs)
	for (auto& child : children)
		child->MAYBE_REFERENCE_FUNCTION(collectTransformedModels)(transformed_models, node_to_world, world_to_node);
}

void
MAYBE_REFERENCE_FUNCTION(animateSphereFlake)(SceneGraphNode& root, float angle_increment)
{
	// TODO: compute incremental rotation matrix for the given root node
	auto rotation = glm::rotate(glm::mat4(1.f), angle_increment, glm::vec3(0, 1, 0));
	auto inverse_rotation = glm::transpose(rotation);

	// TODO: compute the parent-relative transformation matrices for this node
	root.node_to_parent = root.node_to_parent * rotation;
	root.parent_to_node = inverse_rotation * root.parent_to_node;

	// TODO: recursively animate child subgraphs
	for (auto& child : root.children)
		MAYBE_REFERENCE_FUNCTION(animateSphereFlake)(*child, angle_increment);
}

