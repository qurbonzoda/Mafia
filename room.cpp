//
// Created by qurbonzoda on 23.11.15.
//

#include <boost/bind/placeholders.hpp>
#include <boost/bind/bind.hpp>
#include <random>
#include <algorithm>
#include "my_network.h"
#include "server.h"
#include "room.h"
#include "player.h"
#include "command.h"
#include "room_state.h"
#include "constants.h"


Room::Room(uint32_t id, boost::shared_ptr<Server> const &server) : id_(id), positionMask_("00000000000"),
                                                                   timer_(server->getHive()->GetService()),
                                                                   server_(server),
                                                                   timerInterval_(50)
{
}

void Room::join(boost::shared_ptr<Player> &player)
{
    // insert player
    players_.insert(player);
    player->setRoom(shared_from_this());
    assert(players_.size() <= maxPlayers_);

    if (players_.size() == 1)
    {
        startTimer();
    }

    // set room position
    size_t firstFreePosition = positionMask_.find('0');
    player->setRoomPosition(firstFreePosition);
    positionMask_[firstFreePosition] = '1';

    if (players_.size() == maxPlayers_)
    {
        resetPlayerPositionsConsequently();

        assignCharactersRandomly();

        state_ = RoomState::buildStateChain(shared_from_this());

        status_ = PLAYING;
        command::startGame(shared_from_this());
    }
}

void Room::erasePlayer(boost::shared_ptr<Player> &player)
{
    positionMask_[player->getRoomPosition()] = '0';
    players_.erase(player);
    player->setRoom(nullptr);
}

void Room::startTimer()
{
    //std::clog << "Room::[" << __FUNCTION__ << "] " << std::endl;
    lastTime_ = boost::posix_time::microsec_clock::local_time();
    timer_.expires_from_now(boost::posix_time::milliseconds(timerInterval_));
    timer_.async_wait(boost::bind(&Room::handleTimer, shared_from_this(), _1));
}

void Room::handleTimer(const boost::system::error_code &error)
{
    //std::clog << "Room::[" << __FUNCTION__ << "] " << std::endl;

    botCnt_++;
    if (botCnt_ == players_.size())
    {
        botCnt_ = 0;
    }
    auto playerIterator = players_.begin();
    std::advance(playerIterator, botCnt_);
    if ((*playerIterator)->isBot())
    {
        (*playerIterator)->setScreenChanged(true);
    }

    if (!error)
    {
        std::vector<uint8_t> cantSee;

        for (auto player : players_)
        {
            std::vector<uint8_t> const *playerScreen;
            if (player->isVisible())
            {
                playerScreen = &server_->getInvisibilityImage();
            }
            else
            {
                playerScreen = &player->getScreen();
            }
            uint16_t length = playerScreen->size();

            cantSee.push_back((length >> 8));
            cantSee.push_back((length & 0xff));

            cantSee.push_back(player->getRoomPosition());

            cantSee.insert(cantSee.end(), playerScreen->begin(), playerScreen->end());
        }

        std::vector<uint8_t> datagramm;
        for (auto player : players_)
        {
            if (player->isScreenChanged())
            {
                uint16_t length = player->getScreen().size();

                assert(length != 0);
                assert(length > 1000);

                datagramm.push_back((length >> 8));
                datagramm.push_back((length & 0xff));

                datagramm.push_back(player->getRoomPosition());

                datagramm.insert(datagramm.end(), player->getScreen().begin(), player->getScreen().end());

                //if (!player->isBot())
                player->setScreenChanged(false);
            }
        }

        cantSee.insert(cantSee.begin(), 113);
        datagramm.insert(datagramm.begin(), 113);

        for (auto player : players_)
        {
            if (!player->canSee())
            {
                server_->getUdp()->Send(cantSee, boost::asio::ip::udp::endpoint(*(player->getAddress()), 1010));
            }
            else if (!datagramm.empty())
            {
                server_->getUdp()->Send(datagramm, boost::asio::ip::udp::endpoint(*(player->getAddress()), 1010));
            }
        }
        if (getNumberOfPlayers() != 0)
        {
            startTimer();
        }
    }
}

