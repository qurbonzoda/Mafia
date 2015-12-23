//
// Created by qurbonzoda on 23.11.15.
//

#include "Player.h"


Player::Player(size_t id) : id(id)
{
}
Player::Player(boost::shared_ptr<boost::asio::ip::udp::endpoint> endpoint) : endpoint(endpoint)
{
}
size_t Player::getId()
{
    return id;
}
boost::shared_ptr<boost::asio::ip::udp::endpoint> Player::getEndpoint()
{
    return endpoint;
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

void Player::setEndpoint(const boost::shared_ptr<boost::asio::ip::udp::endpoint> &endpoint)
{
    this->endpoint = endpoint;
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

const boost::shared_ptr<Connection> &Player::getConnection() const
{
    return connection;
}

void Player::setConnection(const boost::shared_ptr<Connection> &connection)
{
    Player::connection = connection;
}

Player::~Player()
{
}

