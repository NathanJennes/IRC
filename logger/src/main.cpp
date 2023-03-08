//
// Created by nathan on 3/8/23.
//

#include <arpa/inet.h>
#include <cerrno>
#include <cstring>
#include <fcntl.h>
#include <unistd.h>
#include <csignal>
#include <fstream>
#include <sys/poll.h>
#include "log.h"

#define READ_SIZE 1000

uint16_t listen_port = 54000;
int listen_socket = 0;
int listen_backlog = 10;

int weechat_socket = 0;
int weechat_port = 0;
char *weechat_ip = NULL;

int server_socket = 0;

bool should_stop = false;

void signal_handler(int signal)
{
	(void)signal;
	should_stop = true;
}

void initialize_signals()
{
	struct sigaction sa;
	sa.sa_handler = signal_handler;
	sigemptyset(&sa.sa_mask);
	sa.sa_flags = 0;
	sigaction(SIGINT, &sa, NULL);
	sigaction(SIGTERM, &sa, NULL);
}

bool initialize_server()
{
	listen_socket = socket(AF_INET, SOCK_STREAM, 0);
	if (listen_socket < 0) {
		CORE_ERROR("Can't socket(). listen socket: %s", strerror(errno));
		return false;
	}

	int is_active = 1;
	if (setsockopt(listen_socket, SOL_SOCKET, SO_REUSEADDR, &is_active, sizeof(int)) == -1) {
		CORE_ERROR("Can't setsockopt(). listen setsockopt: %s", strerror(errno));
		return false;
	}

	struct sockaddr_in params = {};
	bzero(&params, sizeof(params));
	params.sin_family = AF_INET;
	params.sin_addr.s_addr = INADDR_ANY;
	params.sin_port = htons(listen_port);

	if (bind(listen_socket, (const struct sockaddr *) &params, sizeof (params)) < 0) {
		CORE_ERROR("Can't bind(). listen bind: %s", strerror(errno));
		return false;
	}

	if (listen(listen_socket, listen_backlog)) {
		CORE_ERROR("Can't listen(). listen listen: %s", strerror(errno));
		return false;
	}

	return true;
}

bool initialize_client()
{
	server_socket = socket(AF_INET, SOCK_STREAM, 0);
	if (server_socket < 0) {
		CORE_ERROR("Can't socket(). output socket: %s", strerror(errno));
		return false;
	}

	struct sockaddr_in serv_addr = {};
	serv_addr.sin_port = htons(6667);
	serv_addr.sin_family = AF_INET;
	inet_pton(AF_INET, "130.239.18.120", &serv_addr.sin_addr);

	if (connect(server_socket, (struct sockaddr*)&serv_addr, sizeof(serv_addr)) < 0) {
		CORE_ERROR("Can't connect(). output socket: %s", strerror(errno));
		return false;
	}

	if (fcntl(server_socket, F_SETFL, O_NONBLOCK) < 0) {
		CORE_ERROR("Can't fcntl(). output fcntl: %s", strerror(errno));
		return false;
	}

	return true;
}

void close_all_connexions()
{
	close(server_socket);
	close(listen_socket);
	close(weechat_socket);
}

bool accept_weechat_connexion()
{
	struct sockaddr_in client = {};
	socklen_t len = sizeof(struct sockaddr_in);

	int new_connexion = accept(listen_socket, (struct sockaddr *)&client, &len);
	if (new_connexion < 0) {
		CORE_ERROR("Can't accept(). listen accept: %s", strerror(errno));
		return false;
	}

	weechat_socket = new_connexion;
	weechat_ip = inet_ntoa(client.sin_addr);
	weechat_port = ntohs(client.sin_port);

	if (fcntl(weechat_socket, F_SETFL, O_NONBLOCK) < 0) {
		CORE_ERROR("Can't fcntl(). listen fcntl: %s", strerror(errno));
		return false;
	}

	return true;
}

ssize_t receive_message(char *buffer, int fd)
{
	ssize_t total_bytes_read = 0;

	while (total_bytes_read < READ_SIZE) {
		ssize_t bytes_read = read(fd, buffer + total_bytes_read, static_cast<size_t>(READ_SIZE - total_bytes_read));

		if (bytes_read < 0 && errno != EAGAIN)
			CORE_ERROR(std::strerror(errno));

		if (bytes_read <= 0)
			break ;

		total_bytes_read += bytes_read;
	}
	buffer[total_bytes_read] = 0;

	return total_bytes_read;
}

int main()
{
	initialize_signals();
	CORE_INFO("Signals initialized");

	if (!initialize_server()) {
		close_all_connexions();
		return 1;
	}
	CORE_INFO("server initialized");

	if (!initialize_client()) {
		close_all_connexions();
		return 1;
	}
	CORE_INFO("client initialized");

	if (!accept_weechat_connexion()) {
		close_all_connexions();
		return 1;
	}
	CORE_INFO("weechat connected");

	std::ofstream log_file("log.log");

	if (!log_file.good()) {
		CORE_ERROR("Couldn't create the log file");
		close_all_connexions();
		return 1;
	}

	std::string weechat_write_buffer;
	std::string server_write_buffer;
	char buffer[READ_SIZE + 1];
	memset(buffer, 0, READ_SIZE + 1);

	while (!should_stop)
	{
		pollfd pollfds[2];
		pollfds[0].fd = server_socket;
		pollfds[0].events = POLLIN | POLLOUT;
		pollfds[0].revents = 0;
		pollfds[1].fd = weechat_socket;
		pollfds[1].events = POLLIN | POLLOUT;
		pollfds[1].revents = 0;

		poll(pollfds, (nfds_t)2, 10);

		if (pollfds[0].revents & POLLIN) {
			CORE_TRACE("Reading from server...");
			if (receive_message(buffer, server_socket) <= 0) {
				CORE_ERROR("receive message from server failed");
				close_all_connexions();
				return 1;
			}
			weechat_write_buffer.append(buffer);
			log_file << "SERVER:	" << buffer;
		}
		if (pollfds[1].revents & POLLIN) {
			CORE_TRACE("Reading from weechat...");
			if (receive_message(buffer, weechat_socket) <= 0) {
				CORE_ERROR("receive message from weechat failed");
				close_all_connexions();
				return 1;
			}
			server_write_buffer.append(buffer);
			log_file << "WEECHAT:	" << buffer;
		}
		if (pollfds[0].revents & POLLOUT && !server_write_buffer.empty()) {
			CORE_TRACE("Writing to server...: [%s]", server_write_buffer.c_str());
			write(server_socket, server_write_buffer.c_str(), server_write_buffer.size());
			server_write_buffer.clear();
		}
		if (pollfds[1].revents & POLLOUT && !weechat_write_buffer.empty()) {
			CORE_TRACE("Writing to weechat...: [%s]", weechat_write_buffer.c_str());
			write(weechat_socket, weechat_write_buffer.c_str(), weechat_write_buffer.size());
			weechat_write_buffer.clear();
		}
	}
	CORE_INFO("closing app...");
	close_all_connexions();
	return 0;
}
