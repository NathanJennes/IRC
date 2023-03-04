//
// Created by Cyril Battistolo on 17/02/2023.
//

#include <iostream>
#include <algorithm>
#include "Channel.h"

Channel::UserEntry::UserEntry(const std::string& nickname)
: m_nickname(nickname), m_is_founder(false), m_is_protected(false),
  m_is_operator(false), m_is_halfop(false), m_has_voice(false) {}

bool Channel::UserEntry::operator==(const std::string &nickname) const
{
	return nickname == m_nickname;
}

bool Channel::UserEntry::operator!=(const std::string &nickname) const
{
	return nickname != m_nickname;
}

std::string Channel::UserEntry::get_highest_prefix() const
{
//	if (m_is_founder)
//		return CHANNEL_USER_PREFIX_FOUNDER;
	if (m_is_protected)
		return CHANNEL_USER_PREFIX_PROTECTED;
	else if (m_is_operator)
		return CHANNEL_USER_PREFIX_OPERATOR;
	else if (m_is_halfop)
		return CHANNEL_USER_PREFIX_HALFOP;
	else if (m_has_voice)
		return CHANNEL_USER_PREFIX_VOICE;
	return "";
}

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
	// TODO: have a look into this
	if (name[0] == CHANNEL_TYPE_SHARED_SYMBOL) {
		m_type = CHANNEL_TYPE_SHARED_SYMBOL;
		m_is_topic_protected = true; // +t
		m_no_outside_messages = true; // +n
	}
	else
		m_type = CHANNEL_TYPE_LOCAL_SYMBOL;

	add_user(user);
	set_user_founder(user, true);
	set_user_operator(user, true);
}

bool Channel::update_mode(const Command& command)
{
	bool value;

	// TODO: parse the mode string

	for (size_t i = 0; i < command.get_parameters().size(); i++)
	{
		std::string modes = command.get_parameters()[i];
		CORE_DEBUG("Parameter %d: %s", i, modes.c_str());

		if (modes[0] != '+')
			value = true;
		else if (modes[0] == '-')
			value = false;
		else
			return false;

		size_t j = 0;
		if (modes.size() == 1 && command.get_parameters().size() > i + 1)
			modes = command.get_parameters()[++i];
		else
			j = 1;

		for (; j < modes.size(); j++)
		{
			switch (modes[j])
			{
				case 'b':
					m_is_ban_protected = value;
					break;
				case 'e':
					m_has_ban_exemptions = value;
					break;
				case 'l':
					// TODO: check argument
					m_is_user_limited = value;
					break;
				case 'i':
					m_is_invite_only = value;
					break;
				case 'I':
					m_has_invite_exemptions = value;
					break;
				case 'k':
//					m_key = command.get_parameters()[];
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
	}
	return true;
}

std::string Channel::get_modes_as_str(User& user) const
{
	std::string modes = "+";
	std::string mode_params;

	bool hidden = has_user(user);

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

void Channel::add_user(const User &user)
{
	m_users.push_back(UserEntry(user.nickname()));
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

bool Channel::has_user(const User &user) const
{
	return has_user(user.nickname());
}

bool Channel::has_user(const std::string &user_nickname) const
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
	NicknameIterator entry = std::find(m_ban_list.begin(), m_ban_list.end(), user_nickname);
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
	NicknameIterator entry = std::find(m_invite_list.begin(), m_invite_list.end(), user_nickname);
	if (entry != m_invite_list.end())
		m_invite_list.erase(entry);
	else CORE_TRACE_IRC_ERR("Failed to remove [%s] from the invite list of channel [%] because it was not present.", user_nickname.c_str(), m_name.c_str());
}

void Channel::set_user_founder(const User& user, bool value)
{
	set_user_founder(user.nickname(), value);
}

void Channel::set_user_founder(const std::string& user_nickname, bool value)
{
	if (!has_user(user_nickname))
		return ;

	UserIterator user = find_user(user_nickname);
	user->set_is_founder(value);
}

void Channel::set_user_protected(const User& user, bool value)
{
	set_user_protected(user.nickname(), value);
}

void Channel::set_user_protected(const std::string& user_nickname, bool value)
{
	if (!has_user(user_nickname))
		return ;

	UserIterator user = find_user(user_nickname);
	user->set_is_protected(value);
}

void Channel::set_user_operator(const User& user, bool value)
{
	set_user_operator(user.nickname(), value);

}

void Channel::set_user_operator(const std::string& user_nickname, bool value)
{
	if (!has_user(user_nickname))
		return ;

	UserIterator user = find_user(user_nickname);
	user->set_is_operator(value);
}

void Channel::set_user_halfop(const User& user, bool value)
{
	set_user_halfop(user.nickname(), value);
}

void Channel::set_user_halfop(const std::string& user_nickname, bool value)
{
	if (!has_user(user_nickname))
		return ;

	UserIterator user = find_user(user_nickname);
	user->set_is_halfop(value);
}

void Channel::set_user_voice_permission(const User& user, bool value)
{
	set_user_voice_permission(user.nickname(), value);
}

void Channel::set_user_voice_permission(const std::string& user_nickname, bool value)
{
	if (!has_user(user_nickname))
		return ;

	UserIterator user = find_user(user_nickname);
	user->set_has_voice(value);
}

Channel::UserIterator Channel::find_user(const User &user)
{
	return find_user(user.nickname());
}

Channel::UserIterator Channel::find_user(const std::string &user_nickname)
{
	return std::find(m_users.begin(), m_users.end(), user_nickname);
}
