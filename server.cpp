//
// Created by qurbonzoda on 04.12.15.
//

#include <boost/thread/pthread/mutex.hpp>
#include <fstream>
#include "server.h"
#include "player_message.h"
#include "command.h"
#include "my_network.h"
#include "player.h"
#include "room.h"
#include "formatter.h"
#include "constants.h"

using namespace boost::asio;

Server::Server(boost::shared_ptr< Hive > hive, std::string ipAddress, uint16_t port)
        : hive_(hive), ipAddress_(ipAddress), port_(port)
{
    std::clog << "Server on " << ipAddress << ": " << port << std::endl;
}

boost::shared_ptr<Player> Server::getOrCreatePlayer(boost::shared_ptr<ip::address> const &address)
{
    for (auto &player : playersById_)
    {
        if (player.second->getAddress() == address)
        {
            return player.second;
        }
    }
    auto player = createNewPlayerInstance();
    player->setAddress(address);
    return player;
}

boost::shared_ptr<Player> Server::createNewPlayerInstance()
{
    std::clog << "create_new_player_instance: player_id_counter = " << playerIdCounter_ << std::endl;
    boost::shared_ptr<Player> player(new Player(++playerIdCounter_));
    playersById_[playerIdCounter_] = player;
    return player;
}

boost::shared_ptr<Room> Server::createNewRoomInstance()
{
    boost::shared_ptr<Room> room(new Room(++roomIdCounter_, shared_from_this()));
    rooms_.insert(room);
    std::clog << "create_new_room_instance: room_id_counter = " << roomIdCounter_ << std::endl;
    return room;
}

boost::shared_ptr<Room> Server::getRoomById(uint32_t id)
{
    for (auto room : rooms_)
    {
        if (room->getId() == id)
        {
            return room;
        }
    }
    std::clog << std::to_string(id) + " FATAL No such room id" << std::endl;
    return nullptr;
}

void Server::deleteRoom(boost::shared_ptr<Room> room)
{
    std::clog << __FUNCTION__;
    if (rooms_.find(room) == rooms_.end())
    {
        std::clog << " FATAL No such room" << std::endl;
        return;
    }
    rooms_.erase(room);
    std::clog << " room deleted" << std::endl;
}

void Server::updateRoomList()
{
    std::clog << "[ " << __FUNCTION__ << " ]" << std::endl;
    for (auto player : playersById_)
    {
        if (player.second->getRoom() == nullptr)
        {
            command::sendRoomList(player.second->getConnection());
        }
    }
}

boost::shared_ptr<Player> Server::getPlayerByConnection(boost::shared_ptr<Connection> connection)
{
    //std::clog << "[ " << __FUNCTION__ << " ]" << std::endl;
    for (auto player : playersById_)
    {
        if (player.second->getConnection() == connection)
        {
            return player.second;
        }
    }
    std::clog << "Connection" << " FATAL No such player Connection" << std::endl;
}

void Server::updateRoomInfo(boost::shared_ptr<Room> room)
{
    std::clog << "[ " << __FUNCTION__ << " ]" << std::endl;
    PlayerMessage message;
    message.setLen(25);
    message.setCommand(command::Type::ROOM_INFO);
    std::vector< std::vector<uint8_t > >param;
    param.push_back(Formatter::vectorOf(std::to_string(room->getId())) );
    param.push_back(Formatter::vectorOf(room->getPassword()) );
    message.setParams(param);
    for (auto player : room->getPlayers())
    {
        message.setId(player->getId());
        command::sendRoomInformation(player->getConnection(), message);
    }
}

boost::shared_ptr<Player> Server::getPlayerByAddress(const boost::asio::ip::address &address)
{
    //std::clog << "[ " << __FUNCTION__ << " ]" << std::endl;
    for (auto player : playersById_)
    {
        if (*player.second->getAddress() == address)
        {
            return player.second;
        }
    }
    std::clog << address << " FATAL No such player address" << std::endl;
}

void Server::deletePlayer(boost::shared_ptr<Player> player)
{
    PlayerMessage message;
    message.setId(player->getId());
    command::leaveRoom(player->getConnection(), message);
    if (playersById_.find(player->getId()) != playersById_.end()) {
        playersById_.erase(player->getId());
    }
    std::clog << "player " << player->getId() << " deleted" << std::endl;
}

void Server::start()
{
    acceptor_.reset(new MyAcceptor(shared_from_this(), hive_) );
    acceptor_->Listen(ipAddress_, port_);
    //acceptor->StartTimer();

    boost::shared_ptr< Connection > connection(
            new MyConnection(shared_from_this(), hive_)
    );
    acceptor_->Accept(connection );
    //connection->StartTimer();

    udp_.reset(new MyUdpConnection(shared_from_this(), hive_, ipAddress_, port_));
    udp_->StartRecv();
    //udp->StartTimer();

    // reading invisibility image
    std::ifstream invisibleSource;
    invisibleSource.open(PATH_TO_INVISIBILITY_SCREEN, std::ios_base::binary);

    invisibleSource.seekg(0, invisibleSource.end);
    size_t imageSize = invisibleSource.tellg();
    invisibleSource.seekg(0, invisibleSource.beg);

    invisibilityImage_.resize(imageSize);
    invisibleSource.read((char *) &invisibilityImage_[0], invisibilityImage_.size());

    // reading botScreen
    std::ifstream botSource;
    botSource.open(PATH_TO_BOT_SCREEN, std::ios_base::binary);

    botSource.seekg(0, botSource.end);
    imageSize = botSource.tellg();
    botSource.seekg(0, botSource.beg);

    botScreen_.resize(imageSize);
    botSource.read((char *) &botScreen_[0], botScreen_.size());

    // reading RIPScreen
    std::ifstream RIPSource;
    RIPSource.open(PATH_TO_RIP_SCREEN, std::ios_base::binary);

    RIPSource.seekg(0, RIPSource.end);
    imageSize = RIPSource.tellg();
    RIPSource.seekg(0, RIPSource.beg);

    RIPScreen_.resize(imageSize);
    RIPSource.read((char *) &RIPScreen_[0], RIPScreen_.size());

    addBots();
}

void Server::addBots()
{
    auto room = createNewRoomInstance();
    room->setMaximumPlayers(9);
    room->setPassword(NO_PASSWORD);


    boost::shared_ptr<ip::address> address(
            new ip::address(boost::asio::ip::address::from_string("192.168.2.10"))
    );
    boost::shared_ptr< MyConnection > newConnection(
            new MyConnection(shared_from_this(), getHive() )
    );

    for (int i = 0; i < 7; i++)
    {
        auto player = createNewPlayerInstance();
        player->setConnection(newConnection);
        player->setAddress(address);
        player->setBot(true);
        room->join(player);
        player->setScreen(botScreen_);
    }
}