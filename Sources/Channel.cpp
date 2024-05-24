/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Channel.cpp                                        :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: inaranjo <inaranjo@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/04/19 19:49:18 by inaranjo          #+#    #+#             */
/*   Updated: 2024/05/07 13:02:24 by inaranjo         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "../Includes/Channel.hpp"

Channel::Channel() : _srv(*new Server())
{
    _topic = 0;
    _key = 0;
    _maxClients = 0;
    _isTopicRestricted = false;
    _name = "";
    _password = "";
    _created_at = "";
    _topicName = "";
}

Channel::Channel(Server &server) : _srv(server)
{
    _topic = 0;
    _key = 0;
    _maxClients = 0;
    _isTopicRestricted = false;
    _name = "";
    _password = "";
    _created_at = "";
    _topicName = "";
}

Channel::~Channel()
{
}

Channel::Channel(Channel const &src) : _srv(src._srv)
{

    *this = src;
}

Channel &Channel::operator=(Channel const &src)
{
    if (this != &src)
    {
        _topic = src._topic;
        _key = src._key;
        _maxClients = src._maxClients;
        _isTopicRestricted = src._isTopicRestricted;
        _name = src._name;
        _password = src._password;
        _created_at = src._created_at;
        _topicName = src._topicName;
        _clients = src._clients;
        _invited = src._invited;
        _admins = src._admins;
    }
    return *this;
}

// Setters
void Channel::setTopic(int topic) { _topic = topic; }
void Channel::setKey(int key) { _key = key; }
void Channel::setMaxUsers(int limit) { _maxClients = limit; }
void Channel::setTopicName(std::string &topic_name) { _topicName = topic_name; }
void Channel::setPass(std::string password) { _password = password; }

void Channel::setName(std::string name) { _name = name; }

void Channel::setInviteOnly(bool flag) { _inviteOnly = flag; }
void Channel::setTopicControl(bool flag) { _isTopicRestricted = flag; }

// Getters
bool Channel::getInviteOnly()
{
    return _inviteOnly;
}
int Channel::getTopic() { return _topic; }
int Channel::getKey() { return _key; }
int Channel::getMaxClients() { return _maxClients; }
int Channel::getNbClients() { return this->_clients.size() + this->_admins.size(); }
bool Channel::getTopicRestric() const { return _isTopicRestricted; }
std::string Channel::getTopicName() { return _topicName; }
std::string Channel::getPass() { return _password; }
std::string Channel::getName() { return _name; }

std::string Channel::getChannelList()
{
    std::string list;
    for (size_t i = 0; i < _admins.size(); i++)
    {
        list += "@" + _admins[i].getNickName();
        if ((i + 1) < _admins.size())
            list += " ";
    }
    if (_clients.size())
        list += " ";
    for (size_t i = 0; i < _clients.size(); i++)
    {
        list += _clients[i].getNickName();
        if ((i + 1) < _clients.size())
            list += " ";
    }
    return list;
}
Client *Channel::getClientFd(int fd)
{
    for (std::vector<Client>::iterator it = _clients.begin(); it != _clients.end(); ++it)
    {
        if (it->getFD() == fd)
            return &(*it);
    }
    return NULL;
}
Client *Channel::getAdminFd(int fd)
{
    for (std::vector<Client>::iterator it = _admins.begin(); it != _admins.end(); ++it)
    {
        if (it->getFD() == fd)
            return &(*it);
    }
    return NULL;
}

Client *Channel::getClientInChannel(std::string name)
{
    for (std::vector<Client>::iterator it = _clients.begin(); it != _clients.end(); ++it)
    {
        if (it->getNickName() == name)
            return &(*it);
    }
    for (std::vector<Client>::iterator it = _admins.begin(); it != _admins.end(); ++it)
    {
        if (it->getNickName() == name)
            return &(*it);
    }
    return NULL;
}

size_t Channel::getSize() const { return _clients.size() + _admins.size(); }

bool Channel::isClientInChannel(int fd)
{

    for (size_t i = 0; i < _clients.size(); i++)
    {
        if (_clients[i].getFD() == fd)
            return true;
    }
    for (size_t i = 0; i < _admins.size(); i++)
    {
        if (_admins[i].getFD() == fd)
            return true;
    }
    return false;
}

bool Channel::isClientInvited(int fd)
{

    for (size_t i = 0; i < _invited.size(); i++)
    {
        if (_invited[i].getFD() == fd)
        {
            return true;
        }
    }
    
    return false;
}

std::vector<Client *> Channel::getClients() const
{
    std::vector<Client *> clientPtrs;
    for (std::vector<Client>::const_iterator it = _clients.begin(); it != _clients.end(); ++it)
    {
        clientPtrs.push_back(const_cast<Client *>(&*it));
    }
    return clientPtrs;
}

bool Channel::isOperator(std::string const nickname)
{
    for (std::vector<Client>::iterator it = _admins.begin(); it != _admins.end(); ++it)
    {
        if (it->getNickName() == nickname)
            return true;
    }
    return false;
}

void Channel::storeClient(Client newClient) { _clients.push_back(newClient); }
void Channel::storeAdmin(Client newClient) { _admins.push_back(newClient); }

void Channel::rmClientFd(int fd)
{
    for (std::vector<Client>::iterator it = _clients.begin(); it != _clients.end(); ++it)
    {
        if (it->getFD() == fd)
        {
            _clients.erase(it);
            break;
        }
    }
}

void Channel::rmAdminFd(int fd)
{
    for (std::vector<Client>::iterator it = _admins.begin(); it != _admins.end(); ++it)
    {
        if (it->getFD() == fd)
        {
            _admins.erase(it);
            break;
        }
    }
}


void Channel::sendMsgToAll(std::string rpl1)
{
    for (size_t i = 0; i < _admins.size(); i++)
        if (send(_admins[i].getFD(), rpl1.c_str(), rpl1.size(), 0) == -1)
            std::cerr << "send() faild" << std::endl;
    for (size_t i = 0; i < _clients.size(); i++)
        if (send(_clients[i].getFD(), rpl1.c_str(), rpl1.size(), 0) == -1)
            std::cerr << "send() faild" << std::endl;
}

void Channel::sendMsgToAll(std::string rpl1, int fd)
{
    for (size_t i = 0; i < _admins.size(); i++)
    {
        if (_admins[i].getFD() != fd)
            if (send(_admins[i].getFD(), rpl1.c_str(), rpl1.size(), 0) == -1)
                std::cerr << "send() faild" << std::endl;
    }
    for (size_t i = 0; i < _clients.size(); i++)
    {
        if (_clients[i].getFD() != fd)
            if (send(_clients[i].getFD(), rpl1.c_str(), rpl1.size(), 0) == -1)
                std::cerr << "send() faild" << std::endl;
    }
}

void Channel::addOperator(Client *client)
{
    // Check if the client is already an operator
    for (std::vector<Client>::iterator it = _admins.begin(); it != _admins.end(); ++it)
    {
        if (it->getNickName() == client->getNickName())
        {
            return;
        }
    }
    _admins.push_back(*client);
}

void Channel::addToInvitedList(Client *client)
{
    for (std::vector<Client>::iterator it = _invited.begin(); it != _invited.end(); ++it)
    {
        if (it->getFD() == client->getFD())
        {
            return;
        }
    }
    _invited.push_back(*client); // Dereference pointer and store the object
}

void Channel::removeOperator(Client *client)
{
    for (std::vector<Client>::iterator it = _admins.begin(); it != _admins.end(); ++it)
    {
        if (it->getFD() == client->getFD())
        {
            _admins.erase(it);
            break;
        }
    }
}

void Channel::changeOperatorStatus(Client *client, const std::string &targetNick, bool adding)
{
    // Vérifier si le client est déjà un opérateur du canal
    if (!isOperator(client->getNickName()))
    {
        _srv.sendMsg(ERR_CHANOPRIVSNEEDED(client->getNickName(), _name), client->getFD());
        return;
    }

    // Recherche du client cible dans le canal
    Client *targetClient = getClientInChannel(targetNick);
    if (!targetClient)
    {
        _srv.sendMsg(ERR_NOSUCHNICK(client->getNickName(), targetNick), client->getFD());
        return;
    }

    // Ajout ou suppression du statut d'opérateur
    if (adding)
    {
        // Ajouter aux opérateurs si pas déjà un opérateur
        if (!isOperator(targetNick))
        {
            storeAdmin(*targetClient);
            _srv.sendMsg(":" + client->getNickName() + " MODE " + _name + " +o " + targetNick, client->getFD());
            sendMsgToAll(":" + client->getNickName() + " MODE " + _name + " +o " + targetNick);
        }
    }
    else
    {
        // Retirer des opérateurs si actuellement un opérateur
        if (isOperator(targetNick))
        {
            removeOperator(targetClient);
            _srv.sendMsg(":" + client->getNickName() + " MODE " + _name + " -o " + targetNick, client->getFD());
            sendMsgToAll(":" + client->getNickName() + " MODE " + _name + " -o " + targetNick);
        }
    }
}

void Channel::broadcastModeChange(const std::string &prefix, const std::string &modes, const std::string &param)
{
    std::string message = ":" + prefix + " MODE " + _name + " " + modes + " " + param + "\r\n";
    sendMsgToAll(message);
}

void Channel::addClient(Client *client)
{
    // Check if the client is already in the channel
    for (std::vector<Client>::iterator it = _clients.begin(); it != _clients.end(); ++it)
    {
        if (it->getNickName() == client->getNickName())
        {
            return; // Client is already in the channel, so do nothing
        }
    }
    _clients.push_back(*client); // Dereference pointer and store the object
}

void Channel::removeClient(int fd)
{
    for (std::vector<Client>::iterator it = _clients.begin(); it != _clients.end(); ++it)
    {
        if (it->getFD() == fd)
        {
            _clients.erase(it);
            break;
        }
    }
}