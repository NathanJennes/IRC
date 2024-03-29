//
// Created by Cyril Battistolo on 25/02/2023.
//

#include <cctype>
#include <algorithm>
#include <ctime>
#include <cstring>
#include "Utils.h"

std::string to_upper(const std::string& str)
{
	std::string str_to_upper(str);
	for (std::size_t i = 0; i < str.size(); i++) {
		if (str[i] >= 'a' && str[i] <= 'z')
			str_to_upper[i] = static_cast<char>(str[i] - 32);
		else
			str_to_upper[i] = str[i];
	}
	return str_to_upper;
}

std::string to_lower(const std::string& str)
{
	std::string str_to_lower(str);
	for (std::size_t i = 0; i < str.size(); i++) {
		if (str[i] >= 'A' && str[i] <= 'Z')
			str_to_lower[i] = static_cast<char>(str[i] + 32);
		else
			str_to_lower[i] = str[i];
	}
	return str_to_lower;
}

bool is_number(const std::string& str)
{
	if (str.empty())
		return false;

	for (std::string::const_iterator it = str.begin(); it != str.end(); ++it)
		if (!std::isdigit(*it))
			return false;

	return true;
}

bool is_number(char *str)
{
	if (!str)
		return false;

	for (size_t i = 0; str[i]; i++)
		if (!std::isdigit(str[i]))
			return false;

	return true;
}

// Get current date/time, format is YYYY-MM-DD.HH:mm:ss
std::string get_current_date()
{
	std::time_t now = time(0);
	std::tm tstruct = *localtime(&now);

	char buffer[80];
	std::memset(buffer, 0, sizeof(buffer));
	std::strftime(buffer, sizeof(buffer), "%A %d %B %Y %X", &tstruct);

	return buffer;
}

std::string format_date()
{
	std::time_t now = time(0);
	std::tm tstruct = *localtime(&now);

	char buffer[80];
	std::memset(buffer, 0, sizeof(buffer));
	std::strftime(buffer, sizeof(buffer), "%A %d %B %Y -- %X %z", &tstruct);

	return buffer;
}
