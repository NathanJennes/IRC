//
// Created by nathan on 2/16/23.
//

#ifndef USER_H
#define USER_H

#include <vector>
#include <string>
#include <netinet/in.h>
#include <sys/poll.h>

class User
{
public:
	User(const std::string& username, const std::string& real_name, const std::string& server_name, int fd);

	ssize_t		receive_message();
	ssize_t		send_message();
	std::string	get_next_command_str();

	// getters
	const std::string&	nickname()			const	{ return m_nickname; }
	const std::string&	name_on_host()		const	{ return m_name_on_host; }
	const std::string&	host_address()		const	{ return m_host_address; }
	const std::string&	server()			const	{ return m_server_name; }
	bool				is_afk()			const	{ return m_is_afk; }
	bool				is_disconnected()	const	{ return m_is_disconnected; }
	const int&			fd()				const	{ return m_fd; }
	bool				is_writable()		const	{ return m_is_writable; }
	bool				is_readable()		const	{ return m_is_readable; }

	const std::string&	read_buffer()		const	{ return m_readbuf; }
	const std::string&	write_buffer()		const	{ return m_writebuf; }

	const int&			active_channel()	const	{ return m_active_channel; }

	// setters
	void	set_nickname(const std::string& nickname)	{ m_nickname = nickname; }
	void	set_is_readable(bool is_readable)			{ m_is_readable = is_readable; }
	void	set_is_writable(bool is_writable)			{ m_is_writable = is_writable; }
	void	update_write_buffer(const std::string& str)	{ m_writebuf.append(str); }

	void	disconnect()								{ m_is_disconnected = true; }

private:
	std::string m_nickname;
	std::string	m_name_on_host;
	std::string	m_host_address;
	std::string	m_server_name;

	int			m_fd;

	bool		m_is_afk;
	bool		m_is_disconnected;
	bool		m_is_readable;
	bool		m_is_writable;

	std::string	m_readbuf;
	std::string	m_writebuf;

	std::vector<std::string>	m_channels;
	int							m_active_channel;
};

#endif //USER_H
