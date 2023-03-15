//
// Created by nathan on 2/16/23.
//

#ifndef SERVER_H
#define SERVER_H

#include <vector>
#include <netdb.h>
#include <map>
#include <string>
#include "User.h"
#include "Channel.h"
#include "Command.h"
#include "ServerInfo.h"

struct OldUserInfo
{
	OldUserInfo() : m_last_time_seen(0) {}
	OldUserInfo(std::time_t time, const User& user);

	bool operator==(const User& user) const;
	bool operator==(const OldUserInfo& user) const;

	      std::time_t	time_last_seen()	const { return m_last_time_seen; }
	const std::string&	nickname()			const { return m_nickname; }
	const std::string&	username()			const { return m_username; }
	const std::string&	realname()			const { return m_realname; }
	const std::string&	host()				const { return m_host; }

	void set_time_last_seen(std::time_t value)	{ m_last_time_seen = value; }
	void set_nickname(const std::string& value)	{ m_nickname = value; }
	void set_username(const std::string& value)	{ m_username = value; }
	void set_realname(const std::string& value)	{ m_realname = value; }
	void set_host(const std::string& value)		{ m_host = value; }

	std::time_t m_last_time_seen;
	std::string m_nickname;
	std::string m_username;
	std::string m_realname;
	std::string m_host;
};

class Server
{
public:
	/// I/O typedefs
	typedef int (*command_function)(User&, const Command&);
	typedef std::map<std::string, command_function>::iterator	CommandIterator;

	typedef std::vector<OldUserInfo>			OldUserVector;
	typedef OldUserVector::iterator				OldUserIterator;
	typedef std::vector<User*>					UserVector;
	typedef UserVector::iterator				UserIterator;
	typedef std::map<std::string, Channel*>		ChannelMap;
	typedef ChannelMap::iterator				ChannelIterator;
	typedef ChannelMap::const_iterator			ConstChannelIterator;
	typedef std::map<std::string, std::size_t>	CommandStatsMap;
	typedef CommandStatsMap::const_iterator		CommandStatsIterator;

	/// Server management
	static bool initialize(uint16_t port);
	static bool update();
	static void shutdown();
	static void signal_handler(int signal);

	/// Operators configuration
	static const std::string& operator_name() 		{ return m_oper_username; }
	static const std::string& operator_password()	{ return m_oper_password; }
	static const std::string& operator_host() 		{ return m_oper_host; }

	/// Replies
	static void broadcast(const std::string& msg);
	static void broadcast(User& user_to_avoid, const std::string &msg);
	static void broadcast_to_channel(Channel& channel, const std::string& msg);
	static void broadcast_to_channel(User& user_to_avoid, Channel& channel, const std::string& msg);
	static void reply(User& user, const std::string& msg);
	static void reply_welcome_user(User& user);
	static void try_reply_list_channel_members_to_user(User& user, const std::string& channel_name);
	static void reply_list_channel_members_to_user(User &user, const Channel& channel);
	static void reply_channel_list_to_user(User& user);
	static void reply_channel_ban_list_to_user(User& user, const Channel& channel);
	static void reply_channel_ban_exempt_list_to_user(User& user, const Channel& channel);
	static void reply_list_of_channel_invite_to_user(User &user);
	static void reply_channel_invite_exempt_list_to_user(User& user, const Channel& channel);

	/// User management
	static void				register_user(User& user);
	static void				change_user_nickname(User& user, const std::string& new_nickname);
	static void				try_reply_part_user_from_channel(User& user, const std::string& channel_name, const std::string& reason = "");
	static void				reply_part_user_from_channel(User& user, Channel& channel, const std::string& reason = "");
	static void				reply_part_user_from_channels(User& user, const std::string& reason = "");
	static bool				user_exists(const std::string& user_nickname);
	static bool				user_exists(const UserIterator& user);
	static bool				old_user_exists(const OldUserIterator& old_user) { return old_user != m_old_users.end(); };
	static UserIterator		find_user(const std::string& user_nickname);
	static OldUserIterator	find_old_user(const std::string& user_nickname, OldUserIterator start = m_old_users.end());
	static bool				is_nickname_taken(const std::string& user_nickname);

