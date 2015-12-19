//
// Created by qurbonzoda on 19.12.15.
//

#include "Formatter.h"

std::string Formatter::getString(std::vector<uint8_t> vector)
{
    return "";
}

std::vector<uint8_t> Formatter::getVector(std::string string)
{
    return std::vector<uint8_t>();
}

std::vector<uint8_t> Formatter::getVector(uint16_t len, uint8_t command, std::string const &str)
{
    std::vector<uint8_t> res;
    res.push_back(uint8_t(len >> 8));
    res.push_back((uint8_t)len);
    res.push_back(command);
    for (int i = 0; i < str.length(); ++i)
    {
        res.push_back(str[i]);
    }
    return res;
}
