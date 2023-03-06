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
	Server::reply(user, user.source() + " NICK :" + command.get_parameters()[0]);
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
		Server::reply(user, ERR_NEEDMOREPARAMS(user, command));
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

	Server::reply(user, RPL_MESSAGE(user, "ERROR", " :Quit"));

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

	// A JOIN command with "0" as its parameter should disconnect the user from all of its channels
	if (params[0] == "0") {
		for (User::ChannelIterator connected_channel_it = user.channels().begin(); connected_channel_it != user.channels().end(); connected_channel_it++) {
			Server::disconnect_user_from_channel(user, get_channel_reference(connected_channel_it));
		}
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

	typedef std::vector<std::string>::const_iterator StringIter;

	Server::ChannelMap& server_channels = Server::channels();
	StringIter current_key_it = keys.begin();

	// For all channels in the command
	for (StringIter requested_channel_name_it = requested_channels.begin(); requested_channel_name_it != requested_channels.end(); requested_channel_name_it++) {
		// Find the associated Channel
		Server::ChannelIterator server_channel_it = Server::find_channel(*requested_channel_name_it);

		// If no associated channel was found, create a new channel with its name
		if (server_channel_it == server_channels.end()) {
			Channel& new_channel = Server::create_new_channel(user, *requested_channel_name_it);
			Server::reply(user, user.source() + " JOIN " + new_channel.name());
			Server::reply(user, RPL_TOPIC(user, new_channel)); //TODO: The channel should manage sending the topic, since if it's empty, other real irc servers don't send this message at all.
			Server::reply_list_channel_members_to_user(user, new_channel);
			return 0;
		}

		Channel& channel = get_channel_reference(server_channel_it);

		// If channel is invite-only
		if (channel.is_invite_only()) {

			// Check if the user is in the invite-list
			bool is_invited = false;
			const std::vector<std::string>& invite_list = channel.invite_list();
			for (StringIter invited_user = invite_list.begin(); invited_user != invite_list.end(); invited_user++) {
				if (user.nickname() == *invited_user) {
					is_invited = true;
					break ;
				}
			}

			// Else, check if the channels allow for invite-exemptions
			if (!is_invited) {

				// Check if the user is part of the exemptions
				const std::vector<std::string>& invite_exempt_list = channel.invite_exemptions();
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
				CORE_TRACE_IRC_ERR("User %s tried and failed to connect to channel [%s] because it was invite only.", user.debug_name(), channel.name().c_str());
				Server::reply(user, ERR_INVITEONLYCHAN(user, channel.name()));
				continue ;
			}
		}

		// If channel has a ban-list
		{

			// Check if the user is banned
			bool is_banned = false;
			const std::vector<std::string>& ban_list = channel.ban_list();
			for (StringIter banned_user_name = ban_list.begin(); banned_user_name != ban_list.end(); banned_user_name++) {
				if (user.nickname() == *banned_user_name) {
					is_banned = true;
					break ;
				}
			}

			// If the user is banned, check if the channel allows for ban-exemptions
			if (is_banned) {
				const std::vector<std::string>& ban_exemptions = channel.ban_exemptions();
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
				CORE_TRACE_IRC_ERR("User %s tried and failed to connect to channel [%s] because it was banned.", user.debug_name(), channel.name().c_str());
				Server::reply(user, ERR_BANNEDFROMCHAN(user, channel.name()));
				continue ;
			}
		}

		// If the channel requires a key
		if (channel.is_key_protected()) {

			// If the user didn't provide a key, send an error and continue
			if (current_key_it == keys.end()) {
				CORE_TRACE_IRC_ERR("User %s failed to connect to channel %s because it didn't provide a key.", user.debug_name(), channel.name().c_str());
				Server::reply(user, ERR_BADCHANNELKEY(user, channel.name()));
				continue ;
			}

			// If the user provided the wrong key, send an error and continue
			if (*current_key_it != channel.key()) {
				CORE_TRACE_IRC_ERR("User %s failed to connect to channel %s because it provided the wrong key.", user.debug_name(), channel.name().c_str());
				Server::reply(user, ERR_BADCHANNELKEY(user, channel.name()));
				continue ;
			}
		}

		// Check the channel capacity
		if (channel.is_user_limited()) {
			if (channel.user_count() >= channel.user_limit()) {
				CORE_TRACE_IRC_ERR("User %s failed to connect to channel %s because it was full.", user.debug_name(), channel.name().c_str());
				Server::reply(user, ERR_CHANNELISFULL(user, channel.name()));
				continue ;
			}
		}

		// Notify all users of the channel of the newcomer
		for (Channel::UserIterator channel_user_it = channel.users().begin(); channel_user_it != channel.users().end(); channel_user_it++) {
			User& connected_user = get_user_reference(channel_user_it);
			Server::reply(connected_user, user.source() + " JOIN " + channel.name());
		}

		CORE_INFO("User %s joined the channel %s", user.debug_name(), channel.name().c_str());
		user.add_channel(channel);
		channel.add_user(user);
		Server::reply(user, user.source() + " JOIN " + channel.name());
		Server::reply(user, RPL_TOPIC(user, channel)); //TODO: The channel should manage sending the topic, since if it's empty, other real irc servers don't send this message at all.
		Server::reply_list_channel_members_to_user(user, channel);
	}
	return 0;
}

