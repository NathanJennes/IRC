//
// Created by Cyril Battistolo on 17/02/2023.
//

#ifndef CHANNEL_H
#define CHANNEL_H

#define REGULAR true
#define LOCAL false

#include <string>
#include <vector>
#include "User.h"
#include "Command.h"

class Channel
{
public:
	explicit Channel(User& user, const std::string& name);

	void	add_to_banlist(const std::string& user);
	void	remove_from_banlist(const std::string& user);

	bool	is_user_in_channel(User &user) const;

	//
	// Getters
	//
	const std::string&				name()				const { return m_name; }
	const std::string&				topic()				const { return m_topic; }
	const std::string&				key()				const { return m_key; }
	const std::vector<std::string>&	ban_list()			const { return m_ban_list; }
	const std::vector<std::string>&	invite_list()		const { return m_invite_list; }
	const std::vector<std::string>&	invite_exemptions()	const { return m_invite_exemptions; }
	const std::vector<std::string>&	ban_exemptions()	const { return m_ban_exemptions; }
	int								user_limit()		const { return m_user_limit; }
	size_t							user_count()		const { return m_users.size(); }
	char							type()				const { return m_type; }
	std::string						modes(User& user)	const;

	//
	// Flags
	//
	bool	is_ban_protected()			const { return m_is_ban_protected; }
	bool	has_ban_exemptions()		const { return m_has_ban_exemptions; }
	bool	is_user_limited()			const { return m_is_user_limited; }
	bool	is_invite_only()			const { return m_is_invite_only; }
	bool	has_invite_exemptions()		const { return m_has_invite_exemptions; }
	bool	is_key_protected()			const { return m_is_key_protected; }
	bool	is_moderated()				const { return m_is_moderated; }
	bool	is_secret()					const { return m_is_secret; }
	bool	is_topic_protected()		const { return m_is_topic_protected; }
	bool	no_outside_messages()		const { return m_no_outside_messages; }

	//
	// setters
	//
	void	set_topic(const std::string& topic)	{ m_topic = topic; }
	void	set_key(const std::string& key)		{ m_key = key; }
	void	set_user_limit(int limit) 			{ m_user_limit = limit; }
	bool	set_mode(const Command& cmd);

	friend bool operator==(const Channel& lhs, const Channel& rhs) {
		return lhs.m_name == rhs.m_name;
	}

	friend bool operator!=(const Channel& lhs, const Channel& rhs) {
		return !(lhs == rhs);
	}

private:
	std::string					m_name;
	char						m_type;

	std::string 				m_topic;
	std::vector<std::string>	m_ban_list;
	std::vector<std::string>	m_ban_exemptions;
	std::vector<std::string>	m_invite_list;
	std::vector<std::string>	m_invite_exemptions;
	std::string 				m_key;

	int		m_user_limit;

	// modes
	bool	m_is_ban_protected;			// +b
	bool	m_has_ban_exemptions;		// +e
	bool	m_is_user_limited;			// +l
	bool	m_is_invite_only;			// +i
	bool	m_has_invite_exemptions;	// +I
	bool	m_is_key_protected;			// +k
	bool	m_is_moderated;				// +m
	bool	m_is_secret;				// +s
	bool	m_is_topic_protected;		// +t
	bool	m_no_outside_messages;		// +n
};


#endif //CHANNEL_H
