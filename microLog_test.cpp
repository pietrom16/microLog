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
#include <chrono>
#include <ctime>
#include <iostream>
#include <string>

using namespace uLog;

MICRO_LOG_INIT;


int BasicTests(std::string logPath, size_t nTestCases = 1)
{
	//+TEST
	int ret = 0;

	Log log(logPath);

	for(size_t i = 0; i < nTestCases; ++i) {
		log << "Test log message " << i << ": unspecified log level, " << 123.4;
		log(error) << "Test log message " << i << ", " << 234.5;
		log(detail) << "Test log message " << i << " below threshold.";
	}

	return ret;
}


int StaticTests(std::string logPath, size_t nTestCases = 1)
{
	//+TEST
	int ret = 0;

	ret += Log::SetLogFile(logPath);

	for(size_t i = 0; i < nTestCases; ++i) {
		Log::msg << "Test log message " << i << ": static, unspecified log level, " << 123.4;
		Log::msg(error) << "Test log message " << i << ": static, " << 234.5;
		Log::msg(detail) << "Test log message " << i << " below threshold.";
	}

	return ret;
}


int MultithreadingTests(std::string logPath, size_t nTestCases = 1)
{
	//+TODO
	int ret = 0;
	return ret;
}


int ComplexTests(std::string logPath, size_t nTestCases = 1)
{
	//+TODO
	int ret = 0;
	return ret;
}


int BorderLineTests(std::string logPath, size_t nTestCases = 1)
{
	//+TODO
	int ret = 0;
	return ret;
}


int PerformaceTests(std::string logPath, size_t nTestCases = 1)
{
	Log log(logPath);

	log.SetMinLogLevel(warning);
	uLogLevels logLevel = nolog;

	for(size_t j = 0; j < 4; ++j)
	{
		if(j == 0) {				// no logs
			std::cout << "PerformaceTests, no logs, t = ";
		}
		else if(j == 1) {			// disabled logs
			std::cout << "PerformaceTests, disabled logs (undefine MICRO_LOG_ACTIVE), t = ";
			logLevel = nolog;
		}
		else if(j == 2) {			// logs below threshold
			std::cout << "PerformaceTests, logs below threshold, t = ";
			logLevel = verbose;
		}
		else if(j == 3) {			// logs above threshold
			std::cout << "PerformaceTests, logs above threshold, t = ";
			logLevel = error;
		}

		auto t0 = std::chrono::high_resolution_clock::now();

		//------Begin test code
		// ...normal processing should take place here

		if(j != 0)
		{
			for(size_t i = 0; i < nTestCases; ++i) {
				log(logLevel) << "Test set n. " << j << ". Test log message " << i << ", " << 234.5;
			}
		}
		//------End test code

		auto t1 = std::chrono::high_resolution_clock::now();

		std::chrono::duration<double> delta_t = t1 - t0;

		std::cout << delta_t.count() << " s" << std::endl;
	}

	return 0;
}


int Test_microLog(std::string logPath, size_t nTestCases = 1)
{
	int ret = 0;

	ret += BasicTests(logPath, nTestCases);
	ret += StaticTests(logPath, nTestCases);
	ret += MultithreadingTests(logPath, nTestCases);
	ret += ComplexTests(logPath, nTestCases);
	ret += BorderLineTests(logPath, nTestCases);
	ret += PerformaceTests(logPath, nTestCases);

	return ret;
}

#if 0
	using std::sin;

	///--- TEST INIT ---

	uLOG_INIT;     // microLog initialization

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
			uLOG(l) << "Test log message with level " << l + 1 << "." << uLOGE;
		}

		uLOG(info) << "Test insertion operator: " << char((n + 65)%255) << " " << n << " " << sin(n + 1.0) << uLOGE;

		uLog::minLogLevel = warning;

		uLOG(detail) << "Log not generated, since below the minimum log level." << uLOGE;
		uLOG(warning) << "Previous log not generated, since below the minimum log level." << uLOGE;
		uLOG(warning) << "Log generated, since above the minimum log level." << uLOGE;

		uLog::minLogLevel = warning;
		uLOG_(detail, MICRO_LOG_LEVEL1) << "Test minimum log levels for specific code areas with macros: not generated." << uLOGE;
		uLOG_(detail, uLog::logConstLevel1) << "Test minimum log levels for specific code areas with constants: not generated." << uLOGE;

		uLog::minLogLevel = warning;
		uLOG_(detail, MICRO_LOG_LEVEL2) << "Test minimum log levels for specific code areas with macros." << uLOGE;
		uLOG_(detail, uLog::logConstLevel2) << "Test minimum log levels for specific code areas with constants." << uLOGE;

		/*
		//+TODO
			uLOG(warning) << "Log made of separate tokens... ";
			uLOGT(warning) << "first token, ";
			uLOGT(warning) << "last token" << uLOGE;
		*/

		// Test with custom log file
		uLOG_TITLES_S(custom_ofs, warning);
		uLOGS(custom_ofs, warning) << "Test log on a different file." << uLOGE;

		// Test without logger initialization
		uLOGF(logPath, warning, info, "Test without logger initialization.");
	}

	return ret;
}
#endif


int main()
{
	int testResult = 0;

	std::cout << "\n--- microLog test ---\n" << std::endl;

	std::string logPath;
	std::string ramDiskPath = "/Volumes/ramdisk/";

	char pathOpt = '2';

	if(pathOpt == '0')
	{
		std::cout << "Select log file path:\n"
				  << "1. Local directory.\n"
				  << "2. Ram disk (" << ramDiskPath << ").\n"
				  << "   Note: check you have a ram disk on your system, and set its path in the source code (microLog_test.cpp).\n" << std::endl;

		std::cin >> pathOpt;
	}

	if(pathOpt == '2')
		logPath.append(ramDiskPath);

	logPath.append("myProg.log");

	std::cout << "Test version:      " << VERSION << "\n";
	std::cout << "microLog version:  " << MICRO_LOG_VERSION << "\n";
	std::cout << "Log file path:     " << logPath << std::endl;

	testResult = Test_microLog(logPath);

	std::cout << "\nTest completed." << std::endl;

	if(testResult == 0)
		std::cout << "\nTest passed." << std::endl;
	else
		std::cout << "\nTest FAILED." << std::endl;

	return testResult;
}

#endif // MICRO_LOG_TEST
