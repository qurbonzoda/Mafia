//
// Created by qurbonzoda on 20.12.15.
//

#include "PlayerMessage.h"
#include "Formatter.h"

PlayerMessage::PlayerMessage(std::vector<uint8_t> const &message)
{
    auto params = Formatter::split(message, ' ');
    if (params.size() < 3)
    {
        throw "undefined message";
        std::clog << "FATAL Message!!" << std::endl;
        return;
    }
    len = std::stoi(Formatter::getStringOf(params[0]));
    id = std::stoi(Formatter::getStringOf(params[1]));
    command = std::stoi(Formatter::getStringOf(params[2]));
    this->params = std::vector< std::vector<uint8_t> >(params.begin() + 3, params.end());
}

PlayerMessage::PlayerMessage(const PlayerMessage &message) : len(message.len), id(message.id),
                                                             command(message.command), params(message.params)
{
}

PlayerMessage::PlayerMessage()
{
}
