//
// Created by Cyril Battistolo on 25/02/2023.
//

#include <algorithm>
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
	// https://modern.ircdocs.horse/#cap-message
	// Command: CAP
	// Parameters: <subcommand> [<subcommand parameters>]

	if (command.get_parameters().empty()) {
		Server::reply(user, RPL_CAP(user, "LS", ""));
		return 1;
	}

	if (command.get_parameters()[0] == "LS") {
		Server::reply(user, RPL_CAP(user, "LS", ""));
		if (!user.is_registered())
			user.set_is_negociating_capabilities(true);
	}
	else if (command.get_parameters()[0] == "LIST") {
		Server::reply(user, RPL_CAP(user, "LIST", ""));
		if (!user.is_registered())
			user.set_is_negociating_capabilities(true);
	}
	else if (command.get_parameters()[0] == "REQ") {
		// TODO: check if capabilities are valid and if they are supported
		Server::reply(user, RPL_CAP(user, "ACK", ""));
		if (!user.is_registered())
			user.set_is_negociating_capabilities(true);
	}
	else if (command.get_parameters()[0] == "END") {
		user.set_is_negociating_capabilities(false);
		if (!user.is_registered())
			user.try_finish_registration();
	}
	else
		Server::reply(user, ERR_INVALIDCAPCMD(user, command.get_parameters()[0]));
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
		Server::reply(user, ERR_NONICKNAMEGIVEN(user));
		return 1;
	}

	if (command.get_parameters()[0] == user.nickname())
		return 0;

	// TODO: check if nickname is valid
	if (command.get_parameters()[0].size() > MAX_NICKNAME_LENGTH) {
		Server::reply(user, ERR_ERRONEUSNICKNAME(user, command.get_parameters()[0]));
		return 1;
	}

	if (Server::user_exists(command.get_parameters()[0])) {
		if (!user.is_registered())
		{
			user.set_nickname("Guest" + to_string(Server::users().size()));
			Server::reply(user, user.source() + " NICK :" + user.nickname());
			return 0;
		}
		Server::reply(user, ERR_NICKNAMEINUSE(user, command.get_parameters()[0]));
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
		Server::reply(user, ERR_NEEDMOREPARAMS(user, command));
		return 1;
	}

	// TODO: implement

	return 0;
}

int pass(User& user, const Command& command)
{
	// https://modern.ircdocs.horse/#pass-message
	// Command: PASS
	// Parameters: <password>

	if (user.is_registered()) {
		Server::reply(user, ERR_ALREADYREGISTERED(user));
		user.disconnect();
		return 1;
	}

	if (command.get_parameters().empty()) {
		Server::reply(user, ERR_NEEDMOREPARAMS(user, command));
		user.disconnect();
		return 1;
	}

	user.set_password(command.get_parameters()[0]);
	return 0;
}

int ping(User& user, const Command& command)
{
	// https://modern.ircdocs.horse/#ping-message
	// Command: PING
	// Parameters: <token>

	if (command.get_parameters().empty()) {
		Server::reply(user, ERR_NEEDMOREPARAMS(user, command));
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
		CORE_TRACE_IRC_ERR("Got PONG command with a <server> parameter from %s. We currently ignore these commands.", user.debug_name());
	}

	return 0;
}

