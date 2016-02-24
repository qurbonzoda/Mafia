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

namespace command {

    void execute(boost::shared_ptr<MyConnection> const & connection, PlayerMessage const &message)
    {
        std::clog << "request from player " << message.getId() << " command " << message.getCommand() << std::endl;
        uint32_t sender_id = message.getId();
        switch (message.getCommand())
        {
            case AUTHORISATION:
                authorizePlayer(connection, message);
                break;
            case NEW_ROOM:
                createNewRoom(connection, message);
                break;
            case ROOMS_LIST:
                sendRoomList(connection);
                break;
            case ENTER_ROOM:
                enterRoom(connection, message);
                break;
            case ROOM_INFO:
                sendRoomInformation(connection, message);
                break;
            case LEAVE_ROOM:
                leaveRoom(connection, message);
                break;
            case NEXT:
                changeRoomState(connection, message);
                break;
            case SELECT:
                handleSelectedTarget(connection, message);
                break;
            default:
                break;
        }
    }

    void authorizePlayer(boost::shared_ptr<MyConnection> const &connection, PlayerMessage const &message)
    {
        std::clog << "AUTHORIZATION" << std::endl;
        boost::shared_ptr<Player> player = connection->getServer()->getPlayerById(message.getId());
        player->setLogin(Formatter::stringOf(message.getParams()[0]));
        player->setPassword(Formatter::stringOf(message.getParams()[1]));
        sendRoomList(connection);
    }

    void createNewRoom(boost::shared_ptr<MyConnection> const &connection, PlayerMessage const &message)
    {
        std::clog << "NEW_ROOM" << std::endl;
        auto server = connection->getServer();
        boost::shared_ptr<Player> player = server->getPlayerById(message.getId());
        boost::shared_ptr<Room> room = server->createNewRoomInstance();

        //std::clog << "room use_count := " + std::to_string(room.use_count()) << std::endl;
        room->setMaximumPlayers(std::stoi(Formatter::stringOf(message.getParams()[0])));
        std::clog << (std::string)"maxPlayers in new room " + std::to_string(room->getId())
                     + " is " + std::to_string(room->getMaximumPlayers()) << std::endl;

        room->setPassword(Formatter::stringOf(message.getParams()[1]));
        PlayerMessage copy_message(message);
        copy_message.setParam(0, Formatter::vectorOf(std::to_string(room->getId())));

        std::clog << std::to_string(room->getId()) + " == " + Formatter::stringOf(copy_message.getParams()[0]) << std::endl;

        enterRoom(connection, copy_message);
    }

    void sendRoomList(boost::shared_ptr<MyConnection> const &connection)
    {
        std::clog << "ROOM_LIST" << std::endl;
        auto rooms = connection->getServer()->getRooms();
        std::string answer = std::to_string(command::Type::ROOMS_LIST) + " " + std::to_string(rooms.size());
        for (auto room : rooms)
        {
            answer += " " + std::to_string(room->getStatus()) + " " + std::to_string(room->isSafe()) + " "
                      + std::to_string(room->getNumberOfPlayers())
                      + " " + std::to_string(room->getMaximumPlayers()) + " " + std::to_string(room->getId());
        }
        answer = Formatter::getMessageFormat(answer);
        sendTo(connection, Formatter::vectorOf(answer));
    }

