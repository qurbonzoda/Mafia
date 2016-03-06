//
// Created by qurbonzoda on 23.11.15.
//

#ifndef NETWORK_NETWORK_H
#define NETWORK_NETWORK_H

#include <boost/asio.hpp>
#include <boost/date_time/posix_time/posix_time.hpp>
#include <boost/enable_shared_from_this.hpp>
#include <boost/shared_ptr.hpp>
#include <string>
#include <vector>
#include <list>
#include <boost/cstdint.hpp>

//-----------------------------------------------------------------------------
class Hive;

class Acceptor;

class Connection;

class UdpConnection;

//-----------------------------------------------------------------------------


class UdpConnection : public boost::enable_shared_from_this<UdpConnection>
{
private:
    boost::shared_ptr<Hive> hive_;
    boost::asio::ip::udp::socket socket_;
    boost::asio::ip::udp::endpoint serverEndpoint_;
    boost::asio::ip::udp::endpoint remoteEndpoint_;
    boost::asio::deadline_timer timer_;
    boost::posix_time::ptime lastTime_;
    std::vector<uint8_t> receivedBuffer_;
    std::list<int32_t> pendingReceives_;
    std::list<std::vector<uint8_t> > pendingSends_;
    int32_t receiveBufferSize_;
    int32_t timerInterval_;
    volatile uint32_t errorState_;
protected:
    UdpConnection(boost::shared_ptr<Hive> hive, std::string ipAddress, uint16_t port);

    virtual ~UdpConnection();

private:

    void handleSend(const boost::system::error_code &error, const std::vector<uint8_t> &buffer,
                    boost::asio::ip::udp::endpoint remote_endpoint);

    void handleReceive(const boost::system::error_code &error, int32_t actual_bytes);

    void handleTimer(const boost::system::error_code &error);

    // Called when data has been sent by the connection.
    virtual void onSend(const std::vector<uint8_t> &buffer, boost::asio::ip::udp::endpoint remote_endpoint) = 0;

    // Called when data has been received by the connection.
    virtual void onReceive(std::vector<uint8_t> &buffer, boost::asio::ip::udp::endpoint remote_endpoint) = 0;

    // Called on each timer event.
    virtual void onTimer(const boost::posix_time::time_duration &delta) = 0;

    // Called when an error is encountered.
    virtual void onError(const boost::system::error_code &error, boost::asio::ip::udp::endpoint remote_endpoint) = 0;

public:

    // Returns the Hive object.
    boost::shared_ptr<Hive> getHive();

    // Returns the socket object.
    boost::asio::ip::udp::socket &getSocket();

    // Sets the application specific receive buffer size used. For stream
    // based protocols such as HTTP, you want this to be pretty large, like
    // 64kb. For packet based protocols, then it will be much smaller,
    // usually 512b - 8kb depending on the protocol. The default value is
    // 4kb.
    void setReceiveBufferSize(int32_t size);

    // Returns the size of the receive buffer size of the current object.
    int32_t setReceiveBufferSize() const;

    // Sets the timer interval of the object. The interval is changed after
    // the next update is called.
    void setTimerInterval(int32_t timer_interval_ms);

    // Returns the timer interval of the object.
    int32_t getTimerInterval() const;

    // Returns true if this object has an error associated with it.
    bool hasError();

    // Binds the socket to the specified interface.
    void bind(const std::string &ipAddress, uint16_t port);

    void send(const std::vector<uint8_t> &buffer, boost::asio::ip::udp::endpoint remote_endpoint);

    void startReceive();

    void startError(const boost::system::error_code &error, boost::asio::ip::udp::endpoint remoteEndpoint);

    void startTimer();

    // Posts an asynchronous disconnect event for the object to process.
    void disconnect();
};

//-----------------------------------------------------------------------------

class Connection : public boost::enable_shared_from_this<Connection>
{
    friend class Acceptor;

    friend class Hive;

private:
    boost::shared_ptr<Hive> hive_;
    boost::asio::ip::tcp::socket socket_;
    boost::asio::strand ioStrand_;
    boost::asio::deadline_timer timer_;
    boost::posix_time::ptime lastTime_;
    std::vector<uint8_t> receiveBuffer_;
    std::list<int32_t> pendingReceives_;
    std::list<std::vector<uint8_t> > pendingSends_;
    int32_t receiveBufferSize_;
    int32_t timerInterval_;
    volatile uint32_t errorState_;

protected:
    Connection(boost::shared_ptr<Hive> hive);

    virtual ~Connection();

private:
    Connection(const Connection &rhs) = delete;

    Connection &operator=(const Connection &rhs) = delete;

    void startSend();

    void startReceive(int32_t totalBytes);

    void startError(const boost::system::error_code &error);

    void dispatchSend(std::vector<uint8_t> buffer);

    void dispatchReceive(int32_t totalBytes);

    void dispatchTimer(const boost::system::error_code &error);

    void handleSend(const boost::system::error_code &error, std::list<std::vector<uint8_t> >::iterator itr);

    void handleReceive(const boost::system::error_code &error, int32_t actualBytes);

    void handleTimer(const boost::system::error_code &error);

private:
    // Called when the connection has successfully connected to the local
    // host.
    virtual void onAccept(const std::string &host, uint16_t port) = 0;

    // Called when data has been sent by the connection.
    virtual void onSend(const std::vector<uint8_t> &buffer) = 0;

    // Called when data has been received by the connection.
    virtual void onReceive(std::vector<uint8_t> &buffer) = 0;

