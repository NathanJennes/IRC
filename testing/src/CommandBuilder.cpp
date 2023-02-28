//
// Created by nathan on 2/25/23.
//

#include "CommandBuilder.h"
#include "log.h"
#include <random>
#include <algorithm>
#include <iostream>

static const size_t max_string_len = 300;

static const char alphanum[] =
	"0123456789"
	"ABCDEFGHIJKLMNOPQRSTUVWXYZ"
	"abcdefghijklmnopqrstuvwxyz";

static const char alpha[] =
	"abcdefghijklmnopqrstuvwxyz"
	"ABCDEFGHIJKLMNOPQRSTUVWXYZ";

static const char digit[] = "0123456789";

static const char spaces[] = "\f\n\r\t\v ";

static std::random_device dev;
static std::mt19937 rng(dev());
static std::uniform_int_distribution<std::mt19937::result_type> one_or_zero(0,1);
static std::uniform_int_distribution<std::mt19937::result_type> one_in_five(0,5);
static std::uniform_int_distribution<std::mt19937::result_type> rand_alphanum(0,sizeof (alphanum) - 2);
static std::uniform_int_distribution<std::mt19937::result_type> rand_alpha(0,sizeof (alpha) - 2);
static std::uniform_int_distribution<std::mt19937::result_type> rand_digits(0,sizeof (digit) - 2);
static std::uniform_int_distribution<std::mt19937::result_type> rand_spaces(0,sizeof (spaces) - 2);
static std::uniform_int_distribution<std::mt19937::result_type> rand_ascii(1, 127);
static std::uniform_int_distribution<std::mt19937::result_type> rand_strlen(0, max_string_len);


CommandBuilder::CommandBuilder(bool valid_tags, bool valid_source, bool valid_command, bool valid_parameters,
	bool valid_crlf, bool fuzz_spaces)
	: m_has_last_param(false), m_valid_tags(valid_tags), m_valid_source(valid_source), m_valid_command(valid_command),
	m_valid_parameters(valid_parameters), m_valid_crlf(valid_crlf), m_fuzz_spaces(fuzz_spaces)
{
	// Tags
	bool tag_valid = true;
	m_tags_address_sign = "@";
	if (!m_valid_tags && one_in_five(rng) == 0) {
		if (one_or_zero(rng))
			m_tags_address_sign = "";
		else
			m_tags_address_sign = generate_ascii_string(0, 10);
		tag_valid = false;
	}

	m_tags.resize(one_in_five(rng) + 1);
	for (size_t i = 0; i < m_tags.size(); i++)
		m_tags[i] = generate_tag(tag_valid);

	m_tags_spaces = " ";
	if (fuzz_spaces || (!m_valid_tags && tag_valid)) {
		m_tags_spaces = generate_spaces(1, 5);
		tag_valid = false;
	}

	// Source
	bool source_valid = true;
	m_colon = ":";
	if (!m_valid_source && one_in_five(rng) == 0) {
		if (one_or_zero(rng))
			m_colon = "";
		else
			m_colon = generate_ascii_string(0, 10);
		source_valid = false;
	}

	m_source = generate_source(source_valid);

	m_source_spaces = " ";
	if (fuzz_spaces)
		m_source_spaces = generate_spaces(1, 5);
	else
		m_source_spaces = " ";

	if (!m_valid_source && source_valid) {
		m_source_spaces = "";
		source_valid = false;
	}

	// Command
	if (m_valid_command) {
		if (one_or_zero(rng) == 0)
			m_command = generate_alpha_string(1, 10);
		else
			m_command = generate_digits_string(3, 3);
	} else {
		size_t result = one_in_five(rng);
		if (result == 0 || result == 1)
			m_command = generate_alphanum_string(1, 50) + "3";
		else if (result == 2 || result == 3)
			m_command = generate_digits_string(4, 10);
		else if (result == 4)
			m_command = "";
		else
			m_command = generate_ascii_string(1, 50);
	}

	// Parameters
	size_t param_count = one_in_five(rng);
	if (m_valid_parameters) {
		if (m_fuzz_spaces)
			m_param_space = generate_spaces(1, 5);
		else
			m_param_space = " ";
		for (size_t i = 0; i < param_count; i++) {

			std::string param;
			param += generate_alphanum_string(1, 10);
			size_t middle_parts = one_in_five(rng);
			for (size_t j = 0; j < middle_parts; j++) {
				if (one_or_zero(rng) == 0)
					param += ":";
				else
					param += generate_alphanum_string(1, 1);
			}

			m_parameters.push_back(param);
		}

		if (one_or_zero(rng) == 0) {
			m_has_last_param = true;
			if (m_fuzz_spaces)
				m_last_param_space = generate_spaces(1, 5);
			else
				m_last_param_space = " ";

			m_last_param_space += ":";

			std::string last_param;
			size_t last_param_parts = one_in_five(rng);
			for (size_t j = 0; j < last_param_parts; j++) {
				for (size_t k = 0; k < 3; k++) {
					size_t random = one_in_five(rng);
					if (random == 0 || random == 1)
						last_param += ":";
					else if (random == 2 || random == 3)
						last_param += " ";
					else
						last_param += generate_alphanum_string(1, 1);
				}
			}

			m_last_parameter = last_param;
		}
	} else {
		m_has_last_param = true;
		m_param_space = "";
		for (size_t i = 0; i < param_count; i++) {

			std::string param;
			param += generate_ascii_string(1, 10);
			size_t middle_parts = one_in_five(rng);
			for (size_t j = 0; j < middle_parts; j++) {
				if (one_or_zero(rng) == 0)
					param += generate_ascii_string(1, 1);
			}

			m_parameters.push_back(param);
		}

		if (one_or_zero(rng) == 0) {
			m_last_param_space = "";

			std::string last_param;
			size_t last_param_parts = one_in_five(rng);
			for (size_t j = 0; j < last_param_parts; j++) {
				for (size_t k = 0; k < 3; k++) {
					last_param += generate_ascii_string(1, 1);
				}
			}

			m_last_parameter = last_param;
		}
	}

	if (m_valid_crlf)
		m_crlf = "\r\n";
	else
		m_crlf = generate_ascii_string(0, 10);
}

