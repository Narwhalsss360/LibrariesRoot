#pragma once

#include <string>
#include <StandardCollections.h>

#define add_test(f) TestService::Test TestContainer_##f = TestService::Test(#f, f)
#define test_out(c) std::cout << c
#define test_expect(expression, expected) if ((expression) != (expected))  { test_out("test_expected failed at line "); test_out(__LINE__); \
											test_out(" of file: "); \
											test_out(__FILE__); \
											test_out(" expression: "); \
											test_out(#expression); \
											test_out(" expected: "); \
											test_out(#expected); \
											test_out('\n'); \
											__debugbreak(); }\

namespace TestService
{
	using string = std::string;

	typedef int TestResultCode;

	typedef TestResultCode (*TestFunctionPointer)();

	struct Test
	{
		Test(string name = "", TestFunctionPointer function = nullptr);
		
		bool operator==(const Test& other) const;
		
		~Test();

		string name;
		TestResultCode code;
		TestFunctionPointer function;
	};

#ifdef TestService_Source
	extern DynamicArray<Test*> Tests;
#else
	DynamicArray<Test*> Tests = DynamicArray<Test*>();
#endif // TestService_Souce

	void RunAllTests();
}