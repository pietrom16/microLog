/**  microLog.hpp
 *
 *   A light, compact, fast logger. It allows to set different minimal log levels for
 *   different kinds of logs, allowing to focus on specific areas of the code during
 *   debugging.
 *
 *   This is the only file needed to generate logs, plus a user written logger's
 *   configuration file named: "microLog_config.hpp".
 *
 *   Author:    Pietro Mele
 */

/* microLog is distributed under the following 3-clause BSD license:

Copyright (c) 2011-2017, Pietro Mele
All rights reserved.

Redistribution and use in source and binary forms, with or without
modification, are permitted provided that the following conditions are met:
* Redistributions of source code must retain the above copyright
notice, this list of conditions and the following disclaimer.
* Redistributions in binary form must reproduce the above copyright
notice, this list of conditions and the following disclaimer in the
documentation and/or other materials provided with the distribution.
* Neither the name of the <organization> nor the
names of its contributors may be used to endorse or promote products
derived from this software without specific prior written permission.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND
ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER BE LIABLE FOR ANY
DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
(INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
(INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/


/** A compact logging utility.

	- A single header file (this one).
	- There are no implementation files, so makefiles do not have to be touched.
	- Thread safe (C++11 threads, Boost threads or pthread).
	- It can be used when no debugger nor other logging system is available.
	- No objects to be defined.
	- To activate:                     #define MICRO_LOG_ACTIVE
	- To initalize, at global scope:   uLOG_INIT;
	- To start, tipically in main():   uLOG_START_APP(logPath);
	- When not activated, or when a log message level is below the threshold, there is no generated binary code.
	- uLOG(level) only logs if level >= MICRO_LOG_MIN_LEVEL and level >= ULog::minLogLevel.
	- uLOG_(level, localLevel) logs if level >= MICRO_LOG_MIN_LEVEL and (level >= ULog::minLogLevel or level >= localLevel).
		  This allows to specify different minimum log levels for different kinds of log messages;
		  localLevel can be a const or a variable.
	- To flush the message use the uLOGE manipulator (stands for log-end) instead of std::endl!
	- The output log file can be:
		- Unique for this executable, with a global static variable file stream (microLog_ofs).
		- Custom, with a stream passed as a parameter to every log message (TODO).
	- Multithreading: the MICRO_LOG_LOCK/MICRO_LOG_UNLOCK macros delimit a critical section.
		- Their values depend on the adopted threading library, and can be defined in microLog_config.hpp.
		- Predefined values available for: C++11 threads, Boost threads, pthread.
	- For better performance, log to a ramdisk and use an external utility to periodically move the logs to
		  a permanent data storage.
	- In case it is not possible to initialize microLog, or it is not possible to modify the main() function
		  to call uLOG_START_APP(), you can use:  uLOGF(logfname, level, minLogLev, logMsg)

	Usage: See microLog_test.cpp as an example.

	Settings:

		Define these macros in the makefile or as arguments to the compiler:

		MICRO_LOG_ACTIVE      to add the logger calls to the executable (if set to 1).
		MICRO_LOG_MIN_LEVEL   to set a minimum log level below which the logger call will not be added.
*/

#ifndef MICRO_LOG_HPP
#define MICRO_LOG_HPP

#define MICRO_LOG_VERSION "8.0.0 alpha"

// Standard threading libraries
#define MICRO_LOG_SINGLE_THREAD  1
#define MICRO_LOG_CPP11_THREAD   2
#define MICRO_LOG_BOOST_THREAD   3
#define MICRO_LOG_PTHREAD        4

//+T+++ To be moved in a config file
#define MICRO_LOG_ACTIVE
#define MICRO_LOG_MIN_LEVEL nolog
#define MICRO_LOG_THREADING MICRO_LOG_SINGLE_THREAD


enum uLogLevels { nolog = 0, verbose, detail, info, warning, error, critical, fatal };

namespace uLog {
	// Log levels (a custom levels enum can be used)
	static const int nLogLevels = 8;
    const char       logLevelTags[nLogLevels][9] = { "  ----  ", "VERBOSE ", "DETAIL  ", "INFO    ", "WARNING ", "ERROR   ", "CRITICAL", "FATAL   " };
}

