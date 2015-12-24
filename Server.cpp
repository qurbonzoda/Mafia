//
// Created by qurbonzoda on 04.12.15.
//

#include <boost/thread/pthread/mutex.hpp>
#include "Server.h"
#include "PlayerMessage.h"
#include "Command.h"
#include "MyNetwork.h"

using namespace boost::asio::ip;

volatile uint32_t Server::player_id_counter = 0;
std::map<size_t, boost::shared_ptr<Player> > Server::players;
std::set< boost::shared_ptr<Room> > Server::rooms;
volatile uint32_t Server::room_id_counter = 0;


boost::shared_ptr<Server> Server::newInstance(boost::shared_ptr< Hive > hive, std::string ip_address, uint16_t port)
{
    boost::shared_ptr<Server> newServer(new Server(hive));
    boost::shared_ptr< MyAcceptor > acceptor( new MyAcceptor( newServer, hive ) );
    acceptor->Listen( ip_address, port );

    boost::shared_ptr< MyConnection > connection( new MyConnection( newServer, hive ) );
    acceptor->Accept( connection );

    assert(newServer != nullptr);
    assert(newServer.get() != nullptr);
    assert(newServer.get() != NULL);

    boost::shared_ptr< MyUdpConnection > udp_connection ( new MyUdpConnection( newServer, hive, ip_address, port ) );
    udp_connection->StartRecv();
    return newServer;
}

Server::Server(boost::shared_ptr< Hive > hive) : hive(hive)
{
}

boost::shared_ptr<Player> Server::get_or_create_player(boost::shared_ptr<udp::endpoint> endpoint)
{
    for (auto &player : players)
    {
        if (player.second->getEndpoint() == endpoint)
        {
            return player.second;
        }
    }
    auto player = create_new_player_instance();
    player->setEndpoint(endpoint);
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
    boost::shared_ptr<Room> room(new Room(++room_id_counter));
    rooms.insert(room);
    std::clog << "create_new_player_instance: room_id_counter = " << room_id_counter << std::endl;
    return room;
}

uint32_t Server::getId_by_player(boost::shared_ptr<Player> player)
{
    for (auto aPlayer : players)
    {
        if (aPlayer.second == player) {
            return aPlayer.second->getId();
        }
    }
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
    std::clog << id << " FATAL No such room id" << std::endl;
}

void Server::room_erase(boost::shared_ptr<Room> room)
{
    rooms.erase(room);
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
    std::clog << "[ " << __FUNCTION__ << " ]" << std::endl;
    for (auto player : players)
    {
        if (player.second->getConnection() == connection)
        {
            return player.second;
        }
    }
    std::clog << "Connection" << " FATAL No such room Connection" << std::endl;
}

void Server::update_room_info(boost::shared_ptr<Room> room)
{
    std::clog << "[ " << __FUNCTION__ << " ]" << std::endl;
    PlayerMessage message;
    message.setLen(25);
    message.setCommand(Command::Type::ROOM_INFO);
    std::vector< std::vector<uint8_t > >param;
    param.push_back( Formatter::getVectorOf(std::to_string(room->getId())) );
    param.push_back( Formatter::getVectorOf(room->getPassword()) );
    message.setParams(param);
    for (auto player : room->getPlayers())
    {
        message.setId(player->getId());
        Command::room_info(player->getConnection(), message);
    }
}

boost::shared_ptr<Player> Server::getPlayer_by_endpoint(const boost::asio::ip::udp::endpoint &endpoint)
{
    std::clog << "[ " << __FUNCTION__ << " ]" << std::endl;
    for (auto player : players)
    {
        if (*player.second->getEndpoint() == endpoint)
        {
            return player.second;
        }
    }
    std::clog << endpoint << " FATAL No such room endpoint" << std::endl;
}

void Server::delete_player(boost::shared_ptr<Player> player)
{
    PlayerMessage message;
    message.setId(player->getId());
    Command::leave_room(player->getConnection(), message);
    players.erase(player->getId());
}
