//
// Created by qurbonzoda on 23.11.15.
//

#include "network.h"
#include <boost/bind.hpp>
#include <boost/lexical_cast.hpp>
#include <boost/interprocess/detail/atomic.hpp>

#define detail ipcdetail

//-----------------------------------------------------------------------------

Hive::Hive()
        : workPointer_(new boost::asio::io_service::work(ioService_) ), shutdown_(0 )
{
}

Hive::~Hive()
{
}

boost::asio::io_service & Hive::getService()
{
    return ioService_;
}

bool Hive::hasStopped()
{
    return (boost::interprocess::detail::atomic_cas32(&shutdown_, 1, 1 ) == 1 );
}

void Hive::poll()
{
    ioService_.poll();
}

void Hive::run()
{
    ioService_.run();
}

void Hive::stop()
{
    if(boost::interprocess::detail::atomic_cas32(&shutdown_, 1, 0 ) == 0 )
    {
        workPointer_.reset();
        ioService_.run();
        ioService_.stop();
    }
}

void Hive::reset()
{
    if(boost::interprocess::detail::atomic_cas32(&shutdown_, 0, 1 ) == 1 )
    {
        ioService_.reset();
        workPointer_.reset(new boost::asio::io_service::work(ioService_) );
    }
}

//-----------------------------------------------------------------------------


Acceptor::Acceptor( boost::shared_ptr< Hive > hive )
        : hive_(hive ), acceptor_(hive->getService() ), ioStrand_(hive->getService() ), timer_(hive->getService() ), timerInterval_(5000 ), errorState_(0 )
{
}

Acceptor::~Acceptor()
{
}

void Acceptor::startTimer()
{
    lastTime_ = boost::posix_time::microsec_clock::local_time();
    timer_.expires_from_now(boost::posix_time::milliseconds(timerInterval_) );
    timer_.async_wait(ioStrand_.wrap(boost::bind(&Acceptor::handleTimer, shared_from_this(), _1) ) );
}

void Acceptor::startError(const boost::system::error_code &error)
{
    if(boost::interprocess::detail::atomic_cas32(&errorState_, 1, 0 ) == 0 )
    {
        boost::system::error_code ec;
        acceptor_.cancel(ec );
        acceptor_.close(ec );
        timer_.cancel(ec );
        onError(error);
    }
}

void Acceptor::dispatchAccept(boost::shared_ptr<Connection> connection)
{
    acceptor_.async_accept(connection->getSocket(), connection->getStrand().wrap(
            boost::bind(&Acceptor::handleAccept, shared_from_this(), _1, connection) ) );
}

void Acceptor::handleTimer(const boost::system::error_code &error)
{
    if(error || hasError() || hive_->hasStopped() )
    {
        startError(error);
    }
    else
    {
        onTimer(boost::posix_time::microsec_clock::local_time() - lastTime_);
        startTimer();
    }
}

void Acceptor::handleAccept(const boost::system::error_code &error, boost::shared_ptr<Connection> connection)
{
    if(error || hasError() || hive_->hasStopped() )
    {
        connection->startError(error);
    }
    else
    {
        if(connection->getSocket().is_open() )
        {
            //connection->StartTimer();
            if(onAccept(connection, connection->getSocket().remote_endpoint().address().to_string(),
                        connection->getSocket().remote_endpoint().port()) )
            {
                connection->onAccept(acceptor_.local_endpoint().address().to_string(),
                                     acceptor_.local_endpoint().port());
            }
        }
        else
        {
            startError(error);
        }
    }
}

void Acceptor::stop()
{
    ioStrand_.post(boost::bind(&Acceptor::handleTimer, shared_from_this(), boost::asio::error::connection_reset) );
}

void Acceptor::accept(boost::shared_ptr<Connection> connection)
{
    ioStrand_.post(boost::bind(&Acceptor::dispatchAccept, shared_from_this(), connection) );
}

void Acceptor::listen(const std::string &host, const uint16_t &port)
{
    boost::asio::ip::tcp::resolver resolver(hive_->getService() );
    boost::asio::ip::tcp::resolver::query query( host, boost::lexical_cast< std::string >( port ) );
    boost::asio::ip::tcp::endpoint endpoint = *resolver.resolve( query );
    acceptor_.open(endpoint.protocol() );
    acceptor_.set_option(boost::asio::ip::tcp::acceptor::reuse_address(false ) );
    acceptor_.bind(endpoint );
    acceptor_.listen(boost::asio::socket_base::max_connections );
    //StartTimer();
}

boost::shared_ptr< Hive > Acceptor::getHive()
{
    return hive_;
}

boost::asio::ip::tcp::acceptor & Acceptor::getAcceptor()
{
    return acceptor_;
}

int32_t Acceptor::getTimerInterval() const
{
    return timerInterval_;
}

void Acceptor::setTimerInterval(int32_t timer_interval)
{
    timerInterval_ = timer_interval;
}

bool Acceptor::hasError()
{
    return (boost::interprocess::detail::atomic_cas32(&errorState_, 1, 1 ) == 1 );
}

//-----------------------------------------------------------------------------

