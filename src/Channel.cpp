//
// Created by Cyril Battistolo on 17/02/2023.
//

#include <algorithm>
#include <string>
#include <map>
#include "Channel.h"
#include "Utils.h"
#include "Server.h"
#include "Numerics.h"

Channel::UserPermissions::UserPermissions()
: m_is_founder(false), m_is_protected(false),
  m_is_operator(false), m_is_halfop(false), m_has_voice(false) {}

std::string Channel::UserPermissions::get_highest_prefix() const
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
	m_creation_date(time(NULL)),
	m_topic_modification_date(m_creation_date),
	m_user_limit(0),
	m_is_user_limited(false),
	m_is_invite_only(false),
	m_is_key_protected(false),
	m_is_moderated(false),
	m_is_secret(false),
	m_is_topic_protected(false),
	m_no_outside_messages(false)
{
	if (name[0] == CHANNEL_TYPE_SHARED_SYMBOL) {
		m_type = CHANNEL_TYPE_SHARED_SYMBOL;
		m_is_topic_protected = true; // +t
		m_no_outside_messages = true; // +n
	}
	else
		m_type = CHANNEL_TYPE_LOCAL_SYMBOL;

	add_user(user);
	user.add_channel(*this);
	set_user_founder(user, true);
	set_user_operator(user, true);
}

bool Channel::update_mode(User &user, const std::vector<ModeParam> &mode_params)
{
	int value;

	for (size_t i = 0; i < mode_params.size(); i++)
	{
		ModeParam mode_param = mode_params[i];
		switch (mode_param.mode) {
			case 'b':
				if (mode_param.arg.empty()) {
					CORE_TRACE("User %s requested ban list for channel %s", user.nickname().c_str(), m_name.c_str());
					Server::reply_channel_ban_list_to_user(user, *this);
					break;
				}
				if (mode_param.is_adding)
					add_to_banlist(mode_param.arg);
				else if (!mode_param.is_adding)
					remove_from_banlist(mode_param.arg);
				break;
			case 'e':
				if (mode_param.arg.empty()) {
					Server::reply_channel_ban_exempt_list_to_user(user, *this);
					break;
				}
				if (mode_param.is_adding)
					add_to_ban_exemptions(mode_param.arg);
				else if (!mode_param.is_adding)
					remove_from_ban_exemptions(mode_param.arg);
				break;
			case 'I':
				if (mode_param.arg.empty()) {
					Server::reply_channel_invite_exempt_list_to_user(user, *this);
					break;
				}
				if (mode_param.is_adding)
					add_to_invite_list_exemptions(mode_param.arg);
				else if (!mode_param.is_adding)
					remove_from_invite_list_exemptions(mode_param.arg);
				break;
			case 'i':
				m_is_invite_only = mode_param.is_adding;
				break;
			case 'k':
				if (mode_param.is_adding) {
					m_key = mode_param.arg;
					m_is_key_protected = mode_param.is_adding;
				}
				else if (!mode_param.is_adding) {
					m_is_key_protected = mode_param.is_adding;
					m_key = "";
				}
				break;
			case 'l':
				value = std::atoi(mode_param.arg.c_str());
				if (mode_param.is_adding && is_number(mode_param.arg) && value > 0) {
					m_is_user_limited = mode_param.is_adding;
					m_user_limit = static_cast<size_t>(value);
				}
				else if (!mode_param.is_adding) {
					m_is_user_limited = false;
					m_user_limit = 0;
				}
				break;
			case 'n':
				m_no_outside_messages = mode_param.is_adding;
				break;
			case 'm':
				m_is_moderated = mode_param.is_adding;
				break;
			case 's':
				m_is_secret = mode_param.is_adding;
				break;
			case 't':
				m_is_topic_protected = mode_param.is_adding;
				break;
			default:
				Server::reply(user, ERR_UNKNOWNMODE(user, mode_param.mode));
				break;
		}
	}
	return true;
}

std::string Channel::get_modes_as_str(User& user) const
{
	std::string modes = "+";
	std::string mode_params;

	bool hidden = has_user(user);

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

	return modes + mode_params;
}

void Channel::set_topic(const std::string &topic, const User& user)
{
	m_topic = topic;
	m_last_user_to_modify_topic = user.source();
	m_topic_modification_date = time(NULL);
}

void Channel::add_user(User &user)
{
	m_users[&user] = UserPermissions();
}

