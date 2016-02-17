//
// Created by qurbonzoda on 20.12.15.
//

#include "Command.h"
#include "PlayerMessage.h"
#include "Formatter.h"
#include "Player.h"
#include "Room.h"
#include "Server.h"
#include "MyNetwork.h"
#include "RoomState.h"

namespace Command {

    void execute(boost::shared_ptr<MyConnection> const & connection, PlayerMessage const &message)
    {
        std::clog << "request from player " << message.getId() << " command " << message.getCommand() << std::endl;
        switch (message.getCommand())
        {
            case AUTHORISATION:
                authorization(connection, message);
                break;
            case NEW_ROOM:
                new_room(connection, message);
                break;
            case ROOMS_LIST:
                room_list(connection, message);
                break;
            case ENTER_ROOM:
                enter_room(connection, message);
                break;
            case ROOM_INFO:
                room_info(connection, message);
                break;
            case LEAVE_ROOM:
                leave_room(connection, message);
                break;
            case NEXT:
                next(connection, message);
                break;
            case SELECT:
                select(connection, message);
                break;
            default:
                break;
        }
    }

    void authorization(boost::shared_ptr<MyConnection> const & connection, PlayerMessage const &message)
    {
        std::clog << "AUTHORIZATION" << std::endl;
        boost::shared_ptr<Player> player = connection->getServer()->getPlayer_by_id(message.getId());
        player->setLogin(Formatter::stringOf(message.getParams()[0]));
        player->setPassword(Formatter::stringOf(message.getParams()[1]));
        room_list(connection, message);
    }

    void new_room(boost::shared_ptr<MyConnection> const & connection, PlayerMessage const &message)
    {
        std::clog << "NEW_ROOM" << std::endl;
        auto server = connection->getServer();
        std::clog << "got server" << std::endl;
        boost::shared_ptr<Player> player = server->getPlayer_by_id(message.getId());
        std::clog << "got player" << std::endl;
        boost::shared_ptr<Room> room = server->create_new_room_instance();
        std::clog << "got room" << std::endl;

        //std::clog << "room use_count := " + std::to_string(room.use_count()) << std::endl;
        room->setMax_players(std::stoi(Formatter::stringOf(message.getParams()[0])));
        std::clog << (std::string)"maxPlayers in new room " + std::to_string(room->getId())
                     + " is " + std::to_string(room->getMax_players()) << std::endl;

        room->setPassword(Formatter::stringOf(message.getParams()[1]));
        std::clog << "set Password" << std::endl;
        PlayerMessage copy_message(message);
        copy_message.setParam(0, Formatter::vectorOf(std::to_string(room->getId())));
        std::clog << std::to_string(room->getId()) + " == " + Formatter::stringOf(copy_message.getParams()[0]) << std::endl;
        enter_room(connection, copy_message);
    }

    void room_list(boost::shared_ptr<MyConnection> const & connection, PlayerMessage const & message)
    {
        room_list(connection);
    }

    void room_list(boost::shared_ptr<MyConnection> const & connection)
    {
        std::clog << "ROOM_LIST" << std::endl;
        auto rooms = connection->getServer()->getRooms();
        std::string answer = std::to_string(Command::Type::ROOMS_LIST) + " " + std::to_string(rooms.size());
        for (auto room : rooms)
        {
            answer += " " + std::to_string(room->getStatus()) + " " + std::to_string(room->isSafe()) + " "
                      + std::to_string(room->getNumber_of_players())
                      + " " + std::to_string(room->getMax_players()) + " " + std::to_string(room->getId());
        }
        answer = Formatter::getMessageFormat(answer);
        sendTo(connection, Formatter::vectorOf(answer));
    }

