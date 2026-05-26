/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Reply.cpp                                          :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: zayminmaw <zayminmaw@student.42.fr>        +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/05/07 12:00:00 by zmin              #+#    #+#             */
/*   Updated: 2026/05/26 19:39:30 by zayminmaw        ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "Reply.hpp"

static std::string _fmt(const std::string &sv, const std::string &nr, const std::string &n) {
	return ":" + sv + " " + nr + " " + (n.empty() ? "*" : n) + " ";	
}

std::string Reply::welcome(const std::string& serverName, const std::string& nick) {
	return _fmt(serverName, "001", nick) + ":Welcome to the Internet Relay Network " + nick;
}

std::string Reply::yourHost(const std::string& serverName, const std::string& nick) {
	return _fmt(serverName, "002", nick) + ":Your host is " + serverName + ", running version 1.0";
}

std::string Reply::created(const std::string& serverName, const std::string& nick) {
	return _fmt(serverName, "003", nick) + ":This server was created " + "May 2026";
}

std::string Reply::myInfo(const std::string& serverName, const std::string& nick) {
	return _fmt(serverName, "004", nick) + serverName + " 1.0 o itkol";
}

std::string Reply::topic(const std::string& serverName, const std::string& nick, const std::string& chan, const std::string& topic) {
	return _fmt(serverName, "332", nick) + chan + " :" + topic; 
}

std::string Reply::noTopic(const std::string& serverName, const std::string& nick, const std::string& chan) {
	return _fmt(serverName, "331", nick) +  chan + " :No topic is set";
}

std::string Reply::channelModeIs(const std::string& serverName, const std::string& nick, const std::string& chan, const std::string& modes, const std::string& args) {
	return _fmt(serverName, "324", nick) + chan + " " + modes + (args.empty() ? "" : " " + args);
}

std::string Reply::rplInviting(const std::string& serverName, const std::string& nick, const std::string& target, const std::string& chan) {
	return _fmt(serverName, "341", nick) + target + " " + chan;
}

std::string Reply::namReply(const std::string& serverName, const std::string& nick, const std::string& chan, const std::string& names) {
	return _fmt(serverName, "353", nick) + "= " + chan + " :" + names;
}

std::string Reply::endOfNames(const std::string& serverName, const std::string& nick, const std::string& chan) {
	return _fmt(serverName, "366", nick) + chan + " :End of /NAMES list";
}

std::string Reply::errNoSuchNick(const std::string& serverName, const std::string& nick, const std::string& target) {
	return _fmt(serverName, "401", nick) + target + " :No such nick/channel";
}

std::string Reply::errNoSuchChannel(const std::string& serverName, const std::string& nick, const std::string& chan) {
	return _fmt(serverName, "403", nick) + chan + " :No such channel";
}

std::string Reply::errCannotSendToChan(const std::string& serverName, const std::string& nick, const std::string& chan) {
	return _fmt(serverName, "404", nick) + chan + " :Cannot send to channel";
}

std::string Reply::errNoRecipient(const std::string& serverName, const std::string& nick, const std::string& cmd) {
	return _fmt(serverName, "411", nick) + ":No recipient given (" + cmd + ")";
}

std::string Reply::errNoTextToSend(const std::string& serverName, const std::string& nick) {
	return _fmt(serverName, "412", nick) + ":No text to send";
}

std::string Reply::errUnknownCommand(const std::string& serverName, const std::string& nick, const std::string& cmd) {
	return _fmt(serverName, "421", nick) + cmd + " :Unknown command";
}

std::string Reply::errNoNicknameGiven(const std::string& serverName, const std::string& nick) {
	return _fmt(serverName, "431", nick) + ":No nickname given";
}

std::string Reply::errErroneousNick(const std::string& serverName, const std::string& nick, const std::string& bad) {
	return _fmt(serverName, "432", nick) + bad + " :Erroneous nickname";
}

std::string Reply::errNickInUse(const std::string& serverName, const std::string& nick, const std::string& bad) {
	return _fmt(serverName, "433", nick) + bad + " :Nickname is already in use";
}

std::string Reply::errUserNotInChan(const std::string& serverName, const std::string& nick, const std::string& target, const std::string& chan) {
	return _fmt(serverName, "441", nick) + target + " " + chan + " :They aren't on that channel";
}

std::string Reply::errNotOnChannel(const std::string& serverName, const std::string& nick, const std::string& chan) {
	return _fmt(serverName, "442", nick) + chan + " :You're not on that channel";
}

std::string Reply::errUserOnChannel(const std::string& serverName, const std::string& nick, const std::string& target, const std::string& chan) {
	return _fmt(serverName, "443", nick) + target + " " + chan + " :is already on channel";
}

std::string Reply::errNotRegistered(const std::string& serverName, const std::string& nick) {
	std::string target = nick.empty() ? "*" : nick;
	return _fmt(serverName, "451", target) + ":You have not registered";
}

std::string Reply::errNeedMoreParams(const std::string& serverName, const std::string& nick, const std::string& cmd) {
	return _fmt(serverName, "461", nick) + cmd + " :Not enough parameters";
}

std::string Reply::errAlreadyRegistered(const std::string& serverName, const std::string& nick) {
	return _fmt(serverName, "462", nick) + ":Unauthorized command (already registered)";
}

std::string Reply::errPasswdMismatch(const std::string& serverName, const std::string& nick) {
	return _fmt(serverName, "464", nick) + ":Password incorrect";
}

std::string Reply::errChannelIsFull(const std::string& serverName, const std::string& nick, const std::string& chan) {
	return _fmt(serverName, "471", nick) + chan + " :Cannot join channel (full)";
}

std::string Reply::errUnknownMode(const std::string& serverName, const std::string& nick, char modeChar) {
	return _fmt(serverName, "472", nick) + modeChar + " :is unknown mode char to me";
}

std::string Reply::errInviteOnlyChan(const std::string& serverName, const std::string& nick, const std::string& chan) {
	return _fmt(serverName, "473", nick) + chan + " :Cannot join channel (invite only)";
}

std::string Reply::errBadChannelKey(const std::string& serverName, const std::string& nick, const std::string& chan) {
	return _fmt(serverName, "475", nick) + chan + " :Cannot join channel (bad key)";
}

std::string Reply::errChanOPrivsNeeded(const std::string& serverName, const std::string& nick, const std::string& chan) {
	return _fmt(serverName, "482", nick) + chan + " :You're not channel operator";
}

std::string Reply::joinMsg(const std::string& prefix, const std::string& chan) {
	return ":" + prefix + " JOIN " + chan;
}

std::string Reply::partMsg(const std::string& prefix, const std::string& chan, const std::string& reason) {
	return ":" + prefix + " PART " + chan + " :" + reason;
}

std::string Reply::quitMsg(const std::string& prefix, const std::string& reason) {
	return ":" + prefix + " QUIT :" + reason;
}

std::string Reply::kickMsg(const std::string& prefix, const std::string& chan, const std::string& target, const std::string& reason) {
	return ":" + prefix + " KICK " + chan + " " + target + " :" + reason;
}

std::string Reply::privmsgMsg(const std::string& prefix, const std::string& target, const std::string& text) {
	return ":" + prefix + " PRIVMSG " + target + " :" + text;
}

std::string Reply::noticeMsg(const std::string& prefix, const std::string& target, const std::string& text) {
	return ":" + prefix + " NOTICE " + target + " :" + text;
}

std::string Reply::topicMsg(const std::string& prefix, const std::string& chan, const std::string& topic) {
	return ":" + prefix + " TOPIC " + chan + " :" + topic;
}

std::string Reply::modeMsg(const std::string& prefix, const std::string& chan, const std::string& modes, const std::string& args) {
	return ":" + prefix + " MODE " + chan + " " + modes + (args.empty() ? "" : args);
}

std::string Reply::inviteMsg(const std::string& prefix, const std::string& target, const std::string& chan) {
	return ":" + prefix + " INVITE " + target + " " + chan;
}

std::string Reply::nickMsg(const std::string& prefix, const std::string& newNick) {
	return ":" + prefix + " NICK " + newNick;
}

std::string Reply::errorMsg(const std::string& reason) {
    return "ERROR :" + reason;
}