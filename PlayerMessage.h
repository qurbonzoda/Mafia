//
// Created by qurbonzoda on 20.12.15.
//

#ifndef MAFIA_PLAYERMESSAGE_H
#define MAFIA_PLAYERMESSAGE_H


#include <stdint.h>
#include "Server.h"
#include "Command.h"

using namespace Command;

class PlayerMessage
{
public:
    PlayerMessage(std::vector<uint8_t> const & message);

    uint32_t getLen() const
    {
        return len;
    }

    uint32_t getId() const
    {
        return id;
    }

    const Command::Type &getCommand() const
    {
        return command;
    }

    const std::vector<uint8_t> &getData() const
    {
        return data;
    }

private:
    uint32_t len;
    uint32_t id;
    Command::Type command;
    std::vector<uint8_t> data;
};


#endif //MAFIA_PLAYERMESSAGE_H