void Room::goToNextState()
{
    std::string nameOfNextState = state_->getNext()->getName();
    std::string nameOfCurrentState = state_->getName();

    if (nameOfNextState == "Voting")
    {
        deletePreviousVotingChain();
        RoomState::buildVotingChain(state_->getNext(), nominees_);
    }
    else if (nameOfCurrentState.find(VOTING_AGAINST) != std::string::npos
             && nameOfNextState == NIGHT)
    {
        int selected = getAccusedPlayer();
        if (selected != -1)
        {
            RoomState::buildMurderChain(state_, selected);
        }
        nominees_.clear();
        std::clog << "After voting player " + std::to_string(selected) << " was selected" << std::endl;
    }
    else if (nameOfCurrentState.find(DOCTOR_CURING) != std::string::npos)
    {
        if (murderedPlayer_ != curedPlayer_ && murderedPlayer_ >= 0)
        {
            RoomState::buildMurderChain(state_, murderedPlayer_);
            std::clog << "This night was murdered player " + std::to_string(murderedPlayer_) << std::endl;
        }
        curedPlayer_ = -1;
        murderedPlayer_ = -1;
    }
    if (nameOfNextState.find(WAS_MURDERED) != std::string::npos)
    {
        beforeAssasiation_ = state_;
    }
    else if (nameOfCurrentState.find(MURDERED_SPEAKS) != std::string::npos)
    {
        deleteMurderedPlayer();
    }

    checkGameOver();

    state_ = state_->getNext();

    setCorrespondingPlayerScreen();
}

void Room::nominateForVoting(size_t roomPosition)
{
    std::clog << "[ " << __FUNCTION__ << " ]" + std::to_string(roomPosition) << std::endl;
    nominees_[roomPosition] = 0;
}

void Room::setNumberOfVotesAgainstNominatedPlayer(size_t amount)
{
    std::string stateName = state_->getName();
    size_t tmp = stateName.find_last_of('_');
    assert(tmp != std::string::npos);
    size_t roomPosition = std::stoi(stateName.substr(tmp + 1));
    nominees_[roomPosition] = amount;
    std::clog << "[ " << __FUNCTION__ << " ] " + std::to_string(roomPosition)
                                         + ": " + std::to_string(amount) << std::endl;
}

void Room::tryToMurderPlayer(size_t roomPosition)
{
    std::clog << "[ " << __FUNCTION__ << " ]" + std::to_string(roomPosition) << std::endl;
    murderedPlayer_ = (murderedPlayer_ == -1 ? roomPosition : -2);
}

void Room::curePlayer(size_t roomPosition)
{
    std::clog << "[ " << __FUNCTION__ << " ]" + std::to_string(roomPosition) << std::endl;
    curedPlayer_ = roomPosition;
}

bool Room::canSee(boost::shared_ptr<const Player> player) const
{
    if (player->getCharacter() == Player::Character::NOT_SPECIFIED)
    {
        return true;
    }
    return state_->canSee(player);
}


bool Room::isVisible(boost::shared_ptr<const Player> player) const
{
    if (player->getCharacter() == Player::Character::NOT_SPECIFIED)
    {
        return true;
    }
    return state_->isVisible(player);
}

bool Room::canSpeak(boost::shared_ptr<const Player> player) const
{
    if (player->getCharacter() == Player::Character::NOT_SPECIFIED)
    {
        return true;
    }
    return state_->canSpeak(player);
}

Room::~Room()
{
    std::clog << "[" << __FUNCTION__ << "] " << std::endl;

    if (state_ != nullptr)
    {
        RoomState *head = state_->getNext();
        while (head != state_)
        {
            RoomState *tmp = head->getNext();
            delete head;
            head = tmp;
        }
        delete state_;
    }
}

bool Room::checkGameOver() const
{
    int numberOfMafia = 0;
    int numberOfVillagers = 0;
    for (auto player : players_)
    {
        numberOfMafia += (player->getCharacter() == Player::Character::MAFIA);
        numberOfVillagers += (player->getCharacter() != Player::Character::MAFIA)
                             && (player->getCharacter() != Player::Character::DEAD)
                             && (player->getCharacter() != Player::Character::MODERATOR);
    }
    if (numberOfMafia == 0)
    {
        RoomState::buildEndGameChain(state_, "Villagers");
        return true;
    }
    if (numberOfVillagers == 0)
    {
        RoomState::buildEndGameChain(state_, "Mafia");
        return true;
    }
    return false;
}

