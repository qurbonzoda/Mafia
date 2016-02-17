//
// Created by qurbonzoda on 20.12.15.
//

#include "PlayerMessage.h"
#include "Formatter.h"

PlayerMessage::PlayerMessage(std::vector<uint8_t> const &message)
{
    auto parts = Formatter::split(message, ' ');
    if (parts.size() < 3)
    {
        throw "undefined message";
    }
    len = std::stoi(Formatter::stringOf(parts[0]));
    id = std::stoi(Formatter::stringOf(parts[1]));
    command = std::stoi(Formatter::stringOf(parts[2]));
    this->params = std::vector< std::vector<uint8_t> >(parts.begin() + 3, parts.end());
}

PlayerMessage::PlayerMessage(const PlayerMessage &message) : len(message.len), id(message.id),
                                                             command(message.command), params(message.params)
{
}

PlayerMessage::PlayerMessage()
{
}
