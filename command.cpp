//
// Created by qurbonzoda on 20.12.15.
//

#include "command.h"
#include "player_message.h"
#include "formatter.h"
#include "player.h"
#include "room.h"
#include "server.h"
#include "my_network.h"
#include "room_state.h"
#include "constants.h"

using boost::shared_ptr;

namespace command {

    void execute(boost::shared_ptr<MyConnection> const &connection, PlayerMessage const &message)
    {
        std::clog << "request from player " << message.getId() << " command " << message.getCommand() << std::endl;
        uint32_t senderId = message.getId();
        std::vector< std::string > messageParameters;
        for (auto param : message.getParams())
        {
            messageParameters.push_back(Formatter::stringOf(param));
        }
        shared_ptr<Server> server = connection->getServer();
        shared_ptr<Player> player = server->getPlayerById(senderId);

        switch (message.getCommand())
        {
            case AUTHORISATION:
            {
                std::string login = messageParameters[0];
                std::string password = messageParameters[1];

                authorizePlayer(player, login, password);
                break;
            }
            case REGISTRATION:
            {
                std::string login = messageParameters[0];
                std::string password = messageParameters[1];

                registerPlayer(player, login, password);
                break;
            }
            case NEW_ROOM:
            {
                size_t maxPlayers = std::stoul(messageParameters[0]);
                std::string password = messageParameters[1];

                createNewRoom(player, maxPlayers, password);
                break;
            }
            case ROOMS_LIST:
            {
                sendRoomList(player);
                break;
            }
            case ENTER_ROOM:
            {
                size_t roomId = std::stoul(messageParameters[0]);
                shared_ptr<Room> room = server->getRoomById(roomId);
                std::string password = messageParameters[1];

                enterRoom(player, room, password);
                break;
            }
            case ROOM_INFO:
            {
                size_t roomId = std::stoul(messageParameters[0]);
                shared_ptr<Room> room = server->getRoomById(roomId);
                std::string password = messageParameters[1];

                sendRoomInformation(player, room, password);
                break;
            }
            case LEAVE_ROOM:
            {
                leaveRoom(player);
                break;
            }
            case NEXT:
            {
                changeRoomState(player->getRoom());
                break;
            }
            case SELECT:
            {
                shared_ptr<Room> room = player->getRoom();
                size_t target = std::stoul(messageParameters[0]);

                handleSelectedTarget(room, target);
                break;
            }
            case CHAT_SEND:
            {
                sendToRoommates(player, messageParameters[0]);
                break;
            }
            case GET_SETTINGS:
            {
                sendCredentials(player);
                break;
            }
            case SAVE_SETTINGS:
            {
                saveCredentials(player, messageParameters);
                break;
            }
            default:
                break;
        }
    }

    void saveCredentials(shared_ptr<Player> const &player, std::vector< std::string > const &credential)
    {
        player->setCredential(credential);
    }

    void sendCredentials(shared_ptr<Player> const &player)
    {
        std::string message = std::to_string(OUT_SETTINGS) + " " + player->getCredentail();
        sendTo(player->getConnection(), message);
    }

    void authorizePlayer(shared_ptr <Player> const &player, std::string const &login, std::string const &password)
    {
        std::clog << "AUTHORIZATION" << std::endl;

        shared_ptr<Server> server = player->getServer();
        if (!(login == "guest" && password == "guest"))
        {
            if (!server->isRegistrated(login, password))
            {
                std::string message = std::to_string(FAILED) + " " + std::to_string(ErrorCode::WRONG_PASSWORD);
                sendTo(player->getConnection(), message);
                return;
            }
        }

        server->loadPlayer(player, login, password);
        sendRoomList(player);
    }

    void registerPlayer(shared_ptr <Player> const &player, std::string const &login, std::string const &password)
    {
        std::clog << "REGISTRATION" << std::endl;
        shared_ptr<Server> server = player->getServer();
        if (server->isBusyLogin(login) || login == "guest")
        {
            std::string message = std::to_string(FAILED) + " " + std::to_string(ErrorCode::LOGIN_IS_BUSY);
            sendTo(player->getConnection(), message);
            return;
        }

        player->setLogin(login);
        player->setPassword(password);
        server->addPlayerCredential(player);
        sendRoomList(player);
    }