    // Called on each timer event.
    virtual void onTimer(const boost::posix_time::time_duration &delta) = 0;

    // Called when an error is encountered.
    virtual void onError(const boost::system::error_code &error) = 0;

public:

    void startTimer();

    // Returns the Hive object.
    boost::shared_ptr<Hive> getHive();

    // Returns the socket object.
    boost::asio::ip::tcp::socket &getSocket();

    // Returns the strand object.
    boost::asio::strand &getStrand();

    // Sets the application specific receive buffer size used. For stream
    // based protocols such as HTTP, you want this to be pretty large, like
    // 64kb. For packet based protocols, then it will be much smaller,
    // usually 512b - 8kb depending on the protocol. The default value is
    // 4kb.
    void setReceiveBufferSize(int32_t size);

    // Returns the size of the receive buffer size of the current object.
    int32_t getReceiveBufferSize() const;

    // Sets the timer interval of the object. The interval is changed after
    // the next update is called.
    void setTimerInterval(int32_t timerInterval);

    // Returns the timer interval of the object.
    int32_t getTimerInterval() const;

    // Returns true if this object has an error associated with it.
    bool hasError();

    // Binds the socket to the specified interface.
    void bind(const std::string &ip, uint16_t port);


    // Posts data to be sent to the connection.
    void send(const std::vector<uint8_t> &buffer);

    // Posts a recv for the connection to process. If total_bytes is 0, then
    // as many bytes as possible up to GetReceiveBufferSize() will be
    // waited for. If Recv is not 0, then the connection will wait for exactly
    // total_bytes before invoking OnRecv.
    void receive(int32_t total_bytes = 0);

    // Posts an asynchronous disconnect event for the object to process.
    void disconnect();
};

//-----------------------------------------------------------------------------

class Acceptor : public boost::enable_shared_from_this<Acceptor>
{
    friend class Hive;

private:
    boost::shared_ptr<Hive> hive_;
    boost::asio::ip::tcp::acceptor acceptor_;
    boost::asio::strand ioStrand_;
    boost::asio::deadline_timer timer_;
    boost::posix_time::ptime lastTime_;
    int32_t timerInterval_;
    volatile uint32_t errorState_;

private:
    Acceptor(const Acceptor &rhs);

    Acceptor &operator=(const Acceptor &rhs);

    void startError(const boost::system::error_code &error);

    void dispatchAccept(boost::shared_ptr<Connection> connection);

    void handleTimer(const boost::system::error_code &error);

    void handleAccept(const boost::system::error_code &error, boost::shared_ptr<Connection> connection);

protected:
    Acceptor(boost::shared_ptr<Hive> hive);

    virtual ~Acceptor();

private:
    // Called when a connection has connected to the server. This function
    // should return true to invoke the connection's OnAccept function if the
    // connection will be kept. If the connection will not be kept, the
    // connection's Disconnect function should be called and the function
    // should return false.
    virtual bool onAccept(boost::shared_ptr<Connection> connection, const std::string &host, uint16_t port) = 0;

    // Called on each timer event.
    virtual void onTimer(const boost::posix_time::time_duration &delta) = 0;

    // Called when an error is encountered. Most typically, this is when the
    // acceptor is being closed via the Stop function or if the Listen is
    // called on an address that is not available.
    virtual void onError(const boost::system::error_code &error) = 0;

public:
    void startTimer();

    // Returns the Hive object.
    boost::shared_ptr<Hive> getHive();

    // Returns the acceptor object.
    boost::asio::ip::tcp::acceptor &getAcceptor();

    // Returns the strand object.
    boost::asio::strand &getStrand();

    // Sets the timer interval of the object. The interval is changed after
    // the next update is called. The default value is 1000 ms.
    void setTimerInterval(int32_t timer_interval_ms);

    // Returns the timer interval of the object.
    int32_t getTimerInterval() const;

    // Returns true if this object has an error associated with it.
    bool hasError();

public:
    // Begin listening on the specific network interface.
    void listen(const std::string &host, const uint16_t &port);

    // Posts the connection to the listening interface. The next client that
    // connections will be given this connection. If multiple calls to Accept
    // are called at a time, then they are accepted in a FIFO order.
    void accept(boost::shared_ptr<Connection> connection);

    // Stop the Acceptor from listening.
    void stop();
};

//-----------------------------------------------------------------------------


class Hive : public boost::enable_shared_from_this<Hive>
{
private:
    boost::asio::io_service ioService_;
    boost::shared_ptr<boost::asio::io_service::work> workPointer_;
    volatile uint32_t shutdown_;

private:
    Hive(const Hive &rhs);

    Hive &operator=(const Hive &rhs);

public:
    Hive();

    virtual ~Hive();

    // Returns the io_service of this object.
    boost::asio::io_service &getService();

    // Returns true if the Stop function has been called.
    bool hasStopped();

    // Polls the networking subsystem once from the current thread and
    // returns.
    void poll();

    // Runs the networking system on the current thread. This function blocks
    // until the networking system is stopped, so do not call on a single
    // threaded application with no other means of being able to call Stop
    // unless you code in such logic.
    void run();

    // Stops the networking system. All work is finished and no more
    // networking interactions will be possible afterwards until Reset is called.
    void stop();

    // Restarts the networking system after Stop as been called. A new work
    // object is created and the shutdown flag is cleared.
    void reset();
};

#endif //NETWORK_NETWORK_H
