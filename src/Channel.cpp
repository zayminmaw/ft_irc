/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Channel.cpp                                        :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: wmin-kha <wmin-kha@student.42bangkok.co    +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/05/07 12:00:00 by zmin              #+#    #+#             */
/*   Updated: 2026/05/11 20:51:17 by wmin-kha         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "Channel.hpp"
#include "Client.hpp"
#include <algorithm>

Channel::Channel(const std::string &name)
	: _name(name), _inviteOnly(false), _topicLocked(false), _hasKey(false), _userLimit(-1)
{
}

Channel::~Channel()
{
}

// Getters
const std::string &Channel::getName() const { return _name; }
const std::string &Channel::getTopic() const { return _topic; }
const std::vector<Client *> &Channel::getMembers() const { return _members; }
size_t Channel::memberCount() const { return _members.size(); }

void Channel::addMember(Client *client)
{
	if (!hasMember(client))
	{
		_members.push_back(client);
	}
}

void Channel::removeMember(Client *client)
{
	// remove from regular member list
	_members.erase(std::remove(_members.begin(), _members.end(), client), _members.end());

	// INVARIANT: if they leave, they lose operator status
	removeOperator(client);

	// INVARIANT: if they leave, consume any related invited
	consumeInvite(client);
}

bool Channel::hasMember(Client *client) const
{
	return std::find(_members.begin(), _members.end(), client) != _members.end();
}

// operator management
void Channel::addOperator(Client *client)
{
	if (hasMember(client) && !isOperator(client))
	{
		_operators.push_back(client);
	}
}

void Channel::removeOperator(Client *client)
{
	_operators.erase(std::remove(_operators.begin(), _operators.end(), client), _operators.end());
}

bool Channel::isOperator(Client *client) const
{
	return std::find(_operators.begin(), _operators.end(), client) != _operators.end();
}

// invite management
void Channel::invite(Client *client)
{
	if (!isInvited(client))
	{
		_invited.push_back(client);
	}
}

void Channel::consumeInvite(Client *client)
{
	_invited.erase(std::remove(_invited.begin(), _invited.end(), client), _invited.end());
}

bool Channel::isInvited(Client *client) const
{
	return std::find(_invited.begin(), _invited.end(), client) != _invited.end();
}

// modes and topic
void Channel::setTopic(const std::string &topic) { _topic = topic; }
void Channel::setInviteOnly(bool val) { _inviteOnly = val; }
bool Channel::isInviteOnly() const { return _inviteOnly; }
void Channel::setTopicLocked(bool val) { _topicLocked = val; }
bool Channel::isTopicLocked() const { return _topicLocked; }

void Channel::setKey(const std::string &key)
{
	_key = key;
	_hasKey = true;
}

void Channel::clearKey()
{
	_key.clear();
	_hasKey = false;
}

bool Channel::hasKey() const { return _hasKey; }
const std::string &Channel::getKey() const { return _key; }

void Channel::setUserLimit(int limit) { _userLimit = limit; }
void Channel::clearUserLimit() { _userLimit = -1; }
int Channel::getUserLimit() const { return _userLimit; }

// BROADCAST
void Channel::broadcast(const std::string &msg, Client *except)
{
	for (size_t i = 0; i < _members.size(); ++i)
	{
		Client *current = _members[i];

		if (current == except)
		{
			continue;
		}
		current->send(msg);
	}
}