int part(User& user, const Command& command)
{
	//  https://modern.ircdocs.horse/#part-message
	//  Command: PART
	//  Parameters: <channel>{,<channel>} [<reason>]

	const std::vector<std::string>& params = command.get_parameters();

	// Check for parameter count
	if (params.empty() || params[0].empty()) {
		Server::reply(user, ERR_NEEDMOREPARAMS(user, command));
		return 0;
	}

	// Retrieve the reason if there is one
	std::string reason;
	if (params.size() == 2) {
		reason = params[1];
	}

	// Iterate over all the channels and disconnect the user from them
	std::string requested_channel;
	const std::string& first_param = params[0];
	size_t last_pos = 0;
	size_t new_pos = first_param.find_first_of(',');
	while (new_pos != std::string::npos)
	{
		requested_channel = first_param.substr(last_pos, new_pos - last_pos);
		last_pos = new_pos;
		new_pos = first_param.find_first_of(',');
		Server::try_disconnect_user_from_channel(user, requested_channel, reason);
	}
	requested_channel = first_param.substr(last_pos);
	Server::try_disconnect_user_from_channel(user, requested_channel, reason);
	return 0;
}

int names(User& user, const Command& command)
{
	//  https://modern.ircdocs.horse/#names-message
	//  Command: NAMES
	//  Parameters: <channel>{,<channel>}

	const std::vector<std::string>& params = command.get_parameters();

	// Check for parameter count
	if (params.empty() || params[0].empty()) {
		Server::reply(user, ERR_NEEDMOREPARAMS(user, command));
		return 0;
	}

	// Iterate over all the channels and list connected users to the user
	std::string requested_channel;
	const std::string& first_param = params[0];
	size_t last_pos = 0;
	size_t new_pos = first_param.find_first_of(',');
	while (new_pos != std::string::npos)
	{
		requested_channel = first_param.substr(last_pos, new_pos - last_pos);
		last_pos = new_pos;
		new_pos = first_param.find_first_of(',');
		Server::try_reply_list_channel_members_to_user(user, requested_channel);
	}
	requested_channel = first_param.substr(last_pos);
	Server::try_reply_list_channel_members_to_user(user, requested_channel);
	return 0;
}

int privmsg(User& user, const Command& command)
{
	// https://modern.ircdocs.horse/#privmsg-message
	// Command: PRIVMSG
	// Parameters: <target>{,<target>} <text to be sent>

	// TODO: Handle when target is empty
	if (command.get_parameters().size() < 2)
	{
		CORE_TRACE_IRC_ERR("User %s sent a PRIVMSG command with less than 2 parameters.", user.debug_name());
		Server::reply(user, ERR_NEEDMOREPARAMS(user, command));
		return 0;
	}

	if (is_channel(command.get_parameters()[0]))
	{
		Server::ChannelIterator target_channel_it = Server::find_channel(command.get_parameters()[0]);
		if (!Server::channel_exists(target_channel_it)) {
			CORE_TRACE_IRC_ERR("User %s tried to send a message to a non-existing channel [%s].", user.debug_name(), command.get_parameters()[0].c_str());
			Server::reply(user, ERR_NOSUCHCHANNEL(user, command.get_parameters()[0]));
			return 1;
		}

		Channel& target_channel = get_channel_reference(target_channel_it);
		if (target_channel.has_user(user)) {
			std::string message = user.source() + " PRIVMSG " + target_channel.name() + " :" + command.get_parameters()[1];
			Server::broadcast_to_channel(user, target_channel, message);
		} else {
			CORE_TRACE_IRC_ERR("User %s tried to send a message to a channel [%s] he is not in.", user.debug_name(), command.get_parameters()[0].c_str());
			Server::reply(user, ERR_CANNOTSENDTOCHAN(user, target_channel));
		}
	}
	else
	{
		Server::UserIterator target_user_it = Server::find_user(command.get_parameters()[0]);
		if (!Server::user_exists(target_user_it)) {
			CORE_TRACE_IRC_ERR("User %s tried to send a message to a non-existing user [%s].", user.debug_name(), command.get_parameters()[0].c_str());
			Server::reply(user, ERR_NOSUCHNICK(user, command.get_parameters()[0]));
			return 1;
		}

		User& target_user = get_user_reference(target_user_it);
		if (target_user.is_away()) {
			CORE_TRACE_IRC_ERR("User %s is away.", target_user.debug_name());
			Server::reply(user, RPL_AWAY(user, target_user));
		}

		std::string message = user.source() + " PRIVMSG " + target_user.nickname() + " :" + command.get_parameters()[1];
		Server::reply(target_user, message);
	}
	return 0;
}

int motd(User& user, const Command& command)
{
	// https://modern.ircdocs.horse/#motd-message
	// Command: MOTD
	// Parameters: [<target>]

	if (command.get_parameters().size() == 1) {
		CORE_TRACE_IRC_ERR("User %s sent a MOTD to a non-existing server", user.debug_name());
		Server::reply(user, ERR_NOSUCHSERVER(user, command.get_parameters()[0]));
		return 1;
	}

	std::stringstream motd(Server::motd_string());
	std::string line;

	Server::reply(user, RPL_MOTDSTART(user));
	while (std::getline(motd, line))
	{
		CORE_TRACE("MOTD line: %s", line.c_str());
		Server::reply(user, RPL_MOTD(user, line));
	}
	Server::reply(user, RPL_ENDOFMOTD(user));

	return 0;
}
