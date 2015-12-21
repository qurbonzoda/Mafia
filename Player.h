//
// Created by qurbonzoda on 23.11.15.
//

#ifndef MAFIA_PLAYER_H
#define MAFIA_PLAYER_H

#include "Room.h"
#include <boost/shared_ptr.hpp>
#include <boost/enable_shared_from_this.hpp>
#include <boost/asio/ip/udp.hpp>

class Room;

class Player
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
    bool setRoom(boost::shared_ptr< Room > room);
    ~Player();
private:
    Room * my_room;
    Character character;
    size_t id;
    std::string login;
    boost::shared_ptr< Room > room;
    boost::shared_ptr< boost::asio::ip::udp::endpoint > endpoint;

};

#endif //MAFIA_PLAYER_H
