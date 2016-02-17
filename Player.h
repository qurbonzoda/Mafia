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
        Moderator, Detective, Doctor, Mafia, Villager, Dead, Not_specified
    };
    Player(size_t id);
    Player(const boost::shared_ptr< boost::asio::ip::address > &address);
    size_t getId();
    boost::shared_ptr<boost::asio::ip::address> getAddress() const;
    boost::shared_ptr< Room > getRoom();

    const Character &getCharacter() const;
    void setCharacter(const Character &character);
    void setId(size_t id);

    void setAddress(const boost::shared_ptr<boost::asio::ip::address> &address);
    bool setRoom(const boost::shared_ptr< Room > & room);
    void setLogin(const std::string &login);
    void setPassword(const std::string &password);
    size_t getRoom_position() const;
    void setRoom_position(size_t room_position);
    const boost::shared_ptr<MyConnection> &getConnection() const;
    void setConnection(const boost::shared_ptr<MyConnection> &connection);
    void setScreen(const std::vector<uint8_t> &image);
    const std::vector<uint8_t> &getScreen() const;
    //bool isInvisiblitySet() const;
    //void setInvisiblitySet(bool invisiblitySet);

    bool canSee() const;
    bool isVisible() const;
    bool canSpeak() const;

    ~Player();

private:
    Character character = Character::Not_specified;
    size_t id;
    size_t room_position;
    std::string password;
    std::string login;
    //bool invisiblitySet = false;

private:
    boost::shared_ptr< Room > room;
    boost::shared_ptr< MyConnection > connection;
    boost::shared_ptr< boost::asio::ip::address > address;
    std::vector<uint8_t> screen;
    bool bot = false;
    bool screenChanged = false;

public:

    bool isBot() const
    {
        return bot;
    }

    void setBot(bool bot)
    {
        Player::bot = bot;
    }
    bool isScreenChanged() const
    {
        return screenChanged;
    }

    void setScreenChanged(bool screenChanged)
    {
        Player::screenChanged = screenChanged;
    }
};

#endif //MAFIA_PLAYER_H