    void enter_room(boost::shared_ptr<MyConnection> const & connection, PlayerMessage const & message)
    {
        std::clog << "ENTER_ROOM" << std::endl;
        boost::shared_ptr<Server> server = connection->getServer();
        uint32_t room_id = std::stoi(Formatter::stringOf(message.getParams()[0]));
        std::string room_password = Formatter::stringOf(message.getParams()[1]);
        boost::shared_ptr<Room> room = server->getRoom_by_id(room_id);

        if (room == nullptr) { return; };

        if (room->getStatus() == Room::Status::playing)
        {
            room_info(connection, message);
            return;
        }

        if (room->getPassword() == room_password || !room->isSafe())
        {
            room->join(server->getPlayer_by_id(message.getId()));
        }
        server->update_room_info(room);
        server->update_room_list();

    }
    void room_info(boost::shared_ptr<MyConnection> const & connection, PlayerMessage const & message)
    {
        std::clog << "ROOM_INFO" << std::endl;
        boost::shared_ptr<Server> server = connection->getServer();
        uint32_t room_id = std::stoi(Formatter::stringOf(message.getParams()[0]));
        std::string room_password = Formatter::stringOf(message.getParams()[1]);
        auto room = server->getRoom_by_id(room_id);
        auto player = server->getPlayer_by_id(message.getId());

        //std::clog << "room use_count := " + std::to_string(room.use_count()) << std::endl;

        bool success = (room->getPlayers().find(player) != room->getPlayers().end())
                       && (room->getPassword() == room_password || !room->isSafe());

        assert(room_id == room->getId() && room_password == room->getPassword());

        std::clog << "room_id = "<< room_id << " room_password = " << room_password << " player_id = " << message.getId() << std::endl;

        std::string answer = std::to_string(Command::Type::ROOM_INFO) + " " + std::to_string(success) + " "
                             + std::to_string(room->getNumber_of_players()) + " "
                             + std::to_string(player->getRoom_position()) + " "
                             + std::to_string(room_id) + " " + room->getPosition_mask();

        answer = Formatter::getMessageFormat(answer);
        sendTo(connection, Formatter::vectorOf(answer));
        //std::clog << "room use_count := " + std::to_string(room.use_count()) << std::endl;
    }
    void leave_room(boost::shared_ptr<MyConnection> const & connection, PlayerMessage const & message)
    {
        std::clog << "LEAVE_ROOM" << std::endl;
        auto server = connection->getServer();
        auto player = server->getPlayer_by_id(message.getId());
        auto room = player->getRoom();
        if (room == nullptr)
        {
            std::clog << "FATAL: Leaving nullptr room" << std::endl;
            return;
        }
        //std::clog << "room use_count before erasing player := " + std::to_string(room.use_count()) << std::endl;
        room->erase(player);
        player->setRoom(nullptr);
        //std::clog << "room use_count after erasing player := " + std::to_string(room.use_count()) << std::endl;
        if (room->getNumber_of_players() == 0)
        {
            server->delete_room(room);
            //std::clog << "use_count := " + std::to_string(room.use_count()) << std::endl;
        }
        else
        {
            server->update_room_info(room);
        }
        server->update_room_list();
        std::clog << "LEAVED_ROOM SUCCESS" << std::endl;
        //std::clog << "room use_count := " + std::to_string(room.use_count()) << std::endl;
    }
    void start_game(boost::shared_ptr<Room> const & room)
    {
        std::clog << "StarT Game" << std::endl;
        room->setStatus(Room::Status::playing);
        for (auto player :room->getPlayers())
        {
            std::string answer = std::to_string(Command::Type::START_GAME)
                                 + " " + std::to_string(player->getCharacter())
                                 + " " + std::to_string(player->getRoom_position());

            answer = Formatter::getMessageFormat(answer);
            sendTo(player->getConnection(), Formatter::vectorOf(answer));
            game_info(player->getConnection());
        }
        auto server = (*(room->getPlayers().begin()))->getConnection()->getServer();
        server->update_room_list();
    }
    void next(boost::shared_ptr<MyConnection> const & connection, PlayerMessage const & message)
    {
        auto server = connection->getServer();
        auto player = server->getPlayer_by_connection(connection);
        auto room = player->getRoom();

        room->goToNextState();
        for (auto player : room->getPlayers())
        {
            game_info(player->getConnection());
        }
    }
    void game_info(boost::shared_ptr<MyConnection> const & connection)
    {
        auto server = connection->getServer();
        auto player = server->getPlayer_by_connection(connection);
        auto room = player->getRoom();

        std::string answer = std::to_string(Type::GAME_INFO)
                             + " " + room->getState()->getPeriod()
                             + " " + room->getState()->getName()
                             + " " + room->getState()->getNext()->getName();

        answer = Formatter::getMessageFormat(answer);
        sendTo(player->getConnection(), Formatter::vectorOf(answer));
    }

    void select(boost::shared_ptr<MyConnection> const & connection, PlayerMessage const & message)
    {
        auto server = connection->getServer();
        auto player = server->getPlayer_by_connection(connection);
        auto room = player->getRoom();

        size_t target = std::stoi(Formatter::stringOf(message.getParams()[0]));

        if (room->getState()->getName().find(RoomState::RoomStateNames[2]) != std::string::npos)
        {
            room->nominate(target);
        }
        else if (room->getState()->getName().find("Voting_against") != std::string::npos)
        {
            room->votesAgainst(target);
        }
        else if (room->getState()->getName().find("murder") != std::string::npos)
        {
            room->tryToMurder(target);
        }
        else if (room->getState()->getName().find("Doctor") != std::string::npos)
        {
            room->curePlayer(target);
        }
    }

    bool sendTo(boost::shared_ptr<MyConnection> const & connection, std::vector<uint8_t> const & buffer)
    {
        if (!(connection->getServer()->getPlayer_by_connection(connection)->isBot()))
        {
            connection->Send(buffer);
        }
    }
}