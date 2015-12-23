//
// Created by qurbonzoda on 23.11.15.
//

#ifndef MAFIA_PLAYER_H
#define MAFIA_PLAYER_H

#include "Room.h"
#include "Server.h"
#include <boost/shared_ptr.hpp>
#include <boost/enable_shared_from_this.hpp>
#include <boost/asio/ip/udp.hpp>
#include "network.h"

class Room;
class Server;
class Connection;

class Player : public boost::enable_shared_from_this<Player>
{

public:
    enum Character
    {
        Moderator, Detective, Doctor, Mafia, Villager
    };
    Player(size_t id);
    Player(boost::shared_ptr< boost::asio::ip::udp::endpoint > endpoint);
    size_t getId();
    boost::shared_ptr<boost::asio::ip::udp::endpoint> getEndpoint();
    boost::shared_ptr< Room > getRoom();

    const Character &getCharacter() const;
    void setCharacter(const Character &character);
    void setId(size_t id);
    const std::string &getLogin() const;
    void setLogin(const std::string &login);
    void setEndpoint(const boost::shared_ptr<boost::asio::ip::udp::endpoint> &endpoint);
    bool setRoom(boost::shared_ptr< Room > room);
    const std::string &getPassword() const;
    void setPassword(const std::string &password);
    size_t getRoom_position() const;
    void setRoom_position(size_t room_position);
    const boost::shared_ptr<Connection> &getConnection() const;
    void setConnection(const boost::shared_ptr<Connection> &connection);
    ~Player();

private:
    Character character;
    size_t id;
    size_t room_position;
    std::string login;
    std::string password;
    boost::shared_ptr< Room > room;
    boost::shared_ptr< Connection > connection;
    boost::shared_ptr< boost::asio::ip::udp::endpoint > endpoint;

};

#endif //MAFIA_PLAYER_H
