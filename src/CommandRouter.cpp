/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   CommandRouter.cpp                                  :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: zayminmaw <zayminmaw@student.42.fr>        +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/05/07 12:00:00 by zmin              #+#    #+#             */
/*   Updated: 2026/05/20 14:40:10 by zayminmaw        ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */


#include <cstdlib>
#include "CommandRouter.hpp"
#include "Reply.hpp"
#include "Client.hpp"
#include "Server.hpp"
#include "Channel.hpp"

CommandRouter::CommandRouter(Server& server)
	: _server(server)
{
	registerHandlers();
}

CommandRouter::~CommandRouter()
{
}

void CommandRouter::dispatch(Client& c, const std::string& line)
{
	IRCMessage msg = tokenize(line);

	if (msg.command.empty())
		return;

	std::map<std::string, Handler>::iterator it = _handlers.find(msg.command);

	if (it != _handlers.end()) {
		(this->*(it->second))(c, msg);
	} else {
		c.queueOutbound(Reply::errUnknownCommand(_server.getName(), c.getNick(), msg.command));
	}
}

IRCMessage CommandRouter::tokenize(const std::string& line)
{
	IRCMessage msg;
	std::string ref = line;
	
	// 1. Handle prefix (starts with :)
	if (!ref.empty() && ref[0] == ':') {
		size_t space = ref.find(' ');
		if (space != std::string::npos)
		{
			msg.prefix = ref.substr(1, space - 1);
			ref = ref.substr(space + 1);
		}
	}

	// 2. Handle trailing (space followed by colon)
	size_t colon = ref.find(" :");
	std::string trailing;
	bool hasTrailing = false;
	if (colon != std::string::npos) {
		trailing = ref.substr(colon + 2);
		ref = ref.substr(0, colon);
		hasTrailing = true;
	}

	// 3. Handle command and middle params
	std::stringstream ss(ref);
	std::string word;
	if (ss >> word) {
		for (size_t i = 0; i < word.length(); i++)
			word[i] = std::toupper(static_cast<unsigned char>(word[i]));
		msg.command = word;
		while (ss >> word) {
			msg.params.push_back(word);
		}
	}

	if (hasTrailing)
		msg.params.push_back(trailing);
		
	return msg;
}

void CommandRouter::registerHandlers() {
	_handlers["PASS"] = &CommandRouter::handlePass;
	_handlers["NICK"] = &CommandRouter::handleNick;
	_handlers["USER"] = &CommandRouter::handleUser;
	_handlers["CAP"] = &CommandRouter::handleCap;
	_handlers["JOIN"] = &CommandRouter::handleJoin;
	_handlers["PART"] = &CommandRouter::handlePart;
	_handlers["TOPIC"] = &CommandRouter::handleTopic;
	_handlers["MODE"] = &CommandRouter::handleMode;
	_handlers["KICK"] = &CommandRouter::handleKick;
	_handlers["INVITE"] = &CommandRouter::handleInvite;
	_handlers["PRIVMSG"] = &CommandRouter::handlePrivmsg;
	_handlers["NOTICE"] = &CommandRouter::handleNotice;
	_handlers["PING"] = &CommandRouter::handlePing;
	_handlers["QUIT"] = &CommandRouter::handleQuit;
}

bool CommandRouter::_ensureRegistered(Client& c) {
	if (c.isRegistered()) {
		return true;
	}
	c.queueOutbound(Reply::errNotRegistered(_server.getName(), c.getNick()));
	return false;
}

std::string CommandRouter::_buildNamesList(Channel* ch) {
	std::string list;
	std::vector<Client*> members = ch->getMembers();
	for (size_t i = 0; i < members.size(); ++i) {
		if (i > 0) list += " ";
		if (ch->isOperator(members[i]))
			list += "@";
		list += members[i]->getNick();
	}
	return list;
}

void CommandRouter::tryRegister(Client& c) { 
	if (c.isRegistered())
		return;
	if (!_server.getPassword().empty() && !c.hasPass())
		return;
	if (!c.hasNick() || !c.hasUser())
		return;

	c.markRegistered();
	c.queueOutbound(Reply::welcome(_server.getName(), c.getNick()));
    c.queueOutbound(Reply::yourHost(_server.getName(), c.getNick()));
    c.queueOutbound(Reply::created(_server.getName(), c.getNick()));
    c.queueOutbound(Reply::myInfo(_server.getName(), c.getNick()));
}

