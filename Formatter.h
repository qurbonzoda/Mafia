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
    static std::string getStringOf(std::vector<uint8_t> const & message);
    static std::vector<uint8_t> getVectorOf(std::string const & str);
    static std::vector<uint8_t> getBytesOf(uint64_t, size_t size);
    static uint64_t getValueOf(std::vector<uint8_t> const & bytes);
    static std::vector<uint8_t> getMessageFormat(uint16_t len, uint8_t command, std::string const &str);
    static std::vector<uint8_t> getMessageFormat(uint16_t len, uint8_t command, std::vector<uint8_t> const &message);
    static std::vector< std::vector<uint8_t> > split(std::vector<uint8_t> const &bytes, uint8_t separator);
};


#endif //MAFIA_FORMATTER_H
