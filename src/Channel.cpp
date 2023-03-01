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

void Channel::add_user(const User &user)
{
	add_user(user.nickname());
}

void Channel::add_user(const std::string &user_nickname)
{
	m_users.push_back(user_nickname);
}

void Channel::remove_user(const User &user)
{
	remove_user(user.nickname());
}

void Channel::remove_user(const std::string &user_nickname)
{
	UserIterator user = std::find(m_users.begin(), m_users.end(), user_nickname);
	if (user != m_users.end())
		m_users.erase(user);
	else CORE_TRACE_IRC_ERR("Failed to remove [%s] from the user list of channel [%] because it was not present.", user_nickname.c_str(), m_name.c_str());
}

bool Channel::has_user(const User &user)
{
	return has_user(user.nickname());
}

bool Channel::has_user(const std::string &user_nickname)
{
	return std::find(m_users.begin(), m_users.end(), user_nickname) != m_users.end();
}

void Channel::add_to_banlist(const User &user)
{
	add_to_banlist(user.nickname());
}

void Channel::add_to_banlist(const std::string &user_nickname)
{
	if (std::find(m_ban_list.begin(), m_ban_list.end(), user_nickname) == m_ban_list.end())
		m_ban_list.push_back(user_nickname);
	else CORE_TRACE_IRC_ERR("Failed to add [%s] to the ban list of channel [%] because it was already present.", user_nickname.c_str(), m_name.c_str());
}

void Channel::remove_from_banlist(const User &user)
{
	remove_from_banlist(user.nickname());
}

void Channel::remove_from_banlist(const std::string &user_nickname)
{
	UserIterator entry = std::find(m_ban_list.begin(), m_ban_list.end(), user_nickname);
	if (entry != m_ban_list.end())
		m_ban_list.erase(entry);
	else CORE_TRACE_IRC_ERR("Failed to remove [%s] from the ban list of channel [%] because it was not present.", user_nickname.c_str(), m_name.c_str());
}

void Channel::add_to_invitelist(const User &user)
{
	add_to_invitelist(user.nickname());
}

void Channel::add_to_invitelist(const std::string &user_nickname)
{
	if (std::find(m_invite_list.begin(), m_invite_list.end(), user_nickname) == m_invite_list.end())
		m_invite_list.push_back(user_nickname);
	else CORE_TRACE_IRC_ERR("Failed to add [%s] to the invite list of channel [%] because it was already present.", user_nickname.c_str(), m_name.c_str());
}

void Channel::remove_from_invitelist(const User &user)
{
	remove_from_invitelist(user.nickname());
}

void Channel::remove_from_invitelist(const std::string &user_nickname)
{
	UserIterator entry = std::find(m_invite_list.begin(), m_invite_list.end(), user_nickname);
	if (entry != m_invite_list.end())
		m_invite_list.erase(entry);
	else CORE_TRACE_IRC_ERR("Failed to remove [%s] from the invite list of channel [%] because it was not present.", user_nickname.c_str(), m_name.c_str());
}
