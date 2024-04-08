/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   User.cpp                                           :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: inaranjo <inaranjo <inaranjo@student.42    +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/03/23 16:12:19 by inaranjo          #+#    #+#             */
/*   Updated: 2024/03/24 11:50:11 by inaranjo         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "command/Command.hpp"

// stntax: USER <username> <hostname> <servername> <realname>
// USER johndoe clienthost example.com John Doe

/*
      Command: USER
   Parameters: <username> <hostname> <servername> <realname>
   The USER message is used at the beginning of connection to specify
   the username, hostname, servername and realname of s new user.  It is
   also used in communication between servers to indicate new user
   arriving on IRC, since only after both USER and NICK have been
   received from a client does a user become registered.
   Between servers USER must to be prefixed with client's NICKname.
   Note that hostname and servername are normally ignored by the IRC
   server when the USER command comes from a directly connected client
   (for security reasons), but they are used in server to server
   communication.  This means that a NICK must always be sent to a
   remote server when a new user is being introduced to the rest of the
   network before the accompanying USER is sent.
   It must be noted that realname parameter must be the last parameter,
   because it may contain space characters and must be prefixed with a
   colon (':') to make sure this is recognised as such.
   Since it is easy for a client to lie about its username by relying
   solely on the USER message, the use of an "Identity Server" is
   recommended.  If the host which a user connects from has such a
   server enabled the username is set to that as in the reply from the
   "Identity Server".
   Numeric Replies:
           ERR_NEEDMOREPARAMS              ERR_ALREADYREGISTRED
*/

User::User(Server* srv, bool auth) : Command(srv, auth) {}

User::~User() {}

void    User::execute(Client* client, std::vector<std::string> args)
{
    if (client->registrationCheck())
    {
        client->reply(ERR_ALREADYREGISTERED(client->getNickname()));
        return;
    }

    if (args.size() < 4) 
    {
        client->reply(ERR_NEEDMOREPARAMS(client->getNickname(), "USER"));
        return;
    }

    client->setUsername(args[0]);
    client->setRealname(args[3]);
    client->welcome();
}