std::string CommandBuilder::to_string()
{
	std::string str;
	str += m_tags_address_sign;
	for (auto& tag : m_tags) {
		str += tag.key.client_prefix;
		str += tag.key.vendor;
		if (!tag.key.vendor.empty() && m_valid_tags)
			str += '/';
		str += tag.key.key_str;
		if (!tag.value.empty())
			str += '=';
		str += tag.value;
		str += ';';
	}
	if (str[str.size() - 1] == ';')
		str.pop_back();
	str += m_tags_spaces;
	str += m_colon;
	str += m_source.source_name;
	str += m_source_user_letter;
	str += m_source.user;
	str += m_source_host_letter;
	str += m_source.host;
	str += m_source_spaces;
	str += m_command;
	for (auto& param : m_parameters) {
		str += m_param_space;
		str += param;
	}
	str += m_last_param_space;
	str += m_last_parameter;
	str += m_crlf;
	return str;
}

Command::Tag CommandBuilder::generate_tag(bool &tag_valid)
{
	Command::Tag tag;

	// Client prefix
	if (one_or_zero(rng) == 0) {
		tag.key.client_prefix = '+';
		if (!m_valid_tags && tag_valid && one_in_five(rng) == 0) {
			// Invalid case
			tag.key.client_prefix = generate_ascii_string(2);
			tag_valid = false;
		}
	}

	// Vendor
	if (one_or_zero(rng) == 0) {
		tag.key.vendor = "127.0.0.1";
		if (!m_valid_tags && tag_valid && one_in_five(rng) == 0) {
			if (one_or_zero(rng) == 0)
				tag.key.vendor = generate_alphanum_string();
			else
				tag.key.vendor = generate_ascii_string(30) + ".";
			tag_valid = false;
		}
	}

	// Key string
	tag.key.key_str = generate_alphanum_string(1, 10);
	if (!m_valid_tags && tag_valid && one_in_five(rng) == 0) {
		if (one_or_zero(rng) == 0)
			tag.key.key_str = generate_alphanum_string() + "_";
		else
			tag.key.key_str = generate_ascii_string() + "_";
		tag_valid = false;
	}

	// Key value
	tag.value = generate_alphanum_string(1, 10);
	if (!m_valid_tags && tag_valid && one_in_five(rng) == 0) {
		if (one_or_zero(rng) == 0)
			tag.value = generate_alphanum_string() + "_";
		else
			tag.value = generate_ascii_string() + "_";
		tag_valid = false;
	}

	return tag;
}

std::string CommandBuilder::generate_alphanum_string(size_t min_len, size_t max_len)
{
	std::size_t string_len = std::clamp(rand_strlen(rng), min_len, std::min(max_string_len, max_len));
	std::string string;
	string.reserve(string_len);

	for (size_t i = 0; i < string_len; i++)
		string += alphanum[rand_alphanum(rng)];

	return string;
}

std::string CommandBuilder::generate_ascii_string(size_t min_len, size_t max_len)
{
	std::size_t string_len = std::clamp(rand_strlen(rng), min_len, std::min(max_string_len, max_len));
	std::string string;
	string.reserve(string_len);

	for (size_t i = 0; i < string_len; i++)
		string += static_cast<char>(rand_ascii(rng));

	return string;
}

