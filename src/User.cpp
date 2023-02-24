//
// Created by nathan on 2/16/23.
//

#include <iostream>
#include <unistd.h>
#include <cerrno>
#include <cstring>
#include "User.h"
#include "IRC.h"

User::User(const std::string &username, const std::string &real_name, const std::string &server_name, int fd)
	: m_name_on_host(username), m_host_address(real_name), m_server_name(server_name), m_fd(fd), m_is_afk(false), m_is_disconnected(false), m_is_readable(false), m_is_writable(false)
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
	std::cout << "Write_buf left: [" << m_writebuf << "]" << std::endl;

	return total_bytes_write;
}

std::string User::get_next_command_str()
{
	std::string command;
	size_t command_end = m_readbuf.find_first_of("\n\r");

	if (command_end != std::string::npos) {
		command = m_readbuf.substr(0, m_readbuf.find_first_of("\n\r"));
		std::cout << "Command: [" << command << "]" << std::endl;
		m_readbuf.erase(0, command.size() + 1);
	}
	return command;
}
