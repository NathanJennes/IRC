//
// Created by Cyril Battistolo on 17/02/2023.
//

#include <iostream>
#include <algorithm>
#include "Channel.h"

Channel::Channel(const std::string &name) : m_name(name), m_user_limit(), m_user_count(),
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
		update_modes("+nt");
	}
	else
		m_type = LOCAL;
	(void)m_user_count;
}

bool Channel::update_modes(const std::string &modes)
{
	bool value;
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
