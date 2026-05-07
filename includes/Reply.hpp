/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Reply.hpp                                          :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: zmin <zmin@student.42bangkok.com>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/05/07 12:00:00 by zmin              #+#    #+#             */
/*   Updated: 2026/05/07 12:00:00 by zmin             ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef REPLY_HPP
# define REPLY_HPP

# include <string>

/*
 * Reply
 * -----
 * Single source of truth for every numeric reply and protocol-formatted
 * message the server sends. No handler should hand-format ":server 401 ..."
 * inline -- always go through this namespace.
 *
 * All functions return a string WITHOUT trailing \r\n. Client::send appends it.
 *
 * The `serverName` argument is the prefix on numeric replies (e.g. "irc.42.fr").
 * Pass Server::getName() at the call site, or wrap in a helper.
 */
namespace Reply
{
	// ---- success / informational ----------------------------------------
	std::string welcome           (const std::string& serverName, const std::string& nick); // 001
	std::string yourHost          (const std::string& serverName, const std::string& nick); // 002
	std::string created           (const std::string& serverName, const std::string& nick); // 003
	std::string myInfo            (const std::string& serverName, const std::string& nick); // 004

	std::string topic             (const std::string& serverName, const std::string& nick,
	                               const std::string& chan, const std::string& topic);     // 332
	std::string noTopic           (const std::string& serverName, const std::string& nick,
	                               const std::string& chan);                                // 331
	std::string namReply          (const std::string& serverName, const std::string& nick,
	                               const std::string& chan, const std::string& names);     // 353
	std::string endOfNames        (const std::string& serverName, const std::string& nick,
	                               const std::string& chan);                                // 366

	// ---- errors ---------------------------------------------------------
	std::string errNoSuchNick     (const std::string& serverName, const std::string& nick,
	                               const std::string& target);                              // 401
	std::string errNoSuchChannel  (const std::string& serverName, const std::string& nick,
	                               const std::string& chan);                                // 403
	std::string errCannotSendToChan(const std::string& serverName, const std::string& nick,
	                               const std::string& chan);                                // 404
	std::string errNoRecipient    (const std::string& serverName, const std::string& nick,
	                               const std::string& cmd);                                 // 411
	std::string errNoTextToSend   (const std::string& serverName, const std::string& nick); // 412
	std::string errUnknownCommand (const std::string& serverName, const std::string& nick,
	                               const std::string& cmd);                                 // 421
	std::string errNoNicknameGiven(const std::string& serverName, const std::string& nick); // 431
	std::string errErroneousNick  (const std::string& serverName, const std::string& nick,
	                               const std::string& bad);                                 // 432
	std::string errNickInUse      (const std::string& serverName, const std::string& nick,
	                               const std::string& bad);                                 // 433
	std::string errUserNotInChan  (const std::string& serverName, const std::string& nick,
	                               const std::string& target, const std::string& chan);    // 441
	std::string errNotOnChannel   (const std::string& serverName, const std::string& nick,
	                               const std::string& chan);                                // 442
	std::string errUserOnChannel  (const std::string& serverName, const std::string& nick,
	                               const std::string& target, const std::string& chan);    // 443
	std::string errNotRegistered  (const std::string& serverName);                          // 451
	std::string errNeedMoreParams (const std::string& serverName, const std::string& nick,
	                               const std::string& cmd);                                 // 461
	std::string errAlreadyRegistered(const std::string& serverName, const std::string& nick);// 462
	std::string errPasswdMismatch (const std::string& serverName, const std::string& nick); // 464
	std::string errChannelIsFull  (const std::string& serverName, const std::string& nick,
	                               const std::string& chan);                                // 471
	std::string errUnknownMode    (const std::string& serverName, const std::string& nick,
	                               char modeChar);                                          // 472
	std::string errInviteOnlyChan (const std::string& serverName, const std::string& nick,
	                               const std::string& chan);                                // 473
	std::string errBadChannelKey  (const std::string& serverName, const std::string& nick,
	                               const std::string& chan);                                // 475
	std::string errChanOPrivsNeeded(const std::string& serverName, const std::string& nick,
	                               const std::string& chan);                                // 482

	// ---- broadcast formatters (with sender prefix) ----------------------
	// These return ":nick!user@ip COMMAND target [...]" without trailing \r\n.
	std::string joinMsg           (const std::string& prefix, const std::string& chan);
	std::string partMsg           (const std::string& prefix, const std::string& chan,
	                               const std::string& reason);
	std::string quitMsg           (const std::string& prefix, const std::string& reason);
	std::string kickMsg           (const std::string& prefix, const std::string& chan,
	                               const std::string& target, const std::string& reason);
	std::string privmsgMsg        (const std::string& prefix, const std::string& target,
	                               const std::string& text);
	std::string noticeMsg         (const std::string& prefix, const std::string& target,
	                               const std::string& text);
	std::string topicMsg          (const std::string& prefix, const std::string& chan,
	                               const std::string& topic);
	std::string modeMsg           (const std::string& prefix, const std::string& chan,
	                               const std::string& modes, const std::string& args);
	std::string inviteMsg         (const std::string& prefix, const std::string& target,
	                               const std::string& chan);
}

#endif