#ifdef MICRO_LOG_ACTIVE
	#include <bitset>
	#include <cstdio>
	#include <cstring>
	#include <ctime>
    //+C++17 #include <filesystem>
	#include <fstream>
	#include <iomanip>
	#include <iostream>
	#include <sstream>
	#include <string>
	#include <vector>

	#ifndef MICRO_LOG_EXECUTABLE_NAME
		#define MICRO_LOG_EXECUTABLE_NAME ""
	#endif

	#if(MICRO_LOG_THREADING == MICRO_LOG_CPP11_THREAD)
		#include <mutex>
	#elif(MICRO_LOG_THREADING == MICRO_LOG_BOOST_THREAD)
		#include <boost/thread/thread.hpp>
	#elif(MICRO_LOG_THREADING == MICRO_LOG_PTHREAD)
		#include <pthread.h>
	#endif

	#ifndef MICRO_LOG_BOOST         // Boost used by default
		#define MICRO_LOG_BOOST 1
	#endif

	#if(MICRO_LOG_BOOST == 1)
		#include <boost/filesystem.hpp>
	#endif

	#ifndef WIN32
		#include <unistd.h>
	#else
		#include <process.h>
		#include <Lmcons.h>
	#endif
#else  // MICRO_LOG_ACTIVE
	#include <ios>
	#include <iosfwd>
#endif  // MICRO_LOG_ACTIVE


namespace uLog {


	struct LogStatistics
		/// Statistical informations about generated logs
	{
		int nLogs;
		int nNoLogs, nVerboseLogs, nDetailLogs, nInfoLogs, nWarningLogs, nErrorLogs, nCriticalLogs, nFatalLogs;
		int highestLevel;

		LogStatistics() { Init(); }
		void Init();
		void Update(int level);
		std::string Log();
	};


	struct LogFields
		/// Flags to enable/disable log message fields
	{
		static bool time, date, llevel,
					exec, pid,
					uid, uname,
					fileName, filePath, funcName, funcSig,
					line, log;

		LogFields() {
			SetDefault();
		}

		static void SetDefault() {
			time = false; date = true;
			llevel = true;
			exec = false; pid = false;
			uid = false; uname = false;
			fileName = false; filePath = false;
			funcName = false; funcSig = false; line = false;
			log = true;
		}

		static void SetDetailed() {
			time = true; date = true;
			llevel = true;
			exec = true; pid = false;
			uid = false; uname = false;
			fileName = false; filePath = false;
			funcName = false; funcSig = false; line = false;
			log = true;
		}

		static void SetSystem() {
			time = false; date = true;
			llevel = true;
			exec = true; pid = true;
			uid = true; uname = true;
			fileName = true; filePath = false;
			funcName = false; funcSig = false; line = false;
			log = true;
		}

		static void SetDebug() {
			time = false; date = false;
			llevel = true;
			exec = true; pid = false;
			uid = false; uname = false;
			fileName = true; filePath = false;
			funcName = true; funcSig = false; line = true;
			log = true;
		}

		static void SetVerbose() {
			time = true; date = true;
			llevel = true;
			exec = true; pid = true;
			uid = true; uname = true;
			fileName = false; filePath = true;
			funcName = false; funcSig = true; line = true;
			log = true;
		}
	};


