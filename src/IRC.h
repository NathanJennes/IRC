//
// Created by nathan on 2/16/23.
//

#ifndef IRC_H
#define IRC_H

#include "Server.h"

#define SOURCE(numeric, user) (":" + Server::server_name() + " " + numeric + " " + user.nickname())

#define RPL_CAP(user, command, msg) (SOURCE("CAP", user) + " " + command + msg)
#define RPL_MESSAGE(user, command, msg) (SOURCE(command, user) + " " + msg)

// RPL CODES
#define RPL_WELCOME(user)										(SOURCE("001", user) + " :Welcome to the " + Server::network_name() + " Network, " + user.nickname())
#define RPL_YOURHOST(user)										(SOURCE("002", user) + " :Your host is " + Server::server_name() + ", running version " SERVER_VERSION)
#define RPL_CREATED(user)										(SOURCE("003", user) + " :This server was created " + Server::creation_date())
#define RPL_MYINFO(user)										(SOURCE("004", user) + " " + Server::server_name() + " " SERVER_VERSION " " + Server::user_modes() + " " + Server::channel_modes() +  " " + Server::channel_modes_param())
#define RPL_ISUPPORT(user, tokens)								(SOURCE("005", user) + " " + tokens +" :are supported by this server")

#define RPL_BOUNCE	// RECOMMENDED BY THE RFC TO NOT BE USED
#define RPL_UMODEIS(set_usermodes)								(" " set_usermodes)

#define RPL_LUSERCLIENT(nbr_users, nbr_invisible, nbr_servers)				(" :There are " nbr_users " users and " nbr_invisible " invisible on " nbr_servers " servers")
#define RPL_LUSEROP(nbr_opers)												(" " nbr_opers " :operator(s) online")
#define RPL_LUSERUNKNOWN(nbr_unknown)										(" " nbr_unknown " :unknown connection(s)")
#define RPL_LUSERCHANNELS(nbr_channels)										(" " nbr_channels " :channels formed")
#define RPL_LUSERME(nbr_users, nbr_servers)									(" :I have " nbr_users " clients and " nbr_servers " servers")
#define RPL_ADMINME(servename)												(" " + servename + " :Administrative info")
#define RPL_ADMINLOC1(admin_location)										(" " admin_location)
#define RPL_ADMINLOC2(hosting_location)										(" " hosting_location)
#define RPL_ADMINEMAIL(admin_email)											(" " admin_email)
#define RPL_TRYAGAIN(command)												(" " command " :Please wait a while and try again.")
#define RPL_LOCALUSERS(current_users, max_users)							(" :Current local users " current_users " max " max_users)
#define RPL_GLOBALUSERS(current_users, max_users)							(" :Current global users " current_users " max " max_users)
#define RPL_WHOISCERTFP(nickname, certfp)									(" " + nickname + " :has client certificate fingerprint " certfp)
#define RPL_NONE															(" :Unknown format")
#define RPL_AWAY(nickname, away_message)									(" " + nickname + " :" away_message)
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
#define RPL_LISTSTART 321
#define RPL_LIST 322
#define RPL_LISTEND 323
#define RPL_CHANNELMODEIS 324
#define RPL_CREATIONTIME 329
#define RPL_WHOISACCOUNT 330
#define RPL_NOTOPIC 331
#define RPL_TOPIC(user, channel)						(SOURCE("332", user) + " " + channel.name() + " :" + channel.topic())
#define RPL_TOPICWHOTIME 333
#define RPL_INVITELIST 336
#define RPL_ENDOFINVITELIST 337
#define RPL_WHOISACTUALLY 338
#define RPL_INVITING 341
#define RPL_INVEXLIST 346
#define RPL_ENDOFINVEXLIST 347
#define RPL_EXCEPTLIST 348
#define RPL_ENDOFEXCEPTLIST 349
#define RPL_VERSION 351
#define RPL_NAMREPLY(user, channel, prefix)				(SOURCE("353", user) + " " + channel.type() + " " + channel.name() + " :" + prefix + user.nickname())
#define RPL_ENDOFNAMES(user, channel)					(SOURCE("366", user) + " " + channel.name() + " :End of /NAMES list.")
#define RPL_LINKS 364
#define RPL_ENDOFLINKS 365
#define RPL_BANLIST 367
#define RPL_ENDOFBANLIST 368
#define RPL_ENDOFWHOWAS 369
#define RPL_INFO 371
#define RPL_ENDOFINFO 374
#define ERR_UNKNOWNERROR(ERRCODE)						(" " ERRCODE " :Unknown error")
#define ERR_NOSUCHNICK(nickname)						(" " + nickname + " :No such nick/channel")
#define ERR_NOSUCHSERVER(servername)					(" " + servername + " :No such server")

