//
// Created by qurbonzoda on 04.02.16.
//

#ifndef MAFIA_ROOMSTATE_H
#define MAFIA_ROOMSTATE_H

#include <boost/smart_ptr.hpp>
#include <map>

class Room;
class Player;

class RoomState
{
    std::string name;
    std::string period;
    std::string videoMask;
    std::string visibleMask;
    std::string audioMask;
    RoomState *next;

public:
    const std::string &getName() const
    {
        return name;
    }

    const std::string &getPeriod() const
    {
        return period;
    }

    RoomState *getNext() const
    {
        return next;
    }

    void setNext(RoomState *next)
    {
        RoomState::next = next;
    }

    bool canSee(boost::shared_ptr<const Player> &player) const;

    bool isVisible(boost::shared_ptr<const Player> &player) const;

    bool canSpeak(boost::shared_ptr<const Player> &player) const;


    ~RoomState();

private:
    RoomState (std::string const &name, std::string const &period, std::string const &videoMask, std::string const &visibleMask, std::string const &audioMask);


public:
    static RoomState * buildStateChain(boost::shared_ptr<Room> room);
    static void buildVotingChain(RoomState * state, std::map< size_t, size_t > const & nominees);
    static void buildMurderChain(RoomState * state, size_t room_position);

    static std::vector< std::string > RoomStateNames;

    static std::vector< std::string > RoomStatePeriods;

    //Moderator, Detective, Doctor, Mafia, Villager

    static std::vector< std::string > RoomStateVideoMasks;
    static std::vector< std::string > RoomStateVisibleMasks;

    static std::vector< std::string > RoomStateAudioMasks;
};


#endif //MAFIA_ROOMSTATE_H
