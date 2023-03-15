//
// Created by Cyril Battistolo on 05/03/2023.
//

#include "Server.h"
#include "Numerics.h"
#include "Mode.h"
#include <algorithm>

std::vector<ModeParam> parse_channel_modes(const Command& command)
{
	std::vector<ModeParam> modes;

	const std::string& cmd_parameter = command.get_parameters()[1];
	size_t param = 1;
	bool value = true;

	for (size_t j = 0; j < cmd_parameter.size(); ++j)
	{
		ModeParam mode_param;

		if (cmd_parameter[j] == '+') {
			value = true;
			continue;
		}
		else if (cmd_parameter[j] == '-') {
			value = false;
			continue;
		} else if (j == 0)
			value = true;

		CORE_DEBUG("Mode: %c, value: %d", cmd_parameter[j], value);

		mode_param.is_adding = value;
		mode_param.mode = cmd_parameter[j];
		if (std::find(Server::info().channel_modes_params().begin(), Server::info().channel_modes_params().end(), mode_param.mode)
			!= Server::info().channel_modes_params().end()
			&& param + 1 < command.get_parameters().size())
			mode_param.arg = command.get_parameters()[++param];
		modes.push_back(mode_param);
	}
	return modes;
}

std::vector<ModeParam> parse_user_modes(const Command& command)
{
	std::vector<ModeParam> modes;
	bool value;

	for (size_t i = 1; i < command.get_parameters().size(); ++i)
	{
		for (size_t j = 0; j < command.get_parameters()[i].size(); ++j)
		{
			ModeParam mode_param = {};

			if (command.get_parameters()[i][j] == '+') {
				value = true;
				continue;
			}
			else if (command.get_parameters()[i][j] == '-') {
				value = false;
				continue;
			}
			else if (j == 0)
				value = true;

			CORE_DEBUG("Mode: %c, value: %d", command.get_parameters()[i][j], value);

			if (command.get_parameters()[i][j] == 'o' && value)
				continue;

			mode_param.is_adding = value;
			mode_param.mode = command.get_parameters()[i][j];
			modes.push_back(mode_param);
		}
	}
	return modes;
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

	if (Channel::is_name_valid(command.get_parameters()[0]))
	{
		Server::ChannelIterator channel_it = Server::find_channel(command.get_parameters()[0]);

		// check if channel exists
		if (!Server::channel_exists(channel_it)) {
			CORE_TRACE_IRC_ERR("User %s tried to set mode on a non-existing channel [%s].", user.debug_name(), command.get_parameters()[0].c_str());
			Server::reply(user, ERR_NOSUCHCHANNEL(user, command.get_parameters()[0]));
			return 1;
		}

		Channel& channel = get_channel_reference(channel_it);

		// get channel mode
		if (command.get_parameters().size() == 1)
		{
			CORE_TRACE("User %s requested channel mode on channel [%s].", user.debug_name(), command.get_parameters()[0].c_str());
			Server::reply(user, RPL_CHANNELMODEIS(user, channel));
			Server::reply(user, RPL_CREATIONTIME(user, channel));
			return 0;
		}

		// update channel mode
		if (channel.is_user_operator(user.nickname())) {
			std::vector<ModeParam> mode_params = parse_channel_modes(command);
			channel.update_mode(user, mode_params);
			return 0;
		}
		else {
			CORE_TRACE_IRC_ERR("User %s tried to set mode on a channel [%s] without being an operator.", user.debug_name(), command.get_parameters()[0].c_str());
			Server::reply(user, ERR_CHANOPRIVSNEEDED(user, channel));
			return 1;
		}
	}
	else
	{
		if (user.nickname() != command.get_parameters()[0]) {
			CORE_TRACE_IRC_ERR("User %s tried to set mode on another user [%s].", user.debug_name(), command.get_parameters()[0].c_str());
			Server::reply(user, ERR_USERSDONTMATCH(user));
			return 1;
		}

		Server::UserIterator user_it = Server::find_user(command.get_parameters()[0]);

		// check if user exists
		if (!Server::user_exists(user_it)) {
			CORE_TRACE_IRC_ERR("User %s tried to set mode on a non-existing user [%s].", user.debug_name(), command.get_parameters()[0].c_str());
			Server::reply(user, ERR_NOSUCHNICK(user, command.get_parameters()[0]));
			return 1;
		}

		User& target_user = get_user_reference(user_it);

		// get user mode
		if (command.get_parameters().size() == 1) {
			Server::reply(user, RPL_UMODEIS(target_user));
			return 0;
		}

		// update user mode
		std::vector<ModeParam> mode_params = parse_user_modes(command);
		target_user.update_mode(mode_params);
		return 0;
	}
}
