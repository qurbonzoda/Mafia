//
// Created by qurbonzoda on 23.11.15.
//

#include "Room.h"
void Room::add(boost::shared_ptr<Player> player)
{
    players.push_back(player);
}