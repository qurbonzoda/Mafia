//
// Created by qurbonzoda on 19.12.15.
//

#include "formatter.h"

std::string Formatter::stringOf(std::vector<uint8_t> const &message)
{
    std::string result(message.begin(), message.end());
    return result;
}

std::vector<uint8_t> Formatter::vectorOf(std::string const &str)
{
    std::vector<uint8_t> result(str.begin(), str.end());
    return result;
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
        result.push_back(token);
    }
    return result;
}

std::string Formatter::getMessageFormat(std::string &message)
{
    message = " " + message;

    switch (message.length())
    {
        case 9:
            return "11" + message;
        case 98:
            return "101" + message;
        default:
            size_t len = message.length() + std::to_string(message.length()).length();
            return std::to_string(len) + message;
    }

}
