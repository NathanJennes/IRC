//
// Created by Cyril Battistolo on 07/03/2023.
//

#include <fstream>
#include <vector>
#include <string>
#include <cstring>
#include "ServerInfo.h"
#include "Utils.h"
#include "log.h"

#define SERVER_VERSION "ft_irc-v0.6"

void ServerInfo::initialize()
{
	m_name = "localhost";
	m_description = ">> The GigaChat IRC server !!! <<";
	m_version = SERVER_VERSION;
	m_network_name = "GigaChat";
	m_server_location = "Lyon, France";
	m_hosting_location = "Lyon, France";
	m_server_creation_date = get_current_date();
	get_server_motd("config/motd.txt");
	m_admin_name = "cybattis njennes";
	m_admin_email = "support@gigachat.net";
	m_max_connected_users = 0;
}

bool ServerInfo::get_server_motd(const std::string& path)
{
	std::fstream modt_file(path.c_str(), std::ios::in);

	if (!modt_file.is_open()) {
		CORE_WARN("%s: %s", path.c_str(), strerror(errno));
		CORE_WARN("MOTD not loaded");
		return false;
	}

	m_motd.clear();

	std::string line;
	while (std::getline(modt_file, line))
		m_motd += line + '\n';

	return true;
}
