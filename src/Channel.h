//
// Created by Cyril Battistolo on 17/02/2023.
//

#ifndef CHANNEL_H
#define CHANNEL_H

#define CHANNEL_TYPE_SHARED_SYMBOL '#'
#define CHANNEL_TYPE_LOCAL_SYMBOL  '&'

#define CHANNEL_USER_PREFIX_FOUNDER   "~"
#define CHANNEL_USER_PREFIX_PROTECTED "&"
#define CHANNEL_USER_PREFIX_OPERATOR  "@"
#define CHANNEL_USER_PREFIX_HALFOP    "%"
#define CHANNEL_USER_PREFIX_VOICE     "+"

#include <string>
#include <vector>
#include <ctime>
#include "Command.h"
#include "Mode.h"

class User;

class Channel
{
public:
	/// Types
	struct UserPermissions
	{
		explicit UserPermissions();

		std::string get_highest_prefix() const;

		void set_is_founder(bool new_value)		{ m_is_founder = new_value; };
		void set_is_protected(bool new_value)	{ m_is_protected = new_value; };
		void set_is_operator(bool new_value)	{ m_is_operator = new_value; };
		void set_is_halfop(bool new_value)		{ m_is_halfop = new_value; };
		void set_has_voice(bool new_value)		{ m_has_voice = new_value; };

		bool	is_founder()	const			{ return m_is_founder; };
		bool	is_protected()	const			{ return m_is_protected; };
		bool	is_operator()	const			{ return m_is_operator; };
		bool	is_halfop()		const			{ return m_is_halfop; };
		bool	has_voice()		const			{ return m_has_voice; };

	private:
		bool	m_is_founder;
		bool	m_is_protected;
		bool	m_is_operator;
		bool	m_is_halfop;
		bool	m_has_voice;
	};

	explicit Channel(User& user, const std::string& name);

	/// Typedefs
private:
	typedef std::map<User*, UserPermissions>	UserMap;
	typedef std::vector<std::string>			NicknameVector;
public:
	typedef UserMap::iterator					UserIterator;
	typedef UserMap::const_iterator				ConstUserIterator;
	typedef NicknameVector::iterator			NicknameIterator;
	typedef NicknameVector::const_iterator		ConstNicknameIterator;

	/// Channel related checks
	static bool is_name_valid(const std::string& channel_name);
	static bool is_channel_type_char(char c);

	/// Channel information
	void set_topic(const std::string& topic, const User& user);
	void set_key(const std::string& key)							{ m_key = key; }

	/// Replies
	void send_topic_to_user(User& user);
	void send_topic_to_user_if_set(User& user);
	void broadcast_topic();
	void broadcast_topic(User& user_to_avoid);

	/// Users
	void set_user_limit(size_t limit) { m_user_limit = limit; }
	void add_user(User& user);
	void remove_user(User& user);
	void remove_user(const std::string& user_nickname);
	bool has_user(User& user) const;
	bool has_user(const std::string& user_nickname) const;
	bool has_user(const UserIterator& user_it) const;
	void set_user_founder(User& user, bool value);
	void set_user_founder(const std::string& user_nickname, bool value);
	void set_user_protected(User& user, bool value);
	void set_user_protected(const std::string& user_nickname, bool value);
	void set_user_operator(User& user, bool value);
	void set_user_operator(const std::string& user_nickname, bool value);
	void set_user_halfop(User& user, bool value);
	void set_user_halfop(const std::string& user_nickname, bool value);
	void set_user_voice_permission(User& user, bool value);
	void set_user_voice_permission(const std::string& user_nickname, bool value);

	/// Modes
	bool update_mode(User &user, const std::vector<ModeParam> &mode_params);

	/// Entry restrictions
	void add_to_banlist(const User& user);
	void add_to_banlist(const std::string& user_nickname);
	void remove_from_banlist(const User& user);
	void remove_from_banlist(const std::string& user_nickname);

