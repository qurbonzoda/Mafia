//
// Created by qurbonzoda on 04.12.15.
//

#include <boost/thread/pthread/mutex.hpp>
#include "Server.h"
#include "PlayerMessage.h"
#include "Command.h"

using namespace boost::asio::ip;

uint32_t Server::player_id_counter = 0;
std::map<size_t, udp::endpoint> Server::players;


Server::Server(boost::shared_ptr< Hive > hive, std::string ip_address, uint16_t port) : hive(hive)
{
    boost::shared_ptr< MyAcceptor > acceptor( new MyAcceptor( hive ) );
    acceptor->Listen( ip_address, port );

    boost::shared_ptr< MyConnection > connection( new MyConnection( hive ) );
    acceptor->Accept( connection );
/*
    boost::shared_ptr< MyUdpConnection > udp_connection ( new MyUdpConnection( hive ) );
    udp_connection->Bind(ip_address, port);
    udp_connection->start_receive();
*/
}

void Server::MyConnection::OnAccept( const std::string & host, uint16_t port )
{
    std::clog << "[" << __FUNCTION__ << "] " << host << ":" << port << std::endl;
    /// id must be sent
    uint16_t len = 2 + 1 + 4;
    uint32_t id = ++player_id_counter;
    std::clog << "new player id " << id << std::endl;

    boost::shared_ptr<udp::endpoint> endpoint(new udp::endpoint(address::from_string( GetSocket().remote_endpoint().address().to_string() ), GetSocket().remote_endpoint().port()));

    players[id] = get_or_create_player(endpoint);
    std::vector<uint8_t> unique_id(Formatter::getMessageFormat(len, Command::Type::PLAYER_ID,
                                                               Formatter::getBytesOf(id, sizeof(id))));

    Send(unique_id);
    Recv();
}
void Server::MyConnection::OnConnect( const std::string & host, uint16_t port )
{
    std::clog << "[" << __FUNCTION__ << "] " << host << ":" << port << std::endl;
    Recv();
}

void Server::MyConnection::OnSend( const std::vector< uint8_t > & buffer )
{
    std::clog << "[" << __FUNCTION__ << "] " << buffer.size() << " bytes" << std::endl;
    for( size_t x = 0; x < buffer.size(); ++x )
    {
        std::clog << std::hex << std::setfill( '0' ) <<
        std::setw( 2 ) << (int)buffer[ x ] << " ";
        if( ( x + 1 ) % 16 == 0 )
        {
            std::clog << std::endl;
        }
    }
    std::clog << std::endl;
}

void Server::MyConnection::OnRecv( std::vector< uint8_t > & buffer )
{
    std::clog << "[" << __FUNCTION__ << "] " << buffer.size() << " bytes" << std::endl;
    for( size_t x = 0; x < buffer.size(); ++x )
    {
        std::clog << std::hex << std::setfill( '0' ) <<
        std::setw( 2 ) << (int)buffer[ x ] << " ";
        if( ( x + 1 ) % 16 == 0 )
        {
            std::clog << std::endl;
        }
    }
    std::clog << std::endl;
    PlayerMessage playerMessage(buffer);
    std::clog << "PlayerMessage len = " << playerMessage.getLen() << std::endl;
    std::clog << "PlayerMessage id = " << playerMessage.getId() << std::endl;
    std::clog << "PlayerMessage command = " << playerMessage.getCommand() << std::endl;
    std::clog << "PlayerMessage data = " << Formatter::getStringOf(playerMessage.getData()) << std::endl;
    Command::execute(playerMessage);
    // Start the next receive
    Recv();

    // Echo the data back
    Send( buffer );
}

void Server::MyConnection::OnTimer( const boost::posix_time::time_duration & delta )
{
    std::clog << "[" << __FUNCTION__ << "] " << delta << std::endl;
}

void Server::MyConnection::OnError( const boost::system::error_code & error )
{
    std::clog << "[" << __FUNCTION__ << "] " << error << std::endl;
}

Server::MyConnection::MyConnection( boost::shared_ptr< Hive > hive )
        : Connection( hive )
{
}

Server::MyConnection::~MyConnection()
{
}
bool Server::MyAcceptor::OnAccept( boost::shared_ptr< Connection > connection, const std::string & host, uint16_t port )
{
    boost::shared_ptr< MyConnection > new_connection( new MyConnection( GetHive() ) );
    this->Accept( new_connection );

    std::clog << "[" << __FUNCTION__ << "] " << host << ":" << port << std::endl;

    return true;
}

void Server::MyAcceptor::OnTimer( const boost::posix_time::time_duration & delta )
{
    std::clog << "[" << __FUNCTION__ << "] " << delta << std::endl;
}

void Server::MyAcceptor::OnError( const boost::system::error_code & error )
{
    std::clog << "[" << __FUNCTION__ << "] " << error << std::endl;
}

Server::MyAcceptor::MyAcceptor( boost::shared_ptr< Hive > hive )
        : Acceptor( hive )
{
}

Server::MyAcceptor::~MyAcceptor()
{
}
/*
void Server::MyUdpConnection::OnRecv( const std::vector< uint8_t > & buffer )
{
    start_receive();

}
void Server::MyUdpConnection::OnSend( const std::vector< uint8_t > & buffer );
*/
static boost::shared_ptr<Player> Server::get_or_create_player(boost::shared_ptr<udp::endpoint> endpoint)
{
    for (auto &player : players)
    {
        if (player.second->getEndpoint() == endpoint)
        {
            return player.second;
        }
    }
    return boost::shared_ptr<Player>(new Player(endpoint));
}
