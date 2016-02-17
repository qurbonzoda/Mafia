//
// Created by qurbonzoda on 23.11.15.
//

#include <boost/bind/placeholders.hpp>
#include <boost/bind/bind.hpp>
#include <random>
#include <algorithm>
#include "MyNetwork.h"
#include "Server.h"
#include "Room.h"
#include "Player.h"
#include "Command.h"
#include "RoomState.h"

Room::Room(size_t id, size_t max_players, std::string &password, boost::shared_ptr<Server> const &server) : id(id), max_players(max_players),
                                                                                                    password(password),
                                                                                                    position_mask("00000000000"),
                                                                                                    m_timer(server->getHive()->GetService()),
                                                                                                    m_server(server),
                                                                                                    m_timer_interval(50)
{
    assert(max_players >= MIN_POSSIBLE_PLAYERS && max_players <= MAX_POSSIBLE_PLAYERS);
}

Room::Room(uint32_t id, boost::shared_ptr<Server> const &server) : id(id), position_mask("00000000000"),
                                                            m_timer(server->getHive()->GetService()),
                                                            m_server(server),
                                                            m_timer_interval(50)
{
}

void Room::join(boost::shared_ptr<Player> &player)
{
    // insert player
    players.insert(player);
    player->setRoom(shared_from_this());
    assert(players.size() <= max_players);

    if (getNumber_of_players() == 1)
    {
        StartTimer();
    }

    // set room position
    size_t i = position_mask.find('0');
    player->setRoom_position(i);
    position_mask[i] = '1';

    if (players.size() == max_players) {


        // reset room positions so that players were consequently
        for (auto player : players)
        {
            size_t playerPosition = player->getRoom_position();
            size_t i = position_mask.find('0');
            if (i < playerPosition)
            {
                player->setRoom_position(i);
                position_mask[i] = '1';
                position_mask[playerPosition] = '0';
            }
        }

        // shuffle
        std::vector< boost::shared_ptr<Player> > list(players.begin(), players.end());

        srand(time(0));
        std::random_shuffle(list.begin(), list.end());

        // no bot is Moderator
        for (int k = 0; k < list.size(); k++)
        {
            if (!list[k]->isBot())
            {
                std::swap(list[k], list[0]);
                break;
            }
        }

        for (int k = 0; k < list.size(); ++k)
        {
            list[k]->setRoom_position(k);
        }



        // character assignment
        size_t mafiaNumber = (max_players - 2) / 3;
        auto it = list.begin();
        (*it++)->setCharacter(Player::Character::Moderator);
        (*it++)->setCharacter(Player::Character::Detective);
        (*it++)->setCharacter(Player::Character::Doctor);
        for (int j = 0; j < mafiaNumber; ++j)
        {
            //std::clog << "MAfia room position " + std::to_string((*it)->getRoom_position()) << std::endl;
            (*it++)->setCharacter(Player::Character::Mafia);

        }
        for (; it != list.end(); it++)
        {
            (*it)->setCharacter(Player::Character::Villager);
        }

        // shuffle
        //srand(time(0));

        std::random_shuffle(list.begin() + 1, list.end());
        for (int k = 1; k < list.size(); ++k)
        {
            list[k]->setRoom_position(k);
        }

        // build game state chain
        state = RoomState::buildStateChain(shared_from_this());

        // start game
        Command::start_game(shared_from_this());

    }
}

