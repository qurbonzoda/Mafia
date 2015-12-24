#include "network.h"
#include <boost/thread/mutex.hpp>
#include <boost/thread.hpp>
#include "Server.h"

boost::mutex global_stream_lock;

void new_thread_handler(boost::shared_ptr< Hive > hive)
{
    std::clog << "[" << boost::this_thread::get_id() << "] " << "Thread started!" << std::endl;
    hive->Run();
    std::clog << "[" << boost::this_thread::get_id() << "] " << "Thread finished!" << std::endl;
}

int main( int argc, char * argv[] )
{
    boost::shared_ptr< Hive > hive( new Hive() );
    boost::shared_ptr< Server > server( Server::newInstance(hive, "192.168.1.3", 7779) );


    boost::thread_group threads;
    threads.create_thread( boost::bind(&new_thread_handler, hive) );

    std::cin.get();

    hive->Stop();

    std::cin.get();

    return 0;
}