//
// Created by nathan on 2/16/23.
//

#ifndef NUMERICS_H
#define NUMERICS_H

#include "Server.h"

#define SERVER_SOURCE(numeric, user) 					(":" + Server::info().name() + " " + numeric + " " + user.nickname())
#define USER_SOURCE(command_or_numeric, user)			(":" + (user).source() + " " + command_or_numeric)

#define RPL_CAP(user, command, msg) 					(SERVER_SOURCE("CAP", user) + " " + command + " :" + msg)
#define RPL_MODE(user, msg) 							(USER_SOURCE("MODE", user) + " " + (user).nickname() + " " + msg)
#define RPL_MESSAGE(user, command, msg) 				(SERVER_SOURCE(command, user) + " " + msg)

// RPL CODES
#define RPL_WELCOME(user)								(SERVER_SOURCE("001", user) + " :Welcome to the " + Server::info().network_name() + " Network, " + user.nickname())
#define RPL_YOURHOST(user)								(SERVER_SOURCE("002", user) + " :Your host is " + Server::info().name() + ", running version " + Server::info().version())
#define RPL_CREATED(user)								(SERVER_SOURCE("003", user) + " :This server was created " + Server::info().creation_date())
#define RPL_MYINFO(user)								(SERVER_SOURCE("004", user) + " " + Server::info().name() + " " + Server::info().version() + " " + "<user modes>" + " " + "<channel modes>" +  " " + "<channel mode param>") //TODO : add user and channel modes
#define RPL_ISUPPORT(user)								Server::supported_tokens(user)
#define RPL_STATSLINKINFO(user)							(SERVER_SOURCE("211", user) + " " + user.nickname() + "[" + user.username() + "@" + user.ip() + "] " \
														+ to_string(user.write_buffer().size()) + " " + to_string(user.sent_messages_count()) + " " + to_string(user.data_sent_size() / 1000) \
														+ " " + to_string(user.received_messages_count()) + " " + to_string(user.data_received_size() / 1000) \
														+ " " + to_string(user.time_connexion_open()))
#define RPL_STATSCOMMANDS(user, command_stats)			(SERVER_SOURCE("212", user) + " " + command_stats->first + " " + to_string(command_stats->second) + " 0 :0")
#define RPL_ENDOFSTATS(user, query_char)				(SERVER_SOURCE("219", user) + " " + query_char + " :End of STATS report")
#define RPL_UMODEIS(user)								(SERVER_SOURCE("221", user) + " " + user.get_modes_as_str())

#define RPL_STATSUPTIME(user, seconds)					(SERVER_SOURCE("242", user) + " :Server Up " + to_string(seconds / 86400) + " days " + to_string(seconds / 3600) + ":" + to_string(seconds / 60) + ":" + to_string(seconds % 60))

#define RPL_LUSERCLIENT(user, current, nbr_invisible)	(SERVER_SOURCE("251", user) + " :There are " + current + " users and " + nbr_invisible + " invisible on 1 server")
#define RPL_LUSEROP(user, nbr_operator)					(SERVER_SOURCE("252", user) + " " + nbr_operator + " :IRC operator(s) online")
#define RPL_LUSERUNKNOWN(user, unknown)					(SERVER_SOURCE("253", user) + " " + unknown + " :unknown connection(s)")
#define RPL_LUSERCHANNELS(user, nbr_channel)			(SERVER_SOURCE("254", user) + " " + nbr_channel + " :channels formed")
#define RPL_LUSERME(user, current)						(SERVER_SOURCE("255", user) + " :I have " + current + " clients and " + "1" + " servers")

#define RPL_ADMINME(user, server_name)					(SERVER_SOURCE("256", user) + " " + server_name + " :Administrative info")
#define RPL_ADMINLOC1(user, server_location)			(SERVER_SOURCE("257", user) + " :Server location - " + server_location)
#define RPL_ADMINLOC2(user, hosting_info)				(SERVER_SOURCE("258", user) + " :Hosting location - " + hosting_info)
#define RPL_ADMINEMAIL(user, admin_info)				(SERVER_SOURCE("259", user) + " :" + admin_info)
#define RPL_TRYAGAIN(user, command)						(SERVER_SOURCE("263", user) + " " + command + " :Please wait a while and try again.")

#define RPL_LOCALUSERS(user, current, max)				(SERVER_SOURCE("265", user) + " " + current + " " + max + " :Current local users " + current + ", max " + max)
#define RPL_GLOBALUSERS(user, current, max)				(SERVER_SOURCE("266", user) + " " + current + " " + max + " :Current global users " + current + ", max " + max)
#define RPL_WHOISCERTFP(user, target)					//NO SSL connection for now

