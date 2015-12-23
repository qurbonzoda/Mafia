//
// Created by qurbonzoda on 04.12.15.
//

#include <boost/thread/pthread/mutex.hpp>
#include "Server.h"
#include "PlayerMessage.h"
#include "Command.h"
#include "MyNetwork.h"

using namespace boost::asio::ip;

volatile uint32_t Server::player_id_counter = 0;
std::map<size_t, boost::shared_ptr<Player> > Server::players;
std::set< boost::shared_ptr<Room> > Server::rooms;
volatile uint32_t Server::room_id_counter = 0;


boost::shared_ptr<Server> Server::newInstance(boost::shared_ptr< Hive > hive, std::string ip_address, uint16_t port)
{
    boost::shared_ptr<Server> newServer(new Server(hive));
    boost::shared_ptr< MyAcceptor > acceptor( new MyAcceptor( newServer, hive ) );
    acceptor->Listen( ip_address, port );

    boost::shared_ptr< MyConnection > connection( new MyConnection( newServer, hive ) );
    acceptor->Accept( connection );

    assert(newServer != nullptr);
    assert(newServer.get() != nullptr);
    assert(newServer.get() != NULL);
/*
    boost::shared_ptr< MyUdpConnection > udp_connection ( new MyUdpConnection( hive ) );
    udp_connection->Bind(ip_address, port);
    udp_connection->start_receive();
*/
    return newServer;
}
Server::Server(boost::shared_ptr< Hive > hive) : hive(hive)
{
}
/*
void Server::MyConnection::OnAccept( const std::string & host, uint16_t port )
{
    std::clog << "[" << __FUNCTION__ << "] " << host << ":" << port << std::endl;
    /// id must be sent

    boost::shared_ptr<udp::endpoint> endpoint(new udp::endpoint(address::from_string( GetSocket().remote_endpoint().address().to_string() ), GetSocket().remote_endpoint().port()));

    boost::shared_ptr<Player> this_player = get_or_create_player(endpoint);
    this_player->setConnection(shared_from_this());
    uint32_t id = getId_by_player(this_player);
    std::string message = Formatter::getMessageFormat(Command::Type::PLAYER_ID, id);
    std::clog << "new player id " << id << std::endl;

    Send(Formatter::getVectorOf(message));
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
        /*
        if( ( x + 1 ) % 16 == 0 )
        {
            std::clog << std::endl;
        }
         *
    }
    std::clog << std::endl;
    std::clog << Formatter::getStringOf(buffer) << std::endl;
}

void Server::MyConnection::OnRecv( std::vector< uint8_t > & buffer )
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
        *
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

void Server::MyConnection::OnTimer( const boost::posix_time::time_duration & delta )
{
    std::clog << "[" << __FUNCTION__ << "] " << delta << std::endl;
}

void Server::MyConnection::OnError( const boost::system::error_code & error )
{
    auto server = this->getServer();
    auto player = server->getPlayer_by_connection(boost::static_pointer_cast<MyConnection>(shared_from_this()));
    // TO BE CONTINUED !!!

    std::clog << "[" << __FUNCTION__ << "] " << error << std::endl;
}

Server::MyConnection::MyConnection( boost::shared_ptr< Hive > hive )
        : Connection( hive )
{
}

Server::MyConnection::MyConnection(boost::shared_ptr<Server> server, boost::shared_ptr<Hive> hive)
        : Connection( hive ), server(server)
{

}


Server::MyConnection::~MyConnection()
{
}
bool Server::MyAcceptor::OnAccept( boost::shared_ptr< Connection > connection, const std::string & host, uint16_t port )
{
    boost::shared_ptr< MyConnection > new_connection( new MyConnection( this->getServer(), GetHive() ) );
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

Server::MyAcceptor::MyAcceptor(boost::shared_ptr<Server> server, boost::shared_ptr<Hive> hive)
        : Acceptor( hive ), server(server)
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
boost::shared_ptr<Player> Server::get_or_create_player(boost::shared_ptr<udp::endpoint> endpoint)
{
    for (auto &player : players)
    {
        if (player.second->getEndpoint() == endpoint)
        {
            return player.second;
        }
    }
    auto player = create_new_player_instance();
    player->setEndpoint(endpoint);
    return player;
}

boost::shared_ptr<Player> Server::create_new_player_instance()
{
    std::clog << "create_new_player_instance: player_id_counter = " << player_id_counter << std::endl;
    boost::shared_ptr<Player> player(new Player(++player_id_counter));
    players[player_id_counter] = player;
    return player;
}

boost::shared_ptr<Room> Server::create_new_room_instance()
{
    boost::shared_ptr<Room> room(new Room(++room_id_counter));
    rooms.insert(room);
    std::clog << "create_new_player_instance: room_id_counter = " << room_id_counter << std::endl;
    return room;
}

uint32_t Server::getId_by_player(boost::shared_ptr<Player> player)
{
    for (auto aPlayer : players)
    {
        if (aPlayer.second == player) {
            return aPlayer.second->getId();
        }
    }
}

boost::shared_ptr<Room> Server::getRoom_by_id(uint32_t id)
{
    for (auto room : rooms)
    {
        if (room->getId() == id)
        {
            return room;
        }
    }
    std::clog << id << " FATAL No such room id" << std::endl;
}

void Server::room_erase(boost::shared_ptr<Room> room)
{
    rooms.erase(room);
}

void Server::update_room_list()
{
    std::clog << "[ " << __FUNCTION__ << " ]" << std::endl;
    for (auto player : players)
    {
        if (player.second->getRoom() == nullptr)
        {
            Command::room_list(player.second->getConnection());
        }
    }
}

boost::shared_ptr<Player> Server::getPlayer_by_connection(boost::shared_ptr<Connection> connection)
{
    std::clog << "[ " << __FUNCTION__ << " ]" << std::endl;
    for (auto player : players)
    {
        if (player.second->getConnection() == connection)
        {
            return player.second;
        }
    }
}

void Server::update_room_info(boost::shared_ptr<Room> room)
{
    std::clog << "[ " << __FUNCTION__ << " ]" << std::endl;
    PlayerMessage message;
    message.setLen(25);
    message.setCommand(Command::Type::ROOM_INFO);
    std::vector< std::vector<uint8_t > >param;
    param.push_back( Formatter::getVectorOf(std::to_string(room->getId())) );
    param.push_back( Formatter::getVectorOf(room->getPassword()) );
    message.setParams(param);
    for (auto player : room->getPlayers())
    {
        message.setId(player->getId());
        Command::room_info(player->getConnection(), message);
    }
}
