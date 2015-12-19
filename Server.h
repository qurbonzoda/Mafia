//
// Created by qurbonzoda on 04.12.15.
//

#ifndef MAFIA_SERVER_H
#define MAFIA_SERVER_H

#include "network.h"
#include "Room.h"
#include "Player.h"
#include "Formatter.h"

enum Command
{
    PLAYER_ID, PLAYERS_IN_ROOM, START_GAME, AUTHORISATION, NEW_ROOM, ENTER_ROOM, LEAVE_ROOM, ROOMS_LIST
};

class Server : public boost::enable_shared_from_this<Server>
{
public:
    Server(boost::shared_ptr< Hive > hive, std::string ip_address, uint16_t port);
    class MyAcceptor : public Acceptor
    {
    private:
        bool OnAccept( boost::shared_ptr< Connection > connection, const std::string & host, uint16_t port );
        void OnTimer( const boost::posix_time::time_duration & delta );
        void OnError( const boost::system::error_code & error );

    public:
        MyAcceptor( boost::shared_ptr< Hive > hive );
        ~MyAcceptor();
    };

    class MyConnection : public Connection
    {
    private:
        void OnAccept( const std::string & host, uint16_t port );
        void OnConnect( const std::string & host, uint16_t port );
        void OnSend( const std::vector< uint8_t > & buffer );
        void OnRecv( std::vector< uint8_t > & buffer );
        void OnTimer( const boost::posix_time::time_duration & delta );
        void OnError( const boost::system::error_code & error );
    public:
        MyConnection( boost::shared_ptr< Hive > hive );
        ~MyConnection();
    };
/*
    class MyUdpConnection : public UdpConnection
    {
    private:
        void OnRecv( const std::vector< uint8_t > & buffer, boost::asio::ip::udp::endpoint &remove_endpoint );
        void OnSend( const std::vector< uint8_t > & buffer, boost::asio::ip::udp::endpoint &remove_endpoint );

    public:
        MyUdpConnection(boost::shared_ptr<Hive> hive);
        ~MyUdpConnection();
    };
    */
private:
    size_t player_id_counter;
    boost::shared_ptr<Hive> hive;
    /*
    std::set< boost::shared_ptr<Room> > rooms;
    std::map<size_t, boost::asio::ip::tcp::endpoint> players;
     */
};

#endif //MAFIA_SERVER_H
