//
// Created by Cyril Battistolo on 05/03/2023.
//

#include "Server.h"
#include "IRC.h"
#include "Mode.h"

static const char my_modes[] = {'b', 'e', 'I', 'k', 'l' };

std::vector<mode_param> parse_channel_modes(const Command& command)
{
	std::vector<mode_param> modes;

	std::string cmd_parameter = command.get_parameters()[1];
	size_t param = 1;
	bool value;

	for (size_t j = 0; j < cmd_parameter.size(); ++j)
	{
		mode_param mode_param;

		if (cmd_parameter[j] == '+') {
			value = true;
			continue;
		}
		else if (cmd_parameter[j] == '-') {
			value = false;
			continue;
		}

		mode_param.is_adding = value;
		mode_param.mode = cmd_parameter[j];
		if (std::find(std::begin(my_modes), std::end(my_modes), mode_param.mode) != std::end(my_modes)
			&& param + 1 < command.get_parameters().size())
			mode_param.arg = command.get_parameters()[++param];
		modes.push_back(mode_param);
	}
	return modes;
}

std::vector<mode_param> parse_user_modes(const Command& command)
{
	std::vector<mode_param> modes;

	std::string cmd_parameter = command.get_parameters()[1];
	size_t param = 1;
	bool value;

	for (size_t j = 0; j < cmd_parameter.size(); ++j)
	{
		mode_param mode_param;

		if (cmd_parameter[j] == '+') {
			value = true;
			continue;
		}
		else if (cmd_parameter[j] == '-') {
			value = false;
			continue;
		}

		mode_param.is_adding = value;
		mode_param.mode = cmd_parameter[j];
		if (std::find(std::begin(my_modes), std::end(my_modes), mode_param.mode) != std::end(my_modes)
			&& param + 1 < command.get_parameters().size())
			mode_param.arg = command.get_parameters()[++param];
		modes.push_back(mode_param);
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

	if (is_channel(command.get_parameters()[0]))
	{
		Server::ChannelIterator channel_it = Server::find_channel(command.get_parameters()[0]);

		// check if channel exists
		if (channel_it == Server::channels().end()) {
			CORE_TRACE_IRC_ERR("User %s tried to set mode on a non-existing channel [%s].", user.debug_name(), command.get_parameters()[0].c_str());
			Server::reply(user, ERR_NOSUCHCHANNEL(user, command.get_parameters()[0]));
			return 1;
		}

		// get channel mode
		if (command.get_parameters().size() == 1)
		{
			Server::reply(user, RPL_CHANNELMODEIS(user, channel_it));
			// TODO: Send RPL_CREATIONTIME (329)
			return 0;
		}

		// update channel mode
		if (channel_it->is_user_operator(user) || channel_it->is_user_halfop(user)) {
			std::vector<mode_param> mode_params = parse_channel_modes(command);
			channel_it->update_mode(user, mode_params);
			return 0;
		}
		else {
			CORE_TRACE_IRC_ERR("User %s tried to set mode on a channel [%s] without being an operator.", user.debug_name(), command.get_parameters()[0].c_str());
			Server::reply(user, ERR_CHANOPRIVSNEEDED(user, channel_it));
			return 1;
		}
	}

	// TODO: Handle user mode

	return 0;
}
