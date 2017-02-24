#include <unistd.h>
#include "AdvLog.h"

#if defined(WIN32)
#include "AdvPlatform.h"
#pragma comment(lib, "AdvLog.lib")
#endif

int main(int argc, char **argv) {
	AdvLog_Control(argc, argv);
}






