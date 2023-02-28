#ifndef LOG_H
# define LOG_H

#define LOG_WARN_ENABLED 1
#define LOG_INFO_ENABLED 1
#define LOG_TRACE_ENABLED 1
#define LOG_TRACE_IRC_ERR 1

#ifdef DEBUG
# define LOG_DEBUG_ENABLED 1
#else
# define LOG_DEBUG_ENABLED 0
#endif

namespace Log
{
void log_message(bool newline, const char *error_level, const char *message, ...);
}


#if LOG_TRACE_ENABLED == 1
# define CORE_TRACE(message, ...) {Log::log_message(true, "TRACE", message, ##__VA_ARGS__);}
#else
# define CORE_TRACE(message, ...)
#endif

#if LOG_DEBUG_ENABLED == 1
# define CORE_DEBUG(message, ...) {Log::log_message(true, "DEBUG", message, ##__VA_ARGS__);}
#else
# define CORE_DEBUG(message, ...)
#endif

#if LOG_TRACE_IRC_ERR == 1
# define CORE_TRACE_IRC_ERR(message, ...) {Log::log_message(true, "TRACE_IRC_ERR", message, ##__VA_ARGS__);}
#else
# define CORE_TRACE_IRC_ERR(message, ...)
#endif

#if LOG_INFO_ENABLED == 1
# define CORE_INFO(message, ...) {Log::log_message(true, "INFO", message, ##__VA_ARGS__);}
#else
# define CORE_INFO(message, ...)
#endif

#if LOG_WARN_ENABLED == 1
# ifdef DEBUG
#  define CORE_WARN(message, ...) {Log::log_message(false, "WARN", "[%s:%d %s()]\t", __FILE__, __LINE__, __func__); Log::log_message(true, nullptr, message, ##__VA_ARGS__);}
# else
#  define CORE_WARN(message, ...) {Log::log_message(true, "WARN", message, ##__VA_ARGS__);}
# endif
#else
# define CORE_WARN(message, ...)
#endif

#ifdef DEBUG
# define CORE_ERROR(message, ...) {Log::log_message(false, "ERROR", "[%s:%d %s()]\t", __FILE__, __LINE__, __func__); Log::log_message(true, nullptr, message, ##__VA_ARGS__);}
#else
# define CORE_ERROR(message, ...) {Log::log_message(true, "ERROR", message, ##__VA_ARGS__);}
#endif

# define CORE_FATAL(message, ...) {Log::log_message(true, "FATAL", message, ##__VA_ARGS__);}

#define TODO_PROPAGATE_ERRORS CORE_DEBUG("TODO: this function should propagate errors!");

#endif