#define RPL_NONE										(" :Unknown format")
#define RPL_AWAY(user, target)							(SERVER_SOURCE("301", user) + " " + target.nickname() + " :" + target.away_message())
#define RPL_USERHOST									// TODO : Implement function to return a list of userhost
#define RPL_UNAWAY(user)								(SERVER_SOURCE("305", user) + " :You are no longer marked as being away")
#define RPL_NOWAWAY(user)								(SERVER_SOURCE("306", user) + " :You have been marked as being away")
#define RPL_WHOREPLY(user, target, channel, flags)		(SERVER_SOURCE("352", user) + " " + channel + " " + target.username() + " " + target.hostname() + " " + Server::info().name() + " " + target.nickname() + " " + flags + " :" + "0 " + target.realname())
#define RPL_ENDOFWHO(user, mask)						(SERVER_SOURCE("315", user) + " " + mask + " :End of /WHO list.")
#define RPL_WHOISREGNICK(nickname)						// no account for now
#define RPL_WHOISUSER(user, target)						(SERVER_SOURCE("311", user) + " " + target.nickname() + " " + target.username() + " " + target.hostname() + " * :" + target.realname())
#define RPL_WHOISSERVER(user, target, serverinfo)		(SERVER_SOURCE("312", user) + " " + target.nickname() + " " + serverinfo.name() + " :" + serverinfo.description())
#define RPL_WHOISOPERATOR(user, target)					(SERVER_SOURCE("313", user) + " " + target.nickname() + " :is an IRC operator")
#define RPL_WHOWASUSER(user, old_user)					(SERVER_SOURCE("314", user) + " " + (old_user).nickname() + " " + (old_user).username() + " " + (old_user).host() + " * :" + (old_user).realname())
#define RPL_WHOISIDLE(user, target)						(SERVER_SOURCE("317", user) + " " + target.nickname() + " " + target.seconde_idle() + " " + target.signon() + " :seconds seconde_idle")
#define RPL_ENDOFWHOIS(user, target)					(SERVER_SOURCE("318", user) + " " + target.nickname() + " :End of /WHOIS list.")
#define RPL_WHOISCHANNELS(user, target) 				target.reply_list_of_channel_to_user(user)
#define RPL_WHOISSPECIAL(user, target, msg)				(SERVER_SOURCE("321", user) + " " + target.nickname() + " :" msg)
#define RPL_LISTSTART(user)								(SERVER_SOURCE("321", user) + " Channel :Users Name")
#define RPL_LIST(user, channel)							(SERVER_SOURCE("322", user) + " " + (channel).name() + " " + (channel).user_count_as_str() + " :" + (channel).topic())
#define RPL_LISTEND(user)								(SERVER_SOURCE("323", user) + " :End of /LIST")
#define RPL_CHANNELMODEIS(user, channel)				(SERVER_SOURCE("324", user) + " " + channel.name() + " " + channel.get_modes_as_str(user))
#define RPL_CREATIONTIME(user, channel)					(SERVER_SOURCE("329", user) + " " + channel.name() + " " + channel.creation_date_as_str())
#define RPL_WHOISACCOUNT(user, target)					(SERVER_SOURCE("330", user) + " " + target.nickname() + " " + "target.account()" + " :is logged in as")
#define RPL_NOTOPIC(user_that_modified, channel)		(":" + user_that_modified + " TOPIC " + (channel).name() + " :")
#define RPL_TOPIC(user_that_modified, channel)			(":" + user_that_modified + " TOPIC " + (channel).name() + " :" + (channel).topic())
#define RPL_TOPICWHOTIME(user,channel)					(SERVER_SOURCE("333", user) + " " + (channel).name() + " " + (channel).last_user_to_modify_topic() + " " + (channel).topic_modification_date_as_str())
#define RPL_INVITELIST(user, channel)					(SERVER_SOURCE("336", user) + " " + channel)
#define RPL_ENDOFINVITELIST(user, channel)				(SERVER_SOURCE("337", user) + " " + channel + " :End of channel invite list")
#define RPL_WHOISACTUALLY(user, target)					(SERVER_SOURCE("338", user) + " " + target.nickname() + " " + target.ip() + " :Is actually using host")
#define RPL_INVITING(user, nick, channel_name)			(SERVER_SOURCE("341", user) + " " + nick + " " + channel_name)
#define RPL_INVEXLIST(user, channel, mask)				(SERVER_SOURCE("346", user) + " " + channel.name() + " " + mask)
#define RPL_ENDOFINVEXLIST(user, channel)				(SERVER_SOURCE("347", user) + " " + channel.name() + " :End of channel invite list")
#define RPL_EXCEPTLIST(user, channel, mask)				(SERVER_SOURCE("348", user) + " " + channel.name() + " " + mask)
#define RPL_ENDOFEXCEPTLIST(user, channel)				(SERVER_SOURCE("349", user) + " " + channel.name() + " :End of channel exception list")
#define RPL_VERSION(user, comment)						(SERVER_SOURCE("351", user) + " " + Server::info().version() + " " + Server::info().name() + " :" + comment)

