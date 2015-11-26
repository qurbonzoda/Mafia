//
// Created by qurbonzoda on 23.11.15.
//

#ifndef MAFIA_ROOM_H
#define MAFIA_ROOM_H


#include <vector>
#include "Player.h"

class Room
{
public:
    void add(boost::shared_ptr<Player> player);

private:
    std::vector< boost::shared_ptr<Player> > players;
};


#endif //MAFIA_ROOM_H
