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

class Channel
{
public:
	/// Types
	struct UserEntry
	{
		explicit UserEntry(const std::string& nickname) :
			m_nickname(nickname), m_is_founder(false), m_is_protected(false),
			m_is_operator(false), m_is_halfop(false), m_has_voice(false) {}

		bool operator==(const std::string& nickname) { return nickname == m_nickname; }

		void is_founder(bool new_value)		{ m_is_founder = new_value; };
		void is_protected(bool new_value)	{ m_is_protected = new_value; };
		void is_operator(bool new_value)	{ m_is_operator = new_value; };
		void is_halfop(bool new_value)		{ m_is_halfop = new_value; };
		void has_voice(bool new_value)		{ m_has_voice = new_value; };

		const std::string&	nickname()		const { return m_nickname; };
		bool				is_founder()	const { return m_is_founder; };
		bool				is_protected()	const { return m_is_protected; };
		bool				is_operator()	const { return m_is_operator; };
		bool				is_halfop()		const { return m_is_halfop; };
		bool				has_voice()		const { return m_has_voice; };

	private:
		std::string	m_nickname;
		bool		m_is_founder;
		bool		m_is_protected;
		bool		m_is_operator;
		bool		m_is_halfop;
		bool		m_has_voice;
	};

	explicit Channel(const std::string& name);

	/// Channel information
	void set_topic(const std::string& topic)	{ m_topic = topic; }
	void set_key(const std::string& key)		{ m_key = key; }

	/// Users
	void set_user_limit(int limit) { m_user_limit = limit; }
	void add_user(const User& user);
	void add_user(const std::string& user_nickname);
	void remove_user(const User& user);
	void remove_user(const std::string& user_nickname);
	bool has_user(const User& user);
	bool has_user(const std::string& user_nickname);

	/// Modes
	bool update_modes(const std::string& modes);

	/// Entry restrictions
	void add_to_banlist(const std::string& user_nickname);
	void remove_from_banlist(const std::string& user_nickname);
	void add_to_banlist(const User& user);
	void remove_from_banlist(const User& user);
	void add_to_invitelist(const std::string& user_nickname);
	void remove_from_invitelist(const std::string& user_nickname);
	void add_to_invitelist(const User& user);
	void remove_from_invitelist(const User& user);

	/// Getters
	const	std::string&	name()	const	{ return m_name; }
	const	std::string&	topic()	const	{ return m_topic; }
	char					type()	const	{ return m_type; }

	int								user_count()	const	{ return m_user_count; }
	int								user_limit()	const	{ return m_user_limit; }
	const	std::vector<UserEntry>&	users()			const	{ return m_users; }

	const std::string&				key()				const	{ return m_key; }
	const std::vector<std::string>&	invite_list()		const { return m_invite_list; }
	const std::vector<std::string>&	invite_exemptions()	const { return m_invite_exemptions; }
	const std::vector<std::string>&	ban_list()			const { return m_ban_list; }
	const std::vector<std::string>&	ban_exemptions()	const { return m_ban_exemptions; }

	bool	is_ban_protected()		const { return m_is_ban_protected; }
	bool	has_ban_exemptions()	const { return m_has_ban_exemptions; }
	bool	is_user_limited()		const { return m_is_user_limited; }
	bool	is_invite_only()		const { return m_is_invite_only; }
	bool	has_invite_exemptions()	const { return m_has_invite_exemptions; }
	bool	is_key_protected()		const { return m_is_key_protected; }
	bool	is_moderated()			const { return m_is_moderated; }
	bool	is_secret()				const { return m_is_secret; }
	bool	is_topic_protected()	const { return m_is_topic_protected; }
	bool	no_outside_messages()	const { return m_no_outside_messages; }

private:
	/// Typedefs
	typedef std::vector<UserEntry>::iterator	UserIterator;
	typedef std::vector<std::string>::iterator	NicknameIterator;

	/// Channel information
	std::string					m_name;
	char						m_type;
	std::string 				m_topic;

	/// Entry restrictions
	std::vector<std::string>	m_ban_list;
	std::vector<std::string>	m_ban_exemptions;
	std::vector<std::string>	m_invite_list;
	std::vector<std::string>	m_invite_exemptions;
	std::string 				m_key;

	/// Users
	int							m_user_limit;
	int							m_user_count;
	std::vector<UserEntry>	m_users;

	/// Modes
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
