//
// Created by Cyril Battistolo on 25/02/2023.
//

#ifndef UTILS_H
#define UTILS_H

#include <string>
#include <sstream>

std::string to_upper(std::string str);

template<typename T>
std::string to_string(T value)
{
	std::stringstream ss;
	ss << value;
	std::string str(ss.str());
	return str;
}

#endif //UTILS_H
