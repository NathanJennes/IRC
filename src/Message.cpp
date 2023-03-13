//
// Created by Cyril Battistolo on 25/02/2023.
//

#include <algorithm>
#include "log.h"
#include "Numerics.h"
#include "Command.h"
#include "Server.h"
#include "Message.h"
#include "ParamSplitter.h"
#include "ConditionalChannelList.h"

int auth(User& user, const Command& command)
{
	// https://modern.ircdocs.horse/#auth-message
	// Command: AUTH
	// Parameters: <mechanism> [<initial-response>]n

	(void)command;
	(void)user;
	//TODO: to implement.
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
		Server::reply(user, RPL_CAP(user, "LS", "multi-prefix"));
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

	const std::string& requested_nickname = command.get_parameters()[0];

	if (requested_nickname == user.nickname())
		return 0;

	if (!User::is_nickname_valid(requested_nickname)) {
		Server::reply(user, ERR_ERRONEUSNICKNAME(user, requested_nickname));
		return 1;
	}

	if (Server::user_exists(requested_nickname)) {
		if (!user.is_registered())
		{
			std::size_t suffix = 1;
			std::string new_nickname = requested_nickname + to_string(suffix);
			while (User::is_nickname_valid(new_nickname) && Server::user_exists(new_nickname))
				new_nickname = requested_nickname + to_string(++suffix);

			if (User::is_nickname_valid(new_nickname) && !Server::user_exists(new_nickname)) {
				user.set_nickname(new_nickname);
				Server::reply(user, USER_SOURCE("NICK", user) + " :" + user.nickname());
				return 0;
			}
		}
		Server::reply(user, ERR_NICKNAMEINUSE(user, requested_nickname));
		return 1;
	}

	if (user.is_registered())
		Server::reply(user, USER_SOURCE("NICK", user) + " :" + requested_nickname);
	user.set_nickname(requested_nickname);
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
	Server::reply(user, "PONG " + Server::info().name() + " " + command.get_parameters()[0]);
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
		CORE_TRACE_IRC_ERR("A PASS message was received but the user doesn't have a nickname yet");
		return 1;
	}

	if (user.is_registered()) {
		CORE_TRACE_IRC_ERR("A PASS message was received but the user is already registered");
		Server::reply(user, ERR_ALREADYREGISTERED(user));
		return 1;
	}

	if (command.get_parameters().size() < 4) {
		CORE_TRACE_IRC_ERR("A PASS message was received but not enough parameters were given");
		Server::reply(user, ERR_NEEDMOREPARAMS(user, command));
		return 1;
	}

	if (command.get_parameters()[0].empty()) {
		CORE_TRACE_IRC_ERR("A PASS message was received but the username given is empty");
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
		Server::broadcast(user, USER_SOURCE("QUIT", user) + " :Quit: ");
	else
		Server::broadcast(user, USER_SOURCE("QUIT", user) + " :Quit: " + command.get_parameters()[0]);
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
		Server::reply_part_user_from_channels(user);
		return 0;
	}


	ParamSplitter<','> channel_splitter(command, 0);
	ParamSplitter<','> key_splitter(command.get_parameters().size() > 1, command, 1);

	// For all channels in the command
	while (!channel_splitter.reached_end()) {
		// Get the next channel name to join and the corresponding key
		std::string requested_channel_name = channel_splitter.next_param();
		std::string current_key = key_splitter.next_param();

		// Check if the requested channel name is valid
		if (!Channel::is_name_valid(requested_channel_name)) {
			Server::reply(user, ERR_NOSUCHCHANNEL(user, requested_channel_name));
			continue ;
		}

		// Find the associated Channel
		Server::ChannelIterator server_channel_it = Server::find_channel(requested_channel_name);

		// If no associated channel was found, create a new channel with its name
		if (!Server::channel_exists(requested_channel_name)) {
			Channel& new_channel = Server::create_new_channel(user, requested_channel_name);
			CORE_INFO("User %s created the channel %s", user.debug_name(), new_channel.name().c_str());
			Server::reply(user, USER_SOURCE("JOIN", user) + " " + new_channel.name());
			new_channel.send_topic_to_user_if_set(user);
			Server::reply_list_channel_members_to_user(user, new_channel);
			continue ;
		}

		Channel& channel = get_channel_reference(server_channel_it);

		// If channel is invite-only
		if (channel.is_invite_only()) {
			// If the user is not invited and is not exempted from the invite-list,
			// send error and continue to next channel
			if (!channel.is_user_invited_or_exempted(user)) {
				CORE_TRACE_IRC_ERR("User %s tried and failed to connect to channel [%s] because it was invite only.", user.debug_name(), channel.name().c_str());
				Server::reply(user, ERR_INVITEONLYCHAN(user, channel.name()));
				continue ;
			}
		}

		// If the user is banned and is not exempted from the ban-list,
		// send error and continue to next channel
		if (channel.is_user_banned(user)) {
			CORE_TRACE_IRC_ERR("User %s tried and failed to connect to channel [%s] because it was banned.", user.debug_name(), channel.name().c_str());
			Server::reply(user, ERR_BANNEDFROMCHAN(user, channel.name()));
			continue ;
		}

		// If the channel requires a key
		if (channel.is_key_protected()) {
			// If the user didn't provide a key, send an error and continue
			if (key_splitter.reached_end()) {
				CORE_TRACE_IRC_ERR("User %s failed to connect to channel %s because it didn't provide a key.", user.debug_name(), channel.name().c_str());
				Server::reply(user, ERR_BADCHANNELKEY(user, channel.name()));
				continue ;
			}

			// If the user provided the wrong key, send an error and continue
			if (current_key != channel.key()) {
				CORE_TRACE_IRC_ERR("User %s failed to connect to channel %s because it provided the wrong key. Provided [%s], needed [%s].",
					user.debug_name(), channel.name().c_str(), current_key.c_str(), channel.key().c_str());
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
			Server::reply(connected_user, USER_SOURCE("JOIN", user) + " " + channel.name());
		}

		CORE_INFO("User %s joined the channel %s", user.debug_name(), channel.name().c_str());
		user.add_channel(channel);
		channel.add_user(user);
		Server::reply(user, USER_SOURCE("JOIN", user) + " " + channel.name());
		channel.send_topic_to_user_if_set(user);
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
	if (params.size() == 2)
		reason = params[1];

	// Iterate over all the channels and disconnect the user from them
	ParamSplitter<','> channel_splitter(command, 0);
	while (!channel_splitter.reached_end())
		Server::try_reply_part_user_from_channel(user, channel_splitter.next_param(), reason);
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
	ParamSplitter<','> channel_splitter(command, 0);
	while (!channel_splitter.reached_end())
		Server::try_reply_list_channel_members_to_user(user, channel_splitter.next_param());
	return 0;
}

int topic(User& user, const Command& command)
{
	//  https://modern.ircdocs.horse/#topic-message
	//  Command: TOPIC
	//  Parameters: <channel> [<topic>]

	const std::vector<std::string>& params = command.get_parameters();

	// Check parameter count
	if (params.empty() || params[0].empty()) {
		Server::reply(user, ERR_NEEDMOREPARAMS(user, command));
		return 0;
	}

	// Retrieve the channel
	const std::string& channel_name = params[0];
	Server::ChannelIterator channel_it = Server::find_channel(channel_name);
	if (!Server::channel_exists(channel_it)) {
		Server::reply(user, ERR_NOSUCHCHANNEL(user, channel_name));
		return 0;
	}

	Channel& channel = get_channel_reference(channel_it);

	// If a topic was supplied, try to update the channel's topic
	if (params.size() > 1) {
		// Check if the user can modify the topic
		if (channel.is_topic_protected()
			&& !channel.is_user_operator(user) && !channel.is_user_halfop(user)) {
			Server::reply(user, ERR_CHANOPRIVSNEEDED(user, channel));
			return 0;
		}

		// Update the channel's topic
		const std::string& new_topic = params[1];
		channel.set_topic(new_topic, user);

		// Notify everyone on the channel of the new topic
		channel.broadcast_topic();
		return 0;
	}

	// Notify the user of the channel's topic
	channel.send_topic_to_user(user);

	return 0;
}

int list(User& user, const Command& command)
{
	//  https://modern.ircdocs.horse/#list-message
	//  Command: LIST
	//  Parameters: [<channel>{,<channel>}] [<elistcond>{,<elistcond>}]

	const std::vector<std::string>& params = command.get_parameters();

	// No parameters supplied, list all channels that are not secret to the user
	if (params.empty()) {
		Server::reply_channel_list_to_user(user);
		return 0;
	}

	ParamSplitter<','> splitter(command, 0);
	ConditionalChannelList channel_list;

	// If the first subparameter of the first parameter is a channel name,
	//  treat this parameter list as a list of channel names
	if (Channel::is_name_valid(splitter.peek_next_param())) {
		while (!splitter.reached_end()) {
			Server::ChannelIterator channel_it = Server::find_channel(splitter.next_param());
			if (Server::channel_exists(channel_it))
				channel_list.add_channel(channel_it->second);
		}

		// Set up the parser for condition parsing
		splitter = ParamSplitter<','>(params.size() > 1, command, 1);
	} else {
		// No channel list was supplied, get all the channels from the server
		channel_list.load_from_server();
	}

	// Else, treat this parameter list as a list of conditions
	while (!splitter.reached_end()) {
		std::string condition = splitter.next_param();

		if (condition.empty())
			continue;

		if (condition[0] == '>' && is_number(&condition[1]))
			channel_list.keep_if_user_count_greater_than(static_cast<std::size_t>(atoi(&condition[1])));
		else if (condition[0] == '<' && is_number(&condition[1]))
			channel_list.keep_if_user_count_less_than(static_cast<std::size_t>(atoi(&condition[1])));
		else if (condition.size() > 2 && is_number(&condition[2])) {
			char condition_type = condition[0];
			char condition_comparison = condition[1];
			std::time_t number = static_cast<std::time_t>(atoi(&condition[2]));

			if (condition_type == 'C' && condition_comparison == '>')
				channel_list.keep_if_created_greater_than(number);
			else if (condition_type == 'C' && condition_comparison == '<')
				channel_list.keep_if_created_less_than(number);
			else if (condition_type == 'T' && condition_comparison == '>')
				channel_list.keep_if_topic_changed_greater_than(number);
			else if (condition_type == 'T' && condition_comparison == '<')
				channel_list.keep_if_topic_changed_less_than(number);
			else
				return 0;
		} else
			return 0;
	}

	Server::reply(user, RPL_LISTSTART(user));
	const std::vector<Channel*> selected_channels = channel_list.channels();
	for (std::size_t i = 0; i < selected_channels.size(); i++)
		Server::reply(user, RPL_LIST(user, *selected_channels[i]));
	Server::reply(user, RPL_LISTEND(user));
	return 0;
}

int privmsg(User& user, const Command& command)
{
	// https://modern.ircdocs.horse/#privmsg-message
	// Command: PRIVMSG
	// Parameters: <target>{,<target>} <text to be sent>

	const std::vector<std::string>& params = command.get_parameters();

	if (params.size() < 2 || params[0].empty())
	{
		CORE_TRACE_IRC_ERR("User %s sent a PRIVMSG command with less than 2 parameters.", user.debug_name());
		Server::reply(user, ERR_NEEDMOREPARAMS(user, command));
		return 0;
	}

	const std::string& message = params[1];
	if (message.empty()) {
		Server::reply(user, ERR_NOTEXTTOSEND(user));
		return 0;
	}

	ParamSplitter<','> target_splitter(command, 0);
	while (!target_splitter.reached_end()) {
		std::string target = target_splitter.next_param();

		if (Channel::is_name_valid(target)) {
			Server::ChannelIterator target_channel_it = Server::find_channel(target);
			if (!Server::channel_exists(target_channel_it)) {
				CORE_TRACE_IRC_ERR("User %s tried to send a message to a non-existing channel [%s].", user.debug_name(),
					target.c_str());
				Server::reply(user, ERR_NOSUCHCHANNEL(user, target));
				continue ;
			}

			Channel& channel = get_channel_reference(target_channel_it);
			if (!channel.is_user_allowed_to_send_messages(user)) {
				Server::reply(user, ERR_CANNOTSENDTOCHAN(user, channel));
				continue;
			}

			Server::broadcast_to_channel(user, channel, USER_SOURCE("PRIVMSG", user) + " " + channel.name() + " :" + message);
		}
		else {
			Server::UserIterator target_user_it = Server::find_user(target);
			if (!Server::user_exists(target_user_it)) {
				CORE_TRACE_IRC_ERR("User %s tried to send a message to a non-existing user [%s].", user.debug_name(),
					target.c_str());
				Server::reply(user, ERR_NOSUCHNICK(user, target));
				return 1;
			}

			User &target_user = get_user_reference(target_user_it);
			if (target_user.is_away())
				Server::reply(user, RPL_AWAY(user, target_user));

			Server::reply(target_user, USER_SOURCE("PRIVMSG", user) + " " + target_user.nickname() + " :" + message);
		}
	}
	user.take_idle_timestamp();
	return 0;
}

int kick(User& user, const Command& command)
{
	//  https://modern.ircdocs.horse/#kick-message
	//  Command: KICK
	//  Parameters: <channel> <user> *( "," <user> ) [<comment>]

	const std::vector<std::string>& params = command.get_parameters();

	if (params.size() < 2) {
		Server::reply(user, ERR_NEEDMOREPARAMS(user, command));
		return 0;
	}

	// Get the channel
	Server::ChannelIterator channel_it = Server::find_channel(params[0]);
	if (!Server::channel_exists(channel_it)) {
		Server::reply(user, ERR_NOSUCHCHANNEL(user, params[0]));
		return 0;
	}
	Channel& channel = get_channel_reference(channel_it);

	// Check if the user is on the channel
	if (!channel.has_user(user)) {
		Server::reply(user, ERR_NOTONCHANNEL(user, channel));
		return 0;
	}

	// Check if the user is a moderator
	if (!channel.is_user_operator(user)) {
		Server::reply(user, ERR_CHANOPRIVSNEEDED(user, channel));
		return 0;
	}

	// Get the kick reason
	std::string kick_reason = "The user didn't give a reason";
	if (params.size() == 3)
		kick_reason = params[2];

	// Get the users
	ParamSplitter<','> splitter(command, 1);
	while (!splitter.reached_end()) {
		std::string user_to_kick_nickname = splitter.next_param();
		Channel::UserIterator user_it = channel.find_user(user_to_kick_nickname);
		if (!channel.has_user(user_it)) {
			Server::reply(user, ERR_USERNOTINCHANNEL(user, user_to_kick_nickname, channel));
			continue ;
		}
		User &user_to_kick = get_user_reference(user_it);
		Server::broadcast_to_channel(channel,
			USER_SOURCE("KICK", user) + " " + channel.name() + " " + user_to_kick.nickname() + " :" + kick_reason);
		channel.remove_user(user_to_kick);
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

	std::stringstream motd(Server::info().motd());
	std::string line;

	Server::reply(user, RPL_MOTDSTART(user));
	while (std::getline(motd, line)) {
		CORE_DEBUG("MOTD line: %s", line.c_str());
		Server::reply(user, RPL_MOTD(user, line));
	}
	Server::reply(user, RPL_ENDOFMOTD(user));

	return 0;
}

int version(User& user, const Command& command)
{
	// https://modern.ircdocs.horse/#version-message
	// Command: VERSION
	// Parameters: [<target>]

	if (command.get_parameters().size() == 1) {
		CORE_TRACE_IRC_ERR("User %s sent a VERSION to a non-existing server", user.debug_name());
		Server::reply(user, ERR_NOSUCHSERVER(user, command.get_parameters()[0]));
		return 1;
	}

	Server::reply(user, RPL_VERSION(user, ""));
	Server::reply(user, RPL_ISUPPORT(user));
	return 0;
}

int admin(User& user, const Command& command)
{
	// https://modern.ircdocs.horse/#admin-message
	// Command: ADMIN
	// Parameters: [<target>]

	if (command.get_parameters().size() == 1 && !Server::user_exists(command.get_parameters()[0])) {
		CORE_TRACE_IRC_ERR("User %s sent an ADMIN to a non-existing server", user.debug_name());
		Server::reply(user, ERR_NOSUCHSERVER(user, command.get_parameters()[0]));
		return 1;
	}

	Server::reply(user, RPL_ADMINME(user, Server::info().name()));
	Server::reply(user, RPL_ADMINLOC1(user, Server::info().server_location()));
	Server::reply(user, RPL_ADMINLOC2(user, Server::info().hosting_location()));
	Server::reply(user, RPL_ADMINEMAIL(user, "Serveur admin - " + Server::info().admin_name()));
	Server::reply(user, RPL_ADMINEMAIL(user, "<" + Server::info().email()) + ">");
	return 0;
}

int invite(User& user, const Command& command)
{
	// https://modern.ircdocs.horse/#invite-message
	// Command: INVITE
	// Parameters: <nickname> <channel>

	if (command.get_parameters().empty()) {
		Server::reply_list_of_channel_invite_to_user(user);
		return 0;
	}

	if (command.get_parameters().size() < 2) {
		CORE_TRACE_IRC_ERR("User %s sent an INVITE command with less than 2 parameters.", user.debug_name());
		Server::reply(user, ERR_NEEDMOREPARAMS(user, command));
		return 1;
	}

	Server::UserIterator target_user_it = Server::find_user(command.get_parameters()[0]);
	if (!Server::user_exists(target_user_it)) {
		CORE_TRACE_IRC_ERR("User %s sent an INVITE command to a non-existing user [%s].", user.debug_name(), command.get_parameters()[0].c_str());
		Server::reply(user, ERR_NOSUCHNICK(user, command.get_parameters()[0]));
		return 1;
	}

	Server::ChannelIterator channel_it = Server::find_channel(command.get_parameters()[1]);
	if (!Server::channel_exists(channel_it)) {
		CORE_TRACE_IRC_ERR("User %s sent an INVITE command to a non-existing channel [%s].", user.debug_name(), command.get_parameters()[1].c_str());
		Server::reply(user, ERR_NOSUCHCHANNEL(user, command.get_parameters()[1]));
		return 1;
	}

	Channel& channel = get_channel_reference(channel_it);
	if (!channel.has_user(user)) {
		CORE_TRACE_IRC_ERR("User %s sent an INVITE command to a channel [%s] he is not in.", user.debug_name(), command.get_parameters()[1].c_str());
		Server::reply(user, ERR_NOTONCHANNEL(user, channel));
		return 1;
	}

	if (!channel.is_user_operator(user)) {
		CORE_TRACE_IRC_ERR("User %s sent an INVITE command to a channel [%s] he is not an operator in.", user.debug_name(), command.get_parameters()[1].c_str());
		Server::reply(user, ERR_CHANOPRIVSNEEDED(user, channel));
		return 1;
	}

	if (channel.has_user(command.get_parameters()[0])) {
		CORE_TRACE_IRC_ERR("User %s sent an INVITE command to [%s] who are already in the channel.", user.debug_name(), command.get_parameters()[1].c_str());
		Server::reply(user, ERR_USERONCHANNEL(user, command.get_parameters()[0], channel));
		return 1;
	}

	if (!channel.is_user_invited_or_exempted(command.get_parameters()[0]))
		channel.add_to_invitelist(command.get_parameters()[0]);

	User& target_user = get_user_reference(target_user_it);

	Server::reply(user, RPL_INVITING(user, target_user.nickname(), channel.name()));
	Server::reply(get_user_reference(target_user_it), USER_SOURCE("INVITE", user) + " " + target_user.nickname() + " :" + channel.name());
	return 0;
}

int notice(User& user, const Command& command)
{
	//  https://modern.ircdocs.horse/#notice-message
	//  Command: NOTICE
	//  Parameters: <target>{,<target>} <text to be sent>

	const std::vector<std::string>& params = command.get_parameters();

	// NOTICE should not return errors
	if (params.size() < 2 || params[0].empty())
		return 0;

	ParamSplitter<','> target_splitter(command, 0);
	const std::string& message = params[1];

	while (!target_splitter.reached_end()) {
		std::string target = target_splitter.next_param();

		// Check if the target is a channel
		if (Channel::is_name_valid(target)) {
			Server::ChannelIterator channel_it = Server::find_channel(target);
			if (!Server::channel_exists(channel_it))
				continue ;

			Channel& channel = get_channel_reference(channel_it);
			if (!channel.is_user_allowed_to_send_messages(user))
				continue ;

			Server::broadcast_to_channel(user, channel, USER_SOURCE("NOTICE", user) + " " + channel.name() + " :" + message);
		} else {
			Server::UserIterator user_it = Server::find_user(target);
			if (!Server::user_exists(user_it))
				continue ;

			User& target_user = get_user_reference(user_it);

			if (!target_user.can_get_notice())
				continue ;

			Server::reply(target_user, USER_SOURCE("NOTICE", user) + " " + target_user.nickname() + " :" + message);
		}
	}
	user.take_idle_timestamp();
	return 0;
}

int time_cmd(User& user, const Command& command)
{
	// https://modern.ircdocs.horse/#time-message
	// Command: TIME
	// Parameters: [<server>]

	if (command.get_parameters().size() > 0 && command.get_parameters()[0] != Server::info().name()) {
		CORE_TRACE_IRC_ERR("User %s sent a TIME command to a non-existing server [%s].", user.debug_name(), command.get_parameters()[0].c_str());
		Server::reply(user, ERR_NOSUCHSERVER(user, command.get_parameters()[0]));
		return 1;
	}

	Server::reply(user, RPL_TIME(user, Server::info().name(), format_date()));
	return 0;
}

int away(User& user, const Command& command)
{
	// https://modern.ircdocs.horse/#away-message
	// Command: AWAY
	// Parameters: [<away message>]

	// TODO check away message length
//	if (command.get_parameters()[0].size() > Server::away_message_max_length()) {
//		Server::reply(user, ERR_INPUTTOOLONG(user));
//		return 1;
//	}

	if (command.get_parameters().empty() && user.is_away()) {
		user.set_away_msg("");
		user.set_is_away(false);
		Server::reply(user, RPL_UNAWAY(user));
		return 0;
	}

	if (!command.get_parameters().empty() && !user.is_away()) {
		user.set_away_msg(command.get_parameters()[0]);
		user.set_is_away(true);
		Server::reply(user, RPL_NOWAWAY(user));
	}
	return 0;
}

int info_cmd(User& user, const Command& command)
{
	// https://modern.ircdocs.horse/#info-message
	// Command: INFO
	// Parameters: [<server>]

	if (command.get_parameters().size() > 0 && command.get_parameters()[0] != Server::info().name()) {
		CORE_TRACE_IRC_ERR("User %s sent an INFO command to a non-existing server [%s].", user.debug_name(), command.get_parameters()[0].c_str());
		Server::reply(user, ERR_NOSUCHSERVER(user, command.get_parameters()[0]));
		return 1;
	}

	Server::reply(user, RPL_INFO(user, "ft_irc --"));
	Server::reply(user, RPL_INFO(user, "Copyright(c) 2023 ft_irc Development Team"));
	Server::reply(user, RPL_INFO(user, ""));
	Server::reply(user, RPL_INFO(user, "Licence: ??"));
	Server::reply(user, RPL_INFO(user, ""));
	Server::reply(user, RPL_INFO(user, "This server:" + Server::info().name() + " is running on version " + Server::info().version() + "."));
	Server::reply(user, RPL_INFO(user, ""));
	Server::reply(user, RPL_INFO(user, "This software was created by:"));
	Server::reply(user, RPL_INFO(user, "Natahan Jennes (njennes): njennes@student.42lyon.fr"));
	Server::reply(user, RPL_INFO(user, "Cyril Battistolo (cybattis): cybattis@student.42lyon.fr"));
	Server::reply(user, RPL_INFO(user, ""));
	Server::reply(user, RPL_INFO(user, "Birth Date: ??."));
	Server::reply(user, RPL_INFO(user, "On-line since " + Server::info().creation_date() + "."));
	Server::reply(user, RPL_ENDOFINFO(user));
	return 0;
}

int lusers(User& user, const Command& command)
{
	// https://modern.ircdocs.horse/#luser-message
	// Command: LUSERS
	// Parameters: none

	(void)command;

	size_t invisible_users = 0;
	size_t operator_users = 0;

	Server::UserIterator it = Server::users().begin();
	for (; it != Server::users().end(); it++) {
		User& user_ref = get_user_reference(it);
		if (user_ref.is_invisible())
			invisible_users++;
		if (user_ref.is_operator())
			operator_users++;
	}

	std::string current_users = std::to_string(Server::users().size());
	std::string max_users = std::to_string(Server::info().max_users());
	std::string operator_users_str = std::to_string(operator_users);
	std::string invisible_users_str = std::to_string(invisible_users);
	std::string nbr_channels = std::to_string(Server::channels().size());
	std::string unknown_connections = std::to_string(Server::unknown_connections());

	Server::reply(user, RPL_LUSERCLIENT(user, current_users, invisible_users_str));
	Server::reply(user, RPL_LUSEROP(user, operator_users_str));
	Server::reply(user, RPL_LUSERUNKNOWN(user, unknown_connections));
	Server::reply(user, RPL_LUSERCHANNELS(user, nbr_channels));
	Server::reply(user, RPL_LUSERME(user, current_users));
	Server::reply(user, RPL_LOCALUSERS(user, current_users, max_users));
	Server::reply(user, RPL_GLOBALUSERS(user, current_users, max_users));
	return 0;

}
