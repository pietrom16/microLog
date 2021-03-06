# microLog

#### - Single header logger (28 KB only).
#### - Three log level thresholds, to tune verbosity on a global and local level.

Distinctive features:

- Three log level thresholds:
   - A global one, defined at build time.
   - A global one, defined either at build time or at run time.
   - A local one, defined either at build time or at run time. This allows to set different log level thresholds for different parts of the code.

Why a local log level threshold?  
Because it can be useful to focus on specific parts of a project, setting different thresholds for different files/code blocks/functionalities. 

Other features:

- A single file project.
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
- In case it is not possible to initialize microLog, or it is not possible to modify the main() function to call uLOG_START_APP(), you can use:  uLOGF(logfname, level, minLogLev, logMsg)

For better performance, consider logging to a ramdisk (TODO: external utility that periodically copies the log file from ramdisk to hard disk).

Quick example:

	uLOG_START_APP(logPath);
	uLOG(warning) << "Test log message " << 123 << uLOGE;
	uLOG_(detail, localLevel) << "Test log message " << 456 << uLOGE;

For a complete example, see:  microLog_test.cpp


Requirements:  

- Built in single threaded mode: C++03 only.
- Built in multithreading mode: C++03 and Boost, or C++11 only.

If these features are not enough for you, I suggest this other, much more advanced, logger: [G3log](https://bitbucket.org/KjellKod/g3log)

