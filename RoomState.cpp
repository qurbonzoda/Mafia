//
// Created by qurbonzoda on 04.02.16.
//

#include "RoomState.h"
#include "Room.h"
#include "Player.h"
#include "Server.h"

std::vector< std::string > RoomState::RoomStateNames {"City_wakes_up", "Speaking_player_", "Finished_speaking_player_", "Voting", "Night", "Mafias_wake_up",
                                           "Mafias_murdered", "Detective_checking", "Doctor_curing", "Early_morning", "Murdered_players_talks"};

std::vector< std::string > RoomState::RoomStatePeriods {"Day", "Day", "Day", "Voting", "Night", "Night",
                                             "Night", "Night", "Night", "Day", "Day"};

//Moderator, Detective, Doctor, Mafia, Villager

std::vector< std::string > RoomState::RoomStateVideoMasks {"11111", "11111", "11111", "11111", "10000", "10010",
                                                "10010", "11000", "10100", "11111", "11111"};

std::vector< std::string > RoomState::RoomStateVisibleMasks {"11111", "11111", "11111", "11111", "10000", "10010",
                                                "10010", "11000", "10100", "11111", "11111"};

std::vector< std::string > RoomState::RoomStateAudioMasks {"10000", "10000", "10000", "10000", "10000", "10000",
                                                "10000", "10000", "10000", "10000", "10000"};

RoomState * RoomState::buildStateChain(boost::shared_ptr<Room> room)
{
    RoomState * head = new RoomState(RoomStateNames[0], RoomStatePeriods[0],
                                     RoomStateVideoMasks[0], RoomStateVisibleMasks[0], RoomStateAudioMasks[0]);
    RoomState * cur = head;

    int playersNum = room->getNumber_of_players();

    for (int i = 1; i < playersNum; i++)
    {
        cur->next = new RoomState(RoomStateNames[1] + std::to_string(i), RoomStatePeriods[1],
                                  RoomStateVideoMasks[1], RoomStateVisibleMasks[1], RoomStateAudioMasks[1]);
        cur = cur->next;

        cur->next = new RoomState(RoomStateNames[2] + std::to_string(i), RoomStatePeriods[2],
                                  RoomStateVideoMasks[2], RoomStateVisibleMasks[2], RoomStateAudioMasks[2]);
        cur = cur->next;
    }

    for (int i = 3; i < RoomStateNames.size(); i++)
    {
        cur->next = new RoomState(RoomStateNames[i], RoomStatePeriods[i],
                                  RoomStateVideoMasks[i], RoomStateVisibleMasks[i], RoomStateAudioMasks[i]);
        cur = cur->next;
    }
    cur->next = head;

    return head;
}


void RoomState::buildVotingChain(RoomState *state, std::map<size_t, size_t> const &nominees)
{
    RoomState * tmp = state->next;
    for (auto nominee : nominees)
    {
        state->next = new RoomState("Voting_agains_player_" + std::to_string(nominee.first), "Day", "11111", "11111", "10000");
        state = state->next;
    }
    state->next = tmp;
}
void RoomState::buildMurderChain(RoomState * state, size_t room_position)
{
    RoomState * tmp = state->next;
    state->next = new RoomState("Was_murdered_player_" + std::to_string(room_position), state->getPeriod(), "11111", "11111", "10000");
    state = state->next;
    state->next = new RoomState("Murdered player's speech" + std::to_string(room_position), state->getPeriod(), "11111", "11111", "10000");
    state = state->next;
    state->next = tmp;
}

RoomState::RoomState (std::string name, std::string period, std::string videoMask, std::string visibleMask, std::string audioMask)
        : name(name), period(period), videoMask(videoMask), visibleMask(visibleMask), audioMask(audioMask)
{
    std::clog << name + " " + period + " " + videoMask + " " + audioMask << std::endl;
}

RoomState::~RoomState()
{
    std::clog << "[" << __FUNCTION__ << "] " << std::endl;
}

bool RoomState::canSee(boost::shared_ptr<Player> player)
{
    /*
    if (!player->isBot())
    {
        return (videoMask[player->getCharacter()] == '1');
    }
    return false;
    */
    return (videoMask[player->getCharacter()] == '1');
}

bool RoomState::isVisible(boost::shared_ptr<Player> player)
{
    /*
    if ((visibleMask[player->getCharacter()] == '1') && player->isInvisiblitySet())
    {
        if (player->isBot())
        {
            player->setScreen(player->getRoom()->getServer()->getBotScreen());
        }
        player->setScreenChanged(true);
        player->setInvisiblitySet(false);
    }*/
    return (visibleMask[player->getCharacter()] == '1');
}

bool RoomState::canSpeak(boost::shared_ptr<Player> player)
{
    std::string speaking = RoomStateNames[1];
    if (name.find(speaking) == std::string::npos)
    {
        return (audioMask[player->getCharacter()] == '1');
    }
    return (name.find(std::to_string(player->getRoom_position())) != std::string::npos)
           || (audioMask[player->getCharacter()] == '1');
}

