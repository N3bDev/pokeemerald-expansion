#ifndef GUARD_ROGUE_GYM_H
#define GUARD_ROGUE_GYM_H

#include "constants/rogue_gym.h"

// Reward option for the selection screen
struct RogueGymReward
{
    u8 type;        // ROGUE_REWARD_*
    u16 itemOrMon;  // Item ID or species ID depending on type
    u8 quantity;    // For consumable items
};

// RogueGymData struct is defined in global.h (inside BattleFrontier save block)

// Temporary working data (not saved, reconstructed each battle)
struct RogueGymTemp
{
    u16 starterChoices[ROGUE_GYM_NUM_STARTER_CHOICES];
    struct RogueGymReward rewardChoices[ROGUE_GYM_NUM_REWARD_CHOICES];
    u8 enemyTrainerClass;
    u8 enemyTeamSize;
    u16 enemySpecies[PARTY_SIZE];
    u8 enemyLevels[PARTY_SIZE];
};

extern struct RogueGymTemp *gRogueGymTemp;

// Main entry point called from scripts via special
void CallRogueGymFunction(void);

// Utility
u8 RogueGym_GetMaxTeamSize(u8 battleNum);
u8 RogueGym_GetTierForBattle(u8 battleNum);
u8 RogueGym_GetPlayerAvgLevel(void);
bool8 RogueGym_IsActive(void);
void RogueGym_AddItem(u16 itemId, u8 quantity);
bool8 RogueGym_HasItem(u16 itemId);
u8 RogueGym_GetPlayerMonCount(void);

#endif // GUARD_ROGUE_GYM_H
