//
// Created by qurbonzoda on 23.11.15.
//

#include "Player.h"
#include "network.h"
#include "Room.h"


Player::Player(size_t id) : id(id)
{
}
Player::Player(const boost::shared_ptr<boost::asio::ip::address> &address) : address(address)
{
}
size_t Player::getId()
{
    return id;
}
boost::shared_ptr<boost::asio::ip::address> Player::getAddress() const
{
    return address;
}
boost::shared_ptr<Room> Player::getRoom()
{
    return room;
}

bool Player::setRoom(boost::shared_ptr<Room> const &room)
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

void Player::setAddress(const boost::shared_ptr<boost::asio::ip::address> &address)
{
    this->address = address;
}


void Player::setPassword(const std::string &password)
{
    this->password = password;
}

void Player::setLogin(const std::string &login)
{
    Player::login = login;
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

void Player::setScreen(std::vector<uint8_t> const &image)
{
    screenChanged = false;
    screen = image;
    screenChanged = true;
/*
    if (!isBot())
        for (auto player : room->getPlayers())
        {
            if (player->isBot())
                player->setScreenChanged(true);
        }
*/
}

const std::vector<uint8_t> &Player::getScreen() const
{
    return screen;
}

bool Player::canSee() const
{
    if (character == Character:: Not_specified)
    {
        return true;
    }
    return room->canSee(shared_from_this());
}

bool Player::isVisible()const
{
    if (character == Character::Not_specified)
    {
        return true;
    }
    return room->isVisible(shared_from_this());
}

bool Player::canSpeak() const
{
    if (character == Not_specified)
    {
        return true;
    }
    return room->canSpeak(shared_from_this());
}
/*
bool Player::isInvisiblitySet() const
{
    return Player::invisiblitySet;
}

void Player::setInvisiblitySet(bool invisiblitySet)
{
    Player::invisiblitySet = invisiblitySet;

}
*/