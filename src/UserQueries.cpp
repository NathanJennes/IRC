//
// Created by Cyril Battistolo on 10/03/2023.
//

#include "UserQueries.h"

#include "Numerics.h"
#include "log.h"

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
			Server::reply(user, ERR_NOSUCHSERVER(user, mask));
			return 1;
		}

		Channel& channel = get_channel_reference(chan_it);
		Channel::UserIterator user_it = channel.users().begin();
		for (; user_it != channel.users().end(); ++user_it) {
			std::string flags = user_it->first->get_user_flags_and_prefix(channel.name());
			Server::reply(user, RPL_WHOREPLY(user, get_user_reference(user_it), channel.name(), flags));
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
			if (!target_user.is_invisible() || (target_user.is_invisible() && target_user.has_channel_in_common(user))) {
				std::string flags = target_user.get_user_flags_and_prefix("*");
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

		// Get last channel name if there is one.
		std::string channel_name = "*";
		if (!target_user.channels().empty())
			channel_name = target_user.channels().back()->name();

		std::string flags = target_user.get_user_flags_and_prefix(channel_name);

		Server::reply(user, RPL_WHOREPLY(user, target_user, channel_name, flags));
		Server::reply(user, RPL_ENDOFWHO(user, mask));
	}
	return 0;
}
