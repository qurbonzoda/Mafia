//
// Created by qurbonzoda on 20.12.15.
//

#ifndef MAFIA_COMMAND_H
#define MAFIA_COMMAND_H

#include <boost/shared_ptr.hpp>

class PlayerMessage;
class MyConnection;
class Room;

namespace command
{
    enum Type
    {
        PLAYER_ID, ROOM_INFO, START_GAME, AUTHORISATION, NEW_ROOM, ENTER_ROOM, LEAVE_ROOM,
        ROOMS_LIST, NEXT, GAME_INFO, END_GAME, SELECT, START_SPEAKING, STOP_SPEAKING
    };
    void execute(boost::shared_ptr<MyConnection> const &connection, PlayerMessage const &message);
    void authorizePlayer(boost::shared_ptr<MyConnection> const &connection, PlayerMessage const &message);
    void createNewRoom(boost::shared_ptr<MyConnection> const &connection, PlayerMessage const &message);
    void sendRoomList(boost::shared_ptr<MyConnection> const &connection);
    void enterRoom(boost::shared_ptr<MyConnection> const &connection, PlayerMessage const &message);
    void sendRoomInformation(boost::shared_ptr<MyConnection> const &connection, PlayerMessage const &message);
    void leaveRoom(boost::shared_ptr<MyConnection> const &connection, PlayerMessage const &message);
    void startGame(boost::shared_ptr<Room> const &room);
    bool sendTo(boost::shared_ptr<MyConnection> const & connection, std::vector<uint8_t> const & buffer);
    void changeRoomState(boost::shared_ptr<MyConnection> const &connection, PlayerMessage const &message);
    void sendGameInformation(boost::shared_ptr<MyConnection> const &connection);
    void handleSelectedTarget(boost::shared_ptr<MyConnection> const &connection, PlayerMessage const &message);
    void endGame(boost::shared_ptr<Room> const &room);
};
#endif //MAFIA_COMMAND_H