	class Log
		/// Main logger class
	{
	public:
		Log() : level(nolog), ostr(0) {}

		Log(const std::string &_logFilepath) : level(nolog) {
			#ifdef MICRO_LOG_ACTIVE
				ostr = new std::ofstream(_logFilepath);
			#else
				ostr = 0;
			#endif
		}

		Log(std::ofstream &_logOStream) : level(nolog) {
			#ifdef MICRO_LOG_ACTIVE
				ostr = &_logOStream;
			#else
				ostr = 0;
			#endif
		}

		~Log()
		{
            #ifdef MICRO_LOG_ACTIVE
			*ostr << std::endl;
            #endif
		}

		Log& operator()(uLogLevels _level) {
			#ifdef MICRO_LOG_ACTIVE
			level = _level;
			#endif
			return *this;
		}

		template<typename T>
		Log& operator<<(T const& _token) {
            #ifdef MICRO_LOG_ACTIVE
			*ostr << _token;
            #endif
			return *this;
		}

		void LogLevels();
		void MinLogLevel();
		void LogStats() const;

		int BackupPrevLog(int _mode = backup_append, const std::string &_backupPath = std::string());

	public:

		static const int
		        backup_store_local  = 0,
		        backup_store_remote = 1,
		        backup_append       = 2,
		        backup_overwrite    = 3;

		static const int
		        backup_ok           = 0,
		        backup_no_file      = 2,
		        backup_nothing_todo = 3,
		        backup_error        = -1;

		static const size_t  maxLogSize = 1024;		// max length of a log message (bytes)

	private:

		bool CheckLogLevel(int _level, int _localLevel = nolog);
		static bool CheckAvailableSpace(const std::string &_logfname);
		bool CheckAvailableSpace();

		static std::string LogTime();
		static std::string LogDate();
		static std::string GetPID();
		static std::string GetUID();
		static std::string GetUserName();

	private:

		uLogLevels     level;
		uLogLevels     minLevel;	// minimum level a message must have to be logged
		int            status;		// OK=0, error otherwise
		std::string    filePath;
		std::ofstream *ostr;
		LogStatistics  stats;

	public:

		/// Static usage

		static int SetLogFile(const std::string &_logFilepath) {
			#ifdef MICRO_LOG_ACTIVE
				s_filePath = _logFilepath;
				s_ostr = new std::ofstream(s_filePath);
			#else
				s_ostr = 0;
			#endif
			return 0;
		}

		static int SetLogFileIfUnset(const std::string &_logFilepath) {
			#ifdef MICRO_LOG_ACTIVE
				if(s_ostr == 0)
					return SetLogFile(_logFilepath);
			#else
				s_ostr = 0;
			#endif
			return 0;
		}

		static int SetLogStream(std::ofstream &_logOStream) {
			#ifdef MICRO_LOG_ACTIVE
				s_ostr = &_logOStream;
			#else
				s_ostr = 0;
			#endif
			return 0;
		}

		static int SetLogStreamIfUnset(std::ofstream &_logOStream) {
			#ifdef MICRO_LOG_ACTIVE
				if(s_ostr == 0)
					s_ostr = &_logOStream;
			#else
				s_ostr = 0;
			#endif
			return 0;
		}

		//Ref: https://stackoverflow.com/questions/8246517/how-to-define-a-static-operator
		struct Msg {
			template<typename T>
			Msg& operator<<(T const& _token) {
				#ifdef MICRO_LOG_ACTIVE
				*Log::s_ostr << _token;
				#endif
				return *this;
			}
		};

		static Msg msg;

		static bool CheckLogLevelS(int _level, int _localLevel = nolog);
		static bool CheckAvailableSpaceS() {
			return CheckAvailableSpace(s_filePath);
		}

	private:

		static int            s_minLevel;	// minimum level a message must have to be logged
		static int            s_status;		// OK=0, error otherwise
		static std::string    s_filePath;
		static std::ofstream *s_ostr;
		static LogStatistics  s_stats;

	};


} // uLog


#ifdef MICRO_LOG_ACTIVE

