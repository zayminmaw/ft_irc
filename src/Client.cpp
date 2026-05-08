/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Client.cpp                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: wmin-kha <wmin-kha@student.42bangkok.co    +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/05/07 12:00:00 by zmin              #+#    #+#             */
/*   Updated: 2026/05/08 19:36:03 by wmin-kha         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "Client.hpp"
#include "algorithm"

Client::Client(int fd, const std::string &ip) : _fd(fd), _ip(ip),
	_hasPass(false), _registered(false)
{
}

Client::~Client()
{
}

// Getter Setters
int Client::getFd() const
{
	return (_fd);
}

const std::string &Client::getIp() const
{
	return (_ip);
}

const std::string &Client::getNick() const
{
	return (_nick);
}

void Client::setNick(const std::string &nick)
{
	_nick = nick;
}

const std::string &Client::getUser() const
{
	return (_user);
}

void Client::setUser(const std::string &user)
{
	_user = user;
}

std::string Client::getPrefix() const
{
	return (_nick + "!" + _user + "@" + _ip);
}

// REgistration state
bool Client::hasPass() const
{
	return (_hasPass);
}
void Client::markPass()
{
	_hasPass = true;
}

bool Client::hasNick() const
{
	return (!_nick.empty());
}

bool Client::hasUser() const
{
	return (!_user.empty());
}

bool Client::isRegistered() const
{
	return (_registered);
}

void Client::markRegistered()
{
	_registered = true;
}

// channel management

void Client::joinChannel(Channel *channel)
{
	_channels.push_back(channel);
}

// std remove to shifts the element to the end and erase from memory
void Client::leaveChannel(Channel *channel)
{
	_channels.erase(std::remove(_channels.begin(), _channels.end(), channel),
		_channels.end());
}

const std::vector<Channel *> &Client::getChannels() const
{
	return (_channels);
}

// BUFFER
void Client::appendToBuffer(const char *data, size_t len)
{
	(void)data;
	(void)len;
}
bool Client::extractLine(std::string &out)
{
	(void)out;
	return (false);
}
bool Client::isInboundOverflow() const
{
	return (false);
}
void Client::send(const std::string &msg)
{
	(void)msg;
}
bool Client::flushOutbound()
{
	return (true);
}
bool Client::hasPendingOutbound() const
{
	return (false);
}