//
// Created by nathan on 3/7/23.
//

#ifndef PARAMSPLITTER_H
#define PARAMSPLITTER_H

#include <string>
#include "Command.h"

template<char separator>
class ParamSplitter
{
public:
	explicit ParamSplitter(const std::string& parameter);
	ParamSplitter(const Command& command, size_t command_parameter_index);
	ParamSplitter(bool condition, const Command& command, size_t command_parameter_index);

	std::string	next_param();
	std::string	peek_next_param()	{ std::string param = next_param(); reset(); return param; }
	bool		reached_end() const { return m_pos >= m_param.size(); }
	bool		failed()			{ return m_failed; }
	void		reset()				{ m_pos = 0; }

private:
	typedef std::string::size_type	index_type;

	index_type	find_separator();

	std::string	m_param;
	index_type	m_pos;
	bool		m_failed;
};

template<char separator>
ParamSplitter<separator>::ParamSplitter(const std::string &parameter)
	: m_param(parameter), m_pos(0), m_failed(false)
{
}

template<char separator>
ParamSplitter<separator>::ParamSplitter(const Command& command, size_t command_parameter_index)
	: m_param(command.get_parameters()[command_parameter_index]), m_pos(0), m_failed(false)
{
}
template<char separator>
ParamSplitter<separator>::ParamSplitter(bool condition, const Command& command, size_t command_parameter_index)
	: m_pos(0), m_failed(false)
{
	if (condition)
		m_param = command.get_parameters()[command_parameter_index];
}

template<char separator>
std::string ParamSplitter<separator>::next_param()
{
	if (reached_end()) {
		m_failed = true;
		return "";
	}

	index_type new_pos = find_separator();
	std::string new_param = m_param.substr(m_pos, new_pos - m_pos);
	m_pos = new_pos + 1;

	return new_param;
}

template<char separator>
typename ParamSplitter<separator>::index_type ParamSplitter<separator>::find_separator()
{
	index_type new_pos = m_pos;
	while (new_pos < m_param.size() && m_param[new_pos] != separator)
		new_pos ++;
	return new_pos;
}

#endif //PARAMSPLITTER_H