int user(User& user, const Command& command)
{
	// https://modern.ircdocs.horse/#user-message
	// Command: USER
	// Parameters: <username> 0 * <realname>

	if (user.nickname().empty()) {
		return 1;
	}

	if (user.is_registered()) {
		Server::reply(user, ERR_ALREADYREGISTERED(user));
		return 1;
	}

	if (command.get_parameters().size() < 4) {
		Server::reply(user, RPL_MESSAGE(user, "USER", ":username too long"));
		return 1;
	}

	if (command.get_parameters()[0].length() < 1) {
		Server::reply(user, RPL_MESSAGE(user, "USER", ":username too short"));
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
	// https://modern.ircdocs.horse/#quit-message
	// Command: QUIT
	// Parameters: [<quit message>]

	if (command.get_parameters().empty())
		Server::broadcast(user, user.source() + " QUIT :Quit: ");
	else
		Server::broadcast(user, user.source() + " QUIT :Quit: " + command.get_parameters()[0]);
	user.disconnect();
	return 0;
}

// Channel operations
// ========================
int join(User& user, const Command& command)
{
	//  Command: JOIN
	//  Parameters: <channel>{,<channel>} [<key>{,<key>}]

	// If we receive a source, simply ignore the command as it may come from a server
	if (!command.get_source().source_name.empty()) {
		CORE_TRACE_IRC_ERR("Got a <source> inside a JOIN message from %s. Did the message come from a server ?", user.debug_name());
		return 0;
	}

	const std::vector<std::string>& params = command.get_parameters();

	// If the parameter list is empty, ignore the command and return an error
	if (params.empty() || params[0].empty()) {
		Server::reply(user, ERR_NEEDMOREPARAMS(user, command));
		return 0;
	}

	// Retrieve all channels names
	std::vector<std::string> requested_channels;
	{
		const std::string& first_param = params[0];
		size_t last_pos = 0;
		size_t new_pos = first_param.find_first_of(',');
		while (new_pos != std::string::npos)
		{
			requested_channels.push_back(first_param.substr(last_pos, new_pos - last_pos));
			last_pos = new_pos;
			new_pos = first_param.find_first_of(',');
		}

		std::string last_channel = first_param.substr(last_pos);
		if (!last_channel.empty())
			requested_channels.push_back(last_channel);
	}

	// Retrieve all keys
	std::vector<std::string> keys;
	if (params.size() > 1) {
		const std::string& second_param = params[1];
		size_t last_pos = 0;
		size_t new_pos = second_param.find_first_of(',');
		while (new_pos != std::string::npos)
		{
			keys.push_back(second_param.substr(last_pos, new_pos - last_pos));
			last_pos = new_pos;
			new_pos = second_param.find_first_of(',');
		}
	}

	typedef std::vector<Channel>::iterator ChannelIter;
	typedef std::vector<std::string>::const_iterator StringIter;

	std::vector<Channel>& server_channels = Server::channels();
	StringIter current_key = keys.begin();

	// For all channels in the command
	for (StringIter requested_chan = requested_channels.begin(); requested_chan != requested_channels.end(); requested_chan++) {
		// Find the associated Channel
		ChannelIter server_channel = server_channels.end();
		for (ChannelIter chan = server_channels.begin(); chan != server_channels.end(); chan++) {
			if (*requested_chan == chan->name()) {
				server_channel = chan;
				break ;
			}
		}

		// If no associated channel was found, create a new channel with its name
		if (server_channel == server_channels.end()) {
			Server::channels().push_back(Channel(user, *requested_chan));
			server_channel = Server::channels().end() - 1;
		}

		// If channel is invite-only
		if (server_channel->is_invite_only()) {

			// Check if the user is in the invite-list
			bool is_invited = false;
			const std::vector<std::string>& invite_list = server_channel->invite_list();
			for (StringIter invited_user = invite_list.begin(); invited_user != invite_list.end(); invited_user++) {
				if (user.nickname() == *invited_user) {
					is_invited = true;
					break ;
				}
			}

			// Else, check if the channels allow for invite-exemptions
			if (!is_invited && server_channel->has_invite_exemptions()) {

				// Check if the user is part of the exemptions
				const std::vector<std::string>& invite_exempt_list = server_channel->invite_exemptions();
				for (StringIter exempted_user = invite_exempt_list.begin(); exempted_user != invite_exempt_list.end(); exempted_user++) {
					if (user.nickname() == *exempted_user) {
						is_invited = true;
						break ;
					}
				}
			}

			// If the user is not invited and is not exempted from the invite-list,
			// send error and continue to next channel
			if (!is_invited) {
				CORE_TRACE_IRC_ERR("User %s tried and failed to connect to channel [%s] because it was invite only.", user.debug_name(), server_channel->name().c_str());
				Server::reply(user, ERR_INVITEONLYCHAN(user, server_channel->name()));
				continue ;
			}
		}

		// If channel has a ban-list
		if (server_channel->is_ban_protected()) {

			// Check if the user is banned
			bool is_banned = false;
			const std::vector<std::string>& ban_list = server_channel->ban_list();
			for (StringIter banned_user_name = ban_list.begin(); banned_user_name != ban_list.end(); banned_user_name++) {
				if (user.nickname() == *banned_user_name) {
					is_banned = true;
					break ;
				}
			}

			// If the user is banned, check if the channel allows for ban-exemptions
			if (is_banned && server_channel->has_ban_exemptions()) {
				const std::vector<std::string>& ban_exemptions = server_channel->ban_exemptions();
				for (StringIter ban_exempt_user_name = ban_exemptions.begin(); ban_exempt_user_name != ban_exemptions.end(); ban_exempt_user_name++) {
					if (user.nickname() == *ban_exempt_user_name) {
						is_banned = false;
						break ;
					}
				}
			}

			// If the user is banned and is not exempted from the ban-list,
			// send error and continue to next channel
			if (is_banned) {
				CORE_TRACE_IRC_ERR("User %s tried and failed to connect to channel [%s] because it was banned.", user.debug_name(), server_channel->name().c_str());
				Server::reply(user, ERR_BANNEDFROMCHAN(user, server_channel->name()));
				continue ;
			}
		}

		// If the channel requires a key
		if (server_channel->is_key_protected()) {

			// If the user didn't provide a key, send an error and continue
			if (current_key == keys.end()) {
				CORE_TRACE_IRC_ERR("User %s failed to connect to channel %s because it didn't provide a key.", user.debug_name(), server_channel->name().c_str());
				Server::reply(user, ERR_BADCHANNELKEY(user, server_channel->name()));
				continue ;
			}

			// If the user provided the wrong key, send an error and continue
			if (*current_key != server_channel->key()) {
				CORE_TRACE_IRC_ERR("User %s failed to connect to channel %s because it provided the wrong key.", user.debug_name(), server_channel->name().c_str());
				Server::reply(user, ERR_BADCHANNELKEY(user, server_channel->name()));
				continue ;
			}
		}

		// Check the channel capacity
		if (server_channel->is_user_limited()) {
			if (server_channel->user_count() >= server_channel->user_limit()) {
				CORE_TRACE_IRC_ERR("User %s failed to connect to channel %s because it was full.", user.debug_name(), server_channel->name().c_str());
				Server::reply(user, ERR_CHANNELISFULL(user, server_channel->name()));
				continue ;
			}
		}

		CORE_INFO("User %s joined the channel %s", user.debug_name(), server_channel->name().c_str());
		user.channels().push_back(server_channel->name());
		server_channel->add_user(user);
		Server::reply(user, user.source() + " JOIN " + server_channel->name());
		Server::reply(user, RPL_TOPIC(user, (*server_channel)));
		Server::reply_list_channel_members_to_user(user, *server_channel);
	}
	return 0;
}

int mode(User& user, const Command& command)
{
	// https://modern.ircdocs.horse/#mode-message
	// Command: MODE
	// Parameters: <target> [<modestring> [<mode arguments>...]]

	if (command.get_parameters().empty()) {
		CORE_TRACE_IRC_ERR("User %s sent a MODE command with no parameters.", user.debug_name());
		Server::reply(user, ERR_NEEDMOREPARAMS(user, command));
		return 0;
	}

	if (is_channel(command.get_parameters()[0]))
	{
		Server::ChannelIterator channel = Server::find_channel(command.get_parameters()[0]);
		if (channel != Server::channels().end())
		{
			if (command.get_parameters().size() == 1) {
				Server::reply(user, RPL_CHANNELMODEIS(user, channel));
				return 0;
			}
			if (command.get_parameters().size() == 2) {
				if (channel->has_user(user))
					channel->update_modes(command);
				else {
					CORE_TRACE_IRC_ERR("User %s tried to set mode on a channel [%s] he is not in.", user.debug_name(), command.get_parameters()[0].c_str());
					Server::reply(user, ERR_CHANOPRIVSNEEDED(user, channel));
				}
			}
			return 0;
		}
		else {
			CORE_TRACE_IRC_ERR("User %s tried to set mode on a non-existing channel [%s].", user.debug_name(), command.get_parameters()[0].c_str());
			Server::reply(user, ERR_NOSUCHCHANNEL(user, command.get_parameters()[0]));
			return 1;
		}
	}
	// TODO: Handle user mode

	return 0;
}
