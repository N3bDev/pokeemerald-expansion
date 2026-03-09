#ifndef GUARD_CONSTANTS_ROGUE_GYM_H
#define GUARD_CONSTANTS_ROGUE_GYM_H

// Roguelike Gym Trainer Mode Constants

// Difficulty tiers
#define ROGUE_GYM_TIER_BEGINNER     0  // Battles 1-5
#define ROGUE_GYM_TIER_INTERMEDIATE 1  // Battles 6-12
#define ROGUE_GYM_TIER_ADVANCED     2  // Battles 13-20
#define ROGUE_GYM_TIER_EXPERT       3  // Battles 21-30
#define ROGUE_GYM_TIER_CHAMPION     4  // Battles 31+
#define ROGUE_GYM_NUM_TIERS         5

// Battle thresholds for each tier
#define ROGUE_GYM_TIER1_START       6
#define ROGUE_GYM_TIER2_START      13
#define ROGUE_GYM_TIER3_START      21
#define ROGUE_GYM_TIER4_START      31
#define ROGUE_GYM_MAX_BATTLES      40  // Winning battle 40 = full run complete

// Team size limits per battle range
#define ROGUE_GYM_MAX_TEAM_BATTLE_2   5   // Can have 2 mons starting at battle 5
#define ROGUE_GYM_MAX_TEAM_BATTLE_3   9   // 3 mons at battle 9
#define ROGUE_GYM_MAX_TEAM_BATTLE_4  14   // 4 mons at battle 14
#define ROGUE_GYM_MAX_TEAM_BATTLE_5  19   // 5 mons at battle 19
#define ROGUE_GYM_MAX_TEAM_BATTLE_6  25   // 6 mons at battle 25

// Enemy team sizes per tier
#define ROGUE_GYM_ENEMY_SIZE_TIER0_MIN  1
#define ROGUE_GYM_ENEMY_SIZE_TIER0_MAX  2
#define ROGUE_GYM_ENEMY_SIZE_TIER1_MIN  2
#define ROGUE_GYM_ENEMY_SIZE_TIER1_MAX  3
#define ROGUE_GYM_ENEMY_SIZE_TIER2_MIN  3
#define ROGUE_GYM_ENEMY_SIZE_TIER2_MAX  4
#define ROGUE_GYM_ENEMY_SIZE_TIER3_MIN  4
#define ROGUE_GYM_ENEMY_SIZE_TIER3_MAX  5
#define ROGUE_GYM_ENEMY_SIZE_TIER4_MIN  5
#define ROGUE_GYM_ENEMY_SIZE_TIER4_MAX  6

// Level scaling - bonus added to player's avg level for enemy mons
#define ROGUE_GYM_LEVEL_BONUS_TIER0   0
#define ROGUE_GYM_LEVEL_BONUS_TIER1   1
#define ROGUE_GYM_LEVEL_BONUS_TIER2   2
#define ROGUE_GYM_LEVEL_BONUS_TIER3   2
#define ROGUE_GYM_LEVEL_BONUS_TIER4   3

// Starting level for starter Pokemon
#define ROGUE_GYM_STARTER_LEVEL       5

// Number of starters to choose from
#define ROGUE_GYM_NUM_STARTER_CHOICES 3

// Number of reward options per reward selection
#define ROGUE_GYM_NUM_REWARD_CHOICES  3

// Reward types
#define ROGUE_REWARD_POKEMON      0
#define ROGUE_REWARD_HEALING      1
#define ROGUE_REWARD_PP_RESTORE   2
#define ROGUE_REWARD_TM           3
#define ROGUE_REWARD_HELD_ITEM    4
#define ROGUE_REWARD_RARE_CANDY   5
#define ROGUE_REWARD_TYPE_COUNT   6

// Max items in roguelike inventory
#define ROGUE_GYM_MAX_ITEMS      20

// Challenge status values (reuses frontier pattern)
#define ROGUE_GYM_STATUS_NONE       0
#define ROGUE_GYM_STATUS_ACTIVE     1
#define ROGUE_GYM_STATUS_WON        2
#define ROGUE_GYM_STATUS_LOST       3
#define ROGUE_GYM_STATUS_PAUSED     4

// Script special function IDs
#define ROGUE_GYM_FUNC_INIT                 0
#define ROGUE_GYM_FUNC_GENERATE_STARTERS    1
#define ROGUE_GYM_FUNC_PICK_STARTER         2
#define ROGUE_GYM_FUNC_SETUP_NEXT_BATTLE    3
#define ROGUE_GYM_FUNC_PROCESS_BATTLE_WIN   4
#define ROGUE_GYM_FUNC_PROCESS_FAINTED      5
#define ROGUE_GYM_FUNC_CHECK_GAME_OVER      6
#define ROGUE_GYM_FUNC_GENERATE_REWARDS     7
#define ROGUE_GYM_FUNC_PICK_REWARD          8
#define ROGUE_GYM_FUNC_GET_STATUS           9
#define ROGUE_GYM_FUNC_GET_BATTLE_NUM      10
#define ROGUE_GYM_FUNC_GET_TIER            11
#define ROGUE_GYM_FUNC_USE_ITEM            12
#define ROGUE_GYM_FUNC_SAVE                13
#define ROGUE_GYM_FUNC_CLEANUP             14
#define ROGUE_GYM_FUNC_COUNT               15

// Frontier mon ranges for tier-based selection
// These index into gBattleFrontierMons (0-881)
// Lower indices = weaker mons, higher = stronger
#define ROGUE_GYM_MON_RANGE_TIER0_MIN    0
#define ROGUE_GYM_MON_RANGE_TIER0_MAX  199
#define ROGUE_GYM_MON_RANGE_TIER1_MIN  100
#define ROGUE_GYM_MON_RANGE_TIER1_MAX  399
#define ROGUE_GYM_MON_RANGE_TIER2_MIN  200
#define ROGUE_GYM_MON_RANGE_TIER2_MAX  599
#define ROGUE_GYM_MON_RANGE_TIER3_MIN  400
#define ROGUE_GYM_MON_RANGE_TIER3_MAX  781
#define ROGUE_GYM_MON_RANGE_TIER4_MIN  600
#define ROGUE_GYM_MON_RANGE_TIER4_MAX  881

#endif // GUARD_CONSTANTS_ROGUE_GYM_H