namespace uLog { // Log implementation

inline void Log::LogLevels() {
	*ostr << "Log levels: ";
	for(size_t i = 0; i < uLog::nLogLevels; ++i)
		*ostr << uLog::logLevelTags[i] << " ";
	*ostr << std::endl;
}

inline void Log::MinLogLevel() {
	*ostr << "Minimum log level to be logged: " << uLog::logLevelTags[minLevel] << std::endl;
}

inline int Log::BackupPrevLog(int _mode, const std::string &_backupPath)
{
	if(_mode == backup_append)
		return backup_nothing_todo;

	std::ifstream ifs(filePath);
	if(!ifs)
		return backup_no_file;

	if(_mode == backup_overwrite) {
		std::remove(filePath.c_str());
		return backup_ok;
	}
	else if(_mode == backup_store_local) {
		//+TODO - Append the date of the log file, not the current date
		//+C++17 auto ftime = std::filesystem::last_write_time(filePath);
		//+C++17 const std::string bufn = filePath.c_str() + std::string("_backup") + ftime;
		const std::string bufn = filePath.c_str() + std::string("_backup") + LogDate() + std::string("_") + LogTime();
		std::rename(filePath.c_str(), bufn.c_str());
		return backup_ok;
	}
	else if(_mode == backup_store_remote) {
		//+TODO - see previous code block
		const std::string bufn = _backupPath + filePath.c_str() + std::string("_backup") + LogDate() + std::string("_") + LogTime();
		std::rename(filePath.c_str(), bufn.c_str());  //+ Check if a rename is enough to move to remote storage
		return backup_ok;
	}

	return backup_nothing_todo;
}


inline bool Log::CheckLogLevel(int _level, int _localLevel)
{
	#ifndef MICRO_LOG_DLL
	    stats.Update(_level);
	#endif

	if(status != 0) {        // cannot log if status is not clean
		if(_level > error)
			std::cerr << "Error " << status << ": logger disabled, and a critical error has been generated!" << std::endl;
		return false;
	}

	if(_level < MICRO_LOG_MIN_LEVEL || _level < _localLevel)
		return false;

	if(_localLevel == nolog && _level < minLevel)
		return false;

	return true;
}

inline bool Log::CheckAvailableSpace(const std::string &_logFName)
	// Check if the next log message can fit in the remaining available space
{
#if(MICRO_LOG_BOOST == 1)
	boost::system::error_code errCode;
	boost::filesystem::space_info space = boost::filesystem::space(_logFName, errCode);
	if(space.available < maxLogSize) {
		std::cerr << "Logger error: not enough space available in the current partition (" << space.available << " bytes)." << std::endl;
		return false;
	}
#endif
	return true;
}

inline bool Log::CheckAvailableSpace()
	// Check if the next log message can fit in the remaining available space
{
	return CheckAvailableSpace(filePath);
}


inline std::string Log::LogTime() {
	float t = float(std::clock())/CLOCKS_PER_SEC;
	const size_t sz = 16;
	char ct[sz];
	std::snprintf(ct, sz, "% 7.3f  ", t);
	return std::string(ct);
}

inline std::string Log::LogDate() {
	std::time_t t = std::time(nullptr);
	char mbstr[32];
	std::strftime(mbstr, sizeof(mbstr), "%F %T  ", std::localtime(&t));
	return std::string(mbstr);
}

inline std::string Log::GetPID() {
	#ifdef _POSIX_VERSION
		return std::to_string(getpid());
	#elif defined WIN32
		return std::to_string(_getpid());
	#else
		return "?";
	#endif
}

inline std::string Log::GetUID() {
	#ifdef _POSIX_VERSION
		return std::to_string(getuid());
	#elif defined WIN32
		return "?";
	#else
		return "?";
	#endif
}

inline std::string Log::GetUserName() {
	#ifdef _POSIX_VERSION
		return getlogin();
	#elif defined WIN32
		GetUserName(username, UNLEN+1);
		return username;
	#else
		return "?";
	#endif
}

inline bool Log::CheckLogLevelS(int _level, int _localLevel)
{
	#ifndef MICRO_LOG_DLL
	    s_stats.Update(_level);
	#endif

	if(s_status != 0) {        // cannot log if status is not clean
		if(_level > error)
			std::cerr << "Error " << s_status << ": logger disabled, and a critical error has been generated!" << std::endl;
		return false;
	}

	if(_level < MICRO_LOG_MIN_LEVEL || _level < _localLevel)
		return false;

	if(_localLevel == nolog && _level < s_minLevel)
		return false;

	return true;
}


} // uLog


