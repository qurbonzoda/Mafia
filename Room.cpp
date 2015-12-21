//
// Created by qurbonzoda on 23.11.15.
//

#include "Room.h"

Room::Room(size_t id, size_t max_players, std::string password) : id(id), max_players(max_players), password(password)
{
}

void Room::join(boost::shared_ptr<Player> player)
{
    players.insert(player);
}

void Room::SendToAllExcept(const std::string &message, size_t playerID)
{

}
