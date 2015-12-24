//
// Created by qurbonzoda on 04.12.15.
//

#ifndef MAFIA_SERVER_H
#define MAFIA_SERVER_H

#include "network.h"
#include "Room.h"
#include "Player.h"
#include "Formatter.h"
#include "PlayerMessage.h"
#include <atomic>

class PlayerMessage;
class Server;
class Player;
class Room;
class Hive;
class Connection;

class Server : public boost::enable_shared_from_this<Server>
{
public:
    Server(boost::shared_ptr< Hive > hive);
    static boost::shared_ptr<Player> get_or_create_player(boost::shared_ptr<boost::asio::ip::udp::endpoint> endpoint);


public:
    static boost::shared_ptr<Server> newInstance(boost::shared_ptr< Hive > hive, std::string ip_address, uint16_t port);
    static boost::shared_ptr<Player> create_new_player_instance();
    static boost::shared_ptr<Room> create_new_room_instance();
    static boost::shared_ptr<Player> getPlayer_by_id(uint32_t id)
    {
        return players.at(id);
    }
    static boost::shared_ptr<Room> getRoom_by_id(uint32_t id);
    static uint32_t getId_by_player(boost::shared_ptr<Player> player);
    static void room_erase(boost::shared_ptr<Room> room);
    static void delete_player(boost::shared_ptr<Player> player);
    static void update_room_list();
    static void update_room_info(boost::shared_ptr<Room> room);
    static boost::shared_ptr<Player> getPlayer_by_connection(boost::shared_ptr<Connection> connection);
    static boost::shared_ptr<Player> getPlayer_by_endpoint(const boost::asio::ip::udp::endpoint &endpoint);

private:
    static volatile uint32_t player_id_counter;
    static volatile uint32_t room_id_counter;
    boost::shared_ptr<Hive> hive;

public:
    static uint32_t getRoom_id_counter()
    {
        return room_id_counter;
    }

    const boost::shared_ptr<Hive> &getHive() const
    {
        return hive;
    }

    static const std::map<size_t, boost::shared_ptr<Player>> &getPlayers()
    {
        return players;
    }

    static const std::set<boost::shared_ptr<Room>> &getRooms()
    {
        return rooms;
    }

private:
    static std::set< boost::shared_ptr<Room> > rooms;
    static std::map<size_t, boost::shared_ptr<Player> > players;

};

namespace Command {
    enum Type
    {
        PLAYER_ID, ROOM_INFO, START_GAME, AUTHORISATION, NEW_ROOM, ENTER_ROOM, LEAVE_ROOM, ROOMS_LIST
    };
    void execute(boost::shared_ptr<Connection> const &connection, PlayerMessage const &message);
    void authorization(boost::shared_ptr<Connection> const & connection, PlayerMessage const & message);
    void new_room(boost::shared_ptr<Connection> const & connection, PlayerMessage const & message);
    void room_list(boost::shared_ptr<Connection> const & connection);
    void room_list(boost::shared_ptr<Connection> const & connection, PlayerMessage const & message);
    void enter_room(boost::shared_ptr<Connection> const & connection, PlayerMessage const & message);
    void room_info(boost::shared_ptr<Connection> const & connection, PlayerMessage const & message);
    void leave_room(boost::shared_ptr<Connection> const & connection, PlayerMessage const & message);
};
#endif //MAFIA_SERVER_H
