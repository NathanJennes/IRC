//
// Created by Cyril Battistolo on 25/02/2023.
//

#ifndef UTILS_H
#define UTILS_H

#include <string>
#include <sstream>

std::string	to_upper(const std::string& str);
std::string to_lower(const std::string& str);
bool 		is_number(const std::string& str);
bool 		is_number(char *str);

template<typename T>
std::string to_string(T value)
{
	std::stringstream ss;
	ss << value;
	std::string str(ss.str());
	return str;
}

std::string get_current_date();
std::string format_date();

#endif //UTILS_H
