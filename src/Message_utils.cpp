//
// Created by Cyril Battistolo on 28/02/2023.
//

#include "IRC.h"
#include "Message.h"

namespace Message
{

void welcome(User &user)
{
	Server::reply(user, RPL_WELCOME(user.nickname()));
	Server::reply(user, RPL_YOURHOST(user.nickname()));
	Server::reply(user, RPL_CREATED(user.nickname()));
	Server::reply(user, RPL_MYINFO(user.nickname()));
}

} // namespace Message
