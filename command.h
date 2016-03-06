//
// Created by qurbonzoda on 20.12.15.
//

#ifndef MAFIA_COMMAND_H
#define MAFIA_COMMAND_H

#include <boost/shared_ptr.hpp>

class PlayerMessage;
class MyConnection;
class Room;
class Player;

namespace command
{
    enum Type
    {
        PLAYER_ID, ROOM_INFO, START_GAME, AUTHORISATION, NEW_ROOM, ENTER_ROOM, LEAVE_ROOM,
        ROOMS_LIST, NEXT, GAME_INFO, END_GAME, SELECT, START_SPEAKING, STOP_SPEAKING, CHAT_SEND, CHAT_MESSAGE,
        FAILED, REGISTRATION, GET_SETTINGS, OUT_SETTINGS, SAVE_SETTINGS
    };

    enum ErrorCode
    {
        LOGIN_IS_BUSY, WRONG_PASSWORD
    };
    void execute(boost::shared_ptr<MyConnection> const &connection, PlayerMessage const &message);
    void authorizePlayer(boost::shared_ptr <Player> const &player, std::string const &login, std::string const &password);
    void registerPlayer(boost::shared_ptr <Player> const &player, std::string const &login, std::string const &password);
    void createNewRoom(boost::shared_ptr<Player> const &player, size_t maxPlayers, std::string const &password);
    void sendRoomList(boost::shared_ptr<Player> const &player);
    void enterRoom(boost::shared_ptr<Player> const &player, boost::shared_ptr<Room> const &room, std::string const &password);
    void sendRoomInformation(boost::shared_ptr<Player> const &player, boost::shared_ptr<Room> const &room,
                                 std::string const &password);
    void leaveRoom(boost::shared_ptr<Player> const &player);
    void startGame(boost::shared_ptr<Room> const &room);
    bool sendTo(boost::shared_ptr<MyConnection> const &connection, std::string message);
    bool sendToRoommates(boost::shared_ptr<Player> const &player, const std::string &quote);
    void changeRoomState(boost::shared_ptr<Room> const &room);
    void sendRoomStateInformation(boost::shared_ptr<Player> const &player);
    void handleSelectedTarget(boost::shared_ptr<Room> const &room, size_t target);
    void endGame(boost::shared_ptr<Room> const &room);
    void sendCredentials(boost::shared_ptr<Player> const &player);
    void saveCredentials(boost::shared_ptr<Player> const &player, std::vector< std::string > const &credential);

};
#endif //MAFIA_COMMAND_H
