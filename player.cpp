//
// Created by qurbonzoda on 23.11.15.
//

#include "player.h"
#include "my_network.h"
#include "room.h"
#include "server.h"

Player::Player(size_t id) : id_(id)
{
}
Player::Player(const boost::shared_ptr<boost::asio::ip::address> &address) : address_(address)
{
}
size_t Player::getId()
{
    return id_;
}
boost::shared_ptr<boost::asio::ip::address> Player::getAddress() const
{
    return address_;
}
boost::shared_ptr<Room> const &Player::getRoom()
{
    return room_;
}

bool Player::setRoom(boost::shared_ptr<Room> const &room)
{
    if (room == nullptr)
    {
        character = Character::NOT_SPECIFIED;
        roomPosition_ = 0;
    }
    this->room_ = room;
    return false;
}

const Player::Character &Player::getCharacter() const
{
    return character;
}

void Player::setCharacter(const Character &character)
{
    this->character = character;
}

void Player::setId(size_t id)
{
    this->id_ = id;
}

void Player::setAddress(const boost::shared_ptr<boost::asio::ip::address> &address)
{
    this->address_ = address;
}


void Player::setPassword(const std::string &password)
{
    credential_.password = password;
}

std::string Player::getPassword()
{
    return credential_.password;
}

void Player::setLogin(const std::string &login)
{
    credential_.login = login;
}

std::string Player::getLogin()
{
    return credential_.login;
}

size_t Player::getRoomPosition() const
{
    return roomPosition_;
}

void Player::setRoomPosition(size_t roomPosition)
{
    Player::roomPosition_ = roomPosition;
}

const boost::shared_ptr<MyConnection> &Player::getConnection() const
{
    return connection_;
}

const boost::shared_ptr<Server> &Player::getServer() const
{
    return connection_->getServer();
}

void Player::setConnection(const boost::shared_ptr<MyConnection> &connection)
{
    Player::connection_ = connection;
}

Player::~Player()
{
    std::clog << "[" << __FUNCTION__ << "] " << std::endl;
}

void Player::setScreen(std::vector<uint8_t> const &image)
{
    screenChanged_ = false;
    screen_ = image;
    screenChanged_ = true;
}

const std::vector<uint8_t> &Player::getScreen() const
{
    return screen_;
}

bool Player::canSee() const
{
    if (character == Character::NOT_SPECIFIED)
    {
        return true;
    }
    return room_->canSee(shared_from_this());
}

bool Player::isVisible() const
{
    if (character == Character::NOT_SPECIFIED)
    {
        return true;
    }
    return room_->isVisible(shared_from_this());
}

bool Player::canSpeak() const
{
    if (character == Character::NOT_SPECIFIED)
    {
        return true;
    }
    return room_->canSpeak(shared_from_this());
}

void Player::setCredential(const std::vector<std::string> &credential)
{
    std::clog << __FUNCTION__ << std::endl;
    std::clog << std::to_string(credential.size()) << std::endl;

    for (auto item : credential)
    {
        std::clog << item << std::endl;
    }

    // assert(credential.size() >= 6);
    switch (credential.size())
    {
        case 6:
            credential_.additional = (credential[5] != "#" ? credential[5] : "");
        case 5:
            credential_.vkAccount = (credential[4] != "#" ? credential[4] : "");
        case 4:
            credential_.fbAccount = (credential[3] != "#" ? credential[3] : "");
        case 3:
            credential_.email = (credential[2] != "#" ? credential[2] : "");
        case 2:
            credential_.age = (credential[1] != "#" ? credential[1] : "");
        case 1:
            credential_.name = (credential[0] != "#" ? credential[0] : "");

    }

    getServer()->updatePlayerCredential(shared_from_this());
}

std::string Player::getCredentail() const
{
    std::string answer = credential_.name + " " + credential_.age
                         + " " + credential_.email + " " + credential_.fbAccount
                         + " " + credential_.vkAccount + " " + credential_.additional;
    return answer;
}
