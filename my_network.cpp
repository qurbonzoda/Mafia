//
// Created by qurbonzoda on 23.12.15.
//

#include <fstream>
#include <boost/thread.hpp>
#include "my_network.h"
#include "server.h"
#include "player.h"
#include "room.h"
#include "formatter.h"
#include "player_message.h"
#include "command.h"

using namespace boost::asio;


void MyConnection::onAccept(const std::string &host, uint16_t port)
{
    std::clog << "[" << __FUNCTION__ << "] " << host << ":" << port << std::endl;
    /// id must be sent

    boost::shared_ptr<ip::address> address(new ip::address(getSocket().remote_endpoint().address()));

    boost::shared_ptr<Player> acceptedPlayer = getServer()->getOrCreatePlayer(address);
    acceptedPlayer->setConnection(shared_from_this());
    uint32_t id = acceptedPlayer->getId();

    std::string message = std::to_string(command::Type::PLAYER_ID) + " " + std::to_string(id);

    message = Formatter::getMessageFormat(message);
    std::clog << "new player id " << id << std::endl;

    send(Formatter::vectorOf(message));
    receive();
}

void MyConnection::onSend(const std::vector<uint8_t> &buffer)
{
    std::clog << "[" << __FUNCTION__ << "] " << std::to_string(buffer.size()) << " bytes" << std::endl;
    std::clog << "buffer = " + Formatter::stringOf(buffer) << std::endl;
}

void MyConnection::onReceive(std::vector<uint8_t> &buffer)
{
    std::clog << "[" << __FUNCTION__ << "] " << std::to_string(buffer.size()) << " bytes" << std::endl;


    if (!carry.empty())
    {
        buffer.insert(buffer.begin(), carry.begin(), carry.end());
        carry.clear();
    }

    size_t separator = std::find(buffer.begin(), buffer.end(), ' ') - buffer.begin();
    size_t len = std::stoul(Formatter::stringOf(std::vector<uint8_t> (buffer.begin(), buffer.begin() + separator)));


    if (len < buffer.size())
    {
        std::clog << "GLUED MESSAGES" << std::endl;
        carry.assign(buffer.begin() + len, buffer.end());
        buffer.resize(len);
    }

    if (len > buffer.size())
    {
        carry = buffer;
        receive(len - buffer.size());
        return;
    }

    for( size_t x = 0; x < buffer.size(); ++x )
    {
        std::clog << std::hex << std::setfill( '0' ) <<
        std::setw( 2 ) << (int)buffer[ x ] << " ";
    }
    std::clog << std::endl;
    std::clog << "buffer = " + Formatter::stringOf(buffer) << std::endl;
    try
    {
        PlayerMessage playerMessage(buffer);
        std::clog << "PlayerMessage len = " << playerMessage.getLen() << std::endl;
        std::clog << "PlayerMessage id = " << playerMessage.getId() << std::endl;
        std::clog << "PlayerMessage command = " << playerMessage.getCommand() << std::endl;

        command::execute(shared_from_this(), playerMessage);

    }
    catch (...)
    {
        // ignore for now
    }

    if (!carry.empty())
    {
        separator = std::find(carry.begin(), carry.end(), ' ') - carry.begin();
        len = std::stoul(Formatter::stringOf(std::vector<uint8_t> (carry.begin(), carry.begin() + separator)));
        if (len <= carry.size())
        {
            buffer = carry;
            carry.clear();
            onReceive(buffer);
            return;
        }
    }

    // Start the next receive
    receive();

    // Echo the data back
    //Send( buffer );
}

void MyConnection::onTimer(const boost::posix_time::time_duration &delta)
{
    std::clog << "MyConnection::[" << __FUNCTION__ << "] " << delta << std::endl;
}

