//
// Created by qurbonzoda on 23.11.15.
//

#ifndef MAFIA_PLAYER_H
#define MAFIA_PLAYER_H

#include "network.h"

enum Character
{
    Moderator, Detective, Doctor, Mafia, Villager
};

class Player
{
public:
    Player(boost::shared_ptr<Connection> connection);
private:
    Character character;
    boost::shared_ptr<Connection> main_connection;
};


#endif //MAFIA_PLAYER_H
