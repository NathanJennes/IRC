//
// Created by nathan on 2/16/23.
//

#ifndef USER_H
#define USER_H

#include <vector>
#include <string>
#include <netinet/in.h>
#include <sys/poll.h>
#include <unistd.h>
#include <ctime>
#include "log.h"
#include "Utils.h"
#include "Mode.h"
#include "UserQueries.h"

#define MAX_MESSAGE_LENGTH 512

class Channel;
struct Mask;

class User
{
public:
	/// Static tests
	static bool is_nickname_valid(const std::string& nickname);
	static bool is_username_valid(const std::string& username);
	static bool is_host_valid(const std::string& username);

	/// I/O typedefs
private:
	typedef std::vector<Channel*>			ChannelVector;
public:
	typedef ChannelVector::iterator			ChannelIterator;
	typedef ChannelVector::const_iterator	ConstChannelIterator;

	explicit User(int fd, const std::string& ip, uint16_t port);

	bool		receive_message();
	bool		send_message();
	std::string	get_next_command_str();
	bool		has_pending_command();
	std::string	source() const;

	void		try_finish_registration();
	bool		check_password();

	void		add_channel(Channel& channel);
	void 		remove_channel(const Channel &channel);

	bool		has_channel_in_common(const User& other_user) const;
	bool		is_visible_to_user(User& user) const;
	void		reply_list_of_channel_to_user(User& user);

	/// Mode
	bool 		update_mode(const std::vector<ModeParam>& mode_params);

	/// Time
	void 		take_ping_timestamp();
	void 		take_idle_timestamp();
	void		recalculate_ping();
	void		recalculate_idle();

	/// getters
	std::string				ping_token()		const	{ return m_ip + to_string(m_port) + m_realname; }
	const std::string&		nickname()			const	{ return m_nickname; }
	const std::string&		nickname_upper()	const	{ return m_nickname_upper; }
	const std::string&		username()			const	{ return m_username; }
	const std::string&		realname()			const	{ return m_realname; }
	const std::string&		hostname()			const	{ return m_hostname; }
	const std::string&		password()			const	{ return m_password; }
	const std::string&		server()			const	{ return m_server_name; }
	const int&				fd()				const	{ return m_fd; }
	const std::string&		ip()				const	{ return m_ip; }
	uint16_t				port()				const	{ return m_port; }
	bool					is_disconnected()	const	{ return m_is_disconnected; }
	bool					is_writable()		const	{ return m_is_writable; }
	bool					is_readable()		const	{ return m_is_readable; }
	bool					is_registered()		const	{ return m_is_registered; }
	bool					need_password()		const	{ return m_need_password; }

	      ChannelVector&	channels()					{ return m_channels; }
	const ChannelVector&	channels()			const	{ return m_channels; }

	const std::string&		read_buffer()				const	{ return m_readbuf; }
	const std::string&		write_buffer()				const	{ return m_writebuf; }
	std::size_t				data_sent_size()			const	{ return m_data_sent_size; }
	std::size_t				data_received_size()		const	{ return m_data_received_size; }
	std::size_t				sent_messages_count()		const	{ return m_sent_messages_count; }
	std::size_t				received_messages_count()	const	{ return m_received_messages_count; }
	const std::string&		away_message()				const	{ return m_away_message; }

	bool					is_away()					const	{ return m_is_afk; }
	bool					is_invisible()				const	{ return m_is_invisible; }
	bool					is_operator()				const	{ return m_is_operator; }
	bool					can_get_notice()			const	{ return m_can_receive_notice; }

	const std::string&		signon()						const	{ return m_signon_timestamp; }
	const std::string		seconde_idle()					const	{ return to_string<long>(m_idle); }
	long					ping()							const	{ return m_ping; }
	std::time_t				connexion_creation_timestamp()	const	{ return m_connexion_creation_timestamp; }
	std::time_t				time_connexion_open()			const	{ return time(NULL) - m_connexion_creation_timestamp; }

	std::string				get_modes_as_str()	const;
	std::string 			get_user_flags()	const;
	bool					has_mask(std::vector<Mask> masks) const;

	/// setters
	void	set_nickname(const std::string& nickname)	{ m_nickname = nickname; m_nickname_upper = to_upper(nickname); }
	void	set_username(const std::string& username)	{ m_username = username; }
	void	set_realname(const std::string& realname)	{ m_realname = realname; }
	void	set_server_name(const std::string& name)	{ m_server_name = name; }
	void	set_is_negociating_capabilities(bool value)	{ m_is_negociating_capabilities = value; }

	void	queue_command_for_sending(const std::string& str)	{ m_writebuf.append(str); m_sent_messages_count++; }

	void	disconnect() 								{ m_is_disconnected = true; }
	void	set_is_readable(bool is_readable)			{ m_is_readable = is_readable; }
	void	set_is_writable(bool is_writable)			{ m_is_writable = is_writable; }
	void	set_password(const std::string& password)	{ m_password = password; }

	void	set_away_msg(const std::string& message)	{ m_away_message = message; }
	void	set_is_away(bool is_afk)					{ m_is_afk = is_afk; }
	void	set_is_invisible(bool is_invisible)			{ m_is_invisible = is_invisible; }
	void	set_is_operator(bool is_operator)			{ m_is_operator = is_operator; }
	void	set_can_receive_notice(bool can_receive)	{ m_can_receive_notice = can_receive; }

	friend bool operator==(const User& lhs, const User& rhs) {
		return lhs.nickname() == rhs.nickname();
	}

	friend bool operator!=(const User& lhs, const User& rhs) {
		return !(lhs == rhs);
	}

	/// Debug
	const char *debug_name();

private:
	void		take_signon_timestamp();

	std::string	m_nickname;
	std::string m_nickname_upper;
	std::string	m_username;
	std::string	m_realname;
	std::string m_hostname;
	std::string	m_password;
	std::string	m_server_name;

	std::string	m_readbuf;
	std::string	m_writebuf;
	std::size_t	m_data_sent_size;
	std::size_t	m_data_received_size;
	std::size_t	m_sent_messages_count;
	std::size_t	m_received_messages_count;

	std::string	m_ip;
	uint16_t	m_port;

	int			m_fd;

	bool		m_is_disconnected;
	bool		m_is_readable;
	bool		m_is_writable;
	bool		m_is_registered;
	bool		m_is_negociating_capabilities;
	bool 		m_need_password;

	ChannelVector	m_channels;

	std::string m_away_message;

	bool		m_is_afk;				// +a
	bool		m_is_operator;			// +o
	bool		m_is_invisible;			// +i
	bool		m_can_receive_notice;	// +s

	std::string	m_signon_timestamp;
	timeval		m_last_idle_timestamp;
	long		m_idle;
	timeval		m_last_ping_timestamp;
	long		m_ping;
	std::time_t	m_connexion_creation_timestamp;
};

inline       Channel&	get_channel_reference(const User::ChannelIterator& channel_it)		{ return *(*channel_it); }
inline const Channel&	get_channel_reference(const User::ConstChannelIterator& channel_it)	{ return *(*channel_it); }

#endif //USER_H