void Channel::remove_user(User &user)
{
	UserIterator user_it = m_users.find(&user);
	if (has_user(user_it))
		m_users.erase(user_it);
}

void Channel::remove_user(const std::string &user_nickname)
{
	UserIterator user_it = find_user(user_nickname);
	if (has_user(user_it))
		m_users.erase(user_it);
	else CORE_TRACE_IRC_ERR("Failed to remove [%s] from the user list of channel [%s] because it was not present.", user_nickname.c_str(), m_name.c_str());
}

bool Channel::has_user(User &user) const
{
	return m_users.find(&user) != m_users.end();
}

bool Channel::has_user(const std::string &user_nickname) const
{
	for (ConstUserIterator user_it = m_users.begin(); user_it != m_users.end(); user_it++) {
		if (get_user_reference(user_it).nickname() == user_nickname)
			return true;
	}
	return false;
}

bool Channel::has_user(const UserMap::iterator &user_it) const
{
	return user_it != m_users.end();
}

void Channel::set_user_founder(User& user, bool value)
{
	if (!has_user(user)) {
		CORE_WARN("Trying to modify permissions on a user not present in a channel");
		return;
	}

	m_users.at(&user).set_is_founder(value);
}

void Channel::set_user_founder(const std::string& user_nickname, bool value)
{
	if (!has_user(user_nickname))
		return ;

	UserIterator user_it = find_user(user_nickname);
	get_user_perms_reference(user_it).set_is_founder(value);
}

void Channel::set_user_protected(User& user, bool value)
{
	if (!has_user(user)) {
		CORE_WARN("Trying to modify permissions on a user not present in a channel");
		return;
	}

	m_users.at(&user).set_is_protected(value);
}

void Channel::set_user_protected(const std::string& user_nickname, bool value)
{
	if (!has_user(user_nickname))
		return ;

	UserIterator user_it = find_user(user_nickname);
	get_user_perms_reference(user_it).set_is_protected(value);
}

void Channel::set_user_operator(User& user, bool value)
{
	if (!has_user(user)) {
		CORE_WARN("Trying to modify permissions on a user not present in a channel");
		return;
	}

	m_users.at(&user).set_is_operator(value);
}

void Channel::set_user_operator(const std::string& user_nickname, bool value)
{
	if (!has_user(user_nickname))
		return ;

	UserIterator user_it = find_user(user_nickname);
	get_user_perms_reference(user_it).set_is_operator(value);
}

void Channel::set_user_halfop(User& user, bool value)
{
	if (!has_user(user)) {
		CORE_WARN("Trying to modify permissions on a user not present in a channel");
		return;
	}

	m_users.at(&user).set_is_halfop(value);
}

void Channel::set_user_halfop(const std::string& user_nickname, bool value)
{
	if (!has_user(user_nickname))
		return ;

	UserIterator user_it = find_user(user_nickname);
	get_user_perms_reference(user_it).set_is_halfop(value);
}

void Channel::set_user_voice_permission(User& user, bool value)
{
	if (!has_user(user)) {
		CORE_WARN("Trying to modify permissions on a user not present in a channel");
		return;
	}

	m_users.at(&user).set_has_voice(value);
}

void Channel::set_user_voice_permission(const std::string& user_nickname, bool value)
{
	if (!has_user(user_nickname))
		return ;

	UserIterator user_it = find_user(user_nickname);
	get_user_perms_reference(user_it).set_has_voice(value);
}

Channel::UserIterator Channel::find_user(const std::string &user_nickname)
{
	for (UserIterator user_it = m_users.begin(); user_it != m_users.end(); user_it++) {
		if (get_user_reference(user_it).nickname() == user_nickname)
			return user_it;
	}
	return m_users.end();
}

Channel::ConstUserIterator Channel::find_user(const std::string &user_nickname) const
{
	for (ConstUserIterator user_it = m_users.begin(); user_it != m_users.end(); user_it++) {
		if (get_user_reference(user_it).nickname() == user_nickname)
			return user_it;
	}
	return m_users.end();
}

bool Channel::is_user_founder(const User &user)
{
	return is_user_founder(user.nickname());
}

bool Channel::is_user_founder(const std::string &user_nickname)
{
	UserIterator it = find_user(user_nickname);
	if (it != m_users.end())
		return get_user_perms_reference(it).is_founder();
	return false;
}