namespace uLog { // LogStatistics implementation

#ifndef MICRO_LOG_DLL

inline void LogStatistics::Init() {
	nLogs = 0;
	nNoLogs = nVerboseLogs = nDetailLogs = nInfoLogs = nWarningLogs = nErrorLogs = nCriticalLogs = nFatalLogs = 0;
	highestLevel = 0;
}

inline void LogStatistics::Update(int level) {
	++nLogs;
	if(level > highestLevel) highestLevel = level;
	switch (level) {
	case nolog:    ++nNoLogs;       break;
	case verbose:  ++nVerboseLogs;  break;
	case detail:   ++nDetailLogs;   break;
	case info:     ++nInfoLogs;     break;
	case warning:  ++nWarningLogs;  break;
	case error:    ++nErrorLogs;    break;
	case critical: ++nCriticalLogs; break;
	case fatal:    ++nFatalLogs;    break;
	}
}

inline std::string LogStatistics::Log()
{
	std::stringstream log;

	log << "Log statistics:"
		<< "\n\tNumber of logs: " << nLogs
		<< "\n\tNumber of 'fatal' logs:    " << nFatalLogs
		<< "\n\tNumber of 'critical' logs: " << nCriticalLogs
		<< "\n\tNumber of 'error' logs:    " << nErrorLogs
		<< "\n\tNumber of 'warning' logs:  " << nWarningLogs
		<< "\n\tNumber of 'info' logs:     " << nInfoLogs
		<< "\n\tNumber of 'detail' logs:   " << nDetailLogs
		<< "\n\tNumber of 'verbose' logs:  " << nVerboseLogs
		<< "\n\tNumber of 'null' logs:     " << nNoLogs << std::endl;
	log << "Highest log level: " << highestLevel << std::endl;

	return log.str();
}

#endif // MICRO_LOG_DLL

} // uLog

#elif // MICRO_LOG_ACTIVE

// Dummy implementations

namespace uLog { // Log implementation

inline Log::Log(const uLogLevels _level) {}

inline void Log::LogLevels() {}

inline void Log::MinLogLevel() {}

inline int Log::BackupPrevLog(int _mode, const std::string &_backupPath) { return 0; }

inline bool Log::CheckLogLevel(int _level, int _localLevel) { return false; }

inline bool Log::CheckAvailableSpace(const std::string &_logFName) { return false; }

inline bool Log::CheckAvailableSpace() { return false; }

inline std::string Log::LogTime() { return ""; }

inline std::string Log::LogDate() { return ""; }

inline std::string Log::GetPID() { return ""; }

inline std::string Log::GetUID() { return ""; }

inline std::string Log::GetUserName() { return ""; }

#ifndef MICRO_LOG_DLL

inline void LogStatistics::Init() {}

inline void LogStatistics::Update(int level) {}

inline std::string LogStatistics::Log() { return ""; }

#endif // MICRO_LOG_DLL

} // uLog

#endif // MICRO_LOG_ACTIVE

#endif // MICRO_LOG_HPP


#if 0
//+TODO --- Old code

namespace uLog {
	#ifdef MICRO_LOG_ACTIVE

		// Begin: Platform specific
		#ifndef WIN32
			#define MICRO_LOG_DIR_SLASH '/'
		#else
			#define MICRO_LOG_DIR_SLASH '\\'
			static char username[UNLEN+1];
		#endif

		#ifdef _MSC_VER
			#define __func__ __FUNCTION__
			#define __PRETTY_FUNCTION__ __FUNCSIG__
		#endif
		// End: Platform specific

		// Log stream: define it at global scope and open it (in append mode) before logging
		extern std::ofstream microLog_ofs;

		#ifndef MICRO_LOG_MIN_LEVEL
			#define MICRO_LOG_MIN_LEVEL 2
		#endif

		// microLog initialization:
		// Use this macro once in the main()'s file at global scope

		#ifndef MICRO_LOG_DLL
			#define uLOG_INIT_0                          \
				namespace uLog {                         \
	            int Log::minLevel = MICRO_LOG_MIN_LEVEL; \
	            int Log::status = 0;                     \
	            std::string Log::filename;               \
	            std::ofstream Log::ofs;                  \
	            LogStatistics Log::stats;                \
	            Log::stats.Init();                       \
				bool LogFields::time = false, LogFields::date = true, LogFields::llevel = true, LogFields::exec = false,                \
					 LogFields::uid = false, LogFields::uname = false, LogFields::pid = false,                                          \
					 LogFields::fileName = false, LogFields::filePath = false, LogFields::funcName = false, LogFields::funcSig = false, \
					 LogFields::line = false, LogFields::log = true;                                                                    \
				}
		#else
			#define uLOG_INIT_0                          \
				namespace uLog {                         \
	            int Log::minLevel = MICRO_LOG_MIN_LEVEL; \
	            int Log::status = 0;                     \
	            std::string Log::filename;               \
	            std::ofstream Log::ofs;                  \
	            LogStatistics Log::stats;                \
				}
		#endif