void CommandRouter::handlePass(Client& c, const IRCMessage& m) { 
	if (c.isRegistered()) {
		c.queueOutbound(Reply::errAlreadyRegistered(_server.getName(), c.getNick()));
		return;
	}
	if (m.params.empty()) {
		c.queueOutbound(Reply::errNeedMoreParams(_server.getName(), c.getNick(), m.command));
		return;
	}
	if (m.params[0] != _server.getPassword()) {
		c.queueOutbound(Reply::errPasswdMismatch(_server.getName(), c.getNick()));
		_server.scheduleDisconnect(&c, "Access denied: Bad password", false);
		return;
	}
	c.markPass();
}
 
void CommandRouter::handleNick(Client& c, const IRCMessage& m) { 
	if (m.params.empty()) {
		c.queueOutbound(Reply::errNoNicknameGiven(_server.getName(), c.getNick()));
		return;
	}
	if (!_server.getPassword().empty() && !c.hasPass()) {
		c.queueOutbound(Reply::errPasswdMismatch(_server.getName(), c.getNick()));
		return;
	}
	std::string newNick = m.params[0];
	if (newNick.find_first_of(" ,*?!@.") != std::string::npos) {
		c.queueOutbound(Reply::errErroneousNick(_server.getName(), c.getNick(), newNick));
		return;
	}
	Client* existing = _server.findClientByNick(newNick);
	if (existing && existing != &c) {
		c.queueOutbound(Reply::errNickInUse(_server.getName(), c.getNick(), newNick));
		return;
	}
	if (c.isRegistered()) {
		c.setNick(newNick);
	} else {
		c.setNick(newNick);
		tryRegister(c);
	}
}

void CommandRouter::handleUser(Client& c, const IRCMessage& m) { 
	if (c.isRegistered()) {
		c.queueOutbound(Reply::errAlreadyRegistered(_server.getName(), c.getNick()));
		return;
	}
	if (m.params.size() < 4) {
		c.queueOutbound(Reply::errNeedMoreParams(_server.getName(), c.getNick(), m.command));
		return;
	}
	if (!_server.getPassword().empty() && !c.hasPass()) {
		c.queueOutbound(Reply::errPasswdMismatch(_server.getName(), c.getNick()));
		return;
	}
	c.setUser(m.params[0]);
	tryRegister(c);
}

void CommandRouter::handleCap(Client& c, const IRCMessage& m) {
    (void)m;
    c.queueOutbound(":" + _server.getName() + " CAP * LS :");
}

void CommandRouter::handleJoin(Client& c, const IRCMessage& m) {
	if (!_ensureRegistered(c)) return;
	if (m.params.empty()) {
		c.queueOutbound(Reply::errNeedMoreParams(_server.getName(), c.getNick(), m.command));
		return;
	}

	// Parse comma-separated channel and key lists ("JOIN #a,#b key1,key2")
	std::vector<std::string> chans;
	std::vector<std::string> keys;
	{
		std::stringstream cs(m.params[0]);
		std::string name;
		while (std::getline(cs, name, ','))
			chans.push_back(name);
		if (m.params.size() > 1) {
			std::stringstream ks(m.params[1]);
			std::string k;
			while (std::getline(ks, k, ','))
				keys.push_back(k);
		}
	}

	for (size_t idx = 0; idx < chans.size(); ++idx) {
		const std::string& chanName = chans[idx];
		if (chanName.empty() || (chanName[0] != '#' && chanName[0] != '&')) {
			c.queueOutbound(Reply::errNoSuchChannel(_server.getName(), c.getNick(), chanName));
			continue;
		}
		Channel* ch = _server.getOrCreateChannel(chanName);
		if (!ch || ch->hasMember(&c)) continue;

		// 1. Enforce channel modes. For a freshly created channel none of these
		//    flags are set, so the checks pass naturally and the joiner becomes op.
		if (ch->isInviteOnly() && !ch->isInvited(&c)) {
			c.queueOutbound(Reply::errInviteOnlyChan(_server.getName(), c.getNick(), chanName));
			continue;
		}
		if (ch->hasKey()) {
			std::string supplied = (idx < keys.size()) ? keys[idx] : "";
			if (supplied != ch->getKey()) {
				c.queueOutbound(Reply::errBadChannelKey(_server.getName(), c.getNick(), chanName));
				continue;
			}
		}
		if (ch->getUserLimit() > 0 && static_cast<int>(ch->memberCount()) >= ch->getUserLimit()) {
			c.queueOutbound(Reply::errChannelIsFull(_server.getName(), c.getNick(), chanName));
			continue;
		}

		// 2. Add to channel; consume any pending invite; first joiner becomes op
		ch->addMember(&c);
		c.joinChannel(ch);
		ch->consumeInvite(&c);
		if (ch->getOperators().empty())
			ch->addOperator(&c);

		// 3. Broadcast JOIN to everyone (including sender)
		ch->broadcast(Reply::joinMsg(c.getPrefix(), chanName), NULL);

		// 4. Send topic (332)
		if (!ch->getTopic().empty())
			c.queueOutbound(Reply::topic(_server.getName(), c.getNick(), chanName, ch->getTopic()));
		else
			c.queueOutbound(Reply::noTopic(_server.getName(), c.getNick(), chanName));

		// 5. Send names list (353 + 366)
		c.queueOutbound(Reply::namReply(_server.getName(), c.getNick(), chanName, _buildNamesList(ch)));
		c.queueOutbound(Reply::endOfNames(_server.getName(), c.getNick(), chanName));
	}
}

