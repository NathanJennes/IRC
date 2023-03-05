//
// Created by Cyril Battistolo on 05/03/2023.
//

#ifndef MODE_H
#define MODE_H

#include <string>
#include "Command.h"

class User;
class Command;

typedef struct mode_param
{
	bool		is_adding;
	char		mode;
	std::string	arg;
}	mode_param;

int mode(User& user, const Command& command);

#endif //MODE_H
