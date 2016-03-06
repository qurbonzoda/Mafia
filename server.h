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
    Server(boost::shared_ptr< Hive > hive, std::string ipAddress, uint16_t port);
    Server(Server const&) = delete;
    void operator=(Server const&) = delete;

private:
    boost::shared_ptr<Player> createNewPlayerInstance();
    void addBots();

public:
    void start();
    boost::shared_ptr<Player> getOrCreatePlayer(boost::shared_ptr<boost::asio::ip::address> const &address);
    boost::shared_ptr<Room> getRoomById(uint32_t id);
    void deleteRoom(boost::shared_ptr<Room> room);
    void deletePlayer(boost::shared_ptr<Player> player);
    void updateRoomList();
    void updateRoomInfo(boost::shared_ptr<Room> const &room);
    boost::shared_ptr<Player> getPlayerByConnection(boost::shared_ptr<Connection> connection);
    boost::shared_ptr<Player> getPlayerByAddress(const boost::asio::ip::address &address);
    boost::shared_ptr<Room> createNewRoomInstance();
    bool isBusyLogin(std::string const &login);
    bool isRegistrated(std::string const &login, std::string const &password);
    void addPlayerCredential(boost::shared_ptr<Player> player);
    void updatePlayerCredential(boost::shared_ptr<Player> player);
    void loadPlayer(boost::shared_ptr<Player> player, std::string const &login, std::string const &password);
private:
    void updateCredentialsFile();
public:
    const boost::shared_ptr<Hive> &getHive() const
    {
        return hive_;
    }
    const std::set<boost::shared_ptr<Room>> &getRooms() const
    {
        return rooms_;
    }
    boost::shared_ptr<Player> &getPlayerById(uint32_t id)
    {
        return playersById_.at(id);
    }
    const boost::shared_ptr<MyUdpConnection> &getUdp()  const
    {
        return udp_;
    }
    const std::vector<uint8_t> &getInvisibilityImage() const
    {
        return invisibilityImage_;
    }

    const std::vector<uint8_t> &getBotScreen() const
    {
        return botScreen_;
    }

    const std::vector<uint8_t> &getRIPScreen() const
    {
        return RIPScreen_;
    }

private:
    std::set< boost::shared_ptr<Room> > rooms_;
    std::map<size_t, boost::shared_ptr<Player> > playersById_;
    std::map<std::string, std::string> credentials;

    volatile uint32_t playerIdCounter_ = 0;
    volatile uint32_t roomIdCounter_ = 0;
    std::vector<uint8_t> invisibilityImage_;
    std::vector<uint8_t> botScreen_;
    std::vector<uint8_t> RIPScreen_;
    std::vector<std::string> loginList_;
    std::vector<std::string> passwordList_;
    std::vector<std::string> credentialList_;
    std::string ipAddress_;
    uint16_t port_;
    boost::shared_ptr<Hive> hive_;
    boost::shared_ptr<MyUdpConnection> udp_;
    boost::shared_ptr<MyAcceptor> acceptor_;
};

#endif //MAFIA_SERVER_H
