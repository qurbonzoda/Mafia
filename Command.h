//
// Created by qurbonzoda on 20.12.15.
//

#ifndef MAFIA_COMMAND_H
#define MAFIA_COMMAND_H

#include <boost/shared_ptr.hpp>

class PlayerMessage;
class MyConnection;
class Room;

namespace Command
{
    enum Type
    {
        PLAYER_ID, ROOM_INFO, START_GAME, AUTHORISATION, NEW_ROOM, ENTER_ROOM, LEAVE_ROOM,
        ROOMS_LIST, NEXT, GAME_INFO, END_GAME, SELECT
    };
    void execute(boost::shared_ptr<MyConnection> const &connection, PlayerMessage const &message);
    void authorization(boost::shared_ptr<MyConnection> const & connection, PlayerMessage const & message);
    void new_room(boost::shared_ptr<MyConnection> const & connection, PlayerMessage const & message);
    void room_list(boost::shared_ptr<MyConnection> const & connection);
    void room_list(boost::shared_ptr<MyConnection> const & connection, PlayerMessage const & message);
    void enter_room(boost::shared_ptr<MyConnection> const & connection, PlayerMessage const & message);
    void room_info(boost::shared_ptr<MyConnection> const & connection, PlayerMessage const & message);
    void leave_room(boost::shared_ptr<MyConnection> const & connection, PlayerMessage const & message);
    void start_game(boost::shared_ptr<Room> const & room);
    bool sendTo(boost::shared_ptr<MyConnection> const & connection, std::vector<uint8_t> const & buffer);
    void next(boost::shared_ptr<MyConnection> const & connection, PlayerMessage const & message);
    void game_info(boost::shared_ptr<MyConnection> const & connection);
    void select(boost::shared_ptr<MyConnection> const & connection, PlayerMessage const & message);
};
#endif //MAFIA_COMMAND_H
