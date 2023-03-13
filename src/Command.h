//
// Created by Cyril Battistolo on 17/02/2023.
//

#ifndef COMMAND_H
#define COMMAND_H

#include <string>
#include <vector>
#include "User.h"

class Command
{
public:
	struct TagKey
	{
		std::string	client_prefix;
		std::string	vendor;
		std::string	key_str;
	};

	struct Tag
	{
		TagKey		key;
		std::string	value;
	};

	struct Source
	{
		std::string source_name;
		std::string user;
		std::string host;
	};

	Command(const std::string& command_str);

	bool	is_valid();

	// Getters
	const std::vector<Tag>&			get_tags()			const { return m_tags; }
	const Source&					get_source()		const { return m_source; }
	const std::string&				get_command()		const { return m_command; }
	const std::vector<std::string>&	get_parameters()	const { return m_parameters; }

	// setters
	void set_command(const std::string& command) 					{ m_command = command; }
	void set_parameters(const std::vector<std::string>& parameters)	{ m_parameters = parameters; }

	// Debug
	void print();

private:
	bool	m_ill_formed;

	std::string	m_str;
	std::size_t	m_index;

	std::vector<Tag>			m_tags;
	Source						m_source;
	std::string					m_command;
	std::vector<std::string>	m_parameters;

	// Message
	void	parse_message();
	bool	consume_spaces();
	bool	consume_crlf();

	// Tags
	std::vector<Tag>	try_parse_tags();
	Tag					try_parse_tag();
	TagKey				try_parse_key();
	std::string			try_parse_client_prefix();
	std::string			try_parse_escaped_value();
	std::string			try_parse_vendor();

	// Source
	Source		try_parse_source();

	// Command
	std::string	try_parse_command();

	// Parameters
	std::vector<std::string>	try_parse_parameter();
	std::string					try_parse_nospcrlfcl();
	std::string					try_parse_middle();
	std::string					try_parse_trailing();

	// Hostname
	std::string	try_parse_host();
	std::string	try_parse_hostname();
	std::string	try_parse_ipv4_address();
	std::string	try_parse_domain_label();
	std::string	try_parse_top_label();

	// ABNF
	bool		consume_char(char c);
	std::string	consume_integer();
	std::string	consume_string();

	// Helpers
	char				current_char();
	std::size_t			characters_left();
	const std::string&	letters();
	const std::string&	digits();
	std::string			return_empty_string_and_restore_index(std::size_t old_index);
};

#endif //COMMAND_H
