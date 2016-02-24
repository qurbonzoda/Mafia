//
// Created by qurbonzoda on 04.02.16.
//

#include "room_state.h"
#include "room.h"
#include "player.h"
#include "server.h"
#include "constants.h"

std::vector< std::string > RoomState::RoomStateNames {CITY_WAKES_UP, SPEAKING, FINISHED_SPEAKING, VOTING, NIGHT,
                                                      MAFIA_WAKES_UP, MAFIA_MURDERED, DETECTIVE_CHECKING, DOCTOR_CURING, EARLY_MORNING};

std::vector< std::string > RoomState::RoomStatePeriods {DAY,  DAY,  DAY,  VOTING,NIGHT,
                                                        NIGHT,NIGHT,NIGHT,NIGHT, DAY};

//Moderator, Detective, Doctor, Mafia, Villager

std::vector< std::string > RoomState::RoomStateVideoMasks {"111111", "111111", "111111", "111111", "100001",
                                                           "100101", "100101", "110001", "101001", "111111"};

std::vector< std::string > RoomState::RoomStateVisibleMasks {"111110", "111110", "111110", "111110", "100000",
                                                             "100100", "100100", "110000", "101000", "111110"};

std::vector< std::string > RoomState::RoomStateAudioMasks {"100000", "100000", "100000", "100000", "100000",
                                                           "100000", "100000", "100000", "100000", "100000"};

RoomState * RoomState::buildStateChain(boost::shared_ptr<Room> room)
{
    RoomState * head = new RoomState(RoomStateNames[0], RoomStatePeriods[0],
                                     RoomStateVideoMasks[0], RoomStateVisibleMasks[0], RoomStateAudioMasks[0]);
    RoomState * cur = head;

    int playersNum = room->getNumberOfPlayers();

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
        state->next = new RoomState(VOTING_AGAINST + std::to_string(nominee.first),
                                    DAY, "11111", "11111", "10000");
        state = state->next;
    }
    state->next = tmp;
}
void RoomState::buildMurderChain(RoomState * state, size_t room_position)
{
    RoomState * tmp = state->next;
    state->next = new RoomState(WAS_MURDERED + std::to_string(room_position),
                                state->getPeriod(), "11111", "11111", "10000");
    state = state->next;
    state->next = new RoomState(MURDERED_SPEAKS + std::to_string(room_position),
                                state->getPeriod(), "11111", "11111", "10000");
    state = state->next;
    state->next = tmp;
}
void RoomState::buildEndGameChain(RoomState * state, std::string winner)
{
    RoomState * tmp = state->next;
    state->next = new RoomState(winner + "_won_the_game",
                                GAME_OVER, "11111", "11111", "10000");
    state = state->next;
    state->next = tmp;
}

RoomState::RoomState (std::string const &name, std::string const &period, std::string const &videoMask,
                      std::string const &visibleMask, std::string const &audioMask)
        : name(name), period(period), videoMask(videoMask), visibleMask(visibleMask), audioMask(audioMask)
{
    std::clog << name + " " + period + " " + videoMask + " " + audioMask << std::endl;
}

RoomState::~RoomState()
{
    std::clog << "[" << __FUNCTION__ << "] " << std::endl;
}

bool RoomState::canSee(boost::shared_ptr<const Player> &player) const
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

bool RoomState::isVisible(boost::shared_ptr<const Player> &player) const
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

bool RoomState::canSpeak(boost::shared_ptr<const Player> &player) const
{
    std::string speaking = RoomStateNames[1];
    if (name.find(speaking) == std::string::npos)
    {
        return (audioMask[player->getCharacter()] == '1');
    }
    return (name.find(std::to_string(player->getRoomPosition())) != std::string::npos)
           || (audioMask[player->getCharacter()] == '1');
}

