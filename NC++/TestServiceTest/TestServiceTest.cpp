#include <iostream>
#include "TestService.h"

inline int Add1(int num)
{
	return num + 2;
}

TestService::TestResultCode Add1_Test()
{
	int arg = 5;
	int expected = 6;
	test_expect(Add1(arg), expected);

	if (Add1(arg) == expected)
		return TestService::TestResultCode();
	else
		return TestService::TestResultCode(-1);
}
add_test(Add1_Test);

int main()
{
	TestService::RunAllTests();
	return 0;
}