Command::Source CommandBuilder::generate_source(bool &source_valid)
{
	Command::Source source;

	// Source name
	source.source_name = generate_alphanum_string(1, 20);
	if (!m_valid_source && source_valid && one_in_five(rng) > 3) {
		source.source_name = "";
		source_valid = false;
	}

	// User
	if (one_or_zero(rng) == 0) {
		m_source_user_letter = "!";
		source.user = generate_alphanum_string(1, 10);
		if (!m_valid_source && source_valid && one_in_five(rng) == 0) {
			source.user.clear();
			if (one_or_zero(rng) == 0) {
				m_source_user_letter = "";
				source.user = generate_alphanum_string(1, 10);
			} else {
				m_source_user_letter = "!";
				source.user = generate_ascii_string(1, 20) + " " + generate_ascii_string(1, 20);
			}
			source_valid = false;
		}
	}

	// Host
	if (one_or_zero(rng) == 0) {
		m_source_host_letter = "@";
		source.host = generate_alphanum_string(1, 10);
		if (!m_valid_source && source_valid && one_in_five(rng) == 0) {
			source.host.clear();
			if (one_or_zero(rng) == 0) {
				m_source_host_letter = "";
				source.host = generate_alphanum_string(1, 10);
			} else {
				m_source_host_letter = "@";
				source.host = generate_ascii_string(1, 20) + " " + generate_ascii_string(1, 20);
			}
			source_valid = false;
		}
	}

	return source;
}

std::string CommandBuilder::generate_spaces(size_t min_len, size_t max_len)
{
	std::size_t string_len = std::clamp(rand_strlen(rng), min_len, std::min(max_string_len, max_len));
	std::string string;
	string.reserve(string_len);

	for (size_t i = 0; i < string_len; i++)
		string += ' ';

	return string;
}

std::string CommandBuilder::generate_alpha_string(size_t min_len, size_t max_len)
{
	std::size_t string_len = std::clamp(rand_strlen(rng), min_len, std::min(max_string_len, max_len));
	std::string string;
	string.reserve(string_len);

	for (size_t i = 0; i < string_len; i++)
		string += alpha[rand_alpha(rng)];

	return string;
}

std::string CommandBuilder::generate_digits_string(size_t min_len, size_t max_len)
{
	std::size_t string_len = std::clamp(rand_strlen(rng), min_len, std::min(max_string_len, max_len));
	std::string string;
	string.reserve(string_len);

	for (size_t i = 0; i < string_len; i++)
		string += digit[rand_digits(rng)];

	return string;
}

static bool return_msg(const std::string& msg)
{
	CORE_ERROR("%s", msg.c_str());
	return false;
}

bool CommandBuilder::check_validity(const Command &command)
{
	auto& tags = command.get_tags();
	if (tags.size() != m_tags.size())
		return return_msg("tags vector size differ: got: [" + std::to_string(tags.size()) + "], expected: [" + std::to_string(m_tags.size()) + "]");
	for (size_t i = 0; i < tags.size(); i++) {
		if (tags[i].key.client_prefix != m_tags[i].key.client_prefix)
			return return_msg("tags client prefix differ: got: [" + tags[i].key.client_prefix + "], expected: [" + m_tags[i].key.client_prefix + "]");
		if (tags[i].key.vendor != m_tags[i].key.vendor)
			return return_msg("tags vendor differ: got: [" + tags[i].key.vendor + "], expected: [" + m_tags[i].key.vendor + "]");
		if (tags[i].key.key_str != m_tags[i].key.key_str)
			return return_msg("tags key str differ: got: [" + tags[i].key.key_str + "], expected: [" + m_tags[i].key.key_str + "]");
		if (tags[i].value != m_tags[i].value)
			return return_msg("tags value differ: got: [" + tags[i].value + "], expected: [" + m_tags[i].value + "]");
	}

	auto& source = command.get_source();
	if (source.source_name != m_source.source_name)
		return return_msg("source_name differ: got: [" + source.source_name + "], expected: [" + m_source.source_name + "]");
	if (source.user != m_source.user)
		return return_msg("source.user differ: got: [" + source.user + "], expected: [" + m_source.user + "]");
	if (source.host != m_source.host)
		return return_msg("source.host differ: got: [" + source.host + "], expected: [" + m_source.host + "]");

	auto& command_str = command.get_command();
	if (command_str != m_command)
		return return_msg("command str differ: got: [" + command_str + "], expected: [" + m_command + "]");

	auto& parameters = command.get_parameters();
	if (m_has_last_param && parameters.size() != m_parameters.size() + 1)
		return return_msg("parameters vector size differ: got:" + std::to_string(parameters.size()) + ", expected:" + std::to_string(m_parameters.size() + 1));
	if (!m_has_last_param && parameters.size() != m_parameters.size())
		return return_msg("parameters vector size differ: got:" + std::to_string(parameters.size()) + ", expected:" + std::to_string(m_parameters.size()));

	for (size_t i = 0; i < m_parameters.size(); i++) {
		if (parameters[i] != m_parameters[i])
			return return_msg("parameter str differ: got: [" + parameters[i] + "], expected: [" + m_parameters[i] + "]");
	}

	if (m_has_last_param && parameters[parameters.size() - 1] != m_last_parameter)
		return return_msg("parameter str differ: got: [" + parameters[parameters.size() - 1] + "], expected: [" + m_last_parameter + "]");

	return true;
}