void CommandRouter::handlePart(Client& c, const IRCMessage& m) { 
	if (!_ensureRegistered(c)) return;
	if (m.params.empty()) {
		c.queueOutbound(Reply::errNeedMoreParams(_server.getName(), c.getNick(), m.command));
		return;
	}
	std::string reason = (m.params.size() > 1) ? m.params[1] : c.getNick();
	std::stringstream ss(m.params[0]);
	std::string chanName;
	while (std::getline(ss, chanName, ',')) {
		Channel* ch = _server.findChannel(chanName);
		if (!ch) {
			c.queueOutbound(Reply::errNoSuchChannel(_server.getName(), c.getNick(), chanName));
			continue;
		} 
		if (!ch->hasMember(&c)) {
			c.queueOutbound(Reply::errNotOnChannel(_server.getName(), c.getNick(), chanName));
			continue;
		}
		ch->broadcast(Reply::partMsg(c.getPrefix(), chanName, reason), NULL);
		ch->removeMember(&c);
		c.leaveChannel(ch);
		_server.removeChannelIfEmpty(chanName);
	}
}

void CommandRouter::handleTopic(Client& c, const IRCMessage& m) { 
	if (!_ensureRegistered(c)) return;
	if (m.params.empty()) {
		c.queueOutbound(Reply::errNeedMoreParams(_server.getName(), c.getNick(), m.command));
		return;
	}
	std::string chanName = m.params[0];
	Channel* ch = _server.findChannel(chanName);
	if (!ch) {
		c.queueOutbound(Reply::errNoSuchChannel(_server.getName(), c.getNick(), chanName));
		return;
	} 
	if (!ch->hasMember(&c)) {
		c.queueOutbound(Reply::errNotOnChannel(_server.getName(), c.getNick(), chanName));
		return;
	}
	
	// 1. Query topic (1 param)
	if (m.params.size() == 1) {
		if (!ch->getTopic().empty())
			c.queueOutbound(Reply::topic(_server.getName(), c.getNick(), chanName, ch->getTopic()));
		else
			c.queueOutbound(Reply::noTopic(_server.getName(), c.getNick(), chanName));
		return;
	}

	// 2. Set Topic (2+ params)
	// Check if +t is set and user is not op
	if (ch->isTopicLocked() && !ch->isOperator(&c)) {
		c.queueOutbound(Reply::errChanOPrivsNeeded(_server.getName(), c.getNick(), chanName));
		return;
	}
	std::string newTopic = m.params[1];
	ch->setTopic(newTopic);
	ch->broadcast(Reply::topicMsg(c.getPrefix(), chanName, newTopic), NULL);
}

