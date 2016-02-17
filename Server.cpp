//
// Created by qurbonzoda on 04.12.15.
//

#include <boost/thread/pthread/mutex.hpp>
#include <fstream>
#include "Server.h"
#include "PlayerMessage.h"
#include "Command.h"
#include "MyNetwork.h"
#include "Player.h"
#include "Room.h"
#include "Formatter.h"

using namespace boost::asio;

Server::Server(boost::shared_ptr< Hive > hive, std::string ip_address, uint16_t port)
        : hive(hive), ip_address(ip_address), port(port)
{
    std::clog << "Server on " << ip_address << ": " << port << std::endl;
}

boost::shared_ptr<Player> Server::get_or_create_player(boost::shared_ptr<ip::address> const & address)
{
    for (auto &player : players)
    {
        if (player.second->getAddress() == address)
        {
            return player.second;
        }
    }
    auto player = create_new_player_instance();
    player->setAddress(address);
    return player;
}

boost::shared_ptr<Player> Server::create_new_player_instance()
{
    std::clog << "create_new_player_instance: player_id_counter = " << player_id_counter << std::endl;
    boost::shared_ptr<Player> player(new Player(++player_id_counter));
    players[player_id_counter] = player;
    return player;
}

boost::shared_ptr<Room> Server::create_new_room_instance()
{
    boost::shared_ptr<Room> room(new Room(++room_id_counter, shared_from_this()));
    rooms.insert(room);
    std::clog << "create_new_room_instance: room_id_counter = " << room_id_counter << std::endl;
    return room;
}

boost::shared_ptr<Room> Server::getRoom_by_id(uint32_t id)
{
    for (auto room : rooms)
    {
        if (room->getId() == id)
        {
            return room;
        }
    }
    std::clog << std::to_string(id) + " FATAL No such room id" << std::endl;
    return nullptr;
}

void Server::delete_room(boost::shared_ptr<Room> room)
{
    std::clog << __FUNCTION__;
    if (rooms.find(room) == rooms.end())
    {
        std::clog << " FATAL No such room" << std::endl;
        return;
    }
    rooms.erase(room);
    std::clog << " room deleted" << std::endl;
}

void Server::update_room_list()
{
    std::clog << "[ " << __FUNCTION__ << " ]" << std::endl;
    for (auto player : players)
    {
        if (player.second->getRoom() == nullptr)
        {
            Command::room_list(player.second->getConnection());
        }
    }
}

boost::shared_ptr<Player> Server::getPlayer_by_connection(boost::shared_ptr<Connection> connection)
{
    //std::clog << "[ " << __FUNCTION__ << " ]" << std::endl;
    for (auto player : players)
    {
        if (player.second->getConnection() == connection)
        {
            return player.second;
        }
    }
    std::clog << "Connection" << " FATAL No such player Connection" << std::endl;
}

void Server::update_room_info(boost::shared_ptr<Room> room)
{
    std::clog << "[ " << __FUNCTION__ << " ]" << std::endl;
    PlayerMessage message;
    message.setLen(25);
    message.setCommand(Command::Type::ROOM_INFO);
    std::vector< std::vector<uint8_t > >param;
    param.push_back(Formatter::vectorOf(std::to_string(room->getId())) );
    param.push_back(Formatter::vectorOf(room->getPassword()) );
    message.setParams(param);
    for (auto player : room->getPlayers())
    {
        message.setId(player->getId());
        Command::room_info(player->getConnection(), message);
    }
}

boost::shared_ptr<Player> Server::getPlayer_by_address(const boost::asio::ip::address &address)
{
    //std::clog << "[ " << __FUNCTION__ << " ]" << std::endl;
    for (auto player : players)
    {
        if (*player.second->getAddress() == address)
        {
            return player.second;
        }
    }
    std::clog << address << " FATAL No such player address" << std::endl;
}

void Server::delete_player(boost::shared_ptr<Player> player)
{
    PlayerMessage message;
    message.setId(player->getId());
    Command::leave_room(player->getConnection(), message);
    if (players.find(player->getId()) != players.end()) {
        players.erase(player->getId());
    }
    std::clog << "player " << player->getId() << " deleted" << std::endl;
}

void Server::Start()
{
    assert(shared_from_this() != nullptr);
    assert(shared_from_this().get() != nullptr);
    assert(shared_from_this().get() != NULL);

    acceptor.reset( new MyAcceptor( shared_from_this(), hive ) );
    acceptor->Listen( ip_address, port );
    //acceptor->StartTimer();

    boost::shared_ptr< Connection > connection( new MyConnection( shared_from_this(), hive ) );
    acceptor->Accept( connection );
    //connection->StartTimer();

    udp.reset(new MyUdpConnection( shared_from_this(), hive, ip_address, port ));
    udp->StartRecv();
    //udp->StartTimer();

    // reading invisibility image
    std::ifstream invisibleSource;
    invisibleSource.open("/home/qurbonzoda/Programming/ClionProjects/Mafia/invisibility.jpg", std::ios_base::binary);

    invisibleSource.seekg(0, invisibleSource.end);
    size_t imageSize = invisibleSource.tellg();
    invisibleSource.seekg(0, invisibleSource.beg);

    invisibilityImage.resize(imageSize);
    invisibleSource.read((char *) &invisibilityImage[0], invisibilityImage.size());

    // reading botScreen
    std::ifstream botSource;
    botSource.open("/home/qurbonzoda/Programming/ClionProjects/Mafia/bot.jpg", std::ios_base::binary);

    botSource.seekg(0, botSource.end);
    imageSize = botSource.tellg();
    botSource.seekg(0, botSource.beg);

    botScreen.resize(imageSize);
    botSource.read((char *) &botScreen[0], botScreen.size());

    add_bots();
}

void Server::add_bots()
{
    auto room = create_new_room_instance();
    room->setMax_players(9);
    room->setPassword("nopass");


    boost::shared_ptr<ip::address> address(new ip::address(boost::asio::ip::address::from_string("192.168.2.10")));

    for (int i = 0; i < 8; i++)
    {
        auto player = create_new_player_instance();
        boost::shared_ptr< MyConnection > new_connection( new MyConnection( shared_from_this(), getHive() ) );
        player->setConnection(new_connection);
        player->setAddress(address);
        player->setBot(true);
        room->join(player);
        player->setScreen(botScreen);
    }
}