		// microLog start:

        #define uLOG_START(logFilename_, backup_mode)                          \
	        uLog::Log::filename = logFilename_;                                \
	        uLog::Log::status = 0;                                             \
	        uLog::Log::BackupPrevLog(backup_mode);                             \
	        uLog::microLog_ofs.open(uLog::logFilename, std::fstream::app);     \
	        if(!uLog::microLog_ofs) {                                          \
	            uLog::Log::status = -1;                                        \
	            std::cerr << "Error opening log file. Cannot produce logs. Check if disk space is available." << std::endl;  \
			}

		// Multithreading: macros used to define a critical section
		//                 according to the adopted threading library:

		#ifndef MICRO_LOG_THREADING
			#define MICRO_LOG_THREADING MICRO_LOG_SINGLE_THREAD
		#endif

		// Single threaded program
		#if(MICRO_LOG_THREADING == MICRO_LOG_SINGLE_THREAD)

			#define uLOG_INIT uLOG_INIT_0
			#define MICRO_LOG_LOCK {
			#define MICRO_LOG_UNLOCK }

		// C++11 Thread library
		#elif(MICRO_LOG_THREADING == MICRO_LOG_CPP11_THREAD)

			#define uLOG_INIT uLOG_INIT_0

			#define MICRO_LOG_LOCK                                          \
				{                                                           \
					std::mutex ulog_mutex;                                  \
					ulog_mutex.lock();
			#define MICRO_LOG_UNLOCK                                        \
					ulog_mutex.unlock();                                    \
				}

		// Boost Thread library      //+TODO
		#elif(MICRO_LOG_THREADING == MICRO_LOG_BOOST_THREAD)

			#define MICRO_LOG_INIT                                          \
				MICRO_LOG_INIT_0;                                           \
				;

			// Use this macro once in the main() function, before any log
			#define MICRO_LOG_START(logFile)                                \
				microLog_ofs.open((logFile))
			#define MICRO_LOG_START_APPEND(logFile)                         \
				microLog_ofs.open((logFile), std::fstream::app)

			#define MICRO_LOG_LOCK                                          \
				;
			#define MICRO_LOG_UNLOCK                                        \
				;

		// PThread library      //+TEST
		#elif(MICRO_LOG_THREADING == MICRO_LOG_PTHREAD)

			#define MICRO_LOG_INIT                                          \
				MICRO_LOG_INIT_0;                                           \
				pthread_mutex_t ulog_mutex;

			// Use this macro once in the main() function, before any log
			#define MICRO_LOG_START(logFile)                                \
				pthread_mutex_init(&ulog_mutex, NULL);                      \
				microLog_ofs.open((logFile))
			#define MICRO_LOG_START_APPEND(logFile)                         \
				pthread_mutex_init(&ulog_mutex, NULL);                      \
				microLog_ofs.open((logFile), std::fstream::app)

			#define MICRO_LOG_LOCK                                          \
				pthread_mutex_lock(&ulog_mutex)
			#define MICRO_LOG_UNLOCK                                        \
				pthread_mutex_unlock(&ulog_mutex)

		#endif


		static std::time_t microLog_time;

		static const char bar[] = "--------------------------------------------------------------------------------";

		template <class charT, class traits>
		std::basic_ostream<charT, traits>& endm(std::basic_ostream<charT, traits>& os)
		{
			os << std::endl;
			return os;
		}



