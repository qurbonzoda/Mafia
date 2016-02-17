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

class Player;
class Server;
class Hive;
class RoomState;

class Room : public boost::enable_shared_from_this<Room>
{
public:
    enum Status {
        waiting, playing
    };

    Room(size_t id, size_t max_players, std::string &password, boost::shared_ptr<Server> const &Server);

    Room(uint32_t id, boost::shared_ptr<Server> const &Server);

    void join(boost::shared_ptr< Player > &player);
    size_t getId() const;
    const std::string &getPassword() const;
    void setPassword(const std::string &password);
    size_t getMax_players() const;
    void setMax_players(size_t max_players);
    const std::set<boost::shared_ptr<Player>> &getPlayers() const;
    bool isSafe() const;
    Status getStatus() const;
    void setStatus(Status status);
    boost::shared_ptr<Server> &getServer();
    size_t getNumber_of_players() const;
    const std::string &getPosition_mask() const;
    void erase(boost::shared_ptr<Player> &player);
    void StartTimer();
    void HandleTimer( const boost::system::error_code & error );
    RoomState *getState() const;
    void goToNextState();
    void nominate(size_t room_position);
    void votesAgainst(size_t amount);
    void tryToMurder(size_t room_position);
    void curePlayer(size_t room_position);
    bool canSee(boost::shared_ptr<const Player> player) const;
    bool isVisible(boost::shared_ptr<const Player> player) const;
    bool canSpeak(boost::shared_ptr<const Player> player) const;

    ~Room();

private:
    const size_t MAX_POSSIBLE_PLAYERS = 11;
    const size_t MIN_POSSIBLE_PLAYERS = 9;
    Status status = Status::waiting ; // 0 - waiting; 1 - playing
    size_t id;
    size_t max_players;
    std::string password;
    std::string position_mask;
    std::set< boost::shared_ptr< Player > > players;
    boost::asio::deadline_timer m_timer;
    boost::posix_time::ptime m_last_time;
    int32_t m_timer_interval;
    boost::shared_ptr<Server> m_server;
    RoomState * state = nullptr;
    RoomState * beforeAssasiation = nullptr;
    std::map< size_t, size_t > nominees;
    std::set< size_t >murderTries;
    int32_t curedPlayer = -1;

    int botCnt = 0;
};


#endif //MAFIA_ROOM_H
