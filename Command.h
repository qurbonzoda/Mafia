//
// Created by qurbonzoda on 20.12.15.
//

#ifndef MAFIA_COMMAND_H
#define MAFIA_COMMAND_H
#include "PlayerMessage.h"

class PlayerMessage;

namespace Command {
    void authorization(std::vector<uint8_t> const & bytes);
    void new_room(std::vector<uint8_t> const & bytes);
};
#endif //MAFIA_COMMAND_H