    void createNewRoom(boost::shared_ptr<Player> const &player, size_t maxPlayers, std::string const &password)
    {
        std::clog << "NEW_ROOM" << std::endl;
        shared_ptr<Server> server = player->getServer();
        shared_ptr<Room> room = server->createNewRoomInstance();

        room->setMaximumPlayers(maxPlayers);
        room->setPassword(password);

        std::clog << (std::string)"maxPlayers in new room " + std::to_string(room->getId())
                     + " is " + std::to_string(room->getMaximumPlayers()) << std::endl;

        enterRoom(player, room, password);
    }

    void sendRoomList(shared_ptr<Player> const &player)
    {
        std::clog << "ROOM_LIST" << std::endl;
        auto rooms = player->getServer()->getRooms();

        std::string answer = std::to_string(command::Type::ROOMS_LIST) + " " + std::to_string(rooms.size());
        for (auto room : rooms)
        {
            answer += " " + std::to_string(room->getStatus()) + " " + std::to_string(room->isSafe()) + " "
                      + std::to_string(room->getNumberOfPlayers())
                      + " " + std::to_string(room->getMaximumPlayers()) + " " + std::to_string(room->getId());
        }

        sendTo(player->getConnection(), answer);
    }

    void enterRoom(boost::shared_ptr<Player> const &player, boost::shared_ptr<Room> const &room, std::string const &password)
    {
        std::clog << "ENTER_ROOM" << std::endl;
        shared_ptr<Server> server = player->getServer();

        if (room == nullptr) { return; };

        if (room->getStatus() == Room::Status::PLAYING)
        {
            sendRoomInformation(player, room, password);
            return;
        }

        if (room->getPassword() == password || !room->isSafe())
        {
            room->join(player);
        }
        server->updateRoomInfo(room);
        server->updateRoomList();

    }
    void sendRoomInformation(boost::shared_ptr<Player> const &player, boost::shared_ptr<Room> const &room,
                                 std::string const &password)
    {
        std::clog << "ROOM_INFO" << std::endl;
        shared_ptr<Server> server = player->getServer();

        //std::clog << "room use_count := " + std::to_string(room.use_count()) << std::endl;

        bool success = (room->getPlayers().find(player) != room->getPlayers().end())
                       && (room->getPassword() == password || !room->isSafe());

        std::string answer = std::to_string(command::Type::ROOM_INFO) + " " + std::to_string(success) + " "
                             + std::to_string(room->getNumberOfPlayers()) + " "
                             + std::to_string(player->getRoomPosition()) + " "
                             + std::to_string(room->getId()) + " " + room->getPositionMask();

        sendTo(player->getConnection(), answer);
        //std::clog << "room use_count := " + std::to_string(room.use_count()) << std::endl;
    }
    void leaveRoom(boost::shared_ptr<Player> const &player)
    {
        std::clog << "LEAVE_ROOM" << std::endl;
        auto server = player->getServer();
        auto room = player->getRoom();
        if (room == nullptr)
        {
            std::clog << "FATAL: Leaving nullptr room" << std::endl;
            return;
        }

        room->erasePlayer(player);
        player->setRoom(nullptr);

        if (room->getNumberOfPlayers() == 0)
        {
            server->deleteRoom(room);
            //std::clog << "use_count := " + std::to_string(room.use_count()) << std::endl;
        }
        else
        {
            server->updateRoomInfo(room);
        }
        server->updateRoomList();
        std::clog << "LEAVED_ROOM SUCCESS" << std::endl;
        //std::clog << "room use_count := " + std::to_string(room.use_count()) << std::endl;
    }
    void startGame(shared_ptr<Room> const &room)
    {
        std::clog << "StarT Game" << std::endl;
        for (auto player :room->getPlayers())
        {
            std::string answer = std::to_string(command::Type::START_GAME)
                                 + " " + std::to_string(player->getCharacter())
                                 + " " + std::to_string(player->getRoomPosition());

            sendTo(player->getConnection(), answer);

            answer = std::to_string(command::Type::START_SPEAKING) + " 0";
            sendTo(player->getConnection(), answer);

            sendRoomStateInformation(player);
        }
        auto server = room->getServer();
        server->updateRoomList();
    }
    void changeRoomState(boost::shared_ptr<Room> const &room)
    {
        std::clog << "NEXT" << std::endl;

        std::string prevStateName = room->getState()->getName();

        room->goToNextState();
        for (auto player : room->getPlayers())
        {
            sendRoomStateInformation(player);
        }
        if (room->getState()->getName().find("won_the_game") != std::string::npos)
        {
            endGame(room);
        }


        RoomState * state = room->getState();
        std::string stateName = state->getName();

        std::string first_message, second_message;

        if (stateName.find(SPEAKING) != std::string::npos
            || stateName.find(MURDERED_SPEAKS) != std::string::npos)
        {
            size_t from = stateName.find_last_of('_');

            assert(from != std::string::npos);

            size_t room_position = std::stoi(stateName.substr(from + 1));
            first_message = std::to_string(command::Type::STOP_SPEAKING) + " 0";
            second_message = std::to_string(command::Type::START_SPEAKING) + " " + std::to_string(room_position);
        }
        if (stateName.find(FINISHED_SPEAKING) != std::string::npos
            || prevStateName.find(MURDERED_SPEAKS) != std::string::npos)
        {
            if (prevStateName.find(MURDERED_SPEAKS) != std::string::npos)
            {
                stateName = prevStateName;
            }

            size_t from = stateName.find_last_of('_');

            assert(from != std::string::npos);

            size_t room_position = std::stoi(stateName.substr(from + 1));

            first_message = std::to_string(command::Type::STOP_SPEAKING) + " " + std::to_string(room_position);
            second_message = std::to_string(command::Type::START_SPEAKING) + " 0";
        }
        if (!first_message.empty())
        {
            for (auto player : room->getPlayers())
            {
                command::sendTo(player->getConnection(), first_message);
                command::sendTo(player->getConnection(), second_message);
            }
        }
    }
    void sendRoomStateInformation(boost::shared_ptr<Player> const &player)
    {
        std::clog << "GAME_INFO" << std::endl;
        auto room = player->getRoom();

        std::string answer = std::to_string(Type::GAME_INFO)
                             + " " + room->getState()->getPeriod()
                             + " " + room->getState()->getName()
                             + " " + room->getState()->getNext()->getName();

        sendTo(player->getConnection(), answer);
    }