// TODO: channel.type() is not what is asked for: https://modern.ircdocs.horse/#rplversion-351
#define RPL_NAMREPLY(user, channel, channel_user, channel_user_perms)	(SERVER_SOURCE("353", user) + " " + channel.status() + " " + channel.name() + " :" + channel_user_perms.get_highest_prefix() + channel_user.nickname())
#define RPL_ENDOFNAMES(user, channel)					(SERVER_SOURCE("366", user) + " " + channel + " :End of /NAMES list.")

#define RPL_LINKS 364

#define RPL_ENDOFLINKS(user)							(SERVER_SOURCE("365", user) + " :End of /LINKS list.")
#define RPL_BANLIST(user, channel, ban_user)			(SERVER_SOURCE("367", user) + " " + channel.name() + " " + ban_user)
#define RPL_ENDOFBANLIST(user, channel)					(SERVER_SOURCE("368", user) + " " + channel.name() + " :End of channel ban list")
#define RPL_ENDOFWHOWAS(user)							(SERVER_SOURCE("369", user) + " :End of WHOWAS")
#define RPL_INFO(user, info)							(SERVER_SOURCE("371", user) + " :" + info)
#define RPL_MOTD(user, motd)							(SERVER_SOURCE("372", user) + " :- " + motd)
#define RPL_ENDOFINFO(user)								(SERVER_SOURCE("374", user) + " :End of /INFO list.")
#define RPL_MOTDSTART(user)								(SERVER_SOURCE("375", user) + " :- " + Server::info().name() + " Message of the day - ")
#define RPL_ENDOFMOTD(user)								(SERVER_SOURCE("376", user) + " :End of /MOTD command.")
#define ERR_UNKNOWNERROR(ERRCODE)						(" " + ERRCODE + " :Unknown error")
#define RPL_TIME(user, Server_name, time)				(SERVER_SOURCE("391", user) + " " + Server_name + " " + time + " :")
#define ERR_NOSUCHNICK(user, nickname)					(SERVER_SOURCE("401", user) + " " + nickname + " :No such nick")
#define ERR_NOSUCHSERVER(user, servername)				(SERVER_SOURCE("402", user) + " " + servername + " :No such server")
#define ERR_NOSUCHCHANNEL(user, chan_name)				(SERVER_SOURCE("403", user) + " " + chan_name + " :No such channel")
#define ERR_CANNOTSENDTOCHAN(user, channel)				(SERVER_SOURCE("404", user) + " " + channel.name() + " :Cannot send to channel")
#define ERR_TOOMANYCHANNELS(user, channel)				(SERVER_SOURCE("405", user) + " " + channel + " :You have joined too many channels")
#define ERR_WASNOSUCHNICK(user, nickname)				(SERVER_SOURCE("406", user) + " " + nickname + " :There was no such nickname")
#define ERR_NOORIGIN(user)								(SERVER_SOURCE("409", user) + " :No origin specified")
#define ERR_INVALIDCAPCMD(user, command)				(SERVER_SOURCE("410", user) + " " + command + " :Invalid CAP command")
#define ERR_NOTEXTTOSEND(user)							(SERVER_SOURCE("412", user) + " :No text to send")
#define ERR_INPUTTOOLONG(user)							(SERVER_SOURCE("414", user) + " :Input too long")
#define ERR_UNKNOWNCOMMAND(user, command)				(SERVER_SOURCE("421", user) + " " + command + " :Unknown command")
#define ERR_NOMOTD(user)								(SERVER_SOURCE(user, "422") + " :MOTD File is missing")
#define ERR_NONICKNAMEGIVEN(user)						(SERVER_SOURCE("431", user) + " :No nickname given")
#define ERR_ERRONEUSNICKNAME(user, new_nick)			(SERVER_SOURCE("431", user) + " " + new_nick + " :Erroneous nickname")
#define ERR_NICKNAMEINUSE(user, new_nick)				(SERVER_SOURCE("432", user) + " " + new_nick + " :Nickname is already in use")
#define ERR_USERNOTINCHANNEL(user, nickname, channel)	(SERVER_SOURCE("441", user) + " " + nickname + " " + channel.name() + " :They aren't on that channel")
#define ERR_NOTONCHANNEL(user, channel)					(SERVER_SOURCE("442", user) + " " + channel.name() + " :You're not on that channel")
#define ERR_USERONCHANNEL(user, nickname, channel)		(SERVER_SOURCE("443", user) + " " + nickname + " " + channel.name() + " :is already on channel")
#define ERR_NOTREGISTERED(user)							(SERVER_SOURCE("451", user) + " :You have not registered")
#define ERR_NEEDMOREPARAMS(user, command)				(SERVER_SOURCE("461", user) + " " + command.get_command() + " :Not enough parameters")
#define ERR_ALREADYREGISTERED(user)						(SERVER_SOURCE("462", user) + " :You may not registered")
#define ERR_PASSWDMISMATCH(user)						(SERVER_SOURCE("464", user) + " :Password incorrect")
#define ERR_YOUREBANNEDCREEP(user)						(SERVER_SOURCE("465", user) + " :You are banned from this server")
#define ERR_CHANNELISFULL(user, channel)				(SERVER_SOURCE("471", user) + " " + channel + " :Cannot join channel (Channel full)")
#define ERR_UNKNOWNMODE(user, mode)						(SERVER_SOURCE("472", user) + " " +   mode  + " :is unknown mode character")
#define ERR_INVITEONLYCHAN(user, channel)				(SERVER_SOURCE("473", user) + " " + channel + " :Cannot join channel (invite-only)")
#define ERR_BANNEDFROMCHAN(user, channel)				(SERVER_SOURCE("474", user) + " " + channel + " :Cannot join channel (you're banned)")
#define ERR_BADCHANNELKEY(user, channel)				(SERVER_SOURCE("475", user) + " " + channel + " :Cannot join channel (Wrong key)")
#define ERR_BADCHANMASK(user, mask)						(SERVER_SOURCE("476", user) + " " + mask + " :Bad Channel Mask")
#define ERR_ILLCHANNAME(user, channel)					(SERVER_SOURCE("479", user) + " " + channel + " :Illegal channel name")
#define ERR_NOPRIVILEGES(user)							(SERVER_SOURCE("481", user) + " :Permission Denied- You're not an IRC operator")
#define ERR_CHANOPRIVSNEEDED(user, channel)				(SERVER_SOURCE("482", user) + " " + channel.name() + " :You're not channel operator")

