/// @ref gtx_scalar_relational
/// @file glm/gtx/scalar_relational.inl

namespace glm
{
	template<typename T>
	GLM_FUNC_QUALIFIER bool lessThan
	(
		T const& x,
		T const& y
	)
	{
		return x < y;
	}

	template<typename T>
	GLM_FUNC_QUALIFIER bool lessThanEqual
	(
		T const& x,
		T const& y
	)
	{
		return x <= y;
	}

	template<typename T>
	GLM_FUNC_QUALIFIER bool greaterThan
	(
		T const& x,
		T const& y
	)
	{
		return x > y;
	}

	template<typename T>
	GLM_FUNC_QUALIFIER bool greaterThanEqual
	(
		T const& x,
		T const& y
	)
	{
		return x >= y;
	}

	template<typename T>
	GLM_FUNC_QUALIFIER bool equal
	(
		T const& x,
		T const& y
	)
	{
		return detail::compute_equal<T>::call(x, y);
	}

	template<typename T>
	GLM_FUNC_QUALIFIER bool notEqual
	(
		T const& x,
		T const& y
	)
	{
		return !detail::compute_equal<T>::call(x, y);
	}

	GLM_FUNC_QUALIFIER bool any
	(
		bool const& x
	)
	{
		return x;
	}

	GLM_FUNC_QUALIFIER bool all
	(
		bool const& x
	)
	{
		return x;
	}

	GLM_FUNC_QUALIFIER bool not_
	(
		bool const& x
	)
	{
		return !x;
	}
}//namespace glm
// CG_REVISION bec57fb16d91551176a88d64c4775a0b7642b469
