//
// Created by qurbonzoda on 23.11.15.
//

#include "Player.h"
#include "network.h"


Player::Player(size_t id) : id(id)
{
}
Player::Player(boost::shared_ptr<boost::asio::ip::address> address) : address(address)
{
}
size_t Player::getId()
{
    return id;
}
boost::shared_ptr<boost::asio::ip::address> Player::getAddress()
{
    return address;
}
boost::shared_ptr<Room> Player::getRoom()
{
    return room;
}

bool Player::setRoom(boost::shared_ptr<Room> room)
{
    this->room = room;
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
    this->id = id;
}

const std::string & Player::getLogin() const
{
    return login;
}

void Player::setLogin(const std::string &login)
{
    this->login = login;
}

void Player::setAddress(const boost::shared_ptr<boost::asio::ip::address> &address)
{
    this->address = address;
}

const std::string &Player::getPassword() const
{
    return password;
}

void Player::setPassword(const std::string &password)
{
    this->password = password;
}

size_t Player::getRoom_position() const
{
    return room_position;
}

void Player::setRoom_position(size_t room_position)
{
    Player::room_position = room_position;
}

const boost::shared_ptr<MyConnection> &Player::getConnection() const
{
    return connection;
}

void Player::setConnection(const boost::shared_ptr<MyConnection> &connection)
{
    Player::connection = connection;
}

Player::~Player()
{
    std::clog << "[" << __FUNCTION__ << "] " << std::endl;
}

void Player::setScreen(std::vector<uint8_t> image)
{
    screen = image;
    screenChanged = true;
}

std::vector<uint8_t> Player::getScreen()
{
    return screen;
}