#define ERR_CANTKILLSERVER								(" :You cant kill a server!")
#define ERR_NOOPERHOST									(" :No O-lines for your host")

#define ERR_UMODEUNKNOWNFLAG(user, mode)				(SERVER_SOURCE("501", user), + " :Unknown MODE flag " + mode)
#define ERR_USERSDONTMATCH(user)						(SERVER_SOURCE("502", user), + " :Can't change mode for other users")
#define ERR_HELPNOTFOUND(user, topic)					(SERVER_SOURCE("524", user) + " " + topic + " :No help for " + topic)

// TSL/SSL replies
#define ERR_INVALIDKEY(target_chan)						(" " target_chan " :Key is not well-formed")
#define RPL_STARTTLS									(" :STARTTLS successful, proceed with TLS handshake")
#define RPL_WHOISSECURE(nickname)						(" " + nickname + " :is using a secure connection")
#define ERR_STARTTLS(msg)								(" :STARTTLS failed, (" msg ")")

#define ERR_INVALIDMODEPARAM(user, mode, param)			(SERVER_SOURCE("717", user) + " " + mode + " " + param + " :is unknown mode parameter")
#define RPL_HELPSTART(user, subject, first_line)		(SERVER_SOURCE("704", user) + " " + subject + " :" + first_line)
#define RPL_HELPTXT(user, subject, line)				(SERVER_SOURCE("705", user) + " " + subject + " :" + line)
#define RPL_ENDOFHELP(user, subject, last_line)			(SERVER_SOURCE("706", user) + " " + subject + " :" + last_line)

// SASL replies
#define ERR_NOPRIVS(priv)								(" " priv " :Insufficient oper privileges.")
#define RPL_LOGGEDIN(nickname, username, account)		(" " + nickname + " " account " :You are now logged in as " username)
#define RPL_LOGGEDOUT(nickname)							(" " + nickname + " :You are now logged out")
#define ERR_NICKLOCKED									(" :You must use a nick assigned to you")
#define RPL_SASLSUCCESS									(" :SASL authentication successful")
#define ERR_SASLFAIL									(" :SASL authentication failed")
#define ERR_SASLTOOLONG									(" :SASL message too long")
#define ERR_SASLABORTED									(" :SASL authentication aborted")
#define ERR_SASLALREADY									(" :You have already authenticated using SASL")
#define RPL_SASLMECHS(mechs)							(" " mechs " :are available SASL mechanisms")

#endif //NUMERICS_H
