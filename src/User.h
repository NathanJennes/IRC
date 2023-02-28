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

#define MAX_MESSAGE_LENGTH 512
#define MAX_NICKNAME_LENGTH 9

class User
{
public:
	explicit User(int fd, const std::string& ip, uint16_t port);
	~User() { close(m_fd); };

	ssize_t		receive_message();
	ssize_t		send_message();
	std::string	get_next_command_str();
	std::string	source();

	void		try_finish_registration();

	// Ping
	void	take_ping_timestamp();
	void	recalculate_ping();

	// getters
	std::string			ping_token()		const	{ return m_ip + std::to_string(m_port) + m_realname; }
	const std::string&	nickname()			const	{ return m_nickname; }
	const std::string&	username()			const	{ return m_username; }
	const std::string&	realname()			const	{ return m_realname; }
	const std::string&	password()			const	{ return m_password; }
	const std::string&	server()			const	{ return m_server_name; }
	bool				is_afk()			const	{ return m_is_afk; }
	bool				is_disconnected()	const	{ return m_is_disconnected; }
	const int&			fd()				const	{ return m_fd; }
	const std::string&	ip()				const	{ return m_ip; }
	uint16_t			port()				const	{ return m_port; }
	bool				is_writable()		const	{ return m_is_writable; }
	bool				is_readable()		const	{ return m_is_readable; }
	bool				is_registered()		const	{ return m_is_registered; }

	const std::string&	read_buffer()		const	{ return m_readbuf; }
	const std::string&	write_buffer()		const	{ return m_writebuf; }

	long	ping()							const	{ return m_ping; }

	// setters
	void	set_nickname(const std::string& nickname)	{ m_nickname = nickname; }
	void	set_username(const std::string& username)	{ m_username = username; }
	void	set_realname(const std::string& realname)	{ m_realname = realname; }
	void	set_server_name(const std::string& name)	{ m_server_name = name; }
	void	set_is_negociating_capabilities(bool value)	{ m_is_negociating_capabilities = value; }

	void	set_is_afk(bool is_afk)						{ m_is_afk = is_afk; }

	void	set_is_readable(bool is_readable)			{ m_is_readable = is_readable; }
	void	set_is_writable(bool is_writable)			{ m_is_writable = is_writable; }
	void	update_write_buffer(const std::string& str)	{ m_writebuf.append(str); }

	void	disconnect()								{ m_is_disconnected = true; }

private:
	std::string	m_nickname;
	std::string	m_username;
	std::string	m_realname;
	std::string	m_password;
	std::string	m_server_name;

	std::string	m_ip;
	uint16_t	m_port;

	int			m_fd;

	bool		m_is_afk;
	bool		m_is_disconnected;
	bool		m_is_readable;
	bool		m_is_writable;
	bool		m_is_registered;
	bool		m_is_negociating_capabilities;

	std::string	m_readbuf;
	std::string	m_writebuf;

	std::vector<std::string>	m_channels;

	struct timeval	m_last_ping_timestamp;
	long			m_ping;
};

bool operator==(const User& lhs, const User& rhs)
{
	return lhs.nickname() == rhs.nickname();
}

#endif //USER_H
