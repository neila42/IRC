/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Server.hpp                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: Probook <Probook@student.42.fr>            +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/03/22 23:17:09 by inaranjo          #+#    #+#             */
/*   Updated: 2024/04/02 14:30:26 by Probook          ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef SERVER_HPP
#define SERVER_HPP

#include <iostream>
#include <stdexcept>
#include <cerrno>
#include <string>
#include <sys/socket.h>
#include <sys/poll.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <poll.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <algorithm>
#include <vector>
#include <map>

class Server;

#include "Client.hpp"
#include "Channel.hpp"
#include "command/Parser.hpp"

#define MAX_USER 500

class Server
{
    private:
        const std::string       _host;
        const std::string       _port;
        const std::string       _pass;
        int                     _serverStatus;
        int                     _socketFD;

        std::vector<pollfd>     _pfds;
        std::vector<Channel *>  _channels;
        std::map<int, Client *> _clients;

        Parser*                 _parser;

        Server();
        Server(const Server& src);

    public:
        Server(const std::string &port, const std::string &pass);
        ~Server();

        /*Run the loop server*/
        void            run();

        /*Accesssors*/
        Client*         getClient(const std::string &nickname);
        Channel*        getChannel(const std::string &name);
        std::string     getPassword() const;
        std::vector<Channel*> getAllChannels() const;
       
        /*Handle CLient network to the SERVER*/
        void            handleClientMessage(int fd);
        void            handleClientConnection();
        void            handleClientDisconnect(int fd);
        std::string     recvMsgFrom(int fd);

        /* Etablish connection to the NET*/
        int             createSocket();

        /* Generate a New CHANNEL, under construction*/
        Channel*         createChannel(const std::string &name, const std::string &key, Client *client);

       
};

#endif
