/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Channel.hpp                                        :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: wmin-kha <wmin-kha@student.42bangkok.co    +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/05/05 21:32:58 by zmin              #+#    #+#             */
/*   Updated: 2026/05/13 19:32:28 by wmin-kha         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef CHANNEL_HPP
#define CHANNEL_HPP

#include <string>
#include <vector>

class Client;

/*
 * Channel
 * -------
 * A single IRC chat room. Owns membership, operator list, modes, topic.
 *
 * Ownership: created by Server::getOrCreateChannel, destroyed by
 * Server::removeChannelIfEmpty. The CommandRouter never news/deletes a Channel.
 *
 * Channel does NOT own Client*. It only borrows pointers. When a Client is
 * destroyed, Server is responsible for removing it from every Channel first.
 */
class Channel
{
public:
	explicit Channel(const std::string &name);
	~Channel();

	const std::string &getName() const;
	const std::string &getTopic() const;
	void setTopic(const std::string &topic);

	// ---- membership ------------------------------------------------------
	void addMember(Client *c);
	void removeMember(Client *c);
	bool hasMember(Client *c) const;
	const std::vector<Client *> &getMembers() const;
	size_t memberCount() const;

	// ---- operators -------------------------------------------------------
	const std::vector<Client *> &getOperators() const;
	void addOperator(Client *c);
	void removeOperator(Client *c);
	bool isOperator(Client *c) const;

	// ---- modes (subject 42 requires: i, t, k, o, l) ----------------------
	bool isInviteOnly() const;
	void setInviteOnly(bool v);

	bool isTopicLocked() const; // +t
	void setTopicLocked(bool v);

	const std::string &getKey() const; // +k
	void setKey(const std::string &key);
	bool hasKey() const;
	void clearKey();

	int getUserLimit() const; // +l (-1 = unlimited)
	void setUserLimit(int n);
	void clearUserLimit();

	// ---- invites (for +i) -----------------------------------------------
	void invite(Client *c);
	bool isInvited(Client *c) const;
	void consumeInvite(Client *c); // remove after successful JOIN

	// ---- broadcast -------------------------------------------------------
	// Sends `msg` to every member. If `except` is non-NULL, that client is
	// skipped (used to suppress local echo for the sender).
	void broadcast(const std::string &msg, Client *except);

private:
	Channel(const Channel &);
	Channel &operator=(const Channel &);

	std::string _name;
	std::string _topic;
	std::string _key;

	std::vector<Client *> _members;
	std::vector<Client *> _operators;
	std::vector<Client *> _invited;

	bool _inviteOnly;
	bool _topicLocked;
	bool _hasKey;
	int _userLimit;
};

#endif
