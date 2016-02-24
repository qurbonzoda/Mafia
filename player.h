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
        MODERATOR, DETECTIVE, DOCTOR, MAFIA, VILLAGER, DEAD, NOT_SPECIFIED
    };

    Player(size_t id);

    Player(const boost::shared_ptr<boost::asio::ip::address> &address);

    size_t getId();

    boost::shared_ptr<boost::asio::ip::address> getAddress() const;

    boost::shared_ptr<Room> getRoom();

    const Character &getCharacter() const;

    void setCharacter(const Character &character);

    void setId(size_t id);

    void setAddress(const boost::shared_ptr<boost::asio::ip::address> &address);

    bool setRoom(const boost::shared_ptr<Room> &room);

    void setLogin(const std::string &login);

    void setPassword(const std::string &password);

    size_t getRoomPosition() const;

    void setRoomPosition(size_t roomPosition);

    const boost::shared_ptr<MyConnection> &getConnection() const;

    void setConnection(const boost::shared_ptr<MyConnection> &connection);

    void setScreen(const std::vector<uint8_t> &image);

    const std::vector<uint8_t> &getScreen() const;

    bool canSee() const;

    bool isVisible() const;

    bool canSpeak() const;

    ~Player();

private:
    Character character = Character::NOT_SPECIFIED;
    size_t id_;
    size_t roomPosition_;
    std::string password_;
    std::string login_;

private:
    boost::shared_ptr<Room> room_;
    boost::shared_ptr<MyConnection> connection_;
    boost::shared_ptr<boost::asio::ip::address> address_;
    std::vector<uint8_t> screen_;
    bool bot_ = false;
    bool screenChanged_ = false;

public:

    bool isBot() const
    {
        return bot_;
    }

    void setBot(bool bot)
    {
        Player::bot_ = bot;
    }

    bool isScreenChanged() const
    {
        return screenChanged_;
    }

    void setScreenChanged(bool screenChanged)
    {
        Player::screenChanged_ = screenChanged;
    }
};

#endif //MAFIA_PLAYER_H
