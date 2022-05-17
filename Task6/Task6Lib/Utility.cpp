#include "Utility.h"

int Utility::getNumberOfProcessors()
{
	SYSTEM_INFO systemInfo = { 0 };
	GetSystemInfo(&systemInfo);
	return (int)systemInfo.dwNumberOfProcessors;
}
