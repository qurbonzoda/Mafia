//
// Created by qurbonzoda on 19.12.15.
//

#ifndef MAFIA_FORMATTER_H
#define MAFIA_FORMATTER_H


#include <vector>
#include <stdint.h>
#include <string>

class Formatter
{
public:
    static std::string getString(std::vector<uint8_t>);
    static std::vector<uint8_t> getVector(std::string);

    static std::vector<uint8_t> getVector(uint16_t len, uint8_t command, std::string const &str);
};


#endif //MAFIA_FORMATTER_H
