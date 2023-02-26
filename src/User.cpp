//
// Created by nathan on 2/16/23.
//

#include <iostream>
#include <unistd.h>
#include <cerrno>
#include <cstring>
#include "User.h"
#include "IRC.h"
#include "log.h"

User::User(int fd) :
		m_nickname("*"), m_username(""), m_realname(""), m_server_name(), m_fd(fd),
		m_is_afk(false), m_is_disconnected(false),
		m_is_readable(false), m_is_writable(false),
		m_is_registered(false)
{
}

ssize_t User::receive_message()
{
	char buffer[MAX_MESSAGE_LENGTH + 1];
	ssize_t total_bytes_read = 0;

	while (total_bytes_read < MAX_MESSAGE_LENGTH) {
		ssize_t bytes_read = read(m_fd, buffer + total_bytes_read, static_cast<size_t>(MAX_MESSAGE_LENGTH - total_bytes_read));

		if (bytes_read < 0 && errno != EAGAIN) {
			std::cerr << "Error: " << std::strerror(errno) << std::endl;
		}
		if (bytes_read <= 0)
			break ;

		total_bytes_read += bytes_read;
	}
	buffer[total_bytes_read] = 0;
	m_readbuf.append(buffer);
	CORE_INFO("Buffer: %s", buffer);
	CORE_INFO("Received: %s", m_readbuf.c_str());
	return total_bytes_read;
}

ssize_t User::send_message()
{
	ssize_t total_bytes_write = 0;

	while (total_bytes_write < MAX_MESSAGE_LENGTH) {
		ssize_t bytes_write = write(fd(), m_writebuf.c_str() + total_bytes_write, m_writebuf.size() - (size_t)total_bytes_write);

		if (bytes_write < 0 && errno != EAGAIN) {
			std::cerr << "Error: " << std::strerror(errno) << std::endl;
		}
		if (bytes_write <= 0)
			break ;

		total_bytes_write += bytes_write;
	}
	m_writebuf.clear();

	return total_bytes_write;
}

std::string User::get_next_command_str()
{
	std::string command;

	std::size_t command_end = m_readbuf.find_first_of("\n\r");
	if (command_end == std::string::npos)
		return "";

	std::size_t crlf_end = command_end + 1;
	if (m_readbuf[crlf_end] == '\r' || m_readbuf[crlf_end] == '\n')
		crlf_end++;

	if (crlf_end != std::string::npos) {
		command = m_readbuf.substr(0, crlf_end);
		m_readbuf.erase(0, command.size());
	}
	CORE_INFO("Command: %s", command.c_str());
	CORE_INFO("Buffer: %s", m_readbuf.c_str());
	return command;
}

std::string User::source()
{
	std::string source = "!" + m_username + "@" + m_realname;
	return source;
}

void User::reply(const std::string &msg)
{
	update_write_buffer(msg + "\n\r");
}
