//
// Created by qurbonzoda on 20.12.15.
//

#ifndef MAFIA_PLAYERMESSAGE_H
#define MAFIA_PLAYERMESSAGE_H


#include <cstdio>
#include <cstdint>
#include <iostream>
#include <vector>

class PlayerMessage
{
public:
    PlayerMessage(std::vector<uint8_t> const & message);
    PlayerMessage(PlayerMessage const & message);
    PlayerMessage();

    uint32_t getLen() const
    {
        return len;
    }

    uint32_t getId() const
    {
        return id;
    }

    const size_t &getCommand() const
    {
        return command;
    }

    const std::vector< std::vector<uint8_t> > &getParams() const
    {
        return params;
    }

    void setParam(size_t i, std::vector<uint8_t> value)
    {
        if (i >= params.size() || i < 0)
        {
            std::clog << "setParam out of range" << std::endl;
        }
        params[i] = value;
    }

public:
    void setLen(uint32_t len)
    {
        PlayerMessage::len = len;
    }

    void setId(uint32_t id)
    {
        PlayerMessage::id = id;
    }

    void setCommand(size_t command)
    {
        PlayerMessage::command = command;
    }

    void setParams(const std::vector<std::vector<uint8_t>> &params)
    {
        PlayerMessage::params = params;
    }

private:
    uint32_t len = 0;
    uint32_t id = 0;
    size_t command = 0;
    std::vector< std::vector<uint8_t> > params;

};


#endif //MAFIA_PLAYERMESSAGE_H
