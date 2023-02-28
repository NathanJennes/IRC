#include <cstdarg>
#include <iostream>
#include <cstring>
#include <cstdio>
#include "log.h"

#define RESET   "\033[0m"
#define BLACK   "\033[30m"      /* Black */
#define RED     "\033[31m"      /* Red */
#define GREEN   "\033[32m"      /* Green */
#define YELLOW  "\033[33m"      /* Yellow */
#define BLUE    "\033[34m"      /* Blue */
#define MAGENTA "\033[35m"      /* Magenta */
#define CYAN    "\033[36m"      /* Cyan */
#define WHITE   "\033[37m"      /* White */
#define BOLDBLACK   "\033[1m\033[30m"      /* Bold Black */
#define BOLDRED     "\033[1m\033[31m"      /* Bold Red */
#define BOLDGREEN   "\033[1m\033[32m"      /* Bold Green */
#define BOLDYELLOW  "\033[1m\033[33m"      /* Bold Yellow */
#define BOLDBLUE    "\033[1m\033[34m"      /* Bold Blue */
#define BOLDMAGENTA "\033[1m\033[35m"      /* Bold Magenta */
#define BOLDCYAN    "\033[1m\033[36m"      /* Bold Cyan */
#define BOLDWHITE   "\033[1m\033[37m"      /* Bold White */

namespace Log
{

static void set_color(const char *error_level);

void log_message(bool newline, const char *error_level, const char *message, ...)
{
	__builtin_va_list args;
	va_start(args, message);
	if (error_level) {
		set_color(error_level);
		printf("%s: ", error_level);
	}
	vprintf(message, args);
	if (newline)
		printf("%s\n", RESET);
	va_end(args);
}

static void set_color(const char *error_level)
{
	if (strcmp(error_level, "DEBUG") == 0)
		printf("%s", BLUE);
	else if (strcmp(error_level, "TRACE_IRC_ERR") == 0)
		printf("%s", BOLDMAGENTA);
	else if (strcmp(error_level, "TRACE") == 0)
		printf("%s", WHITE);
	else if (strcmp(error_level, "INFO") == 0)
		printf("%s", GREEN);
	else if (strcmp(error_level, "WARN") == 0)
		printf("%s", YELLOW);
	else if (strcmp(error_level, "ERROR") == 0)
		printf("%s", RED);
	else if (strcmp(error_level, "FATAL") == 0)
		printf("%s", BOLDRED);
}

}
