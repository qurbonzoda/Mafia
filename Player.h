//
// Created by qurbonzoda on 23.11.15.
//

#ifndef MAFIA_PLAYER_H
#define MAFIA_PLAYER_H

#include "network.h"
#include "Room.h"

enum Character
{
    Moderator, Detective, Doctor, Mafia, Villager
};

class Player
{
    /*
public:
    Player(size_t id);
    size_t getId();
    boost::shared_ptr< Room > getRoom();
    bool setRoom(boost::shared_ptr<Room> room);
    ~Player();
private:
    Character character;
    size_t id;
    std::string login;
    boost::shared_ptr<Room> room;
     */
};

#endif //MAFIA_PLAYER_H