void MyConnection::onError(const boost::system::error_code &error)
{
    auto server = this->getServer();
    auto player = server->getPlayerByConnection(shared_from_this());
    server->deletePlayer(player);

    std::clog << "MyConnection::[" << __FUNCTION__ << "] " << error << std::endl;
}
MyConnection::MyConnection(boost::shared_ptr<Server> server, boost::shared_ptr<Hive> hive)
        : Connection( hive ), server(server)
{
    Connection::setReceiveBufferSize(1000);
}


MyConnection::~MyConnection()
{
    std::clog << "[" << __FUNCTION__ << "] " <<  std::endl;
}
bool MyAcceptor::onAccept(boost::shared_ptr<Connection> connection, const std::string &host, uint16_t port)
{
    boost::shared_ptr< MyConnection > newConnection(new MyConnection(this->getServer(), getHive() ) );
    this->accept(newConnection);

    std::clog << "[" << __FUNCTION__ << "] " << host << ":" << port << std::endl;

    return true;
}

void MyAcceptor::onTimer(const boost::posix_time::time_duration &delta)
{
    std::clog << "[MyAccept::" << __FUNCTION__ << "] " << delta << std::endl;
}

void MyAcceptor::onError(const boost::system::error_code &error)
{
    std::clog << "MyAcceptor::[" << __FUNCTION__ << "] " << error << std::endl;
}

MyAcceptor::MyAcceptor(boost::shared_ptr<Server> server, boost::shared_ptr<Hive> hive)
        : Acceptor( hive ), server(server)
{

}

MyAcceptor::~MyAcceptor()
{
}

MyUdpConnection::MyUdpConnection(boost::shared_ptr<Server> server, boost::shared_ptr<Hive> hive,
                                   const std::string &host, uint16_t port) : UdpConnection(hive, host, port), server(server)
{
    UdpConnection::setReceiveBufferSize(100000);
}

void MyUdpConnection::onSend(const std::vector<uint8_t> &buffer, boost::asio::ip::udp::endpoint remoteEndpoint)
{

    //std::clog << "[ " << __FUNCTION__ << " ]" << "thread " << boost::this_thread::get_id() << std::endl;
    std::clog << "[ " << std::to_string((size_t)buffer.size()) << " ] bytes sent to " << remoteEndpoint << std::endl;
    std::clog << (int)buffer.front() << std::endl;

}

void MyUdpConnection::onReceive(std::vector<uint8_t> &buffer, boost::asio::ip::udp::endpoint remoteEndpoint)
{

    //std::clog << "[ " << __FUNCTION__ << " ]" << "thread " << boost::this_thread::get_id() << std::endl;
    std::clog << "[ " << std::to_string((size_t)buffer.size()) << " ] bytes received from " << remoteEndpoint << std::endl;
    std::clog << (int)buffer.back() << std::endl;

    if (buffer.empty())
        return;

    auto thePlayer = getServer()->getPlayerByAddress(remoteEndpoint.address());

    if (buffer.back() == 113)
    {
        buffer.pop_back();
        if (thePlayer->isVisible())
        {
            thePlayer->setScreen(buffer);
        }
    }
    else
    {
        buffer.insert(buffer.begin(), 111);
        std::clog << "Audio mather fucker !!!" << std::endl;
        auto room = thePlayer->getRoom();
        for (auto player : room->getPlayers())
        {
            if (player != thePlayer)
            {
                send(buffer, boost::asio::ip::udp::endpoint(*(player->getAddress()), 1010));
            }
        }
    }
}

void MyUdpConnection::onError(const boost::system::error_code &error, boost::asio::ip::udp::endpoint remoteEndpoint)
{
    std::clog << "[" << __FUNCTION__ << "] " << error << std::endl;
    std::clog << " error on endpoint " << remoteEndpoint << std::endl;
    //boost::this_thread::sleep(boost::posix_time::milliseconds(2000));
}

void MyUdpConnection::onTimer(const boost::posix_time::time_duration &delta)
{
    std::clog << "MyUdpConnection::[" << __FUNCTION__ << "] " << delta << std::endl;
}

MyUdpConnection::~MyUdpConnection()
{
    std::clog << "[" << __FUNCTION__ << "] " << std::endl;
}
