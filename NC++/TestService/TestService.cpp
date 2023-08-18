#define TestService_Source
#include "TestService.h"

TestService::Test::Test(string name, TestService::TestFunctionPointer function)
	: name(name), code(0), function(function)
{
	Tests += this;
}

bool TestService::Test::operator==(const TestService::Test& other) const
{
	return function == other.function;
}

TestService::Test::~Test()
{
	auto self = Tests.Find(this);
	if (self.valid)
		Tests.Remove(ShrinkFindIndex(self.index));
}

void TestService::RunAllTests()
{
	for (TestService::Test* const test : Tests)
		if (test->function)
			test->code = test->function();
}