Connection::Connection( boost::shared_ptr< Hive > hive )
        : hive_(hive ), socket_(hive->getService() ), ioStrand_(hive->getService() ), timer_(hive->getService() ), receiveBufferSize_(4096 ), timerInterval_(5000 ), errorState_(0 )
{

}

Connection::~Connection()
{
}

void Connection::bind(const std::string &ip, uint16_t port)
{
    boost::asio::ip::tcp::endpoint endpoint( boost::asio::ip::address::from_string( ip ), port );
    socket_.open(endpoint.protocol() );
    socket_.set_option(boost::asio::ip::tcp::acceptor::reuse_address(false ) );
    socket_.bind(endpoint );
}

void Connection::startSend()
{
    if( !pendingSends_.empty() )
    {
        boost::asio::async_write(socket_, boost::asio::buffer(pendingSends_.front() ),
                                 ioStrand_.wrap(boost::bind(&Connection::handleSend, shared_from_this(),
                                                            boost::asio::placeholders::error,
                                                            pendingSends_.begin()) ) );
    }
}

void Connection::startReceive(int32_t totalBytes)
{
    if(totalBytes > 0 )
    {
        receiveBuffer_.resize(totalBytes);
        boost::asio::async_read(socket_, boost::asio::buffer(receiveBuffer_), ioStrand_.wrap(
                boost::bind(&Connection::handleReceive, shared_from_this(), _1, _2) ) );
    }
    else
    {
        receiveBuffer_.resize(receiveBufferSize_);
        socket_.async_read_some(boost::asio::buffer(receiveBuffer_), ioStrand_.wrap(
                boost::bind(&Connection::handleReceive, shared_from_this(), _1, _2) ) );
    }
}

void Connection::startTimer()
{
    lastTime_ = boost::posix_time::microsec_clock::local_time();
    timer_.expires_from_now(boost::posix_time::milliseconds(timerInterval_) );
    timer_.async_wait(ioStrand_.wrap(boost::bind(&Connection::dispatchTimer, shared_from_this(), _1) ) );
}

void Connection::startError(const boost::system::error_code &error)
{
    if(boost::interprocess::detail::atomic_cas32(&errorState_, 1, 0 ) == 0 )
    {
        boost::system::error_code ec;
        socket_.shutdown(boost::asio::ip::tcp::socket::shutdown_both, ec );
        socket_.close(ec );
        timer_.cancel(ec );
        onError(error);
    }
}

void Connection::handleSend(const boost::system::error_code &error, std::list<std::vector<uint8_t> >::iterator itr)
{
    if(error || hasError() || hive_->hasStopped() )
    {
        startError(error);
    }
    else
    {
        onSend(*itr);
        pendingSends_.erase(itr );
        startSend();
    }
}

void Connection::handleReceive(const boost::system::error_code &error, int32_t actualBytes)
{
    if(error || hasError() || hive_->hasStopped() )
    {
        startError(error);
    }
    else
    {
        receiveBuffer_.resize(actualBytes);
        onReceive(receiveBuffer_);
        pendingReceives_.pop_front();
        if( !pendingReceives_.empty() )
        {
            startReceive(pendingReceives_.front());
        }
    }
}

void Connection::handleTimer(const boost::system::error_code &error)
{
    if(error || hasError() || hive_->hasStopped() )
    {
        startError(error);
    }
    else
    {
        onTimer(boost::posix_time::microsec_clock::local_time() - lastTime_);
        startTimer();
    }
}

void Connection::dispatchSend(std::vector<uint8_t> buffer)
{
    bool should_start_send = pendingSends_.empty();
    pendingSends_.push_back(buffer );
    if( should_start_send )
    {
        startSend();
    }
}

void Connection::dispatchReceive(int32_t totalBytes)
{
    bool should_start_receive = pendingReceives_.empty();
    pendingReceives_.push_back(totalBytes);
    if( should_start_receive )
    {
        startReceive(totalBytes);
    }
}

void Connection::dispatchTimer(const boost::system::error_code &error)
{
    ioStrand_.post(boost::bind(&Connection::handleTimer, shared_from_this(), error) );
}

void Connection::disconnect()
{
    ioStrand_.post(boost::bind(&Connection::handleTimer, shared_from_this(), boost::asio::error::connection_reset) );
}

void Connection::receive(int32_t total_bytes)
{
    ioStrand_.post(boost::bind(&Connection::dispatchReceive, shared_from_this(), total_bytes) );
}

void Connection::send(const std::vector<uint8_t> &buffer)
{
    ioStrand_.post(boost::bind(&Connection::dispatchSend, shared_from_this(), buffer) );
}

boost::asio::ip::tcp::socket & Connection::getSocket()
{
    return socket_;
}

boost::asio::strand & Connection::getStrand()
{
    return ioStrand_;
}

boost::shared_ptr< Hive > Connection::getHive()
{
    return hive_;
}

void Connection::setReceiveBufferSize(int32_t size)
{
    receiveBufferSize_ = size;
}

int32_t Connection::getReceiveBufferSize() const
{
    return receiveBufferSize_;
}

