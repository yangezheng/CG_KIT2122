/// @ref gtx_perpendicular
/// @file glm/gtx/perpendicular.inl

namespace glm
{
	template<typename genType>
	GLM_FUNC_QUALIFIER genType perp(genType const& x, genType const& Normal)
	{
		return x - proj(x, Normal);
	}
}//namespace glm
// CG_REVISION bec57fb16d91551176a88d64c4775a0b7642b469
