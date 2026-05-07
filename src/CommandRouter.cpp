/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   CommandRouter.cpp                                  :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: zmin <zmin@student.42bangkok.com>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/05/07 12:00:00 by zmin              #+#    #+#             */
/*   Updated: 2026/05/07 12:00:00 by zmin             ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

// PHASE 0 STUB. Person B replaces this whole file in their Phase 1-3.
// Do not add logic here -- leave it empty so the build stays green until B
// drops in the real router.

#include "CommandRouter.hpp"

CommandRouter::CommandRouter(Server& server)
	: _server(server)
{
}

CommandRouter::~CommandRouter()
{
}

void CommandRouter::dispatch(Client&, const std::string&)
{
}

IRCMessage CommandRouter::tokenize(const std::string&)
{
	return IRCMessage();
}
