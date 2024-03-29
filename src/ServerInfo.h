//
// Created by Cyril Battistolo on 07/03/2023.
//

#ifndef SERVERINFO_H
#define SERVERINFO_H

#include <vector>
#include <string>

struct ServerInfo
{
	void initialize();

	//getter
	const std::string& name() 					const { return m_name; }
	const std::string& description()			const { return m_description; }
	const std::string& version()				const { return m_version; }
	const std::string& network_name()			const { return m_network_name; }
	const std::string& server_location()		const { return m_server_location; }
	const std::string& hosting_location()		const { return m_hosting_location; }
	const std::string& creation_date()			const { return m_server_creation_date; }
	const std::string& motd()					const { return m_motd; }
	const std::string& admin_name()				const { return m_admin_name; }
	const std::string& email()					const { return m_admin_email; }
	      size_t       max_users()				const { return m_max_connected_users; }
	const std::string& user_modes()				const { return m_user_modes; }
	const std::string& channel_modes()			const { return m_channel_modes; }
	const std::string& channel_modes_params()	const { return m_channel_modes_params; }

	//setter
	void set_max_users(size_t max_users) { m_max_connected_users = max_users; }

private:
	bool get_server_motd(const std::string& path);

	std::string m_name;
	std::string m_description;
	std::string m_version;
	std::string m_network_name;
	std::string m_server_location;
	std::string m_hosting_location;
	std::string m_server_creation_date;
	std::string m_motd;
	std::string m_admin_name;
	std::string m_admin_email;
	std::size_t m_max_connected_users;
	std::string m_user_modes;
	std::string m_channel_modes;
	std::string m_channel_modes_params;
};

#endif //SERVERINFO_H
