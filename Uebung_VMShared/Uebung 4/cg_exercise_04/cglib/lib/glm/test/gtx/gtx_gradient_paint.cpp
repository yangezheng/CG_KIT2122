#define GLM_ENABLE_EXPERIMENTAL
#include <glm/gtx/gradient_paint.hpp>

int test_radialGradient()
{
	int Error = 0;
	
	float Gradient = glm::radialGradient(glm::vec2(0), 1.0f, glm::vec2(1), glm::vec2(0.5));
	Error += Gradient != 0.0f ? 0 : 1;
	
	return Error;
}

int test_linearGradient()
{
	int Error = 0;

	float Gradient = glm::linearGradient(glm::vec2(0), glm::vec2(1), glm::vec2(0.5));
	Error += Gradient != 0.0f ? 0 : 1;

	return Error;
}

int main()
{
	int Error = 0;

    Error += test_radialGradient();
    Error += test_linearGradient();
    
	return Error;
}


// CG_REVISION 853babea119d24f6c9aca86013fd5e6a3fb22330
