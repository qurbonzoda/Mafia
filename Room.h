//
// Created by qurbonzoda on 23.11.15.
//

#ifndef MAFIA_ROOM_H
#define MAFIA_ROOM_H


#include <vector>
#include <set>
#include "Player.h"
#include <boost/shared_ptr.hpp>
#include <boost/enable_shared_from_this.hpp>

class Player;

class Room : public boost::enable_shared_from_this<Room>
{
public:
    Room(size_t id, size_t max_players, std::string password);

    Room(uint32_t id);

    void join(boost::shared_ptr< Player > player);
    size_t getId() const;
    void setId(size_t id);
    const std::string &getPassword() const;
    void setPassword(const std::string &password);
    size_t getMax_players() const;
    void setMax_players(size_t max_players);
    const std::string &getName() const;
    void setName(const std::string &name);
    const std::set<boost::shared_ptr<Player>> &getPlayers() const;
    void setPlayers(const std::set<boost::shared_ptr<Player>> &players);
    bool isSafe();
    int getStatus() const;
    void setStatus(int status);
    size_t getNumber_of_players();
    const std::string &getPosition_mask() const;
    void setPosition_mask(const std::string &position_mask);
    void erase(boost::shared_ptr<Player> player);

private:
    int status = 0;
    size_t id;
    size_t max_players;
    std::string password;
    std::string name;
    std::string position_mask;
    std::set< boost::shared_ptr< Player > > players;

};


#endif //MAFIA_ROOM_H
