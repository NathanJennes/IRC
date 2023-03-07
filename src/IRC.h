//
// Created by nathan on 2/16/23.
//

#ifndef IRC_H
#define IRC_H

#include "Server.h"

#define SERVER_SOURCE(numeric, user) 				(":" + Server::server_name() + " " + numeric + " " + user.nickname())
#define USER_SOURCE(command_or_numeric, user)		(":" + user.source() + " " + command_or_numeric)

#define RPL_CAP(user, command, msg) 		(SERVER_SOURCE("CAP", user) + " " + command + " :" + msg)
#define RPL_MESSAGE(user, command, msg) 	(SERVER_SOURCE(command, user) + " " + msg)

// RPL CODES
#define RPL_WELCOME(user)					(SERVER_SOURCE("001", user) + " :Welcome to the " + Server::network_name() + " Network, " + user.nickname())
#define RPL_YOURHOST(user)					(SERVER_SOURCE("002", user) + " :Your host is " + Server::server_name() + ", running version " SERVER_VERSION)
#define RPL_CREATED(user)					(SERVER_SOURCE("003", user) + " :This server was created " + Server::creation_date())
#define RPL_MYINFO(user)					(SERVER_SOURCE("004", user) + " " + Server::server_name() + " " SERVER_VERSION " " + Server::user_modes() + " " + Server::channel_modes() +  " " + Server::channel_modes_param())
#define RPL_ISUPPORT(user)					Server::supported_tokens(user)

#define RPL_BOUNCE	// RECOMMENDED BY THE RFC TO NOT BE USED
#define RPL_UMODEIS(user)								(SERVER_SOURCE("221", user) + " " + user.get_modes_as_str())

#define RPL_LUSERCLIENT(nbr_users, nbr_invisible, nbr_servers)	(" :There are " nbr_users " users and " nbr_invisible " invisible on " nbr_servers " servers")
#define RPL_LUSEROP(nbr_opers)									(" " nbr_opers " :operator(s) online")
#define RPL_LUSERUNKNOWN(nbr_unknown)							(" " nbr_unknown " :unknown connection(s)")
#define RPL_LUSERCHANNELS(nbr_channels)							(" " nbr_channels " :channels formed")
#define RPL_LUSERME(nbr_users, nbr_servers)						(" :I have " nbr_users " clients and " nbr_servers " servers")
#define RPL_ADMINME(servename)									(" " + servename + " :Administrative info")
#define RPL_ADMINLOC1(admin_location)							(" " admin_location)
#define RPL_ADMINLOC2(hosting_location)							(" " hosting_location)
#define RPL_ADMINEMAIL(admin_email)								(" " admin_email)
#define RPL_TRYAGAIN(command)									(" " command " :Please wait a while and try again.")
#define RPL_LOCALUSERS(current_users, max_users)				(" :Current local users " current_users " max " max_users)
#define RPL_GLOBALUSERS(current_users, max_users)				(" :Current global users " current_users " max " max_users)
#define RPL_WHOISCERTFP(nickname, certfp)						(" " + nickname + " :has client certificate fingerprint " certfp)
#define RPL_NONE												(" :Unknown format")

#define RPL_AWAY(user, target)									(SERVER_SOURCE("301", user), " " + target.nickname() + " :" + target.away_message())

#define RPL_USERHOST														// TODO : Implement function to return a list of userhost
#define RPL_UNAWAY															(":You are no longer marked as being away")
#define RPL_NOWAWAY															(":You have been marked as being away")
#define RPL_WHOREPLY(channel, username, hostname, servername, \
					 nickname, flags, hopcount, realname)					(" " channel " " username " " hostname " " + servername + " " + nickname + " " flags " :"hopcount " " realname)
#define RPL_ENDOFWHO(mask)													(" " mask " :End of /WHO list.")
#define RPL_WHOISREGNICK(nickname)											(" " + nickname + " :is a registered nick")
#define RPL_WHOISUSER(nickname, username, hostname, realname)				(" " + nickname + " " username " " hostname " * :" realname)
#define RPL_WHOISSERVER(nickname, servername, serverinfo)					(" " + nickname + " " + servername + " :" serverinfo)
#define RPL_WHOISOPERATOR(nickname)											(" " + nickname + " :is an IRC operator")
#define RPL_WHOWASUSER(nickname, username, hostname, realname)				(" " + nickname + " " username " " hostname " * :" realname)
#define RPL_WHOISIDLE(nickname, idle_time, signon)							(" " + nickname + " " idle_time " :seconds idle, " signon)
#define RPL_ENDOFWHOIS(nickname)											(" " + nickname + " :End of /WHOIS list.")
#define RPL_WHOISCHANNELS() 												""// TODO : Implement function to return a list of channels
#define RPL_WHOISSPECIAL(nickname, msg)										(" " + nickname + " :" msg)
#define RPL_LISTSTART(user)													(SERVER_SOURCE("321", user) + " Channel :Users Name")
#define RPL_LIST(user, channel)												(SERVER_SOURCE("322", user) + " " + (channel).name() + " " + (channel).user_count_as_str() + " :" + (channel).topic())
#define RPL_LISTEND(user)													(SERVER_SOURCE("323", user) + " :End of /LIST")
#define RPL_CHANNELMODEIS(user, channel)				(SERVER_SOURCE("324", user), " " + channel.name() + " : " + channel.get_modes_as_str(user))
#define RPL_CREATIONTIME 329
#define RPL_WHOISACCOUNT 330
#define RPL_NOTOPIC(user_that_modified, channel)		(":" + user_that_modified + " TOPIC " + (channel).name() + " :")
#define RPL_TOPIC(user_that_modified, channel)			(":" + user_that_modified + " TOPIC " + (channel).name() + " :" + (channel).topic())
#define RPL_TOPICWHOTIME(user,channel)					(SERVER_SOURCE("333", user) + " " + (channel).name() + " " + (channel).last_user_to_modify_topic() + " " + (channel).topic_modification_date_as_str())
#define RPL_INVITELIST 336
#define RPL_ENDOFINVITELIST 337
#define RPL_WHOISACTUALLY 338
#define RPL_INVITING 341
#define RPL_INVEXLIST 346
#define RPL_ENDOFINVEXLIST 347
#define RPL_EXCEPTLIST 348
#define RPL_ENDOFEXCEPTLIST 349
#define RPL_VERSION(user, comment)						(SERVER_SOURCE("351", user) + " " SERVER_VERSION " " + Server::server_name() + " :" + comment)

