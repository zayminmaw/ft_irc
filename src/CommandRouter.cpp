/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   CommandRouter.cpp                                  :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: zmin <zmin@student.42bangkok.com>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/05/07 12:00:00 by zmin              #+#    #+#             */
/*   Updated: 2026/05/11 20:58:33 by zmin             ###   ########.fr       */
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
