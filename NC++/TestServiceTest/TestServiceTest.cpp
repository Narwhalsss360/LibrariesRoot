#include <iostream>
#include "TestService.h"

inline int Add1(int num)
{
	return num + 1;
}

class Add1Tester : Tester
{
public:
	Add1Tester()
		: Tester()
	{
	}

	void Run() override
	{
		std::cout << "Running";

		if (Add1(5) != 6)
			success = false;

		if (Add1(INT16_MAX) != INT16_MAX + 1)
			success = false;

		if (Add1(4.0) != 5)
			success = false;

		if (Add1(4.0f) != 5.0f)
			success = false;
	}
	
	bool success = true;
};

int main()
{
	TestService main_Tests;
	Add1Tester add1Tester;
	main_Tests.Run();
}