// TODO: channel.type() is not what is asked for: https://modern.ircdocs.horse/#rplversion-351
#define RPL_NAMREPLY(user, channel, channel_user, channel_user_perms)	(SERVER_SOURCE("353", user) + " " + channel.type() + " " + channel.name() + " :" + channel_user_perms.get_highest_prefix() + channel_user.nickname())
#define RPL_ENDOFNAMES(user, channel)					(SERVER_SOURCE("366", user) + " " + channel + " :End of /NAMES list.")

#define RPL_LINKS 364

#define RPL_ENDOFLINKS(user)							(SERVER_SOURCE("365", user) + " :End of /LINKS list.")
#define RPL_BANLIST(user, channel, ban_user)			(SERVER_SOURCE("367", user) + " " + channel.name() + " " + ban_user)
#define RPL_ENDOFBANLIST(user, channel)					(SERVER_SOURCE("368", user) + " " + channel.name() + " :End of channel ban list")
#define RPL_ENDOFWHOWAS(user)							(SERVER_SOURCE("369", user) + " :End of WHOWAS")

#define RPL_INFO 371

#define RPL_MOTD(user, motd)							(SERVER_SOURCE("372", user) + " :- " + motd)
#define RPL_ENDOFINFO(user)								(SERVER_SOURCE("374", user) + " :End of /INFO list.")
#define RPL_MOTDSTART(user)								(SERVER_SOURCE("375", user) + " :- " + Server::server_name() + " Message of the day - ")
#define RPL_ENDOFMOTD(user)								(SERVER_SOURCE("376", user) + " :End of /MOTD command.")

#define ERR_UNKNOWNERROR(ERRCODE)						(" " ERRCODE " :Unknown error")

#define ERR_NOSUCHNICK(user, nickname)					(SERVER_SOURCE("401", user) + " " + nickname + " :No such nick")
#define ERR_NOSUCHSERVER(user, servername)				(SERVER_SOURCE("402", user) + " " + servername + " :No such server")
#define ERR_NOSUCHCHANNEL(user, chan_name)				(SERVER_SOURCE("403", user) + " " + chan_name + " :No such channel")
#define ERR_CANNOTSENDTOCHAN(user, channel)				(SERVER_SOURCE("404", user) + " " + channel.name() + " :Cannot send to channel")
#define ERR_TOOMANYCHANNELS(user, channel)				(SERVER_SOURCE("405", user) + " " + channel + " :You have joined too many channels")
#define ERR_WASNOSUCHNICK(user, nickname)				(SERVER_SOURCE("406", user) + " " + nickname + " :There was no such nickname")
#define ERR_NOORIGIN(user)								(SERVER_SOURCE("409", user) + " :No origin specified")
#define ERR_INVALIDCAPCMD(user, command)				(SERVER_SOURCE("410", user) + " " + command + " :Invalid CAP command")
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

#define ERR_BADCHANMASK(channel)						(" " channel " :Bad Channel Mask")
#define ERR_NOPRIVILEGES								(" :Permission Denied - You're not an IRC operator")

#define ERR_CHANOPRIVSNEEDED(user, channel)				(SERVER_SOURCE("482", user) + " " + channel + " :You're not channel operator")

#define ERR_CANTKILLSERVER								(" :You cant kill a server!")
#define ERR_NOOPERHOST									(" :No O-lines for your host")

#define ERR_UMODEUNKNOWNFLAG(user, mode)				(SERVER_SOURCE("501", user), + " :Unknown MODE flag " + mode)
#define ERR_USERSDONTMATCH(user)						(SERVER_SOURCE("502", user), + " :Can't change mode for other users")

#define ERR_HELPNOTFOUND(topic)							(" " topic " :No help available on this topic")
#define ERR_INVALIDKEY(target_chan)						(" " target_chan " :Key is not well-formed")
#define RPL_STARTTLS									(" :STARTTLS successful, proceed with TLS handshake")
#define RPL_WHOISSECURE(nickname)						(" " + nickname + " :is using a secure connection")
#define ERR_STARTTLS(msg)								(" :STARTTLS failed, (" msg ")")
#define ERR_INVALIDMODEPARAM
#define RPL_HELPSTART 704
#define RPL_HELPTXT 705
#define RPL_ENDOFHELP 706

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

#endif //IRC_H