	bool is_user_invited(User& user);
	bool is_user_invited(const std::string& user_nickname);
	void add_to_invitelist(const User& user);
	void add_to_invitelist(const std::string& user_nickname);
	void remove_from_invitelist(const User& user);
	void remove_from_invitelist(const std::string& user_nickname);

	bool is_user_banned(User& user);
	bool is_user_banned(const std::string& user_nickname);
	void add_to_ban_exemptions(const User& user);
	void add_to_ban_exemptions(const std::string& user_nickname);
	void remove_from_ban_exemptions(const User& user);
	void remove_from_ban_exemptions(const std::string& user_nickname);

	/// Getters
	const	std::string&			name()								const { return m_name; }
	const	std::time_t&			creation_date()						const { return m_creation_date; }
	const	std::string&			topic()								const { return m_topic; }
	const	std::string&			last_user_to_modify_topic()			const { return m_last_user_to_modify_topic; }
	const	std::time_t&			topic_modification_date()			const { return m_topic_modification_date; }
			std::string				topic_modification_date_as_str()	const { return to_string(topic_modification_date()); }
	char							type()								const { return m_type; }

	size_t							user_count()					const { return m_users.size(); }
	size_t							user_limit()					const { return m_user_limit; }
	const UserMap&					users()							const { return m_users; }
	UserMap&						users()								  { return m_users; }

	const std::string&				key()							const { return m_key; }
	const std::vector<std::string>&	invite_list()					const { return m_invite_list; }
	const std::vector<std::string>&	invite_exemptions()				const { return m_invite_exemptions; }
	const std::vector<std::string>&	ban_list()						const { return m_ban_list; }
	const std::vector<std::string>&	ban_exemptions()				const { return m_ban_exemptions; }
	std::string						get_modes_as_str(User& user)	const;

	bool	is_user_founder(const User& user);
	bool	is_user_founder(const std::string &user_nickname);
	bool	is_user_operator(const User& user);
	bool	is_user_operator(const std::string &user_nickname);
	bool	is_user_halfop(const User& user);
	bool	is_user_halfop(const std::string &user_nickname);
	bool	is_user_has_voice(const User& user);
	bool	is_user_has_voice(const std::string &user_nickname);

	bool	is_user_limited()		const { return m_is_user_limited; }
	bool	is_invite_only()		const { return m_is_invite_only; }
	bool	is_key_protected()		const { return m_is_key_protected; }
	bool	is_moderated()			const { return m_is_moderated; }
	bool	is_secret()				const { return m_is_secret; }
	bool	is_topic_protected()	const { return m_is_topic_protected; }
	bool	no_outside_messages()	const { return m_no_outside_messages; }

private:
	/// Users
	UserIterator	find_user(const std::string& user_nickname);

	/// Channel information
	std::string	m_name;
	std::time_t	m_creation_date;
	char		m_type;
	std::string	m_topic;
	std::string	m_last_user_to_modify_topic;
	std::time_t	m_topic_modification_date;

	/// Entry restrictions
	NicknameVector	m_ban_list;
	NicknameVector	m_ban_exemptions;
	NicknameVector	m_invite_list;
	NicknameVector	m_invite_exemptions;
	std::string 	m_key;

	/// Users
	size_t	m_user_limit;
	UserMap	m_users;

	/// Modes
	bool	m_is_user_limited;			// +l
	bool	m_is_invite_only;			// +i
	bool	m_is_key_protected;			// +k
	bool	m_is_moderated;				// +m
	bool	m_is_secret;				// +s
	bool	m_is_topic_protected;		// +t
	bool	m_no_outside_messages;		// +n
};

inline const User&		get_user_reference(const Channel::ConstUserIterator& user_it)	{ return *(user_it->first); }
inline User&			get_user_reference(const Channel::UserIterator& user_it)		{ return *(user_it->first); }

inline Channel::UserPermissions&		get_user_perms_reference(const Channel::UserIterator& user_it)		{ return user_it->second; }
inline const Channel::UserPermissions&	get_user_perms_reference(const Channel::ConstUserIterator& user_it)	{ return user_it->second; }

#endif //CHANNEL_H
