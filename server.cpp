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
            command::sendRoomList(player.second);
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

void Server::updateRoomInfo(boost::shared_ptr<Room> const &room)
{
    std::clog << "[ " << __FUNCTION__ << " ]" << std::endl;
    for (auto player : room->getPlayers())
    {
        command::sendRoomInformation(player, room, room->getPassword());
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
    command::leaveRoom(player);
    if (playersById_.find(player->getId()) != playersById_.end()) {
        playersById_.erase(player->getId());
    }
    std::clog << "player " << player->getId() << " deleted" << std::endl;
}

void Server::start()
{
    acceptor_.reset(new MyAcceptor(shared_from_this(), hive_) );
    acceptor_->listen(ipAddress_, port_);
    //acceptor->StartTimer();

    boost::shared_ptr< Connection > connection(
            new MyConnection(shared_from_this(), hive_)
    );
    acceptor_->accept(connection);
    //connection->StartTimer();

    udp_.reset(new MyUdpConnection(shared_from_this(), hive_, ipAddress_, port_));
    udp_->startReceive();
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

    // reading credentials
    std::ifstream credentialsSource;
    credentialsSource.open(PATH_TO_CREDENTIALS);

    std::string login;
    std::string password;
    std::string credential;

    while (std::getline(credentialsSource, login))
    {
        std::getline(credentialsSource, password);
        std::getline(credentialsSource, credential);
        credentials[login] = password;
        std::clog << login << std::endl;
        std::clog << password << std::endl;
        std::clog << credential << std::endl;
        loginList_.push_back(login);
        passwordList_.push_back(password);
        credentialList_.push_back(login);
    }

    addBots();
}

void Server::addBots()
{
    auto room = createNewRoomInstance();
    room->setMaximumPlayers(10);
    room->setPassword(NO_PASSWORD);


    boost::shared_ptr<ip::address> address(
            new ip::address(boost::asio::ip::address::from_string("192.168.2.10"))
    );
    boost::shared_ptr< MyConnection > newConnection(
            new MyConnection(shared_from_this(), getHive() )
    );

    for (int i = 0; i < 8; i++)
    {
        auto player = createNewPlayerInstance();
        player->setConnection(newConnection);
        player->setAddress(address);
        player->setBot(true);
        room->join(player);
        player->setScreen(botScreen_);
    }
}

bool Server::isBusyLogin(std::string const &login)
{
    return (credentials.find(login) != credentials.end());
}


bool Server::isRegistrated(std::string const &login, std::string const &password)
{
    return  (credentials.find(login) != credentials.end()
             && credentials[login] == password);
}

void Server::updateCredentialsFile()
{
    std::clog << __FUNCTION__ << std::endl;
    std::ofstream credentialsSource(PATH_TO_CREDENTIALS);

    for (int i = 0; i < loginList_.size(); ++i)
    {
        credentialsSource << loginList_[i] << std::endl;
        credentialsSource << passwordList_[i] << std::endl;
        credentialsSource << credentialList_[i] << std::endl;

        std::clog << loginList_[i] << std::endl;
        std::clog << passwordList_[i] << std::endl;
        std::clog << credentialList_[i] << std::endl;
    }
}

void Server::loadPlayer(boost::shared_ptr<Player> player, std::string const &login, std::string const &password)
{
    std::clog << __FUNCTION__ << std::endl;

    player->setLogin(login);
    player->setPassword(password);

    for (int i = 0; i < loginList_.size(); ++i)
    {
        if (loginList_[i] == login && passwordList_[i] == password)
        {
            std::istringstream iss(credentialList_[i]);

            std::vector< uint8_t > v(credentialList_[i].begin(), credentialList_[i].end());

            auto parts = Formatter::split(v, ' ');

            std::vector<std::string> playerCredential;

            for (auto part : parts)
            {
                playerCredential.push_back(Formatter::stringOf(part));
            }

            player->setCredential(playerCredential);
        }
    }
}

void Server::updatePlayerCredential(boost::shared_ptr<Player> player)
{
    for (int i = 0; i < loginList_.size(); ++i)
    {
        if (loginList_[i] == player->getLogin() && passwordList_[i] == player->getPassword())
        {
            credentialList_[i] = player->getCredentail();
        }
    }
    updateCredentialsFile();
}

void Server::addPlayerCredential(boost::shared_ptr<Player> player)
{
    credentials[player->getLogin()] = player->getPassword();
    loginList_.push_back(player->getLogin());
    passwordList_.push_back(player->getPassword());
    credentialList_.push_back(player->getCredentail());
    updateCredentialsFile();
}
