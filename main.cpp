#include <boost/thread/mutex.hpp>
#include <boost/thread.hpp>
#include "network.h"

#include "server.h"

void new_thread_handler(boost::shared_ptr< Hive > hive)
{
    std::clog << "[" << boost::this_thread::get_id() << "] " << "Thread started!" << std::endl;
    hive->run();
    std::clog << "[" << boost::this_thread::get_id() << "] " << "Thread finished!" << std::endl;
}

int main( int argc, char * argv[] )
{
    boost::shared_ptr< Hive > hive( new Hive() );
    boost::shared_ptr< Server > server(new Server(hive, "192.168.43.116", 7777 ));
    server->start();


    boost::thread_group threads;
    for (int i = 0; i < 3; ++i)
    {
        threads.create_thread( boost::bind(&new_thread_handler, hive) );
    }

    std::cin.get();


    hive->stop();

    std::cin.get();
    threads.join_all();


    return 0;
}