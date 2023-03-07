//
// Created by nathan on 3/7/23.
//

#ifndef CONDITIONALCHANNELLIST_H
#define CONDITIONALCHANNELLIST_H

#include "Channel.h"

struct ConditionalChannelList
{
public:
	ConditionalChannelList();

	void add_channel(Channel *new_channel) { m_channels.push_back(new_channel); }
	void load_from_server();

	void keep_if_user_count_less_than(std::size_t user_count);
	void keep_if_user_count_greater_than(std::size_t user_count);
	void keep_if_created_less_than(std::time_t minutes_ago);
	void keep_if_created_greater_than(std::time_t minutes_ago);
	void keep_if_topic_changed_less_than(std::time_t minutes_ago);
	void keep_if_topic_changed_greater_than(std::time_t minutes_ago);

	const std::vector<Channel*>& channels() const { return m_channels; };

private:
	typedef std::vector<Channel*>	ChannelVector;
	typedef ChannelVector::iterator	ChannelIterator;

	Channel& get_channel_reference(ChannelIterator chan_it) const { return **chan_it; }

	ChannelVector					m_channels;
	std::time_t						m_current_time;
};

#endif //CONDITIONALCHANNELLIST_H
