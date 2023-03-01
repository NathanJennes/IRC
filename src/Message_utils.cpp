//
// Created by Cyril Battistolo on 28/02/2023.
//

#include "Message.h"

bool is_channel(const std::string& name)
{
	return name[0] == '#' || name[0] == '&';
}