    void enterRoom(boost::shared_ptr<MyConnection> const &connection, PlayerMessage const &message)
    {
        std::clog << "ENTER_ROOM" << std::endl;
        boost::shared_ptr<Server> server = connection->getServer();
        uint32_t room_id = std::stoi(Formatter::stringOf(message.getParams()[0]));
        std::string room_password = Formatter::stringOf(message.getParams()[1]);
        boost::shared_ptr<Room> room = server->getRoomById(room_id);

        if (room == nullptr) { return; };

        if (room->getStatus() == Room::Status::PLAYING)
        {
            sendRoomInformation(connection, message);
            return;
        }

        if (room->getPassword() == room_password || !room->isSafe())
        {
            room->join(server->getPlayerById(message.getId()));
        }
        server->updateRoomInfo(room);
        server->updateRoomList();

    }
    void sendRoomInformation(boost::shared_ptr<MyConnection> const &connection, PlayerMessage const &message)
    {
        std::clog << "ROOM_INFO" << std::endl;
        boost::shared_ptr<Server> server = connection->getServer();
        uint32_t room_id = std::stoi(Formatter::stringOf(message.getParams()[0]));
        std::string room_password = Formatter::stringOf(message.getParams()[1]);
        auto room = server->getRoomById(room_id);
        auto player = server->getPlayerById(message.getId());

        //std::clog << "room use_count := " + std::to_string(room.use_count()) << std::endl;

        bool success = (room->getPlayers().find(player) != room->getPlayers().end())
                       && (room->getPassword() == room_password || !room->isSafe());

        std::clog << "room_id = "<< room_id << " room_password = " << room_password << " player_id = " << message.getId() << std::endl;

        std::string answer = std::to_string(command::Type::ROOM_INFO) + " " + std::to_string(success) + " "
                             + std::to_string(room->getNumberOfPlayers()) + " "
                             + std::to_string(player->getRoomPosition()) + " "
                             + std::to_string(room_id) + " " + room->getPositionMask();

        answer = Formatter::getMessageFormat(answer);
        sendTo(connection, Formatter::vectorOf(answer));
        //std::clog << "room use_count := " + std::to_string(room.use_count()) << std::endl;
    }
    void leaveRoom(boost::shared_ptr<MyConnection> const &connection, PlayerMessage const &message)
    {
        std::clog << "LEAVE_ROOM" << std::endl;
        auto server = connection->getServer();
        auto player = server->getPlayerById(message.getId());
        auto room = player->getRoom();
        if (room == nullptr)
        {
            std::clog << "FATAL: Leaving nullptr room" << std::endl;
            return;
        }
        //std::clog << "room use_count before erasing player := " + std::to_string(room.use_count()) << std::endl;
        room->erasePlayer(player);
        player->setRoom(nullptr);
        //std::clog << "room use_count after erasing player := " + std::to_string(room.use_count()) << std::endl;
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
    void startGame(boost::shared_ptr<Room> const &room)
    {
        std::clog << "StarT Game" << std::endl;
        for (auto player :room->getPlayers())
        {
            std::string answer = std::to_string(command::Type::START_GAME)
                                 + " " + std::to_string(player->getCharacter())
                                 + " " + std::to_string(player->getRoomPosition());

            answer = Formatter::getMessageFormat(answer);
            sendTo(player->getConnection(), Formatter::vectorOf(answer));

            answer = std::to_string(command::Type::START_SPEAKING) + " 0";
            answer = Formatter::getMessageFormat(answer);
            sendTo(player->getConnection(), Formatter::vectorOf(answer));

            sendGameInformation(player->getConnection());
        }
        auto server = room->getServer();
        server->updateRoomList();
    }
    void changeRoomState(boost::shared_ptr<MyConnection> const &connection, PlayerMessage const &message)
    {
        std::clog << "NEXT" << std::endl;
        auto server = connection->getServer();
        auto moderator = server->getPlayerByConnection(connection);
        auto room = moderator->getRoom();

        std::string prevStateName = room->getState()->getName();

        room->goToNextState();
        for (auto player : room->getPlayers())
        {
            sendGameInformation(player->getConnection());
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
            std::string answer = Formatter::getMessageFormat(first_message) + Formatter::getMessageFormat(second_message);
            for (auto player : room->getPlayers())
            {
                command::sendTo(player->getConnection(), Formatter::vectorOf(answer));
            }
        }
    }
    void sendGameInformation(boost::shared_ptr<MyConnection> const &connection)
    {
        std::clog << "GAME_INFO" << std::endl;
        auto server = connection->getServer();
        auto player = server->getPlayerByConnection(connection);
        auto room = player->getRoom();

        std::string answer = std::to_string(Type::GAME_INFO)
                             + " " + room->getState()->getPeriod()
                             + " " + room->getState()->getName()
                             + " " + room->getState()->getNext()->getName();

        answer = Formatter::getMessageFormat(answer);
        sendTo(player->getConnection(), Formatter::vectorOf(answer));
    }

    void handleSelectedTarget(boost::shared_ptr<MyConnection> const &connection, PlayerMessage const &message)
    {
        std::clog << "SELECT" << std::endl;
        auto server = connection->getServer();
        auto player = server->getPlayerByConnection(connection);
        auto room = player->getRoom();

        size_t target = std::stoi(Formatter::stringOf(message.getParams()[0]));

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

    void endGame(boost::shared_ptr<Room> const &room)
    {
        std::string message = "4 " + std::to_string(command::END_GAME);
        for (auto player : room->getPlayers())
        {
            sendTo(player->getConnection(), Formatter::vectorOf(message));
        }
    }

    bool sendTo(boost::shared_ptr<MyConnection> const & connection, std::vector<uint8_t> const & buffer)
    {
        if (!(connection->getServer()->getPlayerByConnection(connection)->isBot()))
        {
            connection->Send(buffer);
        }
    }
}