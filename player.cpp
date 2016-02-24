//
// Created by qurbonzoda on 23.11.15.
//

#include "player.h"
#include "network.h"
#include "room.h"


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
boost::shared_ptr<Room> Player::getRoom()
{
    return room_;
}

bool Player::setRoom(boost::shared_ptr<Room> const &room)
{
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
    this->password_ = password;
}

void Player::setLogin(const std::string &login)
{
    Player::login_ = login;
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
    if (character == NOT_SPECIFIED)
    {
        return true;
    }
    return room_->canSpeak(shared_from_this());
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