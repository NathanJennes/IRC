//
// Created by Cyril Battistolo on 10/03/2023.
//

#include "UserQueries.h"
#include "Numerics.h"
#include "log.h"
#include "ParamSplitter.h"

std::vector<Mask> parse_masks(const std::string& mask)
{
	std::vector<Mask> masks;
	Mask sub_mask;

	sub_mask.after = false;
	sub_mask.before = false;

	for (size_t i = 0; i < mask.size(); ++i) {
		if (mask[i] != '*')
			sub_mask.str += mask[i];
		else if (mask[i] == '*' && sub_mask.str.empty())
			sub_mask.before = true;
		else {
			sub_mask.after = true;
			masks.push_back(sub_mask);
			CORE_DEBUG("sub_mask: %s -> before: %d | after: %d", sub_mask.str.c_str(), sub_mask.before, sub_mask.after);
			sub_mask.str.clear();
			sub_mask.before = false;
			sub_mask.after = false;
		}
	}
	if (!sub_mask.str.empty())
		masks.push_back(sub_mask);
	CORE_DEBUG("masks size %d", masks.size());
	return masks;
}

int who(User& user, const Command& command)
{
	// https://modern.ircdocs.horse/#who-message
	// Command: WHO
	// Parameters: <mask>

	if (command.get_parameters().empty()) {
		Server::reply(user, ERR_NEEDMOREPARAMS(user, command));
		return 0;
	}

	std::string mask = command.get_parameters()[0];

	// handle channel name
	if (Channel::is_name_valid(mask))
	{
		Server::ChannelIterator chan_it = Server::find_channel(mask);
		if (!Server::channel_exists(chan_it)) {
			CORE_TRACE_IRC_ERR("User %s sent a WHO command to a non-existing channel [%s].", user.debug_name(), mask.c_str());
			Server::reply(user, ERR_NOSUCHCHANNEL(user, mask));
			return 1;
		}

		Channel& channel = get_channel_reference(chan_it);
		Channel::UserIterator user_it = channel.users().begin();
		for (; user_it != channel.users().end(); ++user_it) {
			const User& uref = get_user_reference(user_it);
			std::string flags = uref.get_user_flags();
			flags += channel.get_user_prefix(uref);
			Server::reply(user, RPL_WHOREPLY(user, uref, channel.name(), flags));
		}
		Server::reply(user, RPL_ENDOFWHO(user, mask));
		return 0;
	}

	// handle mask
	if (mask.find('*') != std::string::npos)
	{
		CORE_DEBUG("mask: %s", mask.c_str());
		if (mask.size() == 1)
			return 1;

		std::vector<Mask> masks = parse_masks(mask);
		Server::UserIterator user_it = Server::users().begin();
		for (; user_it != Server::users().end(); ++user_it)
		{
			User& target_user = get_user_reference(user_it);
			if (!target_user.has_mask(masks))
				continue ;
			if (target_user.is_visible_to_user(user)) {
				std::string flags = target_user.get_user_flags();
				Server::reply(user, RPL_WHOREPLY(user, get_user_reference(user_it), "*", flags));
			}
		}
		Server::reply(user, RPL_ENDOFWHO(user, mask));
	}

	// handle exact nickname
	Server::UserIterator user_it = Server::find_user(mask);
	if (Server::user_exists(user_it))
	{
		User& target_user = get_user_reference(user_it);

		std::string flags = target_user.get_user_flags();

		// Get last channel name if there is one.
		std::string channel_name = "*";
		if (!target_user.channels().empty()) {
			channel_name = target_user.channels().back()->name();
			flags += target_user.channels().back()->get_user_prefix(target_user);
		}

		Server::reply(user, RPL_WHOREPLY(user, target_user, channel_name, flags));
		Server::reply(user, RPL_ENDOFWHO(user, mask));
	}
	return 0;
}

int whowas(User& user, const Command& command)
{
	//  https://modern.ircdocs.horse/#whowas-message
	//  Command: WHOWAS
	//  Parameters: <nick> [<count>]

	const std::vector<std::string>& params = command.get_parameters();

	if (params.empty() || params[0].empty()) {
		Server::reply(user, ERR_NEEDMOREPARAMS(user, command));
		return 0;
	}

	const std::string& nickname = params[0];
	std::size_t max_entries = Server::old_users_count();

	// If a <count> parameter is given
	if (params.size() == 2) {
		// If the <count> parameter is negative or empty, ignore it
		if (!params[1].empty() && is_number(params[1])) {
			max_entries = static_cast<size_t>(std::atol(params[1].c_str()));

			// If <count> was 0
			if (max_entries == 0) {
				Server::reply(user, RPL_ENDOFWHOWAS(user));
				return 0;
			}
		}
	}

	Server::OldUserIterator user_it = Server::find_old_user(nickname);
	if (!Server::old_user_exists(user_it)) {
		Server::reply(user, ERR_WASNOSUCHNICK(user, nickname));
		Server::reply(user, RPL_ENDOFWHOWAS(user));
		return 0;
	}

	for (std::size_t i = 0; i < max_entries; i++) {
		if (Server::old_user_exists(user_it))
			Server::reply(user, RPL_WHOWASUSER(user, *user_it));
		else
			break ;
		user_it = Server::find_old_user(nickname, user_it);
	}
	Server::reply(user, RPL_ENDOFWHOWAS(user));
	return 0;
}
