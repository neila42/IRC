/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Server.cpp                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: inaranjo <inaranjo <inaranjo@student.42    +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/04/19 11:03:35 by inaranjo          #+#    #+#             */
/*   Updated: 2024/05/07 09:16:04 by inaranjo         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "../Includes/Server.hpp"

bool Server::_signal = false;

Server::Server() : _socketFD(-1) {}

Server::~Server() {}

Server::Server(Server const &src) { *this = src; }

Server &Server::operator=(Server const &src)
{
    if (this != &src)
    {
        this->_port = src._port;
        this->_socketFD = src._socketFD;
        this->_pass = src._pass;
        this->_capLS = src._capLS;
        this->_clients = src._clients;
        this->_channels = src._channels;
        this->_pfds = src._pfds;
        this->_serverAddr = src._serverAddr;
        this->_clientAddr = src._clientAddr;
        this->_newConnection = src._newConnection;
    }
    return *this;
}

void Server::handleSignal(int signum)
{
    (void)signum;
    std::cout << std::endl
              << "Signal Received!" << std::endl;
    Server::_signal = true;
}

Channel *Server::createChannel(const std::string &name, const std::string &key, Client *client)
{
    Channel newChannel;
    // remove first character # of string name
    newChannel.setName(name.substr(1));
    newChannel.setPass(key);

    // When a client creates a channel, they are automatically an operator
    newChannel.addOperator(client);

    // newChannel.addClient(client);
    _channels.push_back(newChannel);

    return &_channels.back();
}

int Server::getFD() { return this->_socketFD; }
int Server::getPort() { return this->_port; }
std::string Server::getPass() { return this->_pass; }
Client *Server::getClient(int fd)
{
    for (size_t i = 0; i < this->_clients.size(); i++)
    {
        if (this->_clients[i].getFD() == fd)
            return &this->_clients[i];
    }
    return nullptr;
}
Client *Server::getNickClient(std::string nickname)
{
    for (size_t i = 0; i < this->_clients.size(); i++)
    {
        if (this->_clients[i].getNickName() == nickname)
            return &this->_clients[i];
    }
    return nullptr;
}
Channel *Server::getChannel(std::string name)
{

    if (name[0] == '#')
        name = name.substr(1);
    for (size_t i = 0; i < this->_channels.size(); i++)
    {
        if (this->_channels[i].getName() == name)
            return &this->_channels[i];
    }
    return nullptr;
}

std::vector<Channel> &Server::getChannels() { return _channels; }

void Server::setFD(int fd) { this->_socketFD = fd; }
void Server::setPort(int port) { this->_port = port; }
void Server::setPass(std::string password) { this->_pass = password; }

void Server::storeClient(Client newClient) { this->_clients.push_back(newClient); }
void Server::storePfds(pollfd newFd) { this->_pfds.push_back(newFd); }
void Server::storeChannel(Channel newChannel) { this->_channels.push_back(newChannel); }

void Server::rmClient(int fd)
{
    for (size_t i = 0; i < this->_clients.size(); i++)
    {
        if (this->_clients[i].getFD() == fd)
        {
            this->_clients.erase(this->_clients.begin() + i);
            return;
        }
    }
}
void Server::rmFds()
{
    for (size_t i = 0; i < _clients.size(); i++)
    {
        std::cout << RED << "Client <" << _clients[i].getFD() << "> Disconnected" << RESET << std::endl;
        close(_clients[i].getFD());
    }
    if (_socketFD != -1)
    {
        std::cout << RED << "Server <" << _socketFD << "> Disconnected" << RESET << std::endl;
        close(_socketFD);
    }
}
void Server::rmPfds(int fd)
{
    for (size_t i = 0; i < this->_pfds.size(); i++)
    {
        if (this->_pfds[i].fd == fd)
        {
            this->_pfds.erase(this->_pfds.begin() + i);
            return;
        }
    }
}

void Server::rmClientFromChan(int fd)
{
    for (size_t i = 0; i < this->_channels.size(); i++)
    {
        int flag = 0;
        if (this->_channels[i].getClientFd(fd))
        {
            this->_channels[i].rmClientFd(fd);
            flag = 1;
        }
        else if (this->_channels[i].getAdminFd(fd))
        {
            this->_channels[i].rmAdminFd(fd);
            flag = 1;
        }
        if (this->_channels[i].getNbClients() == 0)
        {
            this->_channels.erase(this->_channels.begin() + i);
            i--;
            continue;
        }
        if (flag)
        {
            std::string rpl = ":" + getClient(fd)->getNickName() + "!~" + getClient(fd)->getUserName() + "@localhost 3 QUIT Quit\r\n";
            this->_channels[i].sendMsgToAll(rpl);
        }
    }
}

void Server::sendMsg(std::string msg, int fd)
{
    if (send(fd, msg.c_str(), msg.size(), 0) == -1)
        std::cerr << "msg send() failed" << std::endl;
}
void Server::sendErrToClient(int code, std::string clientname, int fd, std::string msg)
{
    std::stringstream ss;
    ss << ":localhost 2" << code << " " << clientname << msg;
    std::string resp = ss.str();
    if (send(fd, resp.c_str(), resp.size(), 0) == -1)
        std::cerr << "send() failed" << std::endl;
}
void Server::sendErrInChannel(int code, std::string clientname, std::string channelname, int fd, std::string msg)
{
    std::stringstream ss;
    ss << ":localhost1 " << code << " " << clientname << " " << channelname << msg;
    std::string resp = ss.str();
    if (send(fd, resp.c_str(), resp.size(), 0) == -1)
        std::cerr << "send() failed" << std::endl;
}

void Server::sendWelcome() const
{
    std::cout << "*****************************************\n"
                 "*                                       *\n"
                 "*   SERVER IRC                          *\n"
                 "*                                       *\n"
                 "*   By: Isaac/Leonel/Neila              *\n"
                 "*                                       *\n"
                 "*****************************************\n\n";
}

void *Server::blinkDots(void *arg)
{
    (void)arg;

    std::string waitingMSG = "Waiting for a client connection";
    for (int i = 0; i < 4; ++i)
    {
        std::cout << "\r" << ORANGE << waitingMSG << RESET;
        std::cout.flush();
        for (int j = 0; j < 3; ++j)
        {
            std::cout << '.';
            std::cout.flush();
            usleep(250000); // wait(half a second)
        }
        usleep(250000);
    }
    std::cout << std::endl;
    return NULL;
}

void Server::run(int port, std::string pass)
{
    _port = port;
    _pass = pass;
    _capLS = false;
    createSocket();
    sendWelcome();

    pthread_t thread_id;
    if (pthread_create(&thread_id, NULL, Server::blinkDots, NULL) != 0)
    {
        std::cerr << "Failed creating thread" << std::endl;
        return;
    }
    while (_signal == false)
    {
        if ((poll(&_pfds[0], _pfds.size(), -1) == -1) && _signal == false)
            throw(std::runtime_error("poll() failed"));
        for (size_t i = 0; i < _pfds.size(); i++)
        {
            if (_pfds[i].revents & POLLIN)
            {
                if (_pfds[i].fd == _socketFD)
                    this->handleClientConnection();
                else
                    this->handleClientInput(_pfds[i].fd);
            }
        }
    }
    if (pthread_join(thread_id, NULL) != 0)
    {
        std::cerr << "Failed to join blinking dots thread" << std::endl;
    }
    rmFds();
}

void Server::handleClientDisconnect(int fd)
{
    std::cout << RED << "Client <" << fd << "> Disconnected" << RESET << std::endl;
    rmClientFromChan(fd);
    rmClient(fd);
    rmPfds(fd);
    close(fd);
}

void Server::createSocket()
{
    int reuseAddr = 1;
    bool socketCreationFailed = false;
    bool setsockoptFailed = false;
    bool setNonBlockingFailed = false;
    bool bindFailed = false;
    bool listenFailed = false;

    _serverAddr.sin_family = AF_INET;
    _serverAddr.sin_addr.s_addr = INADDR_ANY;
    _serverAddr.sin_port = htons(_port);

    _socketFD = socket(AF_INET, SOCK_STREAM, 0);
    if (_socketFD == -1)
    {
        std::cerr << "Failed to create socket" << std::endl;
        socketCreationFailed = true;
    }

    if (!socketCreationFailed && setsockopt(_socketFD, SOL_SOCKET, SO_REUSEADDR, &reuseAddr, sizeof(reuseAddr)) == -1)
    {
        std::cerr << "Failed to set option (SO_REUSEADDR) on socket" << std::endl;
        setsockoptFailed = true;
    }

    if (!socketCreationFailed && !setsockoptFailed && fcntl(_socketFD, F_SETFL, O_NONBLOCK) == -1)
    {
        std::cerr << "Failed to set option (O_NONBLOCK) on socket" << std::endl;
        setNonBlockingFailed = true;
    }

    if (!socketCreationFailed && !setsockoptFailed && !setNonBlockingFailed &&
        bind(_socketFD, (struct sockaddr *)&_serverAddr, sizeof(_serverAddr)) == -1)
    {
        std::cerr << "Failed to bind socket" << std::endl;
        bindFailed = true;
    }

    if (!socketCreationFailed && !setsockoptFailed && !setNonBlockingFailed && !bindFailed &&
        listen(_socketFD, SOMAXCONN) == -1)
    {
        std::cerr << "listen() failed" << std::endl;
        listenFailed = true;
    }

    if (socketCreationFailed || setsockoptFailed || setNonBlockingFailed || bindFailed || listenFailed)
    {
        close(_socketFD);
        throw std::runtime_error("Failed to create socket");
    }
    _newConnection.fd = _socketFD;
    _newConnection.events = POLLIN;
    _newConnection.revents = 0;
    _pfds.push_back(_newConnection);
}

void Server::handleClientConnection()
{
    Client newClient;
    memset(&_clientAddr, 0, sizeof(_clientAddr));
    socklen_t addrlen = sizeof(_clientAddr);
    int cliSocket = accept(_socketFD, (sockaddr *)&_clientAddr, &addrlen);
    if (cliSocket == -1)
    {
        std::cout << "accept() failed" << std::endl;
        return;
    }
    if (fcntl(cliSocket, F_SETFL, O_NONBLOCK) == -1)
    {
        std::cout << "fcntl() failed" << std::endl;
        return;
    }
    _newConnection.fd = cliSocket;
    _newConnection.events = POLLIN;
    _newConnection.revents = 0;
    newClient.setFD(cliSocket);
    newClient.setClientIP(inet_ntoa((_clientAddr.sin_addr)));
    _clients.push_back(newClient);
    _pfds.push_back(_newConnection);
    std::cout << GREEN << "Client <" << cliSocket << "> Connected" << RESET << std::endl;
}

void Server::handleClientInput(int fd)
{
    std::vector<std::string> cmds;
    char buff[1024];
    memset(buff, 0, sizeof(buff));
    Client *currentClient = getClient(fd);
    ssize_t bytes = recv(fd, buff, sizeof(buff) - 1, 0);
    if (bytes <= 0)
        handleClientDisconnect(fd);
    else
    {
        currentClient->setBuffer(buff);
        if (currentClient->getBuffer().find_first_of("\r\n") == std::string::npos)
            return;
        cmds = parseBuffer(currentClient->getBuffer());
        std::cout << "Client <" << currentClient->getFD() << "> Debug: " << currentClient->getBuffer();
        for (size_t i = 0; i < cmds.size(); i++)
            execCmd(cmds[i], fd);
        if (getClient(fd))
            getClient(fd)->clearBuffer();
    }
}

bool Server::checkAuth(int fd)
{
    if (!getClient(fd) || 
            getClient(fd)->getNickName().empty() || 
            getClient(fd)->getUserName().empty() || 
            getClient(fd)->getNickName() == "*" || 
            !getClient(fd)->getLogedIn())
        return false;
    return true;
}

void Server::rmChannel(std::string name)
{

    if (name[0] == '#')
        name = name.substr(1);

    for (size_t i = 0; i < _channels.size(); i++)
    {
        if (_channels[i].getName() == name)
        {
            _channels.erase(_channels.begin() + i);
            return;
        }
    }
}

std::vector<std::string> Server::parseBuffer(std::string str)
{
    std::vector<std::string> vec;
    std::istringstream stm(str);
    std::string line;
    while (std::getline(stm, line))
    {
        size_t pos = line.find_first_of("\r\n");
        if (pos != std::string::npos)
            line = line.substr(0, pos);
        vec.push_back(line);
    }
    return vec;
}
std::vector<std::string> Server::parseCmd(std::string &cmd)
{
    std::vector<std::string> vec;
    std::istringstream stm(cmd);
    std::string token;
    while (stm >> token)
    {
        vec.push_back(token);
        token.clear();
    }
    return vec;
}

void Server::setCapStatus(bool value)
{
    _capLS = value;
}

bool Server::getCapStatus()
{
    return _capLS;
}

bool Server::isNicknameInUse(std::string &nickname)
{
    for (size_t i = 0; i < _clients.size(); i++)
    {
        if (_clients[i].getNickName() == nickname)
            return true;
    }
    return false;
}

void Server::execCmd(std::string &cmd, int fd)
{
    Commands cmdHandler(*this);
    cmdHandler.handleCommand(fd, cmd);
}
