//
// Created by qurbonzoda on 20.12.15.
//

#include "Command.h"

namespace Command {

    void execute(boost::shared_ptr<Connection> const &connection, PlayerMessage const &message)
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
            default:
                break;
        }
    }

    void authorization(boost::shared_ptr<Connection> const &connection, PlayerMessage const &message)
    {
        std::clog << "AUTHORIZATION" << std::endl;
        assert(connection->getServer() != nullptr);
        std::clog << "Assertion passed" << std::endl;
        boost::shared_ptr<Player> player = connection->getServer()->getPlayer_by_id(message.getId());
        player->setLogin(Formatter::getStringOf(message.getParams()[0]));
        player->setPassword(Formatter::getStringOf(message.getParams()[1]));
        room_list(connection, message);
    }

    void new_room(boost::shared_ptr<Connection> const &connection, PlayerMessage const &message)
    {
        std::clog << "NEW_ROOM" << std::endl;
        auto server = connection->getServer();
        boost::shared_ptr<Player> player = server->getPlayer_by_id(message.getId());
        boost::shared_ptr<Room> room = server->create_new_room_instance();
        room->setMax_players(std::stoi(Formatter::getStringOf(message.getParams()[0])));
        //std::clog << "maxPlayers in new room " << room->getId() << " is " + room->getMax_players() << std::endl;
        //std::clog.flush();
        room->setPassword(Formatter::getStringOf(message.getParams()[1]));
        player->setRoom(room);
        room->join(player);

        PlayerMessage copy_message(message);
        std::clog << "room_id = " << room->getId() << std::endl;
        copy_message.setParam(0, Formatter::getVectorOf(std::to_string(room->getId())));
        std::clog << Formatter::getStringOf(copy_message.getParams()[0]) << std::endl;
        room_info(connection, copy_message);
        server->update_room_list();
    }

    void room_list(boost::shared_ptr<Connection> const & connection, PlayerMessage const & message)
    {
        room_list(connection);
    }

    void room_list(boost::shared_ptr<Connection> const & connection)
    {
        std::clog << "ROOM_LIST" << std::endl;
        auto rooms = connection->getServer()->getRooms();
        std::string answer = std::to_string(Command::Type::ROOMS_LIST) + " " + std::to_string(rooms.size());
        for (auto room : rooms)
        {
            answer += " " + std::to_string(room->getStatus()) + " " + std::to_string(room->isSafe()) + " "
                      + std::to_string(room->getNumber_of_players())
                      + " " + std::to_string(room->getMax_players()) + " " + std::to_string(room->getId());

            //std::clog << "maxPlayers in room " << room->getId() << " is " + room->getMax_players() << std::endl;
            //std::clog.flush();
        }
        answer = Formatter::getMessageFormat(answer);
        connection->Send(Formatter::getVectorOf(answer));
    }

    void enter_room(boost::shared_ptr<Connection> const & connection, PlayerMessage const & message)
    {
        std::clog << "ENTER_ROOM" << std::endl;
        boost::shared_ptr<Server> server = connection->getServer();
        uint32_t room_id = std::stoi(Formatter::getStringOf(message.getParams()[0]));
        std::string room_password = Formatter::getStringOf(message.getParams()[1]);
        boost::shared_ptr<Room> room = server->getRoom_by_id(room_id);
        if (room->getPassword() == room_password || !room->isSafe())
        {
            room->join(server->getPlayer_by_id(message.getId()));
        }
        //room_info(connection, message);
        server->update_room_info(room);
        server->update_room_list();

    }
    void room_info(boost::shared_ptr<Connection> const & connection, PlayerMessage const & message)
    {
        std::clog << "ROOM_INFO" << std::endl;
        boost::shared_ptr<Server> server = connection->getServer();
        uint32_t room_id = std::stoi(Formatter::getStringOf(message.getParams()[0]));
        std::string room_password = Formatter::getStringOf(message.getParams()[1]);
        boost::shared_ptr<Room> room = server->getRoom_by_id(room_id);

        bool success = (room->getPassword() == room_password || !room->isSafe());

        assert(room_id == room->getId() && room_password == room->getPassword());

        std::clog << "room_id = "<< room_id << " room_password = " << room_password << " player_id = " << message.getId() << std::endl;

        std::string answer = std::to_string(Command::Type::ROOM_INFO) + " " + std::to_string(success) + " "
                             + std::to_string(room->getNumber_of_players()) + " "
                             + std::to_string(server->getPlayer_by_id(message.getId())->getRoom_position()) + " "
                             + std::to_string(room_id) + " " + room->getPosition_mask();

        answer = Formatter::getMessageFormat(answer);
        connection->Send(Formatter::getVectorOf(answer));
    }
    void leave_room(boost::shared_ptr<Connection> const & connection, PlayerMessage const & message)
    {
        std::clog << "LEAVE_ROOM" << std::endl;
        auto server = connection->getServer();
        auto player = server->getPlayer_by_id(message.getId());
        auto room = player->getRoom();
        room->erase(player);
        if (room->getNumber_of_players() == 0)
        {
            server->room_erase(room);
        }
        else
        {
            server->update_room_info(room);
        }
        server->update_room_list();
        //room_list(connection, message);
    }
}