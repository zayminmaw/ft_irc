/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Client.cpp                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: wmin-kha <wmin-kha@student.42bangkok.co    +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/05/07 12:00:00 by zmin              #+#    #+#             */
/*   Updated: 2026/05/28 18:56:04 by wmin-kha         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "Client.hpp"
#include "algorithm"
#include <sys/socket.h> // For send()
#include <cerrno>		// For errno EAGAIN, EWOULDBLOCK
#include <iostream>

Client::Client(int fd, const std::string &ip) : _fd(fd), _ip(ip),
												_hasPass(false), _registered(false), _markedForQuit(false), _overflowTriggered(false)

{
	_connectionTime = time(NULL);
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

void Client::markForQuit()
{
	_markedForQuit = true;
}

bool Client::isMarkedForQuit() const
{
	return _markedForQuit;
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
	// Delimit on '\n'. RFC 1459 mandates CRLF, but tolerate a bare LF so that
	// clients/tools that send "\n" only (e.g. plain `nc` without -C) still work.
	size_t pos = _inBuffer.find('\n');

	// if no full line exist yet, return false to wait for more data
	if (pos == std::string::npos)
	{
		return false;
	}

	// Strip an optional trailing '\r' so "\r\n" and "\n" both yield the same body.
	size_t end = pos;
	if (end > 0 && _inBuffer[end - 1] == '\r')
		--end;

	// RFC 1459 §2.3: a message is at most 512 bytes including CRLF,
	// so the body cannot exceed 510 bytes. Truncate rather than reject.
	if (end > 510)
		end = 510;
	out = _inBuffer.substr(0, end);

	// still consume the full line up to and including the '\n'
	_inBuffer.erase(0, pos + 1);

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
	// SendQ cap: once the buffer crosses 64KB the client is doomed; drop new
	// bytes so the cap stays sticky and the server can spot it next loop.
	if (_overflowTriggered)
		return;

	// strip any trailing CRLF so we can measure / truncate the body alone
	std::string body = msg;
	if (body.length() >= 2 && body.substr(body.length() - 2) == "\r\n")
		body.erase(body.length() - 2);

	// RFC 1459 §2.3: a message line is at most 512 bytes including CRLF.
	// Truncate the body to 510 so the wire line never exceeds the limit.
	if (body.length() > 510)
		body.erase(510);

	// 3. Add to the buffer
	_outBuffer += body;
	_outBuffer += "\r\n";

	// std::cout << "[DEBUG] FD " << _fd << " OutBuffer size: " << _outBuffer.length() << std::endl;

	if (_outBuffer.length() >= 65536)
	{
		// std::cout << "[DEBUG] 64KB KILL SWITCH TRIPPED FOR FD " << _fd << "!" << std::endl;
		_overflowTriggered = true;
		_outBuffer.clear();
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

bool Client::isOutboundOverflow() const
{
	return _overflowTriggered;
}

void Client::log() const
{
	std::cout << getNick() << "\n";
	std::cout << getUser() << "\n";
	std::cout << isRegistered() << "\n";
}

time_t Client::getConnectionTime() const
{
	return _connectionTime;
}