	/// Channel management
	static Channel&			create_new_channel(User& first_user, const std::string& channel_name);
	static bool				channel_exists(const std::string& channel_name);
	static bool				channel_exists(const ChannelIterator& channel);
	static ChannelIterator	find_channel(const std::string& channel_name);

	/// Server information
	static       ServerInfo& 	info() 					{ return m_server_info; }
	static       size_t&		unknown_connections()	{ return m_unknown_connections; }
	static       bool			is_running()			{ return m_is_running; }
	static       std::time_t	start_timestamp()		{ return m_start_timestamp; }
	static const std::string&	password()				{ return m_password; }
	static       std::string	supported_tokens(User& user);

	/// User & Channels
	static       ChannelMap&	channels()				{ return m_channels; }
	static       UserVector&	users()					{ return m_users; }
	static       std::size_t	old_users_count()		{ return m_old_users.size(); }

	/// Setters
	static void	set_is_running(bool new_state)				{ m_is_running = new_state; }
	static void set_password(const std::string& password)	{ m_password = password; }

	/// Config
	static std::size_t	awaylen()						{ return m_awaylen; }
	static std::size_t	chan_name_len()					{ return m_chan_name_len; }
	static std::size_t	kicklen()						{ return m_kicklen; }
	static std::size_t	topiclen()						{ return m_topiclen; }
	static std::size_t	userlen()						{ return m_userlen; }

	/// Stats
	static const CommandStatsMap&	commands_stats()	{ return m_command_stats; }

private:
	// Member functions
	static bool		initialize_operator_credential();
	static void		initialize_command_functions();
	static void		accept_new_connections();
	static void		poll_events();
	static void		handle_events();
	static void		handle_messages();
	static void		execute_command(User& user, const Command& command);
	static void		check_for_closed_connexions();
	static void		check_for_empty_channels();

	/// Users
	static User&	create_new_user(int fd, const std::string& ip, uint16_t port);
	static void		remove_user(User& user);
	static void		add_to_old_users_list(User& user);
	static void		store_user_list_to_file();
	static void		load_old_user_list_from_file();

	// Member variables
	static ServerInfo			m_server_info;
	static int					m_server_socket;
	static std::string			m_password;

	static std::string			m_oper_username;
	static std::string			m_oper_password;
	static std::string			m_oper_host;

	static std::vector<pollfd>	m_pollfds;
	static UserVector			m_users;
	static OldUserVector		m_old_users;
	static ChannelMap			m_channels;

	static bool					m_is_running;
	static std::time_t			m_start_timestamp;

	static const int			m_server_backlog;
	static const int			m_timeout;

	static size_t				m_unknown_connections;

	// Config
	static const std::size_t	m_awaylen;
	static const std::size_t	m_chan_name_len;
	static const std::size_t	m_kicklen;
	static const std::size_t	m_max_lists_entries;
	static const std::size_t	m_userlen;
	static const std::size_t	m_topiclen;


	// Message function prototype
	static std::map<std::string, command_function>	m_commands;
	static std::map<std::string, command_function>	m_connection_commands;
	static CommandStatsMap							m_command_stats;
};

inline Channel&			get_channel_reference(const Server::ChannelIterator& channel_it)		{ return *(channel_it->second); }
inline const Channel&	get_channel_reference(const Server::ConstChannelIterator& channel_it)	{ return *(channel_it->second); }
inline User&			get_user_reference(const Server::UserIterator& user_it)					{ return *(*user_it); }
inline User&			get_user_reference(User *user_ptr)										{ return *user_ptr; }

#endif //SERVER_H
