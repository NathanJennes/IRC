//
// Created by Cyril Battistolo on 25/02/2023.
//

#include "log.h"
#include "Command.h"
#include "IRC.h"

int auth(User& user, const Command& command)
{
	(void)command;
	user.update_write_buffer("AUTH");
	return 0;
}

int cap(User& user, const Command& command)
{
	(void)command;
	user.update_write_buffer("CAP");
	return 0;
}

int error(User& user, const Command& command)
{
	(void)command;
	user.update_write_buffer("ERROR");
	return 0;
}

int nick(User& user, const Command& command)
{
	if (command.get_parameters().empty()) {
		user.reply(ERR_NONICKNAMEGIVEN);
		return 1;
	}
	user.set_nickname(command.get_parameters()[0]);
	user.reply(" NICK :" + user.nickname());
	return 0;
}

int oper(User& user, const Command& command)
{
	(void)command;
	user.update_write_buffer("OPER");
	return 0;
}

int pass(User& user, const Command& command)
{
	(void)command;
	user.update_write_buffer("PASS");
	return 0;
}

int ping(User& user, const Command& command)
{
	(void)command;
	user.update_write_buffer("PING");
	return 0;
}

int pong(User& user, const Command& command)
{
	(void)command;
	user.update_write_buffer("PONG");
	return 0;
}

int user(User& user, const Command& command)
{
	if (user.is_registered()) {
		user.reply(ERR_ALREADYREGISTERED);
		return 1;
	}

	if (command.get_parameters().empty()) {
		user.reply(ERR_NEEDMOREPARAMS(command.get_command(), "Not enough parameters"));
		user.set_username(user.nickname());
		user.set_realname(user.nickname());
		return 1;
	}

	bool is_valid = true;
	if (command.get_parameters().size() < 4) {
		user.reply(ERR_NEEDMOREPARAMS(command.get_command(), "username too long"));
		user.set_realname(user.nickname());
		is_valid = false;
	}
	if (command.get_parameters()[0].length() < 2) {
		user.reply(ERR_NEEDMOREPARAMS(command.get_command(), "username too short"));
		user.set_username(user.nickname());
		is_valid = false;
	}

	if (is_valid && command.get_parameters().size() == 4) {
		user.set_username(command.get_parameters()[0]);
		user.set_realname(command.get_parameters()[3]);
	}

	user.set_registered();
	CORE_DEBUG("User %s registered", user.nickname().c_str());
	return 0;
}


int quit(User& user, const Command& command)
{
	(void)command;
	user.update_write_buffer("QUIT");
	return 0;
}

// Channel operations
// ========================
int join(User& user, const Command& command)
{
	(void)command;
	user.update_write_buffer("JOINNED");
	return 0;
}