void CommandRouter::handleMode(Client& c, const IRCMessage& m) { 
	if (!_ensureRegistered(c)) return;
	if (m.params.empty()) {
		c.queueOutbound(Reply::errNeedMoreParams(_server.getName(), c.getNick(), m.command));
		return;
	}
	Channel* ch = _server.findChannel(m.params[0]);
	if (!ch) {
		c.queueOutbound(Reply::errNoSuchChannel(_server.getName(), c.getNick(), m.params[0]));
		return;
	}
	
	// 1. Query current modes (324)
	if (m.params.size() == 1) {
		std::string modeStr = "+";
		std::string argStr;
		if (ch->isInviteOnly())  modeStr += "i";
		if (ch->isTopicLocked()) modeStr += "t";
		if (ch->hasKey()) {
			modeStr += "k";
			// Only show the key to channel members
			if (ch->hasMember(&c)) {
				if (!argStr.empty()) argStr += " ";
				argStr += ch->getKey();
			}
		}
		if (ch->getUserLimit() > 0) {
			modeStr += "l";
			std::stringstream ls;
			ls << ch->getUserLimit();
			if (!argStr.empty()) argStr += " ";
			argStr += ls.str();
		}
		c.queueOutbound(Reply::channelModeIs(_server.getName(), c.getNick(), ch->getName(), modeStr, argStr));
		return;
	}

	// 2. Change modes (requires operator)
	if (!ch->isOperator(&c)) {
		c.queueOutbound(Reply::errChanOPrivsNeeded(_server.getName(), c.getNick(), ch->getName()));
		return;
	}

	std::string modes = m.params[1];
	std::string appliedModes = "";
	std::string appliedArgs = "";
	size_t argIdx = 2;
	char currentSign = '\0';

	for (size_t i = 0; i < modes.length(); ++i) {
		char mode = modes[i];
		if (mode == '+' || mode == '-') {
			currentSign = mode;
			continue;
		}
		if (currentSign == '\0') continue;
		bool plus = (currentSign == '+');

		bool applied = false;
		std::string appliedArg;

		if (mode == 'i') {
			ch->setInviteOnly(plus);
			applied = true;
		} else if (mode == 't') {
			ch->setTopicLocked(plus);
			applied = true;
		} else if (mode == 'k') {
			if (plus && argIdx < m.params.size()) {
				ch->setKey(m.params[argIdx]);
				appliedArg = m.params[argIdx++];
				applied = true;
			} else if (!plus) {
				ch->clearKey();
				applied = true;
			}
		} else if (mode == 'l') {
			if (plus && argIdx < m.params.size()) {
				int limit = std::atoi(m.params[argIdx].c_str());
				if (limit > 0) {
					ch->setUserLimit(limit);
					appliedArg = m.params[argIdx];
					applied = true;
				}
				argIdx++;
			} else if (!plus) {
				ch->setUserLimit(0);
				applied = true;
			}
		} else if (mode == 'o') {
			if (argIdx < m.params.size()) {
				Client* target = _server.findClientByNick(m.params[argIdx]);
				if (target && ch->hasMember(target)) {
					if (plus) ch->addOperator(target);
					else      ch->removeOperator(target);
					appliedArg = m.params[argIdx];
					applied = true;
				} else {
					c.queueOutbound(Reply::errUserNotInChan(_server.getName(), c.getNick(), m.params[argIdx], ch->getName()));
				}
				argIdx++;
			}
		} else {
			c.queueOutbound(Reply::errUnknownMode(_server.getName(), c.getNick(), mode));
		}

		if (applied) {
			appliedModes += currentSign;
			appliedModes += mode;
			if (!appliedArg.empty()) appliedArgs += " " + appliedArg;
		}
	}
	if (!appliedModes.empty()) {
		ch->broadcast(Reply::modeMsg(c.getPrefix(), ch->getName(), appliedModes, appliedArgs), NULL);
	}
}

void CommandRouter::handleKick(Client& c, const IRCMessage& m) { 
	if (!_ensureRegistered(c)) return;
	if (m.params.size() < 2) {
		c.queueOutbound(Reply::errNeedMoreParams(_server.getName(), c.getNick(), m.command));
		return;
	}
	std::string chanName = m.params[0];
	std::string targetNick = m.params[1];
	std::string reason = (m.params.size() > 2) ? m.params[2] : c.getNick();
	Channel* ch = _server.findChannel(chanName);
	if (!ch) {
		c.queueOutbound(Reply::errNoSuchChannel(_server.getName(), c.getNick(), chanName));
		return;
	}
	if (!ch->hasMember(&c)) {
		c.queueOutbound(Reply::errNotOnChannel(_server.getName(), c.getNick(), ch->getName()));
		return;
	}
	if (!ch->isOperator(&c)) {
		c.queueOutbound(Reply::errChanOPrivsNeeded(_server.getName(), c.getNick(), ch->getName()));
		return;
	}
	Client* targetClient = _server.findClientByNick(targetNick);
	if (!targetClient || !ch->hasMember(targetClient)) {
		c.queueOutbound(Reply::errUserNotInChan(_server.getName(), c.getNick(), targetNick, ch->getName()));
		return;
	}
	ch->broadcast(Reply::kickMsg(c.getPrefix(), ch->getName(), targetNick, reason), NULL);
	ch->removeMember(targetClient);
	targetClient->leaveChannel(ch);
	if (ch->getOperators().empty() && !ch->getMembers().empty()) {
		Client* newOp = ch->getMembers()[0];
		ch->addOperator(newOp);
		ch->broadcast(Reply::modeMsg(_server.getName(), ch->getName(), "+o", newOp->getNick()),NULL);
	}
	_server.removeChannelIfEmpty(chanName);
}

