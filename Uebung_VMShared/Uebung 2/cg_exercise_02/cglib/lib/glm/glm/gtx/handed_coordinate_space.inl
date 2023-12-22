/// @ref gtx_handed_coordinate_space
/// @file glm/gtx/handed_coordinate_space.inl

namespace glm
{
	template<typename T, qualifier Q>
	GLM_FUNC_QUALIFIER bool rightHanded
	(
		vec<3, T, Q> const& tangent,
		vec<3, T, Q> const& binormal,
		vec<3, T, Q> const& normal
	)
	{
		return dot(cross(normal, tangent), binormal) > T(0);
	}

	template<typename T, qualifier Q>
	GLM_FUNC_QUALIFIER bool leftHanded
	(
		vec<3, T, Q> const& tangent,
		vec<3, T, Q> const& binormal,
		vec<3, T, Q> const& normal
	)
	{
		return dot(cross(normal, tangent), binormal) < T(0);
	}
}//namespace glm
// CG_REVISION 2bae04205e2a2016dbed23f9191c5cea7926ff82