		#define uLOGS_(logstream, level, localMinLevel)                               \
			if(uLog::CheckLogLevel(level, localMinLevel) && uLog::CheckAvailableSpace())    \
				MICRO_LOG_LOCK;                                                       \
				logstream                                                             \
					<< (uLog::LogFields::time?uLog::LogTime():"")                     \
					<< (uLog::LogFields::date?uLog::LogDate():"")                     \
					<< (uLog::LogFields::llevel?uLog::logLevelTags[level]:"")         \
					<< (uLog::LogFields::llevel?uLog::separator:"")                   \
					<< (uLog::LogFields::exec?MICRO_LOG_EXECUTABLE_NAME:"")           \
					<< (uLog::LogFields::exec?uLog::separator:"")                     \
					<< (uLog::LogFields::pid?uLog::GetPID():"")                       \
					<< (uLog::LogFields::pid?uLog::separator:"")                      \
					<< (uLog::LogFields::uid?uLog::GetUID():"")                       \
					<< (uLog::LogFields::uid?uLog::separator:"")                      \
					<< (uLog::LogFields::uname?uLog::GetUserName():"")                \
					<< (uLog::LogFields::uname?uLog::separator:"")                    \
					<< (uLog::LogFields::fileName?(strrchr(__FILE__, MICRO_LOG_DIR_SLASH) ? strrchr(__FILE__, MICRO_LOG_DIR_SLASH) + 1 : __FILE__):"")  \
					<< (uLog::LogFields::fileName?uLog::separator:"")                 \
					<< (uLog::LogFields::filePath?__FILE__:"")                        \
					<< (uLog::LogFields::filePath?uLog::separator:"")                 \
					<< (uLog::LogFields::funcName?__func__:"")                        \
					<< (uLog::LogFields::funcName?uLog::separator:"")                 \
					<< (uLog::LogFields::funcSig?__PRETTY_FUNCTION__:"")              \
					<< (uLog::LogFields::funcSig?uLog::separator:"")                  \
					<< (uLog::LogFields::line?std::to_string(__LINE__):"")            \
					<< (uLog::LogFields::line?uLog::separator:"")                     \
					<< ": "

		#define uLOGS(logstream, level)  uLOGS_(logstream, level, nolog)

		#define uLOG_(level, localMinLevel)  uLOGS_(uLog::microLog_ofs, level, localMinLevel)

		#define uLOG(level)  uLOGS_(uLog::microLog_ofs, level, nolog)

		#define uLOGF(logfname, level, minLogLev, logMsg) {                                 \
			if(level >= minLogLev && uLog::CheckAvailableSpace(logfname)) {                 \
				MICRO_LOG_LOCK;                                                             \
				std::ofstream ofs(logfname, std::fstream::app);                             \
				ofs << uLog::LogDate()                                                      \
					<< uLog::logLevelTags[level] << uLog::separator                         \
					<< logMsg << std::endl;                                                 \
				MICRO_LOG_UNLOCK;                                                           \
			}                                                                               \
		}

		#define uLOG_TITLES_S(logstream, level)                                       \
			if(uLog::CheckLogLevel(level))                                            \
				MICRO_LOG_LOCK;                                                       \
				logstream                                                             \
					<< uLog::bar << "\n"                                              \
					<< (uLog::LogFields::time?"Time     ":"")                         \
					<< (uLog::LogFields::date?"Date                 ":"")             \
					<< (uLog::LogFields::llevel?"Level   ":"")                        \
					<< (uLog::LogFields::llevel?uLog::separator:"")                   \
					<< (uLog::LogFields::exec?"Exec.  ":"")                           \
					<< (uLog::LogFields::llevel?uLog::separator:"")                   \
					<< (uLog::LogFields::pid?"PID  ":"")                              \
					<< (uLog::LogFields::llevel?uLog::separator:"")                   \
					<< (uLog::LogFields::uid?"UID":"")                                \
					<< (uLog::LogFields::llevel?uLog::separator:"")                   \
					<< (uLog::LogFields::uname?"User  ":"")                           \
					<< (uLog::LogFields::llevel?uLog::separator:"")                   \
					<< (uLog::LogFields::fileName?"Filename  ":"")                    \
					<< (uLog::LogFields::llevel?uLog::separator:"")                   \
					<< (uLog::LogFields::filePath?"Filepath  ":"")                    \
					<< (uLog::LogFields::llevel?uLog::separator:"")                   \
					<< (uLog::LogFields::funcName?"Function  ":"")                    \
					<< (uLog::LogFields::llevel?uLog::separator:"")                   \
					<< (uLog::LogFields::funcSig?"Function_signature  ":"")           \
					<< (uLog::LogFields::llevel?uLog::separator:"")                   \
					<< (uLog::LogFields::line?"Line  ":"")                            \
					<< (uLog::LogFields::llevel?uLog::separator:"")                   \
					<< "Log"                                                          \
					<< "\n" << uLog::bar << uLog::endm;                               \
				MICRO_LOG_UNLOCK

		#define uLOG_TITLES(level)  uLOG_TITLES_S(uLog::microLog_ofs, level)


		// uLOG log terminator
		#define uLOGE uLog::endm; MICRO_LOG_UNLOCK

		#define uLOGT(level) \
			if(uLog::CheckLogLevel(level)) \
				uLog::microLog_ofs

		#define uLOG_DATE \
			if(std::time(&uLog::microLog_time)) \
				uLog::microLog_ofs << "\nDate: " << std::ctime(&uLog::microLog_time)

		#define uLOGD(level) \
			if(std::time(&uLog::microLog_time), uLog::CheckLogLevel(level)) \
				uLog::microLog_ofs << "\nDate: " << std::ctime(&uLog::microLog_time)

		#define uLOGB(level) \
			if(uLog::CheckLogLevel(level)) \
				uLog::microLog_ofs << bar << uLog::endm

		#ifndef MICRO_LOG_DLL

		#endif // MICRO_LOG_DLL

