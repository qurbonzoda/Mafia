//
// Created by qurbonzoda on 23.12.15.
//

#ifndef MAFIA_MYNETWORK_H
#define MAFIA_MYNETWORK_H

#include "network.h"

class Server;

class MyAcceptor : public Acceptor
{
private:
    bool onAccept(boost::shared_ptr<Connection> connection, const std::string &host, uint16_t port);
    void onTimer(const boost::posix_time::time_duration &delta);
    void onError(const boost::system::error_code &error);
    boost::shared_ptr<MyAcceptor> shared_from_this()
    {
        return boost::static_pointer_cast<MyAcceptor>(Acceptor::shared_from_this());
    }

public:
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
    std::vector<uint8_t> carry;
    void onAccept(const std::string &host, uint16_t port);
    void onSend(const std::vector<uint8_t> &buffer);
    void onReceive(std::vector<uint8_t> &buffer);
    void onTimer(const boost::posix_time::time_duration &delta);
    void onError(const boost::system::error_code &error);
    boost::shared_ptr<MyConnection> shared_from_this()
    {
        return boost::static_pointer_cast<MyConnection>(Connection::shared_from_this());
    }
public:
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


class MyUdpConnection : public UdpConnection
{
private:
    void onSend(const std::vector<uint8_t> &buffer, boost::asio::ip::udp::endpoint remoteEndpoint);
    void onReceive(std::vector<uint8_t> &buffer, boost::asio::ip::udp::endpoint remoteEndpoint);
    void onTimer(const boost::posix_time::time_duration &delta);
    void onError(const boost::system::error_code &error, boost::asio::ip::udp::endpoint remoteEndpoint);
    boost::shared_ptr<MyUdpConnection> shared_from_this()
    {
        return boost::static_pointer_cast<MyUdpConnection>(UdpConnection::shared_from_this());
    }

public:
    MyUdpConnection( boost::shared_ptr<Server> server, boost::shared_ptr< Hive > hive, const std::string & host, uint16_t port );
    ~MyUdpConnection();

public:
    const boost::shared_ptr<Server> &getServer() const
    {
        return server;
    }

private:
    boost::shared_ptr<Server> server;
};


#endif //MAFIA_MYNETWORK_H
