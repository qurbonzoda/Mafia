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

Player::~Player()
{
}

