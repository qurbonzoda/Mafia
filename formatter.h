//
// Created by qurbonzoda on 19.12.15.
//

#ifndef MAFIA_FORMATTER_H
#define MAFIA_FORMATTER_H


#include <vector>
#include <stdint.h>
#include <string>

namespace Formatter
{
    std::string stringOf(std::vector<uint8_t> const &message);
    std::vector<uint8_t> vectorOf(std::string const &str);
    std::vector< std::vector<uint8_t> > split(std::vector<uint8_t> const &bytes, uint8_t separator);
    std::string getMessageFormat(std::string &message);
};


#endif //MAFIA_FORMATTER_H
