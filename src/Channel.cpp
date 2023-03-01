//
// Created by Cyril Battistolo on 17/02/2023.
//

#include <iostream>
#include <algorithm>
#include "Channel.h"

Channel::Channel(User& user, const std::string &name) :
	m_name(name),
	m_user_limit(),
	m_is_ban_protected(),
	m_has_ban_exemptions(),
	m_is_user_limited(),
	m_is_invite_only(),
	m_has_invite_exemptions(),
	m_is_key_protected(),
	m_is_moderated(),
	m_is_secret(),
	m_is_topic_protected(),
	m_no_outside_messages()
{
	if (name[0] == '#') {
		m_type = REGULAR;
		m_is_topic_protected = true; // +t
		m_no_outside_messages = true; // +n
	}
	else
		m_type = LOCAL;
}

// TODO: check if user is already in the channel
bool Channel::set_mode(const Command& command)
{
	bool value;
	std::string modes = command.get_parameters()[0]; // TODO put it in a loop

	if (modes[0] != '+')
		value = true;
	else if (modes[0] == '-')
		value = false;
	else
		return false;

	for (size_t i = 1; i < modes.size(); i++) {
		switch (modes[i]) {
			case 'b':
				m_is_ban_protected = value;
				break;
			case 'e':
				m_has_ban_exemptions = value;
				break;
			case 'l':
				m_is_user_limited = value;
				break;
			case 'i':
				m_is_invite_only = value;
				break;
			case 'I':
				m_has_invite_exemptions = value;
				break;
			case 'k':
				m_is_key_protected = value;
				break;
			case 'm':
				m_is_moderated = value;
				break;
			case 's':
				m_is_secret = value;
				break;
			case 't':
				m_is_topic_protected = value;
				break;
			case 'n':
				m_no_outside_messages = value;
				break;
			default:
				return false;
		}
	}
	return true;
}

std::string Channel::modes(User& user) const
{
	std::string modes = "+";
	std::string mode_params;

	bool hidden = is_user_in_channel(user);

	if (m_is_ban_protected)
		modes += "b";
	if (m_has_ban_exemptions)
		modes += "e";
	if (m_is_invite_only)
		modes += "i";

	if (m_is_key_protected) {
		modes += "k";
		if (!hidden)
			mode_params += "<Key>";
		else
			mode_params += " " + m_key;
	}

	if (m_is_user_limited) {
		modes += "l";
		mode_params += " " + to_string(m_user_limit);
	}

	if (m_no_outside_messages)
		modes += "n";
	if (m_is_moderated)
		modes += "m";
	if (m_is_secret)
		modes += "s";
	if (m_is_topic_protected)
		modes += "t";
	if (m_has_invite_exemptions)
		modes += "I";

	return modes + mode_params;
}

bool Channel::is_user_in_channel(User &user) const
{
	return false;
}

void Channel::add_to_banlist(const std::string &user)
{
	if (std::find(m_ban_list.begin(), m_ban_list.end(), user) == m_ban_list.end())
		m_ban_list.push_back(user);
	else
		std::cerr << "User already in ban list" << std::endl;
}

void Channel::remove_from_banlist(const std::string &user)
{
	(void)user;
}
