//
// Created by qurbonzoda on 23.11.15.
//

#include <boost/bind/placeholders.hpp>
#include <boost/bind/bind.hpp>
#include "MyNetwork.h"
#include "Server.h"
#include "Room.h"
#include "Player.h"
#include "Command.h"

Room::Room(size_t id, size_t max_players, std::string password, boost::shared_ptr<Server> server) : id(id), max_players(max_players),
                                                                                                    password(password),
                                                                                                    position_mask("00000000000"),
                                                                                                    m_timer(server->getHive()->GetService()),
                                                                                                    m_server(server),
                                                                                                    m_timer_interval(50)
{
    assert(max_players >= MIN_POSSIBLE_PLAYERS && max_players <= MAX_POSSIBLE_PLAYERS);
}

Room::Room(uint32_t id, boost::shared_ptr<Server> server) : id(id), position_mask("00000000000"),
                                                            m_timer(server->getHive()->GetService()),
                                                            m_server(server),
                                                            m_timer_interval(50)
{
}

void Room::join(boost::shared_ptr<Player> player)
{
    players.insert(player);
    player->setRoom(shared_from_this());
    assert(players.size() <= max_players);

    for (int i = 0; i < position_mask.length(); ++i)
    {
        if (position_mask[i] == '0')
        {
            player->setRoom_position(i);
            position_mask[i] = '1';
            break;
        }
    }

    if (players.size() == max_players) {
        for (auto player : players)
        {
            size_t playerPosition = player->getRoom_position();
            for (int i = 0; i < playerPosition; ++i)
            {
                if (position_mask[i] == '0') {
                    player->setRoom_position(i);
                    position_mask[i] = '1';
                    position_mask[playerPosition] = '0';
                    break;
                }
            }
        }
        /*
         *  shuffle
         */
        std::vector< boost::shared_ptr<Player> > list(players.begin(), players.end());
        std::random_shuffle ( list.begin(), list.end() );
        /*
         *  character assignment
         */
        size_t mafiaNumber = (max_players - 2) / 3;
        auto it = list.begin();
        (*it++)->setCharacter(Player::Character::Moderator);
        (*it++)->setCharacter(Player::Character::Detective);
        (*it++)->setCharacter(Player::Character::Doctor);
        for (int j = 0; j < mafiaNumber; ++j)
        {
            (*it++)->setCharacter(Player::Character::Mafia);
        }
        for (; it != list.end(); it++)
        {
            (*it)->setCharacter(Player::Character::Villager);
        }
        Command::start_game(shared_from_this());

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
    assert(max_players >= MIN_POSSIBLE_PLAYERS && max_players <= MAX_POSSIBLE_PLAYERS);
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

Room::Status Room::getStatus() const
{
    return status;
}

void Room::setStatus(Room::Status status)
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

Room::~Room()
{
    std::clog << "[" << __FUNCTION__ << "] " << std::endl;
}

void Room::StartTimer()
{
    //std::clog << "Room::[" << __FUNCTION__ << "] " << std::endl;
    m_last_time = boost::posix_time::microsec_clock::local_time();
    m_timer.expires_from_now( boost::posix_time::milliseconds( m_timer_interval ) );
    m_timer.async_wait( boost::bind(&Room::HandleTimer, shared_from_this(), _1) );

}

void Room::HandleTimer(const boost::system::error_code &error)
{
    //std::clog << "Room::[" << __FUNCTION__ << "] " << std::endl;
    if (!error)
    {
        for (auto player : players)
        {
            if (player->isScreenChanged() && !player->getScreen().empty())
            {
                for (auto anotherPlayer : players)
                {
                    m_server->getUdp()->Send(player->getScreen(),
                                             boost::asio::ip::udp::endpoint(*(anotherPlayer->getAddress()), 1010));
                }
                player->setScreenChanged(false);
            }
        }
        if (getNumber_of_players() != 0)
        {
            StartTimer();
        }
    }
}

boost::shared_ptr<Server> &Room::getServer()
{
    return m_server;
}
