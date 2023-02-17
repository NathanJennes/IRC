//
// Created by Cyril Battistolo on 17/02/2023.
//

#ifndef CHANNEL_H
#define CHANNEL_H

#define REGULAR true
#define LOCAL false

#include <string>
#include <vector>

class Channel
{
public:
	explicit Channel(const std::string& name);

	//setters
	void	set_topic(const std::string& topic)	{ m_topic = topic; }
	void	set_key(const std::string& key)		{ m_key = key; }
	void	set_user_limit(int limit) 			{ m_user_limit = limit; }

	void	add_to_banlist(const std::string& user);
	void	remove_from_banlist(const std::string& user);

	bool	update_modes(const std::string& modes);

private:
	std::string					m_name;
	bool 						m_type;

	std::string 				m_topic;
	std::vector<std::string>	m_ban_list;
	std::vector<std::string>	m_invite_list;
	std::string 				m_key;

	int		m_user_limit;
	int		m_user_count;

	// modes
	bool	m_is_ban_protected;		// +b
	bool	m_is_ban_exceptions;	// +e
	bool	m_is_user_limited;		// +l
	bool	m_is_invite_only;		// +i
	bool	m_is_invite_exempt;		// +I
	bool	m_is_key_protected;		// +k
	bool	m_is_moderated;			// +m
	bool	m_is_secret;			// +s
	bool	m_is_topic_protected;	// +t
	bool	m_no_outside_messages;	// +n
};


#endif //CHANNEL_H
