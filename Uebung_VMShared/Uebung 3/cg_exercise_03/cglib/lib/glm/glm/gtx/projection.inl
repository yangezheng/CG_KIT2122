/// @ref gtx_projection
/// @file glm/gtx/projection.inl

namespace glm
{
	template<typename genType>
	GLM_FUNC_QUALIFIER genType proj(genType const& x, genType const& Normal)
	{
		return glm::dot(x, Normal) / glm::dot(Normal, Normal) * Normal;
	}
}//namespace glm
// CG_REVISION 30dc06794809ebd993cc23fa03a37b35c2d244d5
