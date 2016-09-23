/// microLog_test.cpp

// clang++ -std=c++11 ./microLog_test.cpp
// clang++ -std=c++11 -stdlib=libc++ ./microLog_test.cpp

// These macros can also be defined in the makefile, or in microLog_config.hpp:
/*
#define MICRO_LOG_ACTIVE 1
#define MICRO_LOG_MIN_LEVEL 4
*/

#ifdef MICRO_LOG_TEST

//#define uLOG_TEST_NO_INIT

#ifdef uLOG_TEST_NO_INIT        // Test without logger initialization
	#define MICRO_LOG_DLL
#endif

#include "microLog.hpp"

#include <cmath>
#include <iostream>
#include <string>


#ifdef uLOG_TEST_NO_INIT        // Test without logger initialization

int Test_microLog(std::string logPath, int nTestCases = 1)
{
	for(size_t n = 0; n < nTestCases; ++n)
	{
		uLOGF(logPath, warning, info, "Test n. " << n + 1 << " without logger initialization.");
	}

	return 0;
}

#else // uLOG_TEST_NO_INIT

uLOG_INIT;     // microLog initialization

int Test_microLog(std::string logPath, size_t nTestCases = 1)
{
	/// Tests:
	// - Plain tests
	// - Multithreading tests - TODO
	// - Complex tests - TODO
	// - Border line tests - TODO
	// - Performace tests - TODO

	using std::sin;

	///--- TEST INIT ---

	uLOG_START(logPath, uLog::backup_append);

	// Test with custom log file
	std::ofstream custom_ofs("/Volumes/ramdisk/custom.log");

	uLOG_DATE;              // date

	uLog::LogLevels();

	uLog::minLogLevel = nolog;
	uLog::MinLogLevel();

	///--- TEST CODE ---

	for(size_t n = 0; n < nTestCases; ++n)
	{
		uLog::LogFields::SetSystem();
		uLOG_TITLES(info);		// columns' titles

		uLog::minLogLevel = nolog;

		for(int l = nolog; l <= fatal; ++l)
		{
			uLOG(l) << "Test log message with level " << l + 1 << ".";
		}

		uLOG(info) << "Test insertion operator: " << char((n + 65)%255) << " " << n << " " << sin(n + 1.0);

		uLog::minLogLevel = warning;

		uLOG(detail) << "Log not generated, since below the minimum log level.";
		uLOG(warning) << "Previous log not generated, since below the minimum log level.";
		uLOG(warning) << "Log generated, since above the minimum log level.";

		uLog::minLogLevel = warning;
		uLOG_(detail, MICRO_LOG_LEVEL1) << "Test minimum log levels for specific code areas with macros: not generated.";
		uLOG_(detail, uLog::logConstLevel1) << "Test minimum log levels for specific code areas with constants: not generated.";

		uLog::minLogLevel = warning;
		uLOG_(detail, MICRO_LOG_LEVEL2) << "Test minimum log levels for specific code areas with macros.";
		uLOG_(detail, uLog::logConstLevel2) << "Test minimum log levels for specific code areas with constants.";

		/*
		//+TODO
			uLOG(warning) << "Log made of separate tokens... ";
			uLOGT(warning) << "first token, ";
			uLOGT(warning) << "last token";
		*/

		// Test with custom log file
		uLOG_TITLES_S(custom_ofs, warning);
		uLOGS(custom_ofs, warning) << "Test log on a different file.";

		// Test without logger initialization
		uLOGF(logPath, warning, info, "Test without logger initialization.");
	}

	return 0;
}

#endif  // uLOG_TEST_NO_INIT


int main()
{
	int testResult = 0;
	int nErrors = 0;
	int nWarnings = 0;

	std::cout << "\n--- microLog test ---\n" << std::endl;

	std::string logPath;
	std::string ramDiskPath = "/Volumes/ramdisk/";

	char pathOpt = '2';

	if(pathOpt == '0')
	{
		std::cout << "Select log file path:\n"
				  << "1. Local directory.\n"
				  << "2. Ram disk (" << ramDiskPath << ").\n"
				  << "   Note: check you have a ram disk on your system, a set its path in the source code (microLog_test.cpp).\n" << std::endl;

		std::cin >> pathOpt;
	}

	if(pathOpt == '2')
		logPath.append("/Volumes/ramdisk/");

	logPath.append("myProg.log");

	std::cout << "Test version:      " << VERSION << "\n";
	std::cout << "microLog version:  " << MICRO_LOG_VERSION << "\n";
	std::cout << "Log file path:     " << logPath << std::endl;

	testResult = Test_microLog(logPath);

#ifndef uLOG_TEST_NO_INIT        // Test without logger initialization
	uLog::Statistics::Log();
#endif

	std::cout << "\nTest completed." << std::endl;

	if(testResult == 0)
		std::cout << "\nTest passed." << std::endl;
	else
		std::cout << "\nTest FAILED." << std::endl;

	return testResult;
}

#endif // MICRO_LOG_TEST