void CommandRouter::handleInvite(Client& c, const IRCMessage& m) { 
	if (!_ensureRegistered(c)) return;
	if (m.params.size() < 2) {
		c.queueOutbound(Reply::errNeedMoreParams(_server.getName(), c.getNick(), m.command));
		return;
	}
	std::string targetNick = m.params[0];
	std::string chanName = m.params[1];
	Channel* ch = _server.findChannel(chanName);
	if (!ch) {
		c.queueOutbound(Reply::errNoSuchChannel(_server.getName(), c.getNick(), chanName));
		return;
	}
	if (!ch->hasMember(&c)) {
		c.queueOutbound(Reply::errNotOnChannel(_server.getName(), c.getNick(), ch->getName()));
		return;
	}
	// If invite-only (+i) mode is active, you must be a chan operator to invite someone
	if (ch->isInviteOnly() && !ch->isOperator(&c)) {
		c.queueOutbound(Reply::errChanOPrivsNeeded(_server.getName(), c.getNick(), ch->getName()));
		return;
	}
	Client* targetClient = _server.findClientByNick(targetNick);
	if (!targetClient) {
		c.queueOutbound(Reply::errNoSuchNick(_server.getName(), c.getNick(), targetNick));
		return;
	}
	if (ch->hasMember(targetClient)) {
		c.queueOutbound(Reply::errUserOnChannel(_server.getName(), c.getNick(), targetNick, ch->getName()));
		return;
	}
	ch->invite(targetClient);
	targetClient->queueOutbound(Reply::inviteMsg(c.getPrefix(), targetNick, ch->getName()));
	c.queueOutbound(Reply::rplInviting(_server.getName(), c.getNick(), targetNick, ch->getName()));
}

void CommandRouter::handlePrivmsg(Client& c, const IRCMessage& m) { 
	if (!_ensureRegistered(c)) return;
	if (m.params.empty()) {
		c.queueOutbound(Reply::errNoRecipient(_server.getName(), c.getNick(), m.command));
		return;
	}
	if (m.params.size() < 2) {
		c.queueOutbound(Reply::errNoTextToSend(_server.getName(), c.getNick()));
		return;
	}
	std::string text = m.params[1];
	std::stringstream ss(m.params[0]);
	std::string target;
	while (std::getline(ss,target, ',')) {
		if (target.empty()) continue;
		if (target[0] == '#' || target[0] == '&') {
			Channel* ch = _server.findChannel(target);
			if (!ch) {
				c.queueOutbound(Reply::errNoSuchChannel(_server.getName(), c.getNick(), target));
			} else if (!ch->hasMember(&c)) {
				c.queueOutbound(Reply::errCannotSendToChan(_server.getName(), c.getNick(), target));
			}
			else {
				ch->broadcast(Reply::privmsgMsg(c.getPrefix(), target, text), &c);
			}
		} else {
			Client* targetClient = _server.findClientByNick(target);
			if (!targetClient)
				c.queueOutbound(Reply::errNoSuchNick(_server.getName(), c.getNick(), target));
			else
				targetClient->queueOutbound(Reply::privmsgMsg(c.getPrefix(), target, text));
		}
	}
}

void CommandRouter::handleNotice(Client& c, const IRCMessage& m) { 
	if (!c.isRegistered() || m.params.size() < 2) return;

	std::string text = m.params[1];
	std::stringstream ss(m.params[0]);
	std::string target;
	while (std::getline(ss, target, ',')) {
        if (target.empty()) continue;
        if (target[0] == '#' || target[0] == '&') {
            Channel* ch = _server.findChannel(target);
            if (ch) ch->broadcast(Reply::noticeMsg(c.getPrefix(), target, text), &c);
        } else {
            Client* targetClient = _server.findClientByNick(target);
            if (targetClient) targetClient->queueOutbound(Reply::noticeMsg(c.getPrefix(), target, text));
        }
    }
}

void CommandRouter::handlePing(Client& c, const IRCMessage& m) { 
	if (m.params.empty()) {
		c.queueOutbound(Reply::errNeedMoreParams(_server.getName(), c.getNick(), m.command));
		return;
	}
	c.queueOutbound(":" + _server.getName() + " PONG " + _server.getName() + " :" + m.params[0]);
}

void CommandRouter::handleQuit(Client& c, const IRCMessage& m) {
	std::string reason = m.params.empty() ? "Client quit" : "Quit: " + m.params[0];
	_server.scheduleDisconnect(&c, reason, false);
}

