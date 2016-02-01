//
// Created by qurbonzoda on 23.11.15.
//

#ifndef MAFIA_PLAYER_H
#define MAFIA_PLAYER_H

#include <boost/shared_ptr.hpp>
#include <boost/enable_shared_from_this.hpp>
#include <boost/asio/ip/udp.hpp>

class Room;
class Server;
class MyConnection;

class Player : public boost::enable_shared_from_this<Player>
{

public:
    enum Character
    {
        Moderator, Detective, Doctor, Mafia, Villager, Bitch
    };
    Player(size_t id);
    Player(boost::shared_ptr< boost::asio::ip::address > address);
    size_t getId();
    boost::shared_ptr<boost::asio::ip::address> getAddress();
    boost::shared_ptr< Room > getRoom();

    const Character &getCharacter() const;
    void setCharacter(const Character &character);
    void setId(size_t id);
    const std::string &getLogin() const;
    void setLogin(const std::string &login);
    void setAddress(const boost::shared_ptr<boost::asio::ip::address> &address);
    bool setRoom(boost::shared_ptr< Room > room);
    const std::string &getPassword() const;
    void setPassword(const std::string &password);
    size_t getRoom_position() const;
    void setRoom_position(size_t room_position);
    const boost::shared_ptr<MyConnection> &getConnection() const;
    void setConnection(const boost::shared_ptr<MyConnection> &connection);
    void setScreen(std::vector<uint8_t> image);
    std::vector<uint8_t> getScreen();
    ~Player();

private:
    Character character;
    size_t id;
    size_t room_position;
    std::string login;
    std::string password;
    boost::shared_ptr< Room > room;
    boost::shared_ptr< MyConnection > connection;
    boost::shared_ptr< boost::asio::ip::address > address;
    std::vector<uint8_t> screen;
public:
    bool isScreenChanged() const
    {
        return screenChanged;
    }

public:
    void setScreenChanged(bool screenChanged)
    {
        Player::screenChanged = screenChanged;
    }

private:
    bool screenChanged = false;

};

#endif //MAFIA_PLAYER_H
