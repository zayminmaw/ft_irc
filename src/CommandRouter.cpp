/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   CommandRouter.cpp                                  :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: zmin <zmin@student.42bangkok.com>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/05/07 12:00:00 by zmin              #+#    #+#             */
/*   Updated: 2026/05/13 21:17:44 by zmin             ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

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
		c.send(Reply::errUnknownCommand(_server.getName(), c.getNick(), msg.command));
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
	c.send(Reply::errNotRegistered(_server.getName(), c.getNick()));
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
	c.send(Reply::welcome(_server.getName(), c.getNick()));
    c.send(Reply::yourHost(_server.getName(), c.getNick()));
    c.send(Reply::created(_server.getName(), c.getNick()));
    c.send(Reply::myInfo(_server.getName(), c.getNick()));
}

void CommandRouter::handlePass(Client& c, const IRCMessage& m) { 
	if (c.isRegistered()) {
		c.send(Reply::errAlreadyRegistered(_server.getName(), c.getNick()));
		return;
	}
	if (m.params.empty()) {
		c.send(Reply::errNeedMoreParams(_server.getName(), c.getNick(), m.command));
		return;
	}
	if (m.params[0] != _server.getPassword()) {
		c.send(Reply::errPasswdMismatch(_server.getName(), c.getNick()));
		return;
	}
	c.markPass();
}
 
void CommandRouter::handleNick(Client& c, const IRCMessage& m) { 
	if (m.params.empty()) {
		c.send(Reply::errNoNicknameGiven(_server.getName(), c.getNick()));
		return;
	}
	if (!_server.getPassword().empty() && !c.hasPass()) {
		c.send(Reply::errPasswdMismatch(_server.getName(), c.getNick()));
		return;
	}
	std::string newNick = m.params[0];
	if (newNick.find_first_of(" ,*?!@.") != std::string::npos) {
		c.send(Reply::errErroneousNick(_server.getName(), c.getNick(), newNick));
		return;
	}
	Client* existing = _server.findClientByNick(newNick);
	if (existing && existing != &c) {
		c.send(Reply::errNickInUse(_server.getName(), c.getNick(), newNick));
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
		c.send(Reply::errAlreadyRegistered(_server.getName(), c.getNick()));
		return;
	}
	if (m.params.size() < 4) {
		c.send(Reply::errNeedMoreParams(_server.getName(), c.getNick(), m.command));
		return;
	}
	if (!_server.getPassword().empty() && !c.hasPass()) {
		c.send(Reply::errPasswdMismatch(_server.getName(), c.getNick()));
		return;
	}
	c.setUser(m.params[0]);
	tryRegister(c);
}

void CommandRouter::handleCap(Client& c, const IRCMessage& m) {
    (void)m;
    c.send(":" + _server.getName() + " CAP * LS :");
}

void CommandRouter::handleJoin(Client& c, const IRCMessage& m) { 
	if (!_ensureRegistered(c)) return;
	if (m.params.empty()) {
		c.send(Reply::errNeedMoreParams(_server.getName(), c.getNick(), m.command));
		return;
	}
	std::stringstream ss(m.params[0]);
	std::string chanName;
	while (std::getline(ss, chanName, ',')) {
		if (chanName.empty() || (chanName[0] != '#' && chanName[0] != '&')) {
			c.send(Reply::errNoSuchChannel(_server.getName(), c.getNick(), chanName));
			continue;
		}
		Channel* ch = _server.getOrCreateChannel(chanName);
		if (!ch || ch->hasMember(&c)) continue;

		// 1. Add to channel and if first join add as operator
		ch->addMember(&c);
		if (ch->getOperators().empty())
			ch->addOperator(&c);
		
		// 2. Broadcast JOIN to everyone (including sender)
		ch->broadcast(Reply::joinMsg(c.getPrefix(), chanName), NULL);

		// 3. Send topic (332)
		if (!ch->getTopic().empty())
			c.send(Reply::topic(_server.getName(), c.getNick(), chanName, ch->getTopic()));
		else
			c.send(Reply::noTopic(_server.getName(), c.getNick(), chanName));
		
		// 4. Send names list (353 + 366)
		c.send(Reply::namReply(_server.getName(), c.getNick(), chanName, _buildNamesList(ch)));
		c.send(Reply::endOfNames(_server.getName(), c.getNick(), chanName)); 
	}
}

void CommandRouter::handlePart(Client& c, const IRCMessage& m) { 
	if (!_ensureRegistered(c)) return;
	if (m.params.empty()) {
		c.send(Reply::errNeedMoreParams(_server.getName(), c.getNick(), m.command));
		return;
	}
	std::string reason = (m.params.size() > 1) ? m.params[1] : c.getNick();
	std::stringstream ss(m.params[0]);
	std::string chanName;
	while (std::getline(ss, chanName, ',')) {
		Channel* ch = _server.findChannel(chanName);
		if (!ch) {
			c.send(Reply::errNoSuchChannel(_server.getName(), c.getNick(), chanName));
			continue;
		} 
		if (!ch->hasMember(&c)) {
			c.send(Reply::errNotOnChannel(_server.getName(), c.getNick(), chanName));
			continue;
		}
		ch->broadcast(Reply::partMsg(c.getPrefix(), chanName, reason), NULL);
		ch->removeMember(&c);
		_server.removeChannelIfEmpty(chanName);
	}
}

void CommandRouter::handleTopic(Client& c, const IRCMessage& m) { 
	if (!_ensureRegistered(c)) return;
	if (m.params.empty()) {
		c.send(Reply::errNeedMoreParams(_server.getName(), c.getNick(), m.command));
		return;
	}
	std::string chanName = m.params[0];
	Channel* ch = _server.findChannel(chanName);
	if (!ch) {
		c.send(Reply::errNoSuchChannel(_server.getName(), c.getNick(), chanName));
		return;
	} 
	if (!ch->hasMember(&c)) {
		c.send(Reply::errNotOnChannel(_server.getName(), c.getNick(), chanName));
		return;
	}
	
	// 1. Query topic (1 param)
	if (m.params.size() == 1) {
		if (!ch->getTopic().empty())
			c.send(Reply::topic(_server.getName(), c.getNick(), chanName, ch->getTopic()));
		else
			c.send(Reply::noTopic(_server.getName(), c.getNick(), chanName));
		return;
	}

	// 2. Set Topic (2+ params)
	// Check if +t is set and user is not op
	if (ch->isTopicLocked() && !ch->isOperator(&c)) {
		c.send(Reply::errChanOPrivsNeeded(_server.getName(), c.getNick(), chanName));
		return;
	}
	std::string newTopic = m.params[1];
	ch->setTopic(newTopic);
	ch->broadcast(Reply::topicMsg(c.getPrefix(), chanName, newTopic), NULL);
}

void CommandRouter::handleMode(Client& c, const IRCMessage& m) { if (!_ensureRegistered(c)) return; (void)m; }
void CommandRouter::handleKick(Client& c, const IRCMessage& m) { if (!_ensureRegistered(c)) return; (void)m; }
void CommandRouter::handleInvite(Client& c, const IRCMessage& m) { if (!_ensureRegistered(c)) return; (void)m; }

void CommandRouter::handlePrivmsg(Client& c, const IRCMessage& m) { 
	if (!_ensureRegistered(c)) return;
	if (m.params.empty()) {
		c.send(Reply::errNoRecipient(_server.getName(), c.getNick(), m.command));
		return;
	}
	if (m.params.size() < 2) {
		c.send(Reply::errNoTextToSend(_server.getName(), c.getNick()));
	}
	std::string text = m.params[1];
	std::stringstream ss(m.params[0]);
	std::string target;
	while (std::getline(ss,target, ',')) {
		if (target[0] == '#' || target[0] == '&') {
			Channel* ch = _server.findChannel(target);
			if (!ch)
				c.send(Reply::errNoSuchChannel(_server.getName(), c.getNick(), target));
			else 
				ch->broadcast(Reply::privmsgMsg(c.getPrefix(), target, text), &c);
		} else {
			Client* targetClient = _server.findClientByNick(target);
			if (!targetClient)
				c.send(Reply::errNoSuchNick(_server.getName(), c.getNick(), target));
			else
				targetClient->send(Reply::privmsgMsg(c.getPrefix(), target, text));
		}
	}
}

void CommandRouter::handleNotice(Client& c, const IRCMessage& m) { 
	if (!c.isRegistered() || m.params.size() < 2) return;

	std::string text = m.params[1];
	std::stringstream ss(m.params[0]);
	std::string target;
	while (std::getline(ss, target, ',')) {
        if (target[0] == '#' || target[0] == '&') {
            Channel* ch = _server.findChannel(target);
            if (ch) ch->broadcast(Reply::noticeMsg(c.getPrefix(), target, text), &c);
        } else {
            Client* targetClient = _server.findClientByNick(target);
            if (targetClient) targetClient->send(Reply::noticeMsg(c.getPrefix(), target, text));
        }
    }
}

void CommandRouter::handlePing(Client& c, const IRCMessage& m) { 
	if (m.params.empty()) {
		c.send(Reply::errNeedMoreParams(_server.getName(), c.getNick(), m.command));
		return;
	}
	c.send(":" + _server.getName() + " PONG " + _server.getName() + " :" + m.params[0]);
}

void CommandRouter::handleQuit(Client& c, const IRCMessage& m) { (void)c; (void)m; }