int32_t Connection::getTimerInterval() const
{
    return timerInterval_;
}

void Connection::setTimerInterval(int32_t timerInterval)
{
    timerInterval_ = timerInterval;
}

bool Connection::hasError()
{
    return (boost::interprocess::detail::atomic_cas32(&errorState_, 1, 1 ) == 1 );
}


//-----------------------------------------------------------------------------

UdpConnection::UdpConnection(boost::shared_ptr<Hive> hive, std::string ipAddress, uint16_t port) :
        socket_(hive->getService(), boost::asio::ip::udp::endpoint(boost::asio::ip::address::from_string(ipAddress), port)), timer_(
        hive->getService())
{
    receiveBufferSize_ = 200000;
    timerInterval_ = 5000;
    errorState_ = 0;
    serverEndpoint_ = boost::asio::ip::udp::endpoint(boost::asio::ip::address::from_string(ipAddress), port);
    hive_ = hive;
    //StartTimer();
}
void UdpConnection::bind(std::string const &ipAddress, uint16_t port)
{
    serverEndpoint_ = boost::asio::ip::udp::endpoint(boost::asio::ip::address::from_string(ipAddress), port);
    socket_.open(serverEndpoint_.protocol() );
    socket_.bind(serverEndpoint_);
}

void UdpConnection::startReceive()
{
    receivedBuffer_.resize(receiveBufferSize_);
    socket_.async_receive_from(boost::asio::buffer(receivedBuffer_), remoteEndpoint_,
                               boost::bind(&UdpConnection::handleReceive, shared_from_this(), _1, _2));
}

void UdpConnection::handleReceive(const boost::system::error_code &error, int32_t actual_bytes)
{
    if (!error)
    {
        try {
            receivedBuffer_.resize(actual_bytes );
            onReceive(receivedBuffer_, remoteEndpoint_);
        }
        catch (std::exception ex) {
            std::clog << (std::string)"handle_receive: Error parsing incoming message:" + ex.what();
        }
        catch (...) {
            std::clog << "handle_receive: Unknown error while parsing incoming message" << std::endl;
        }
    }
    else
    {
        std::clog << (std::string)"handle_receive: error: " + error.message()
                     + " while receiving from address " << remoteEndpoint_ << std::endl;
        startError(error, remoteEndpoint_);
    }
    startReceive();
}
/*
handle_receive: error: Bad file descriptor while receiving from address 192.168.43.165:1010
handle_receive: error: Bad file descriptor while receiving from address 192.168.43.165:1010
*/
void UdpConnection::handleSend(const boost::system::error_code &error, const std::vector<uint8_t> &buffer,
                               boost::asio::ip::udp::endpoint remote_endpoint)
{
    if(error || hasError() || hive_->hasStopped() )
    {
        startError(error, remote_endpoint);
    }
    else
    {
        onSend(buffer, remote_endpoint);
    }
}

void UdpConnection::send(const std::vector<uint8_t> &buffer, boost::asio::ip::udp::endpoint remote_endpoint)
{
    if (remote_endpoint.address() == boost::asio::ip::address::from_string("192.168.2.10") || buffer.size() < 10)
    {
        return;
    }
    socket_.async_send_to(boost::asio::buffer(buffer), remote_endpoint,
                          boost::bind(&UdpConnection::handleSend, shared_from_this(), boost::asio::placeholders::error,
                                      buffer, remote_endpoint));
}

boost::asio::ip::udp::socket & UdpConnection::getSocket()
{
    return socket_;
}

boost::shared_ptr< Hive > UdpConnection::getHive()
{
    return hive_;
}

void UdpConnection::setReceiveBufferSize(int32_t size)
{
    receiveBufferSize_ = size;
}

int32_t UdpConnection::setReceiveBufferSize() const
{
    return receiveBufferSize_;
}

int32_t UdpConnection::getTimerInterval() const
{
    return timerInterval_;
}

void UdpConnection::setTimerInterval(int32_t timer_interval)
{
    timerInterval_ = timer_interval;
}

bool UdpConnection::hasError()
{
    return (boost::interprocess::detail::atomic_cas32(&errorState_, 1, 1 ) == 1 );
}

void UdpConnection::startError(const boost::system::error_code &error, boost::asio::ip::udp::endpoint remoteEndpoint)
{
    if(boost::interprocess::detail::atomic_cas32(&errorState_, 1, 0 ) == 0 )
    {
        onError(error, remoteEndpoint);
    }

}

void UdpConnection::startTimer()
{
    lastTime_ = boost::posix_time::microsec_clock::local_time();
    timer_.expires_from_now(boost::posix_time::milliseconds(timerInterval_) );
    timer_.async_wait(boost::bind(&UdpConnection::handleTimer, shared_from_this(), _1));
}


void UdpConnection::handleTimer(const boost::system::error_code &error)
{
    if(error || hasError() || hive_->hasStopped() )
    {

    }
    else
    {
        onTimer(boost::posix_time::microsec_clock::local_time() - lastTime_);
        startTimer();
    }
}

UdpConnection::~UdpConnection()
{
}