size_t Room::getId() const
{
    return id;
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


const std::set<boost::shared_ptr<Player>> &Room::getPlayers() const
{
    return players;
}

Room::Status Room::getStatus() const
{
    return status;
}

void Room::setStatus(Room::Status status)
{
    Room::status = status;
}

bool Room::isSafe() const
{
    return password != "nopass";
}

size_t Room::getNumber_of_players() const
{
    return players.size();
}

const std::string &Room::getPosition_mask() const
{
    return position_mask;
}

void Room::erase(boost::shared_ptr<Player> &player)
{
    position_mask[ player->getRoom_position() ] = '0';
    players.erase(player);
    player->setRoom(nullptr);
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


    botCnt++;

    if (botCnt == players.size())
    {
        botCnt = 0;
    }

    if (!error)
    {
        int k = 0;
        std::vector<uint8_t>datagram;

        for (auto player : players)
        {
            if (k == botCnt && player->isBot())
            {
                player->setScreenChanged(true);
            }
            k++;

            if (player->isScreenChanged())
            {
                uint16_t length = player->getScreen().size();

                assert(length != 0);
                assert(length > 1000);

                datagram.push_back((length >> 8));
                datagram.push_back((length & 0xff));

                assert((datagram[datagram.size() - 2] << 8) + datagram.back() == length);

                datagram.push_back(player->getRoom_position());

                assert(datagram.back() == player->getRoom_position());
                assert(datagram.back() < Room::max_players);

                datagram.insert(datagram.end(), player->getScreen().begin(), player->getScreen().end());

                if (!player->isBot())
                    player->setScreenChanged(false);
            }
/*
            if (!player->isVisible())
            {
                if (!player->isInvisiblitySet())
                {
                    player->setScreen(m_server->getInvisibilityImage());
                }
                player->setInvisiblitySet(true);
            }
            else
            {
                player->setInvisiblitySet(false);
            }

            if ((player->isScreenChanged()) && !(player->getScreen().empty()))
            {
                auto playerScreen = player->getScreen();
                playerScreen.insert(playerScreen.begin(), player->getRoom_position());

                assert(playerScreen[0] < Room::MAX_POSSIBLE_PLAYERS);
                assert(playerScreen[0] == player->getRoom_position());

                for (auto anotherPlayer : players)
                {
                    if (anotherPlayer->canSee())
                    {
                        m_server->getUdp()->Send(playerScreen,
                                                 boost::asio::ip::udp::endpoint(*(anotherPlayer->getAddress()), 1010));
                    }
                }
                player->setScreenChanged(false);
            }
*/
        }

        for (auto player : players)
        {
            if (!datagram.empty())
            {
                m_server->getUdp()->Send(datagram, boost::asio::ip::udp::endpoint(*(player->getAddress()), 1010));
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
RoomState *Room::getState() const
{
    return state;
}

void Room::goToNextState()
{

    if (state->getNext()->getName() == "Voting")
    {
        RoomState::buildVotingChain(state->getNext(), nominees);
    }
    else if (state->getName().find("Voting_against") != std::string::npos
        && state->getNext()->getName() == "Night")
    {
        std::pair<size_t, size_t>max1 = {0, 0};
        std::pair<size_t, size_t>max2 = {0, 0};

        for (auto nominee : nominees)
        {
            std::clog << "nominee: " << nominee.first << " " << nominee.second << std::endl;
            if (nominee.second > max1.second)
            {
                max2 = max1;
                max1 = nominee;
            }
            else if (nominee.second > max2.second)
            {
                max2 = nominee;
            }
        }
        assert(max1.second >= max2.second);

        if (max1.second != max2.second)
        {
            RoomState::buildMurderChain(state, max1.first);
            std::clog << "After voting died player " + std::to_string(max1.first) << std::endl;
        }
        else {
            std::clog << "After voting equal max votes have " + std::to_string(max2.first)
                         + " and " + std::to_string(max1.first) << std::endl;
        }
        nominees.clear();
    }
    else if (state->getName().find("Doctor") != std::string::npos)
    {
        if (murderTries.size() == 1 && *murderTries.begin() != curedPlayer)
        {
            RoomState::buildMurderChain(state, *murderTries.begin());
            std::clog << "After curing died player " + std::to_string(*murderTries.begin()) << std::endl;
        }
        murderTries.clear();
        curedPlayer = -1;
    }
    if (state->getNext()->getName().find("Was_murdered_player_") != std::string::npos)
    {
        beforeAssasiation = state;
    }
    else if (state->getName().find("Murdered_player's_speech") != std::string::npos)
    {
        std::string stateName = beforeAssasiation->getNext()->getName();
        size_t from = stateName.find_last_of('_');

        assert(from != std::string::npos);

        size_t room_position = std::stoi(stateName.substr(from + 1));

        RoomState *tmp = state->getNext();
        state = beforeAssasiation;
        delete state->getNext()->getNext();
        delete state->getNext();
        state->setNext(tmp);

        RoomState *expiredState = state;
        while (expiredState->getNext()->getName().find("Speaking_player_" + std::to_string(room_position)) == std::string::npos) {
            expiredState = expiredState->getNext();
        }
        tmp = expiredState->getNext()->getNext()->getNext();
        delete expiredState->getNext()->getNext();
        delete expiredState->getNext();
        expiredState->setNext(tmp);

        for (auto player : players)
        {
            if (player->getRoom_position() == room_position)
            {
                player->setCharacter(Player::Character::Dead);
            }
        }

    }
    state = state->getNext();
    std::clog << state->getName() << std::endl;
    for (auto player : players)
    {
        std::clog << "player #" << std::to_string(player->getRoom_position()) << std::endl;
        std::clog << "Character: " << std::to_string(player->getCharacter()) << std::endl;
        std::clog << "isVisible: " << std::to_string(player->isVisible()) << std::endl;
        std::clog << "canSee: " << std::to_string(player->canSee()) << std::endl;
        std::clog << "canSpeak: " << std::to_string(player->canSpeak()) << std::endl;
        std::clog << std::endl;

        if (player->isVisible() && player->isBot())
        {
            player->setScreen(m_server->getBotScreen());
        }
        if (!player->isVisible())
        {
            player->setScreen(m_server->getInvisibilityImage());
        }
    }
    std::clog << std::endl;
}

void Room::nominate(size_t room_position)
{
    std::clog << "[ " << __FUNCTION__ << " ]" + std::to_string(room_position) << std::endl;
    nominees[room_position] = 0;
}

void Room::votesAgainst(size_t amount)
{
    std::string stateName = state->getName();
    size_t tmp = stateName.find_last_of('_');
    assert(tmp != std::string::npos);
    size_t room_position = std::stoi(stateName.substr(tmp + 1));
    nominees[room_position] = amount;
    std::clog << "[ " << __FUNCTION__ << " ] " + std::to_string(room_position)
                                         + ": " + std::to_string(amount) << std::endl;
}

void Room::tryToMurder(size_t room_position)
{
    std::clog << "[ " << __FUNCTION__ << " ]" + std::to_string(room_position) << std::endl;
    murderTries.insert(room_position);
}

void Room::curePlayer(size_t room_position)
{
    std::clog << "[ " << __FUNCTION__ << " ]" + std::to_string(room_position) << std::endl;
    curedPlayer = room_position;
}

bool Room::canSee(boost::shared_ptr<const Player> player) const
{
    if (player->getCharacter() == Player::Character::Not_specified)
    {
        return true;
    }
    return state->canSee(player);
}


bool Room::isVisible(boost::shared_ptr<const Player> player) const
{
    if (player->getCharacter() == Player::Character::Not_specified)
    {
        return true;
    }
    return state->isVisible(player);
}

bool Room::canSpeak(boost::shared_ptr<const Player> player) const
{
    if (player->getCharacter() == Player::Character::Not_specified)
    {
        return true;
    }
    return state->canSpeak(player);
}

Room::~Room()
{
    std::clog << "[" << __FUNCTION__ << "] " << std::endl;

    if (state != nullptr)
    {
        RoomState *head = state->getNext();
        while (head != state)
        {
            RoomState *tmp = head->getNext();
            delete head;
            head = tmp;
        }
        delete state;
    }
}

