//
// Created by qurbonzoda on 23.11.15.
//

#include "Room.h"

Room::Room(size_t id, size_t max_players, std::string password) : id(id), max_players(max_players), password(password), position_mask("00000000000")
{
}

Room::Room(uint32_t id) : id(id), position_mask("00000000000")
{
}

void Room::join(boost::shared_ptr<Player> player)
{
    players.insert(player);
    player->setRoom(shared_from_this());
    for (int i = 0; i < position_mask.length(); ++i)
    {
        if (position_mask[i] == '0')
        {
            player->setRoom_position(i);
            position_mask[i] = '1';
            break;
        }
    }
}

size_t Room::getId() const
{
    return id;
}

void Room::setId(size_t id)
{
    Room::id = id;
}

const std::string &Room::getPassword() const
{
    return password;
}

void Room::setPassword(const std::string &password)
{
    Room::password = password;
}

size_t Room::getMax_players() const
{
    return max_players;
}

void Room::setMax_players(size_t max_players)
{
    Room::max_players = max_players;
}

const std::string &Room::getName() const
{
    return name;
}

void Room::setName(const std::string &name)
{
    Room::name = name;
}

const std::set<boost::shared_ptr<Player>> &Room::getPlayers() const
{
    return players;
}

void Room::setPlayers(const std::set<boost::shared_ptr<Player>> &players)
{
    Room::players = players;
}

int Room::getStatus() const
{
    return status;
}

void Room::setStatus(int status)
{
    Room::status = status;
}

bool Room::isSafe()
{
    return password != "nopass";
}

size_t Room::getNumber_of_players()
{
    return players.size();
}

const std::string &Room::getPosition_mask() const
{
    return position_mask;
}

void Room::setPosition_mask(const std::string &position_mask)
{
    Room::position_mask = position_mask;
}

void Room::erase(boost::shared_ptr<Player> player)
{
    position_mask[ player->getRoom_position() ] = '0';
    players.erase(player);
    player->setRoom(nullptr);
}
