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
