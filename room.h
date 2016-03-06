//
// Created by qurbonzoda on 23.11.15.
//

#ifndef MAFIA_ROOM_H
#define MAFIA_ROOM_H


#include <vector>
#include <set>
#include <boost/shared_ptr.hpp>
#include <boost/enable_shared_from_this.hpp>
#include <boost/date_time/posix_time/posix_time.hpp>
#include <boost/asio.hpp>
#include "constants.h"

class Player;

class Server;

class Hive;

class RoomState;

class Room : public boost::enable_shared_from_this<Room>
{
public:
    enum Status
    {
        WAITING, PLAYING
    };

    size_t getId() const
    {
        return id_;
    }

    const std::string &getPassword() const
    {
        return password_;
    }

    void setPassword(const std::string &password)
    {
        password_ = password;
    }

    size_t getMaximumPlayers() const
    {
        return maxPlayers_;
    }

    void setMaximumPlayers(size_t maxPlayers)
    {
        maxPlayers_ = maxPlayers;
    }

    const std::set<boost::shared_ptr<Player>> &getPlayers() const
    {
        return players_;
    }

    bool isSafe() const
    {
        return password_ != NO_PASSWORD;
    }

    Status getStatus() const
    {
        return status_;
    }

    boost::shared_ptr<Server> getServer() const
    {
        return server_;
    }

    size_t getNumberOfPlayers() const
    {
        return players_.size();
    }

    const std::string &getPositionMask() const
    {
        return positionMask_;
    }

    RoomState *getState() const
    {
        return state_;
    }

    Room(uint32_t id, boost::shared_ptr<Server> const &server);

    void join(boost::shared_ptr<Player> const &player);

    void erasePlayer(boost::shared_ptr<Player> const &player);

    void goToNextState();

    void nominateForVoting(size_t roomPosition);

    void setNumberOfVotesAgainstNominatedPlayer(size_t amount);

    void tryToMurderPlayer(size_t room_position);

    void curePlayer(size_t room_position);

    bool canSee(boost::shared_ptr<const Player> player) const;

    bool isVisible(boost::shared_ptr<const Player> player) const;

    bool canSpeak(boost::shared_ptr<const Player> player) const;

    ~Room();

private:
    void resetPlayerPositionsConsequently();

    void assignCharactersRandomly();

    void startTimer();

    void handleTimer(const boost::system::error_code &error);

    bool checkGameOver() const;

    void setCorrespondingPlayerScreen();

    void deleteMurderedPlayer();

    void deletePreviousVotingChain();

    int getAccusedPlayer();

private:
    const size_t MAX_POSSIBLE_PLAYERS = 11;
    const size_t MIN_POSSIBLE_PLAYERS = 9;
    Status status_ = Status::WAITING; // 0 - WAITING; 1 - PLAYING
    size_t id_;
    size_t maxPlayers_;
    std::string password_;
    std::string positionMask_;
    std::set<boost::shared_ptr<Player> > players_;
    boost::asio::deadline_timer timer_;
    boost::posix_time::ptime lastTime_;
    int32_t timerInterval_;
    boost::shared_ptr<Server> server_;
    RoomState *state_ = nullptr;
    RoomState *beforeAssasiation_ = nullptr;
    std::map<size_t, size_t> nominees_;
    int32_t murderedPlayer_ = -1;
    int32_t curedPlayer_ = -1;

    int botCnt_ = 0;
};


#endif //MAFIA_ROOM_H
