/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Client.hpp                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: wmin-kha <wmin-kha@student.42bangkok.co    +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/05/04 20:05:19 by zmin              #+#    #+#             */
/*   Updated: 2026/05/08 19:21:42 by wmin-kha         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef CLIENT_HPP
# define CLIENT_HPP

# include <string>
# include <vector>

class Channel;

/*
 * Client
 * ------
 * Wraps a single connected socket. Owns its inbound/outbound TCP buffers
 * and tracks IRC registration state (PASS -> NICK -> USER -> registered).
 *
 * Ownership: created by Server on accept(), destroyed by Server on disconnect.
 * The CommandRouter never news/deletes a Client.
 *
 * Threading: single-threaded. No locks.
 */
class Client
{
public:
	Client(int fd, const std::string& ip);
	~Client();

	// ---- identity ---------------------------------------------------------
	int                getFd() const;
	const std::string& getNick() const;
	const std::string& getUser() const;
	const std::string& getIp() const;

	// "nick!user@ip" -- used as the prefix on every outbound broadcast.
	std::string        getPrefix() const;

	void               setNick(const std::string& nick);
	void               setUser(const std::string& user);

	// ---- registration state machine --------------------------------------
	bool               hasPass() const;
	bool               hasNick() const;
	bool               hasUser() const;
	bool               isRegistered() const;

	void               markPass();           // PASS accepted
	void               markRegistered();     // all three steps satisfied

	// ---- inbound buffer (network layer writes; parser doesn't touch) -----
	void               appendToBuffer(const char* data, size_t len);

	// Pops one complete line (without trailing \r\n) into `out`.
	// Returns true if a line was extracted, false if buffer has no full line yet.
	// Lines exceeding 510 bytes are truncated per RFC 1459.
	bool               extractLine(std::string& out);

	// True if the inbound buffer has grown past the safety cap without
	// producing a line. Server should disconnect on true.
	bool               isInboundOverflow() const;

	// ---- outbound buffer -------------------------------------------------
	// Queues a message for sending. Appends \r\n if not already present.
	// Actual write happens in the poll loop's POLLOUT branch.
	void               send(const std::string& msg);

	// Server-side: drain queued bytes into the socket. Returns false on fatal
	// send error (EPIPE, ECONNRESET, etc.) so Server can disconnect.
	bool               flushOutbound();

	bool               hasPendingOutbound() const;

	// ---- channel membership ---------------------------------------------
	void               joinChannel(Channel* ch);
	void               leaveChannel(Channel* ch);
	const std::vector<Channel*>& getChannels() const;

private:
	Client(const Client&);
	Client& operator=(const Client&);

	int                     _fd;
	std::string             _ip;
	std::string             _nick;
	std::string             _user;

	std::string             _inBuffer;
	std::string             _outBuffer;

	bool                    _hasPass;
	bool                    _registered;

	std::vector<Channel*>   _channels;
};

#endif
