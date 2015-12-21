//
// Created by qurbonzoda on 20.12.15.
//

#include "PlayerMessage.h"
#include "Command.h"

PlayerMessage::PlayerMessage(std::vector<uint8_t> const &message)
{
    if (message.size() < 5)
    {
        std::clog << "FATAL Message!!" << std::endl;
        return;
    }
    len = Formatter::getValueOf(std::vector<uint8_t>(message.begin(), message.begin() + 2));
    id = Formatter::getValueOf(std::vector<uint8_t>(message.begin() + 2, message.begin() + 4));
    command = (Command::Type)Formatter::getValueOf(std::vector<uint8_t>(message.begin() + 4, message.begin() + 5));
    data = std::vector<uint8_t>(message.begin() + 5, message.end());
}
