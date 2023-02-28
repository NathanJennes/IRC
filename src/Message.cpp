//
// Created by Cyril Battistolo on 25/02/2023.
//

#include "log.h"
#include "IRC.h"
#include "Command.h"
#include "Server.h"
#include "Message.h"

int auth(User& user, const Command& command)
{
	(void)command;
	user.update_write_buffer("AUTH");
	return 0;
}

int cap(User& user, const Command& command)
{
	if (command.get_parameters().empty()) {
		Server::reply(user, "CAP : Error no subcommand");
		return 1;
	}

	if (command.get_parameters()[0] == "LS") {
		Server::reply(user, RPL_CAP(user.nickname(), "LS", ""));
		if (!user.is_registered())
			user.set_is_negociating_capabilities(true);
	}
	else if (command.get_parameters()[0] == "LIST") {
		Server::reply(user, RPL_CAP(user.nickname(), "LIST", ""));
		if (!user.is_registered())
			user.set_is_negociating_capabilities(true);
	}
	else if (command.get_parameters()[0] == "REQ") {
		// TODO: check if capabilities are valid and if they are supported
		Server::reply(user, RPL_CAP(user.nickname(), "ACK", ""));
		if (!user.is_registered())
			user.set_is_negociating_capabilities(true);
	}
	else if (command.get_parameters()[0] == "END") {
		user.set_is_negociating_capabilities(false);
		if (!user.is_registered())
			user.try_finish_registration();
	} else
		Server::reply(user, ERR_INVALIDCAPCMD(user.nickname(), command.get_parameters()[0]));
	return 0;
}

int error(User& user, const Command& command)
{
	if (command.get_parameters().empty())
		Server::reply(user, "ERROR :");
	else
		Server::reply(user, "ERROR :" + command.get_parameters()[0]);
	return 0;
}

int nick(User& user, const Command& command)
{
	// https://modern.ircdocs.horse/#nick-message
	//	Command: NICK
	//	Parameters: <nickname>

	if (command.get_parameters().empty()) {
		Server::reply(user, ERR_NONICKNAMEGIVEN(user.nickname()));
		return 1;
	}

	if (command.get_parameters()[0] == user.nickname())
		return 0;

	// TODO: check if nickname is valid
	if (command.get_parameters()[0].size() > MAX_NICKNAME_LENGTH) {
		Server::reply(user, ERR_ERRONEUSNICKNAME(user.nickname(), command.get_parameters()[0]));
		return 1;
	}

	if (Server::is_nickname_taken(command.get_parameters()[0])) {
		Server::reply(user, ERR_NICKNAMEINUSE(user.nickname(), command.get_parameters()[0]));
		return 1;
	}

	if (user.is_registered())
		Server::reply(user, user.source() + " NICK :" + command.get_parameters()[0]);
	user.set_nickname(command.get_parameters()[0]);
	return 0;
}

int oper(User& user, const Command& command)
{
	// https://modern.ircdocs.horse/#oper-message
	// Command: OPER
	// Parameters: <name> <password>

	if (command.get_parameters().size() < 2) {
		Server::reply(user, ERR_NEEDMOREPARAMS(user.nickname(), command.get_command()));
		return 1;
	}

	// TODO: implement

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
	// https://modern.ircdocs.horse/#ping-message
	// Command: PING
	// Parameters: <token>

	if (command.get_parameters().empty()) {
		Server::reply(user, ERR_NEEDMOREPARAMS(user.nickname(), command.get_command()));
		return 1;
	}

	// Simply reply with a PONG command and pass through the given token
	Server::reply(user, "PONG " + Server::server_name() + " " + command.get_parameters()[0]);
	return 0;
}

int pong(User& user, const Command& command)
{
	// https://modern.ircdocs.horse/#pong-message
	// Command: PONG
	// Parameters: [<server>] <token>

	// The spec specifies no error reply in this case,
	// so we simply ignore it
	if (command.get_parameters().empty())
		return 0;

	// In the case where no <server> is specified (i.e. the pong came from a client)
	if (command.get_parameters().size() == 1) {
		if (command.get_parameters()[0] == user.ping_token())
			user.recalculate_ping();
	}

	// Otherwise if <server> is specified (i.e. the pong came from a server)
	else if (command.get_parameters().size() == 2) {
		CORE_DEBUG("Got PONG command with a <server> parameter. We currently ignore these commands.");
	}

	return 0;
}

int user(User& user, const Command& command)
{
	if (user.is_registered()) {
		Server::reply(user, ERR_ALREADYREGISTERED);
		return 1;
	}

	if (command.get_parameters().size() < 4) {
		Server::reply(user, ERR_NEEDMOREPARAMS(command.get_command(), "username too long"));
		return 1;
	}

	if (command.get_parameters()[0].length() < 1) {
		Server::reply(user, ERR_NEEDMOREPARAMS(command.get_command(), "username too short"));
		return 1;
	}

	user.set_username(command.get_parameters()[0]);
	user.set_realname(command.get_parameters()[3]);
	if (!user.is_registered())
		user.try_finish_registration();
	return 0;
}

int quit(User& user, const Command& command)
{
	if (command.get_parameters().empty())
		Server::reply(user, "QUIT :");
	else
		Server::reply(user, "QUIT :" + command.get_parameters()[0]);
	user.disconnect();
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