    void handleSelectedTarget(boost::shared_ptr<Room> const &room, size_t target)
    {
        std::clog << "SELECT" << std::endl;

        if (room->getState()->getName().find(RoomState::RoomStateNames[2]) != std::string::npos)
        {
            room->nominateForVoting(target);
        }
        else if (room->getState()->getName().find(VOTING_AGAINST) != std::string::npos)
        {
            room->setNumberOfVotesAgainstNominatedPlayer(target);
        }
        else if (room->getState()->getName().find(MAFIA_MURDERED) != std::string::npos)
        {
            room->tryToMurderPlayer(target);
        }
        else if (room->getState()->getName().find(DOCTOR_CURING) != std::string::npos)
        {
            room->curePlayer(target);
        }
    }

    void endGame(shared_ptr<Room> const &room)
    {
        std::string message = std::to_string(command::END_GAME);
        for (auto player : room->getPlayers())
        {
            sendTo(player->getConnection(), message);
        }
    }

    bool sendToRoommates(boost::shared_ptr<Player> const &player, const std::string &quote)
    {
        std::clog << "CHAT_sendTo" << std::endl;

        std::string message = std::to_string(command::CHAT_MESSAGE)
                              + " " + std::to_string(player->getRoomPosition())
                              + " " + quote;

        shared_ptr<Room> room = player->getRoom();

        for (auto addressee : room->getPlayers())
        {
            sendTo(addressee->getConnection(), message);
        }
    }

    bool sendTo(shared_ptr<MyConnection> const &connection, std::string message)
    {
        std::vector<uint8_t> buffer = Formatter::vectorOf(Formatter::getMessageFormat(message));
        if (!(connection->getServer()->getPlayerByConnection(connection)->isBot()))
        {
            connection->send(buffer);
        }
    }
}