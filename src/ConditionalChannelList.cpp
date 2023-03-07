//
// Created by nathan on 3/7/23.
//

#include <algorithm>
#include "ConditionalChannelList.h"
#include "Server.h"

ConditionalChannelList::ConditionalChannelList()
	: m_channels(), m_current_time(time(NULL))
{
}

void ConditionalChannelList::load_from_server()
{
	for (Server::ChannelIterator channel_it = Server::channels().begin(); channel_it != Server::channels().end(); channel_it++)
		m_channels.push_back(channel_it->second);
}

void ConditionalChannelList::keep_if_user_count_less_than(std::size_t user_count)
{
	for (ChannelIterator channel_it = m_channels.begin(); channel_it != m_channels.end();) {
		if (get_channel_reference(channel_it).user_count() >= user_count)
			channel_it = m_channels.erase(channel_it);
		else
			channel_it++;
	}
}

void ConditionalChannelList::keep_if_user_count_greater_than(std::size_t user_count)
{
	for (ChannelIterator channel_it = m_channels.begin(); channel_it != m_channels.end();) {
		if (get_channel_reference(channel_it).user_count() <= user_count)
			channel_it = m_channels.erase(channel_it);
		else
			channel_it++;
	}
}

void ConditionalChannelList::keep_if_created_less_than(std::time_t minutes_ago)
{
	for (ChannelIterator channel_it = m_channels.begin(); channel_it != m_channels.end();) {
		if (get_channel_reference(channel_it).creation_date() + (minutes_ago * 60) < m_current_time)
			channel_it = m_channels.erase(channel_it);
		else
			channel_it++;
	}
}

void ConditionalChannelList::keep_if_created_greater_than(std::time_t minutes_ago)
{
	for (ChannelIterator channel_it = m_channels.begin(); channel_it != m_channels.end();) {
		if (get_channel_reference(channel_it).creation_date() + (minutes_ago * 60) > m_current_time)
			channel_it = m_channels.erase(channel_it);
		else
			channel_it++;
	}
}

void ConditionalChannelList::keep_if_topic_changed_less_than(std::time_t minutes_ago)
{
	for (ChannelIterator channel_it = m_channels.begin(); channel_it != m_channels.end();) {
		CORE_DEBUG("channel %s topic modif date %s, current date %s, difference %s", get_channel_reference(channel_it).name().c_str(),
			get_channel_reference(channel_it).topic_modification_date_as_str().c_str(), to_string(m_current_time).c_str(), to_string(
			m_current_time - get_channel_reference(channel_it).topic_modification_date()).c_str());
		if (get_channel_reference(channel_it).topic_modification_date() + (minutes_ago * 60) < m_current_time)
			channel_it = m_channels.erase(channel_it);
		else
			channel_it++;
	}
}

void ConditionalChannelList::keep_if_topic_changed_greater_than(std::time_t minutes_ago)
{
	for (ChannelIterator channel_it = m_channels.begin(); channel_it != m_channels.end();) {
		CORE_DEBUG("channel %s topic modif date %s, current date %s, difference %s", get_channel_reference(channel_it).name().c_str(),
			get_channel_reference(channel_it).topic_modification_date_as_str().c_str(), to_string(m_current_time).c_str(), to_string(
			m_current_time - get_channel_reference(channel_it).topic_modification_date()).c_str());
		if (get_channel_reference(channel_it).topic_modification_date() + (minutes_ago * 60) > m_current_time)
			channel_it = m_channels.erase(channel_it);
		else
			channel_it++;
	}
}
