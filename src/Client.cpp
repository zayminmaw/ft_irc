/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Client.cpp                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: wmin-kha <wmin-kha@student.42bangkok.co    +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/05/07 12:00:00 by zmin              #+#    #+#             */
/*   Updated: 2026/05/13 18:49:33 by wmin-kha         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "Client.hpp"
#include "algorithm"
#include <sys/socket.h> // For send()
#include <cerrno>		// For errno EAGAIN, EWOULDBLOCK
#include <iostream>

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

// INBOUND BUFFER
void Client::appendToBuffer(const char *data, size_t len)
{
	// append() because += will stop at the first null byte it see
	// append() for every single raw byte from recv()
	//! Protect server from null-byte injection attack
	_inBuffer.append(data, len);
}

bool Client::extractLine(std::string &out)
{
	size_t pos = _inBuffer.find("\r\n");

	// if no full line exist yet, return false to wait for more data
	if (pos == std::string::npos)
	{
		return false;
	}

	// extract the command (exculde \r\n) into out-parameter
	out = _inBuffer.substr(0, pos);

	// erase the command and 2 bytes for \r\n from the front
	_inBuffer.erase(0, pos + 2);

	return true;
}

bool Client::isInboundOverflow() const
{
	// Anti-DDoS limit
	// if this is true server must drop the client
	// reason: "Excess flood"
	return _inBuffer.length() > 8192;
}

// OUTBOUND BUFFER
void Client::queueOutbound(const std::string &msg)
{
	_outBuffer += msg;

	// check if the message already ends with \r\n
	// if not, append it sefely
	size_t len = _outBuffer.length();
	if (len < 2 || _outBuffer.substr(len - 2) != "\r\n")
	{
		_outBuffer += "\r\n";
	}
}
bool Client::flushOutbound()
{
	if (_outBuffer.empty())
	{
		return true;
	}

	int bytes_send = send(_fd, _outBuffer.c_str(), _outBuffer.length(), 0);

	if (bytes_send > 0)
	{
		// remove the bytes that got through
		_outBuffer.erase(0, bytes_send);
		return true;
	}
	else if (bytes_send == -1)
	{
		// sice socket is O_NONBLOCk, send() returning -1 isn't always a crash
		// EWOULDBLOC or EAGAIN means Kernel's network buffer full right now
		if (errno == EWOULDBLOCK || errno == EAGAIN)
		{
			return true; // not fatal error, try again with next POLLOUT
		}
		//  if it's other error
		// like EPIPE from a disconnected user
		// its' fatal
		return false;
	}
	// bytes_send == 0 means the connection was closed by peer
	return false;
}

bool Client::hasPendingOutbound() const
{
	return !_outBuffer.empty();
}