#else // MICRO_LOG_ACTIVE

		struct nullstream {
			void open(const char *filename = 0, std::ios_base::openmode mode = std::ios_base::in | std::ios_base::out)
			{}
		};

		template <typename T>
		nullstream& operator<<(nullstream& s, T const&) { return s; }

		inline nullstream& operator<<(nullstream &s, std::ostream &(std::ostream&)) { return s; }

		inline nullstream& endm(nullstream& os) { return os; }

		#ifndef MICRO_LOG_DLL
			#define uLOG_INIT                                  \
				using namespace uLog;                          \
				int uLog::minLogLevel = MICRO_LOG_MIN_LEVEL;   \
				nullstream uLog::microLog_ofs;                 \
				int Statistics::nLogs = 0, Statistics::nNoLogs = 0, Statistics::nVerboseLogs = 0, Statistics::nDetailLogs = 0, Statistics::nInfoLogs = 0, Statistics::nWarningLogs = 0, Statistics::nErrorLogs = 0, Statistics::nCriticalLogs = 0, Statistics::nFatalLogs = 0; \
				int Statistics::highestLevel = 0;              \
				bool LogFields::time = false, LogFields::date = true, LogFields::llevel = true, LogFields::exec = false, \
					 LogFields::uid = false, LogFields::uname = false, LogFields::pid = false, \
					 LogFields::fileName = false, LogFields::filePath = false, LogFields::funcName = false, LogFields::funcSig = false, \
					 LogFields::line = false, LogFields::log = true
		#else
			#define uLOG_INIT                                  \
				using namespace uLog;                          \
				int uLog::minLogLevel = MICRO_LOG_MIN_LEVEL;   \
				nullstream uLog::microLog_ofs
		#endif

		#define uLOG_START(logFilename)                        \
			std::cout << "Logger disabled." << std::endl

		#define uLOG_START_APP(logFilename)                    \
			std::cout << "Logger disabled." << std::endl

		#ifdef MICRO_LOG_FILE_NAME
			static nullstream microLog_ofs;
		#else
			extern nullstream microLog_ofs;
			// Define it at global scope and open it (in append mode) before logging.
		#endif

		#define uLOG(level)                  if(0) microLog_ofs
		#define uLOG_(level, localMinLevel)  if(0) microLog_ofs
		#define uLOGF(logfname, level, minLogLev, logMsg)
		#define uLOGE                        ""
		#define uLOG_DATE                    if(0) microLog_ofs
		#define uLOG_TITLES(level)           if(0) microLog_ofs
		#define uLOGT(level)                 if(0) microLog_ofs
		#define uLOGD(level)                 if(0) microLog_ofs
		#define uLOGB(level)                 if(0) microLog_ofs
		#define uLOG_LEVEL                   if(0) microLog_ofs

		#ifndef MICRO_LOG_DLL
		inline void LogLevels() {}
		inline void MinLogLevel() {}
		inline void Statistics::Update(int level) {}
		inline void Statistics::Log() {}
		#endif

		inline int BackupPrevLog(int mode, const std::string &backupPath) { return 0; }

	#endif // MICRO_LOG_ACTIVE


} // namespace uLog

#endif
