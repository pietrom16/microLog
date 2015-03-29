# microLog

#### - Single header logger (27 KB only).
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
- To activate it, #define MICRO_LOG_ACTIVE.
- When not activated, or when a log message level is below the threshold, there is no generated binary code.
- uLOG(level) only logs if level >= MICRO_LOG_MIN_LEVEL and level >= ULog::minLogLevel.
- uLOG(level, localLevel) logs if level >= MICRO_LOG_MIN_LEVEL and (level >= ULog::minLogLevel or level >= localLevel).

For better performance, consider logging to a ramdisk (TODO: external utility that periodically copies the log file from ramdisk to hard disk).

Usage: See microLog_test.cpp as an example.

If these features are not enough for you, I suggest this other, much more advanced, logger: [G3log](https://bitbucket.org/KjellKod/g3log)

