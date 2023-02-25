//
// Created by Cyril Battistolo on 25/02/2023.
//

#include "Command.h"
#include "IRC.h"

int auth(User& user, const Command& command)
{
	(void)command;
	user.update_write_buffer("AUTH\r\n");
	return 0;
}

int cap(User& user, const Command& command)
{
	(void)command;
	user.update_write_buffer("CAP\r\n");
	return 0;
}

int error(User& user, const Command& command)
{
	(void)command;
	user.update_write_buffer("ERROR\r\n");
	return 0;
}

int nick(User& user, const Command& command)
{
	if (!command.get_parameters().empty())
		user.set_nickname(command.get_parameters()[0]);
	user.update_write_buffer(user.source() + " NICK :" + user.nickname() + "\r\n");
	return 0;
}

int oper(User& user, const Command& command)
{
	(void)command;
	user.update_write_buffer("OPER\r\n");
	return 0;
}

int pass(User& user, const Command& command)
{
	(void)command;
	user.update_write_buffer("PASS\r\n");
	return 0;
}

int ping(User& user, const Command& command)
{
	(void)command;
	user.update_write_buffer("PING\r\n");
	return 0;
}

int pong(User& user, const Command& command)
{
	(void)command;
	user.update_write_buffer("PONG\r\n");
	return 0;
}

int user(User& user, const Command& command)
{
	(void)command;
	user.update_write_buffer("USER\r\n");
	return 0;
}


int quit(User& user, const Command& command)
{
	(void)command;
	user.update_write_buffer("QUIT\r\n");
	return 0;
}

// Channel operations
// ========================
int join(User& user, const Command& command)
{
	(void)command;
	user.update_write_buffer("JOINNED\r\n");
	return 0;
}