bool Channel::is_user_operator(const User &user)
{
	return is_user_operator(user.nickname());
}

bool Channel::is_user_operator(const std::string &user_nickname)
{
	UserIterator it = find_user(user_nickname);
	if (it != m_users.end())
		return get_user_perms_reference(it).is_operator();
	return false;
}

bool Channel::is_user_halfop(const User &user)
{
	return is_user_halfop(user.nickname());
}

bool Channel::is_user_halfop(const std::string &user_nickname)
{
	UserIterator it = find_user(user_nickname);
	if (it != m_users.end())
		return get_user_perms_reference(it).is_halfop();
	return false;
}

bool Channel::is_user_has_voice(const User &user)
{
	return is_user_has_voice(user.nickname());
}

bool Channel::is_user_has_voice(const std::string &user_nickname)
{
	UserIterator it = find_user(user_nickname);
	if (it != m_users.end())
		return get_user_perms_reference(it).has_voice();
	return false;
}

void Channel::send_topic_to_user_if_set(User &user)
{
	if (!m_topic.empty())
		send_topic_to_user(user);
}

void Channel::send_topic_to_user(User &user)
{
	// If the topic was never set, do not send a topic
	if (m_topic_modification_date == 0)
		return ;

	if (m_topic.empty())
		Server::reply(user, RPL_NOTOPIC(m_last_user_to_modify_topic, *this));
	else {
		Server::reply(user, RPL_TOPIC(m_last_user_to_modify_topic, *this));
		Server::reply(user, RPL_TOPICWHOTIME(user, *this));
	}
}

void Channel::broadcast_topic()
{
	for (UserIterator user_it = m_users.begin(); user_it != m_users.end(); user_it++)
		send_topic_to_user(get_user_reference(user_it));
}

void Channel::broadcast_topic(User &user_to_avoid)
{
	for (UserIterator user_it = m_users.begin(); user_it != m_users.end(); user_it++) {
		User& user = get_user_reference(user_it);
		if (user != user_to_avoid)
			send_topic_to_user(user);
	}
}

bool Channel::is_user_invited_or_exempted(User &user)
{
	return is_user_invited_or_exempted(user.nickname());
}

bool Channel::is_user_invited_or_exempted(const std::string &user_nickname)
{
	if (is_user_in_invite_list(user_nickname))
		return true;

	if (is_user_in_invite_list_exemptions(user_nickname))
		return true;

	return false;
}

bool Channel::is_user_banned(User &user)
{
	return is_user_banned(user.nickname());
}

bool Channel::is_user_banned(const std::string &user_nickname)
{
	if (is_user_in_ban_exemptions(user_nickname))
		return false;

	if (is_user_in_banlist(user_nickname))
		return true;

	return false;
}

bool Channel::is_user_in_invite_list(User &user)
{
	return is_user_in_invite_list(user.nickname());
}

bool Channel::is_user_in_invite_list(const std::string &user_nickname)
{
	for (ConstNicknameIterator nickname_it = m_invite_list.begin(); nickname_it != m_invite_list.end(); nickname_it++) {
		if (user_nickname == *nickname_it)
			return true;
	}
	return false;
}

bool Channel::is_user_in_invite_list_exemptions(User &user)
{
	return is_user_in_invite_list_exemptions(user.nickname());
}

bool Channel::is_user_in_invite_list_exemptions(const std::string &user_nickname)
{
	for (ConstNicknameIterator nickname_it = m_invite_exemptions.begin(); nickname_it != m_invite_exemptions.end(); nickname_it++) {
		if (user_nickname == *nickname_it)
			return true;
	}
	return false;
}

bool Channel::is_user_in_banlist(User &user)
{
	return is_user_in_banlist(user.nickname());
}

bool Channel::is_user_in_banlist(const std::string &user_nickname)
{
	for (ConstNicknameIterator nickname_it = m_ban_list.begin(); nickname_it != m_ban_list.end(); nickname_it++) {
		if (user_nickname == *nickname_it)
			return true;
	}
	return false;
}

bool Channel::is_user_in_ban_exemptions(User &user)
{
	return is_user_in_ban_exemptions(user.nickname());
}

bool Channel::is_user_in_ban_exemptions(const std::string &user_nickname)
{
	for (ConstNicknameIterator nickname_it = m_ban_exemptions.begin(); nickname_it != m_ban_exemptions.end(); nickname_it++) {
		if (user_nickname == *nickname_it)
			return true;
	}
	return false;
}

