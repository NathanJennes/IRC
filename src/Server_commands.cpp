//
// Created by Cyril Battistolo on 25/02/2023.
//

#include "log.h"
#include "IRC.h"
#include "Command.h"
#include "Server.h"

int auth(Server& server, User& user, const Command& command)
{
	(void)server;
	(void)command;
	user.update_write_buffer("AUTH");
	return 0;
}

int cap(Server& server, User& user, const Command& command)
{
	(void)server;
	if (command.get_parameters().empty()) {
		user.reply("CAP : Error no subcommand");
		return 1;
	}

	if (command.get_parameters()[0] == "LS")
		user.reply("CAP * LS :");
	else if (command.get_parameters()[0] == "LIST")
		user.reply("CAP * LIST :");
	else if (command.get_parameters()[0] == "REQ")
		user.reply("CAP * ACK :");
	else if (command.get_parameters()[0] == "END")
		user.reply(RPL_WELCOME(server.server_name(), user.nickname()));
	else
		user.reply("CAP : Error invalid subcommand");
	return 0;
}

int error(Server& server, User& user, const Command& command)
{
	(void)server;
	if (command.get_parameters().empty())
		user.reply("ERROR :");
	else
		user.reply("ERROR :" + command.get_parameters()[0]);
	return 0;
}

int nick(Server& server, User& user, const Command& command)
{
	(void)server;
	if (command.get_parameters().empty()) {
		user.reply(ERR_NONICKNAMEGIVEN);
		return 1;
	}
	std::string old_nickname = user.nickname();
	user.set_nickname(command.get_parameters()[0]);
//	user.reply(":" + old_nickname + user.source() + " NICK :" + user.nickname());
	return 0;
}

int oper(Server& server, User& user, const Command& command)
{
	(void)server;
	(void)command;
	user.update_write_buffer("OPER");
	return 0;
}

int pass(Server& server, User& user, const Command& command)
{
	(void)server;
	(void)command;
	user.update_write_buffer("PASS");
	return 0;
}

int ping(Server& server, User& user, const Command& command)
{
	(void)server;
	(void)command;
	user.reply("PING");
	return 0;
}

int pong(Server& server, User& user, const Command& command)
{
	(void)server;
	(void)command;
	user.reply("PONG");
	return 0;
}

int user(Server& server, User& user, const Command& command)
{
	(void)server;
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


int quit(Server& server, User& user, const Command& command)
{
	(void)server;
	if (command.get_parameters().empty())
		user.reply("QUIT :");
	else
		user.reply("QUIT :" + command.get_parameters()[0]);
	user.disconnect();
	return 0;
}

// Channel operations
// ========================
int join(Server& server, User& user, const Command& command)
{
	(void)server;
	(void)command;
	user.update_write_buffer("JOINNED");
	return 0;
}
