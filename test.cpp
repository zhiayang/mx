#include <string>
#include <cstdio>

int main()
{
	const char* lol = "hahahahah";

	std::string test = std::string(lol);
	printf("%s", test.c_str());

	return 0;
}