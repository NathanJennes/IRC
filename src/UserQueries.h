//
// Created by Cyril Battistolo on 10/03/2023.
//

#ifndef USERQUERIES_H
#define USERQUERIES_H

#include <string>
#include <vector>
#include "User.h"

struct Mask
{
	std::string str;
	bool before;
	bool after;
};

std::vector<Mask> parse_masks(const std::string& mask);

int who(User& user, const Command& command);


#endif //USERQUERIES_H
