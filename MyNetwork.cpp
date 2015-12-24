//
// Created by qurbonzoda on 23.12.15.
//

#include "MyNetwork.h"

using namespace boost::asio::ip;

void MyConnection::OnAccept( const std::string & host, uint16_t port )
{
    std::clog << "[" << __FUNCTION__ << "] " << host << ":" << port << std::endl;
    /// id must be sent

    boost::shared_ptr<udp::endpoint> endpoint(new udp::endpoint(address::from_string( GetSocket().remote_endpoint().address().to_string() ), GetSocket().remote_endpoint().port()));

    boost::shared_ptr<Player> this_player = getServer()->get_or_create_player(endpoint);
    this_player->setConnection(shared_from_this());
    uint32_t id = getServer()->getId_by_player(this_player);
    std::string message = Formatter::getMessageFormat(Command::Type::PLAYER_ID, id);
    std::clog << "new player id " << id << std::endl;

    Send(Formatter::getVectorOf(message));
    Recv();
}
void MyConnection::OnConnect( const std::string & host, uint16_t port )
{
    std::clog << "[" << __FUNCTION__ << "] " << host << ":" << port << std::endl;
    Recv();
}

void MyConnection::OnSend( const std::vector< uint8_t > & buffer )
{
    std::clog << "[" << __FUNCTION__ << "] " << buffer.size() << " bytes" << std::endl;
    for( size_t x = 0; x < buffer.size(); ++x )
    {
        std::clog << std::hex << std::setfill( '0' ) <<
        std::setw( 2 ) << (int)buffer[ x ] << " ";
        /*
        if( ( x + 1 ) % 16 == 0 )
        {
            std::clog << std::endl;
        }
         */
    }
    std::clog << std::endl;
    std::clog << Formatter::getStringOf(buffer) << std::endl;
}

void MyConnection::OnRecv( std::vector< uint8_t > & buffer )
{
    std::clog << "[" << __FUNCTION__ << "] " << buffer.size() << " bytes" << std::endl;
    for( size_t x = 0; x < buffer.size(); ++x )
    {
        std::clog << std::hex << std::setfill( '0' ) <<
        std::setw( 2 ) << (int)buffer[ x ] << " ";
        /*
        if( ( x + 1 ) % 16 == 0 )
        {
            std::clog << std::endl;
        }
        */
    }
    std::clog << std::endl;
    std::clog << "buffer = " << Formatter::getStringOf(buffer) << std::endl;
    PlayerMessage playerMessage(buffer);
    std::clog << "PlayerMessage len = " << playerMessage.getLen() << std::endl;
    std::clog << "PlayerMessage id = " << playerMessage.getId() << std::endl;
    std::clog << "PlayerMessage command = " << playerMessage.getCommand() << std::endl;


    Command::execute(boost::static_pointer_cast<MyConnection>(shared_from_this()), playerMessage);
    // Start the next receive
    Recv();

    // Echo the data back
    //Send( buffer );
}

void MyConnection::OnTimer( const boost::posix_time::time_duration & delta )
{
    std::clog << "[" << __FUNCTION__ << "] " << delta << std::endl;
}

void MyConnection::OnError( const boost::system::error_code & error )
{
    auto server = this->getServer();
    auto player = server->getPlayer_by_connection(shared_from_this());
    server->delete_player(player);

    std::clog << "[" << __FUNCTION__ << "] " << error << std::endl;
}
MyConnection::MyConnection(boost::shared_ptr<Server> server, boost::shared_ptr<Hive> hive)
        : Connection( hive ), server(server)
{

}


MyConnection::~MyConnection()
{
}
bool MyAcceptor::OnAccept( boost::shared_ptr< Connection > connection, const std::string & host, uint16_t port )
{
    boost::shared_ptr< MyConnection > new_connection( new MyConnection( this->getServer(), GetHive() ) );
    this->Accept( new_connection );

    std::clog << "[" << __FUNCTION__ << "] " << host << ":" << port << std::endl;

    return true;
}

void MyAcceptor::OnTimer( const boost::posix_time::time_duration & delta )
{
    std::clog << "[" << __FUNCTION__ << "] " << delta << std::endl;
}

void MyAcceptor::OnError( const boost::system::error_code & error )
{
    std::clog << "[" << __FUNCTION__ << "] " << error << std::endl;
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
}

void MyUdpConnection::OnSend(const std::vector<uint8_t> &buffer, boost::asio::ip::udp::endpoint remote_endpoint)
{
    std::clog << "[ " << __FUNCTION__ << " ]" << std::endl;
    std::clog << "[ " << buffer.size() << " ] bytes sent to " << remote_endpoint << std::endl;
}

void MyUdpConnection::OnRecv(std::vector<uint8_t> &buffer, boost::asio::ip::udp::endpoint remote_endpoint)
{
    std::clog << "[ " << __FUNCTION__ << " ]" << std::endl;
    std::clog << "[ " << buffer.size() << " ] bytes received from " << remote_endpoint << std::endl;
    Send(buffer, remote_endpoint);
    Send(buffer, udp::endpoint(remote_endpoint.address(), 1010));

    /*
    auto player = getServer()->getPlayer_by_endpoint(remote_endpoint);
    auto room = player->getRoom();

    for (auto player : room->getPlayers())
    {
        Send(buffer, *player->getEndpoint());
    }
    */
}

void MyUdpConnection::OnError(const boost::system::error_code &error, boost::asio::ip::udp::endpoint remote_endpoint)
{
    std::clog << "[" << __FUNCTION__ << "] " << error << std::endl;
    std::clog << " error on endpoint " << remote_endpoint << std::endl;
}

void MyUdpConnection::OnTimer(const boost::posix_time::time_duration &delta)
{
    std::clog << "[" << __FUNCTION__ << "] " << delta << std::endl;
}

MyUdpConnection::~MyUdpConnection()
{
}
