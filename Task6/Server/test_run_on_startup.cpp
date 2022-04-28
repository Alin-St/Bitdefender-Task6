#include "tests.h"
#include "run_on_startup.h"
#include <cassert>

void test_run_on_startup(const wchar_t* appPath)
{
	bool ret{}, pathMatches{};
	ret = runProgramOnStartup(L"Task6_TEST", appPath);
	assert(ret);

	ret = stopRunningProgramOnStartup(L"Task6_TEST", appPath, true, false);
	assert(ret);

	ret = stopRunningProgramOnStartup(L"Task6_TEST", appPath, true, false);
	assert(!ret);

	ret = runProgramOnStartup(L"Task6_TEST", appPath, true, true, &pathMatches);
	assert(ret);

	ret = runProgramOnStartup(L"Task6_TEST", appPath, true, true, &pathMatches);
	assert(!ret);
	assert(pathMatches);

	ret = stopRunningProgramOnStartup(L"Task6_TEST", nullptr, true, true);
	assert(ret);

	ret = stopRunningProgramOnStartup(L"Task6_TEST", nullptr, true, true);
	assert(!ret);
}
