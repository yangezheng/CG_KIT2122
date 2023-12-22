/// @ref core
/// @file glm/detail/func_geometric.inl

#include <limits>

namespace glm {
namespace detail
{
	template<typename T, qualifier Q, bool Aligned>
	struct compute_cross_vec2
	{
		GLM_FUNC_QUALIFIER static T call(vec<2, T, Q> const& v, vec<2, T, Q> const& u)
		{
			GLM_STATIC_ASSERT(std::numeric_limits<T>::is_iec559, "'cross' accepts only floating-point inputs");

			return v.x * u.y - u.x * v.y;
		}
	};
}//namespace detail

	template<typename T, qualifier Q>
	GLM_FUNC_QUALIFIER T cross(vec<2, T, Q> const& x, vec<2, T, Q> const& y)
	{
		return detail::compute_cross_vec2<T, Q, detail::is_aligned<Q>::value>::call(x, y);
	}
}//namespace glm

// CG_REVISION 853babea119d24f6c9aca86013fd5e6a3fb22330
