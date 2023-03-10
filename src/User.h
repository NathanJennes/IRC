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
#include "log.h"
#include "Utils.h"
#include "Mode.h"

#define MAX_MESSAGE_LENGTH 512
#define MAX_NICKNAME_LENGTH 9

class Channel;

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

	ssize_t		receive_message();
	ssize_t		send_message();
	std::string	get_next_command_str();
	bool		has_pending_command();
	std::string	source() const;

	void		try_finish_registration();
	bool		check_password();

	void		add_channel(Channel& channel);

	// Mode
	bool 		update_mode(const std::vector<ModeParam>& mode_params);

	// Ping
	void		take_ping_timestamp();
	void		recalculate_ping();

	// getters
	std::string				ping_token()		const	{ return m_ip + to_string(m_port) + m_realname; }
	const std::string&		nickname()			const	{ return m_nickname; }
	const std::string&		username()			const	{ return m_username; }
	const std::string&		realname()			const	{ return m_realname; }
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

	const std::string&		read_buffer()		const	{ return m_readbuf; }
	const std::string&		write_buffer()		const	{ return m_writebuf; }
	const std::string&		away_message()		const	{ return m_away_message; }

	bool					is_away()			const	{ return m_is_afk; }
	bool					is_invisible()		const	{ return m_is_invisible; }
	bool					is_operator()		const	{ return m_is_operator; }
	bool					can_get_wallop()	const	{ return m_can_receive_wallop; }
	bool					can_get_notice()	const	{ return m_can_receive_notice; }

	long					ping()				const	{ return m_ping; }

	std::string				get_modes_as_str()	const;

	/// setters
	void	set_nickname(const std::string& nickname)	{ m_nickname = nickname; }
	void	set_username(const std::string& username)	{ m_username = username; }
	void	set_realname(const std::string& realname)	{ m_realname = realname; }
	void	set_server_name(const std::string& name)	{ m_server_name = name; }
	void	set_is_negociating_capabilities(bool value)	{ m_is_negociating_capabilities = value; }

	void	update_write_buffer(const std::string& str)	{ m_writebuf.append(str); }

	void	disconnect() 								{ m_is_disconnected = true; }
	void	set_is_readable(bool is_readable)			{ m_is_readable = is_readable; }
	void	set_is_writable(bool is_writable)			{ m_is_writable = is_writable; }
	void	set_password(const std::string& password)	{ m_password = password; }

	void	set_away_msg(const std::string& message)	{ m_away_message = message; }
	void	set_is_afk(bool is_afk)						{ m_is_afk = is_afk; }
	void	set_is_invisible(bool is_invisible)			{ m_is_invisible = is_invisible; }
	void	set_is_operator(bool is_operator)			{ m_is_operator = is_operator; }
	void	set_can_receive_wallop(bool can_receive)	{ m_can_receive_wallop = can_receive; }
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
	std::string	m_nickname;
	std::string	m_username;
	std::string	m_realname;
	std::string	m_password;
	std::string	m_server_name;

	std::string	m_readbuf;
	std::string	m_writebuf;

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
	bool		m_is_afk;

	bool		m_is_operator;			// +o
	bool		m_is_invisible;			// +i
	bool		m_can_receive_wallop;	// +w
	bool		m_can_receive_notice;	// +s

	timeval		m_last_ping_timestamp;
	long		m_ping;
};

inline Channel&		get_channel_reference(const User::ChannelIterator& channel_it)			{ return *(*channel_it); }
inline const Channel&	get_channel_reference(const User::ConstChannelIterator& channel_it)		{ return *(*channel_it); }

#endif //USER_H
