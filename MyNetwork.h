//
// Created by qurbonzoda on 23.12.15.
//

#ifndef MAFIA_MYNETWORK_H
#define MAFIA_MYNETWORK_H

#include "Server.h"
#include "network.h"

class MyAcceptor : public Acceptor
{
private:
    bool OnAccept( boost::shared_ptr< Connection > connection, const std::string & host, uint16_t port );
    void OnTimer( const boost::posix_time::time_duration & delta );
    void OnError( const boost::system::error_code & error );

public:
    MyAcceptor( boost::shared_ptr< Hive > hive );
    MyAcceptor( boost::shared_ptr<Server> server, boost::shared_ptr< Hive > hive );
    ~MyAcceptor();

public:
    const boost::shared_ptr<Server> &getServer() const
    {
        return server;
    }

private:
    boost::shared_ptr<Server> server;
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
    MyConnection( boost::shared_ptr<Server> server, boost::shared_ptr< Hive > hive );
    ~MyConnection();

public:
    const boost::shared_ptr<Server> &getServer() const
    {
        return server;
    }

private:
    boost::shared_ptr<Server> server;
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

#endif //MAFIA_MYNETWORK_H
