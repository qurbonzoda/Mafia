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

class Room : public boost::enable_shared_from_this<Room>
{
public:
    enum Status {
        waiting, playing
    };

    Room(size_t id, size_t max_players, std::string password, boost::shared_ptr<Server> Server);

    Room(uint32_t id, boost::shared_ptr<Server> Server);

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
    Status getStatus() const;
    void setStatus(Status status);
    boost::shared_ptr<Server> &getServer();
    size_t getNumber_of_players();
    const std::string &getPosition_mask() const;
    void setPosition_mask(const std::string &position_mask);
    void erase(boost::shared_ptr<Player> player);
    void StartTimer();
    void HandleTimer( const boost::system::error_code & error );
    ~Room();

private:
    const size_t MAX_POSSIBLE_PLAYERS = 11;
    const size_t MIN_POSSIBLE_PLAYERS = 9;
    Status status = Status::waiting ; // 0 - waiting; 1 - playing
    size_t id;
    size_t max_players;
    std::string password;
    std::string name;
    std::string position_mask;
    std::set< boost::shared_ptr< Player > > players;
    boost::asio::deadline_timer m_timer;
    boost::posix_time::ptime m_last_time;
    int32_t m_timer_interval;
    boost::shared_ptr<Server> m_server;
};


#endif //MAFIA_ROOM_H
