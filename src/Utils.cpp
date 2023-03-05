//
// Created by Cyril Battistolo on 25/02/2023.
//

#include <cctype>
#include <algorithm>
#include "Utils.h"

std::string to_upper(std::string str)
{
	std::transform(str.begin(), str.end(), str.begin(), ::toupper);
	return str;
}

bool is_channel(const std::string& name)
{
	if (name.empty())
	  return false;
	return name[0] == '#' || name[0] == '&';
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