void Channel::add_to_banlist(const User &user)
{
	add_to_banlist(user.nickname());
}

void Channel::add_to_banlist(const std::string &user_nickname)
{
	if (std::find(m_ban_list.begin(), m_ban_list.end(), user_nickname) == m_ban_list.end())
		m_ban_list.push_back(user_nickname);
	else CORE_TRACE_IRC_ERR("Failed to add [%s] to the ban list of channel [%s] because it was already present.", user_nickname.c_str(), m_name.c_str());
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
	else CORE_TRACE_IRC_ERR("Failed to remove [%s] from the ban list of channel [%s] because it was not present.", user_nickname.c_str(), m_name.c_str());
}

void Channel::add_to_ban_exemptions(const User &user)
{
	add_to_ban_exemptions(user.nickname());
}

void Channel::add_to_ban_exemptions(const std::string &user_nickname)
{
	if (std::find(m_ban_exemptions.begin(), m_ban_exemptions.end(), user_nickname) == m_ban_exemptions.end())
		m_ban_exemptions.push_back(user_nickname);
	else CORE_TRACE_IRC_ERR("Failed to add [%s] to the ban exemption list of channel [%s] because it was already present.", user_nickname.c_str(), m_name.c_str());
}

void Channel::remove_from_ban_exemptions(const User &user)
{
	remove_from_ban_exemptions(user.nickname());
}

void Channel::remove_from_ban_exemptions(const std::string &user_nickname)
{
	NicknameIterator entry = std::find(m_ban_exemptions.begin(), m_ban_exemptions.end(), user_nickname);
	if (entry != m_ban_exemptions.end())
		m_ban_exemptions.erase(entry);
	else CORE_TRACE_IRC_ERR("Failed to remove [%s] from the ban exemption list of channel [%s] because it was not present.", user_nickname.c_str(), m_name.c_str());
}

void Channel::add_to_invitelist(const User &user)
{
	add_to_invitelist(user.nickname());
}

void Channel::add_to_invitelist(const std::string &user_nickname)
{
	if (std::find(m_invite_list.begin(), m_invite_list.end(), user_nickname) == m_invite_list.end())
		m_invite_list.push_back(user_nickname);
	else CORE_TRACE_IRC_ERR("Failed to add [%s] to the invite list of channel [%s] because it was already present.", user_nickname.c_str(), m_name.c_str());
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
	else CORE_TRACE_IRC_ERR("Failed to remove [%s] from the invite list of channel [%s] because it was not present.", user_nickname.c_str(), m_name.c_str());
}

void Channel::add_to_invite_list_exemptions(const User &user)
{
	add_to_invite_list_exemptions(user.nickname());
}

void Channel::add_to_invite_list_exemptions(const std::string &user_nickname)
{
	if (std::find(m_invite_exemptions.begin(), m_invite_exemptions.end(), user_nickname) == m_invite_exemptions.end())
		m_invite_exemptions.push_back(user_nickname);
	else CORE_TRACE_IRC_ERR("Failed to add [%s] to the invite list of channel [%s] because it was already present.", user_nickname.c_str(), m_name.c_str());
}

void Channel::remove_from_invite_list_exemptions(const User &user)
{
	remove_from_invite_list_exemptions(user.nickname());
}

void Channel::remove_from_invite_list_exemptions(const std::string &user_nickname)
{
	NicknameIterator entry = std::find(m_invite_exemptions.begin(), m_invite_exemptions.end(), user_nickname);
	if (entry != m_invite_exemptions.end())
		m_invite_exemptions.erase(entry);
	else CORE_TRACE_IRC_ERR("Failed to remove [%s] from the invite list of channel [%s] because it was not present.", user_nickname.c_str(), m_name.c_str());
}

bool Channel::is_name_valid(const std::string &channel_name)
{
	if (channel_name.empty())
		return false;

	if (!is_channel_type_char(channel_name[0]))
		return false;

	const char invalid_characters[] = {0x20, 0x07, 0x2C, 0x00};
	std::string::size_type invalid_character_pos = channel_name.find_first_of(invalid_characters);
	if (invalid_character_pos != std::string::npos)
		return false;

	return true;
}

bool Channel::is_channel_type_char(char c)
{
	return c == CHANNEL_TYPE_SHARED_SYMBOL || c == CHANNEL_TYPE_LOCAL_SYMBOL;
}
