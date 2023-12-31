#include <glm/gtx/functions.hpp>
#include <vector>

int test_gauss_1d()
{
	int Error = 0;

	std::vector<float> Result(20);
	for(std::size_t i = 0, n = Result.size(); i < n; ++i)
		Result[i] = glm::gauss(static_cast<float>(i) * 0.1f, 0.0f, 1.0f);

	return Error;
}

int test_gauss_2d()
{
	int Error = 0;

	std::vector<float> Result(20);
	for(std::size_t i = 0, n = Result.size(); i < n; ++i)
		Result[i] = glm::gauss(glm::vec2(static_cast<float>(i)) * 0.1f, glm::vec2(0.0f), glm::vec2(1.0f));

	return Error;
}

int main()
{
	int Error = 0;

	Error += test_gauss_1d();
	Error += test_gauss_2d();

	return Error;
}

// CG_REVISION 853babea119d24f6c9aca86013fd5e6a3fb22330
