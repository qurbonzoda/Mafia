//
// Created by qurbonzoda on 04.12.15.
//

#ifndef MAFIA_SERVER_H
#define MAFIA_SERVER_H

#include "network.h"
#include <set>
#include <map>
#include <atomic>

class PlayerMessage;
class Player;
class Room;
class MyUdpConnection;
class MyConnection;
class MyAcceptor;

class Server : public boost::enable_shared_from_this<Server>
{
public:
    Server(boost::shared_ptr< Hive > hive, std::string ip_address, uint16_t port);
    Server(Server const&) = delete;
    void operator=(Server const&) = delete;

private:
    boost::shared_ptr<Player> create_new_player_instance();

public:
    void Start();
    boost::shared_ptr<Player> get_or_create_player(boost::shared_ptr<boost::asio::ip::address> const &address);
    boost::shared_ptr<Room> getRoom_by_id(uint32_t id);
    void delete_room(boost::shared_ptr<Room> room);
    void delete_player(boost::shared_ptr<Player> player);
    void update_room_list();
    void update_room_info(boost::shared_ptr<Room> room);
    boost::shared_ptr<Player> getPlayer_by_connection(boost::shared_ptr<Connection> connection);
    boost::shared_ptr<Player> getPlayer_by_address(const boost::asio::ip::address &address);
    boost::shared_ptr<Room> create_new_room_instance();
    void add_bots();
public:
    const boost::shared_ptr<Hive> &getHive() const
    {
        return hive;
    }
    const std::set<boost::shared_ptr<Room>> &getRooms() const
    {
        return rooms;
    }
    boost::shared_ptr<Player> &getPlayer_by_id(uint32_t id)
    {
        return players.at(id);
    }
    const boost::shared_ptr<MyUdpConnection> &getUdp()  const
    {
        return udp;
    }
    const std::vector<uint8_t> &getInvisibilityImage() const
    {
        return invisibilityImage;
    }

    const std::vector<uint8_t> &getBotScreen() const
    {
        return botScreen;
    }

private:
    std::set< boost::shared_ptr<Room> > rooms;
    std::map<size_t, boost::shared_ptr<Player> > players;
    volatile uint32_t player_id_counter = 0;
    volatile uint32_t room_id_counter = 0;
    std::vector<uint8_t> invisibilityImage;
    std::vector<uint8_t> botScreen;
    std::string ip_address;
    uint16_t port;
    boost::shared_ptr<Hive> hive;
    boost::shared_ptr<MyUdpConnection> udp;
    boost::shared_ptr<MyAcceptor> acceptor;
};

#endif //MAFIA_SERVER_H