#define ERR_NOSUCHCHANNEL(user, channel)				(SOURCE("403", user) + " " + channel + " :No such channel")
#define ERR_CANNOTSENDTOCHAN(user, channel)				(SOURCE("404", user) + " " + channel + " :Cannot send to channel")
#define ERR_TOOMANYCHANNELS(user, channel)				(SOURCE("405", user) + " " + channel + " :You have joined too many channels")

#define ERR_WASNOSUCHNICK(nickname)						(" " + nickname + " :There was no such nickname")
#define ERR_NOORIGIN									(" :No origin specified") // 409

#define ERR_INVALIDCAPCMD(user, command)				(SOURCE("410", user) + " " + command + " :Invalid CAP command")

#define ERR_INPUTTOOLONG								(" :Input too long")
#define ERR_UNKNOWNCOMMAND(command)						(" " + command + " :Unknown command")
#define ERR_NOMOTD										(" :MOTD File is missing")

#define ERR_NONICKNAMEGIVEN(user)						(SOURCE("431", user) + " :No nickname given")
#define ERR_ERRONEUSNICKNAME(user, new_nick)			(SOURCE("431", user) + " " + new_nick + " :Erroneous nickname")
#define ERR_NICKNAMEINUSE(user, new_nick)				(SOURCE("432", user) + " " + new_nick + " :Nickname is already in use")

#define ERR_USERNOTINCHANNEL(nickname, channel)			(" " + nickname + " " channel " :They aren't on that channel")
#define ERR_NOTONCHANNEL(channel)						(" " channel " :You're not on that channel")
#define ERR_USERONCHANNEL(nickname, channel)			(" " + nickname + " " channel " :is already on channel")
#define ERR_NOTREGISTERED(user)							(SOURCE("451", user) + " :You have not registered")

#define ERR_NEEDMOREPARAMS(user, command)				(SOURCE("461", user) + " " + command.get_command() + " :Not enough parameters")
#define ERR_ALREADYREGISTERED(user)						(SOURCE("462", user) + " :You may not registered")

#define ERR_PASSWDMISMATCH(user)						(SOURCE("464", user) + " :Password incorrect")
#define ERR_YOUREBANNEDCREEP							(" :You are banned from this server")
#define ERR_CHANNELISFULL(user, channel)				(SOURCE("471", user) + " " + channel + " :Cannot join channel (+l)")
#define ERR_UNKNOWNMODE(user, mode)						(SOURCE("472", user) + " " + mode + " :is unknown mode char to me")
#define ERR_INVITEONLYCHAN(user, channel)				(SOURCE("473", user) + " " + channel + " :Cannot join channel (+i)")
#define ERR_BANNEDFROMCHAN(user, channel)				(SOURCE("474", user) + " " + channel + " :Cannot join channel (+b)")
#define ERR_BADCHANNELKEY(user, channel)				(SOURCE("475", user) + " " + channel + " :Cannot join channel (+k)")
#define ERR_BADCHANMASK(channel)						(" " channel " :Bad Channel Mask")
#define ERR_NOPRIVILEGES								(" :Permission Denied- You're not an IRC operator")
#define ERR_CHANOPRIVSNEEDED(channel)					(" " channel " :You're not channel operator")
#define ERR_CANTKILLSERVER								(" :You cant kill a server!")
#define ERR_NOOPERHOST									(" :No O-lines for your host")
#define ERR_UMODEUNKNOWNFLAG							(" :Unknown MODE flag")
#define ERR_USERSDONTMATCH								(" :Cant change mode for other users")
#define ERR_HELPNOTFOUND(topic)							(" " topic " :No help available on this topic")
#define ERR_INVALIDKEY(target_chan)						(" " target_chan " :Key is not well-formed")
#define RPL_STARTTLS									(" :STARTTLS successful, proceed with TLS handshake")
#define RPL_WHOISSECURE(nickname)						(" " + nickname + " :is using a secure connection")
#define ERR_STARTTLS(msg)								(" :STARTTLS failed, (" msg ")")
#define ERR_INVALIDMODEPARAM
#define RPL_HELPSTART 704
#define RPL_HELPTXT 705
#define RPL_ENDOFHELP 706
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
