//
// Created by qurbonzoda on 20.12.15.
//

#include "Command.h"
void Command::execute(PlayerMessage const &message)
{
    switch (message.getCommand())
    {
        case PLAYER_ID:
            break;
        case PLAYERS_IN_ROOM:
            break;
        default:
            break;
    }
}