void Room::setCorrespondingPlayerScreen()
{
    for (auto player : players_)
    {
        std::clog << "player #" << std::to_string(player->getRoomPosition()) << std::endl;
        std::clog << "Character: " << std::to_string(player->getCharacter()) << std::endl;
        std::clog << "isVisible: " << std::to_string(player->isVisible()) << std::endl;
        std::clog << "canSee: " << std::to_string(player->canSee()) << std::endl;
        std::clog << "canSpeak: " << std::to_string(player->canSpeak()) << std::endl;
        std::clog << std::endl;

        if (player->isVisible() && player->isBot())
        {
            player->setScreen(server_->getBotScreen());
        }
        if (!player->isVisible())
        {
            player->setScreen(server_->getInvisibilityImage());
        }
        if (player->getCharacter() == Player::Character::DEAD)
        {
            player->setScreen(server_->getRIPScreen());
        }

    }
    std::clog << std::endl;
}

void Room::deleteMurderedPlayer()
{
    std::string stateName = beforeAssasiation_->getNext()->getName();
    size_t from = stateName.find_last_of('_');

    assert(from != std::string::npos);

    size_t roomPosition = std::stoi(stateName.substr(from + 1));

    RoomState *tmp = state_->getNext();
    state_ = beforeAssasiation_;
    delete state_->getNext()->getNext();
    delete state_->getNext();
    state_->setNext(tmp);

    RoomState *expiredState = state_;
    while (expiredState->getNext()->getName().find(SPEAKING + std::to_string(roomPosition)) == std::string::npos)
    {
        expiredState = expiredState->getNext();
    }
    tmp = expiredState->getNext()->getNext()->getNext();
    delete expiredState->getNext()->getNext();
    delete expiredState->getNext();
    expiredState->setNext(tmp);

    auto lambda = [roomPosition](const boost::shared_ptr<Player> &player) { return player->getRoomPosition() == roomPosition; };
    auto murderedPlayer = std::find_if(players_.begin(), players_.end(), lambda);
    (*murderedPlayer)->setCharacter(Player::Character::DEAD);
}

void Room::deletePreviousVotingChain()
{
    for (RoomState *i = state_->getNext(); i != state_;)
    {
        if (i->getNext()->getName().find("Voting_against") != std::string::npos)
        {
            RoomState *j = i->getNext()->getNext();
            delete i->getNext();
            i->setNext(j);
        }
        else
        {
            i = i->getNext();
        }
    }
}

int Room::getAccusedPlayer()
{
    std::pair<size_t, size_t> first_maximum = {0, 0};
    std::pair<size_t, size_t> second_maximum = {0, 0};

    for (auto nominee : nominees_)
    {
        std::clog << "nominee: " << nominee.first << " " << nominee.second << std::endl;
        if (nominee.second > first_maximum.second)
        {
            second_maximum = first_maximum;
            first_maximum = nominee;
        }
        else if (nominee.second > second_maximum.second)
        {
            second_maximum = nominee;
        }
    }
    if (first_maximum.second != second_maximum.second)
    {
        return first_maximum.first;
    }
    return -1;
}

void Room::resetPlayerPositionsConsequently()
{
    for (auto player : players_)
    {
        size_t playerPosition = player->getRoomPosition();
        size_t firstFreePosition = positionMask_.find('0');
        if (firstFreePosition < playerPosition)
        {
            positionMask_[playerPosition] = '0';
            positionMask_[firstFreePosition] = '1';
            player->setRoomPosition(firstFreePosition);
        }
    }
}

void Room::assignCharactersRandomly()
{
    std::vector<boost::shared_ptr<Player> > playerList(players_.begin(), players_.end());
    
    // no bot is Moderator
    auto isNotBot = [](const boost::shared_ptr<Player> &x) { return !x->isBot(); };
    int firstNonBot = std::find_if(playerList.begin(), playerList.end(), isNotBot) - playerList.begin();
    std::swap(playerList[firstNonBot], playerList[0]);

    srand(time(0));
    std::random_shuffle(playerList.begin() + 1, playerList.end());
    for (int k = 0; k < playerList.size(); ++k)
    {
        playerList[k]->setRoomPosition(k);
    }

    // character assignment
    size_t numberOfMafia = (maxPlayers_ - 2) / 3;
    auto it = playerList.begin();
    (*it++)->setCharacter(Player::Character::MODERATOR);
    (*it++)->setCharacter(Player::Character::DETECTIVE);
    (*it++)->setCharacter(Player::Character::DOCTOR);
    for (int j = 0; j < numberOfMafia; ++j)
    {
        (*it++)->setCharacter(Player::Character::MAFIA);
    }
    for (; it != playerList.end(); it++)
    {
        (*it)->setCharacter(Player::Character::VILLAGER);
    }

    std::random_shuffle(playerList.begin() + 1, playerList.end());
    for (int k = 0; k < playerList.size(); ++k)
    {
        playerList[k]->setRoomPosition(k);
    }
}
