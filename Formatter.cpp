//
// Created by qurbonzoda on 19.12.15.
//

#include "Formatter.h"

std::string Formatter::getStringOf(std::vector<uint8_t> const &message)
{
    std::string result(message.begin(), message.end());
    return result;
}

std::vector<uint8_t> Formatter::getVectorOf(std::string const & str)
{
    std::vector<uint8_t> result(str.begin(), str.end());
    return result;
}

std::vector<uint8_t> Formatter::getMessageFormat(uint16_t len, uint8_t command, std::vector<uint8_t> const &message)
{
    std::vector<uint8_t> result;
    result.push_back(uint8_t(len >> 8));
    result.push_back((uint8_t)len);
    result.push_back(command);
    result.insert(result.end(), result.begin(), result.end());
    return result;
}std::vector<uint8_t> Formatter::getMessageFormat(uint16_t len, uint8_t command, std::string const &str)
{
    std::vector<uint8_t> result = getMessageFormat(len, command, getVectorOf(str));
    return result;
}
std::vector<uint8_t> Formatter::getBytesOf(uint64_t value, size_t size)
{
    std::vector<uint8_t> result;
    for (int i = (int)size - 1; i >= 0; --i)
    {
        result.push_back(value >> (i * 8));
    }
    return result;
}

uint64_t Formatter::getValueOf(std::vector<uint8_t> const &bytes)
{
    uint64_t value = 0;
    for (int i = 0; i < bytes.size(); ++i)
    {
        value <<= 8;
        value += bytes[i];
    }
    return value;
}

std::vector<std::vector<uint8_t> > Formatter::split(std::vector<uint8_t> const &bytes, uint8_t separator)
{
    std::vector< std::vector<uint8_t> > result;
    for (int i = 0; i < bytes.size(); ++i)
    {
        std::vector<uint8_t> token;
        while(i < bytes.size() && bytes[i] != separator)
        {
            token.push_back(bytes[i]);
            i++;
        }
        if (!token.empty())
        {
            result.push_back(token);
        }
    }
    return result;
}
