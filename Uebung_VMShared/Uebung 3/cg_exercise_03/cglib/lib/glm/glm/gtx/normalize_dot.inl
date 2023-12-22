/// @ref gtx_normalize_dot
/// @file glm/gtx/normalize_dot.inl

namespace glm
{
	template<length_t L, typename T, qualifier Q>
	GLM_FUNC_QUALIFIER T normalizeDot(vec<L, T, Q> const& x, vec<L, T, Q> const& y)
	{
		return glm::dot(x, y) * glm::inversesqrt(glm::dot(x, x) * glm::dot(y, y));
	}

	template<length_t L, typename T, qualifier Q>
	GLM_FUNC_QUALIFIER T fastNormalizeDot(vec<L, T, Q> const& x, vec<L, T, Q> const& y)
	{
		return glm::dot(x, y) * glm::fastInverseSqrt(glm::dot(x, x) * glm::dot(y, y));
	}
}//namespace glm
// CG_REVISION 30dc06794809ebd993cc23fa03a37b35c2d244d5
