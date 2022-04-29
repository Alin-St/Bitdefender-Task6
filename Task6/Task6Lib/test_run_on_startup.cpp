#include "tests.h"
#include "Utility.h"
#include <cassert>

void test_run_on_startup(const wchar_t* appPath)
{
	bool ret{}, pathMatches{};
	ret = Utility::runProgramOnStartup(L"Task6_TEST", appPath);
	assert(ret);

	ret = Utility::stopRunningProgramOnStartup(L"Task6_TEST", appPath, true, false);
	assert(ret);

	ret = Utility::stopRunningProgramOnStartup(L"Task6_TEST", appPath, true, false);
	assert(!ret);

	ret = Utility::runProgramOnStartup(L"Task6_TEST", appPath, true, true, &pathMatches);
	assert(ret);

	ret = Utility::runProgramOnStartup(L"Task6_TEST", appPath, true, true, &pathMatches);
	assert(!ret);
	assert(pathMatches);

	ret = Utility::stopRunningProgramOnStartup(L"Task6_TEST", nullptr, true, true);
	assert(ret);

	ret = Utility::stopRunningProgramOnStartup(L"Task6_TEST", nullptr, true, true);
	assert(!ret);
}
