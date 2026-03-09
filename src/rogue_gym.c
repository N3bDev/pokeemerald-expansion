#include "global.h"
#include "rogue_gym.h"
#include "battle.h"
#include "battle_frontier.h"
#include "battle_setup.h"
#include "event_data.h"
#include "frontier_util.h"
#include "item.h"
#include "malloc.h"
#include "move.h"
#include "overworld.h"
#include "party_menu.h"
#include "pokemon.h"
#include "random.h"
#include "string_util.h"
#include "strings.h"
#include "constants/battle_ai.h"
#include "constants/battle_frontier.h"
#include "constants/battle_frontier_mons.h"
#include "constants/items.h"
#include "constants/moves.h"
#include "constants/species.h"
#include "constants/trainers.h"

// External data from battle frontier
extern const struct TrainerMon gBattleFrontierMons[];

EWRAM_DATA struct RogueGymTemp *gRogueGymTemp = NULL;

// Forward declarations
static void RogueGym_InitFunc(void);
static void RogueGym_GenerateStartersFunc(void);
static void RogueGym_PickStarterFunc(void);
static void RogueGym_SetupNextBattleFunc(void);
static void RogueGym_ProcessBattleWinFunc(void);
static void RogueGym_ProcessFaintedFunc(void);
static void RogueGym_CheckGameOverFunc(void);
static void RogueGym_GenerateRewardsFunc(void);
static void RogueGym_PickRewardFunc(void);
static void RogueGym_GetStatusFunc(void);
static void RogueGym_GetBattleNumFunc(void);
static void RogueGym_GetTierFunc(void);
static void RogueGym_UseItemFunc(void);
static void RogueGym_SaveFunc(void);
static void RogueGym_CleanupFunc(void);

static void (*const sRogueGymFunctions[])(void) =
{
    [ROGUE_GYM_FUNC_INIT]               = RogueGym_InitFunc,
    [ROGUE_GYM_FUNC_GENERATE_STARTERS]  = RogueGym_GenerateStartersFunc,
    [ROGUE_GYM_FUNC_PICK_STARTER]       = RogueGym_PickStarterFunc,
    [ROGUE_GYM_FUNC_SETUP_NEXT_BATTLE]  = RogueGym_SetupNextBattleFunc,
    [ROGUE_GYM_FUNC_PROCESS_BATTLE_WIN] = RogueGym_ProcessBattleWinFunc,
    [ROGUE_GYM_FUNC_PROCESS_FAINTED]    = RogueGym_ProcessFaintedFunc,
    [ROGUE_GYM_FUNC_CHECK_GAME_OVER]    = RogueGym_CheckGameOverFunc,
    [ROGUE_GYM_FUNC_GENERATE_REWARDS]   = RogueGym_GenerateRewardsFunc,
    [ROGUE_GYM_FUNC_PICK_REWARD]        = RogueGym_PickRewardFunc,
    [ROGUE_GYM_FUNC_GET_STATUS]         = RogueGym_GetStatusFunc,
    [ROGUE_GYM_FUNC_GET_BATTLE_NUM]     = RogueGym_GetBattleNumFunc,
    [ROGUE_GYM_FUNC_GET_TIER]           = RogueGym_GetTierFunc,
    [ROGUE_GYM_FUNC_USE_ITEM]           = RogueGym_UseItemFunc,
    [ROGUE_GYM_FUNC_SAVE]               = RogueGym_SaveFunc,
    [ROGUE_GYM_FUNC_CLEANUP]            = RogueGym_CleanupFunc,
};

// Healing items that can appear as rewards per tier
static const u16 sHealingRewards[][3] = {
    [ROGUE_GYM_TIER_BEGINNER]     = {ITEM_POTION, ITEM_POTION, ITEM_SUPER_POTION},
    [ROGUE_GYM_TIER_INTERMEDIATE] = {ITEM_SUPER_POTION, ITEM_SUPER_POTION, ITEM_HYPER_POTION},
    [ROGUE_GYM_TIER_ADVANCED]     = {ITEM_HYPER_POTION, ITEM_HYPER_POTION, ITEM_FULL_RESTORE},
    [ROGUE_GYM_TIER_EXPERT]       = {ITEM_HYPER_POTION, ITEM_FULL_RESTORE, ITEM_FULL_RESTORE},
    [ROGUE_GYM_TIER_CHAMPION]     = {ITEM_FULL_RESTORE, ITEM_FULL_RESTORE, ITEM_MAX_REVIVE},
};

// PP restore items per tier
static const u16 sPPRewards[][3] = {
    [ROGUE_GYM_TIER_BEGINNER]     = {ITEM_ETHER, ITEM_ETHER, ITEM_ETHER},
    [ROGUE_GYM_TIER_INTERMEDIATE] = {ITEM_ETHER, ITEM_ETHER, ITEM_ELIXIR},
    [ROGUE_GYM_TIER_ADVANCED]     = {ITEM_ETHER, ITEM_ELIXIR, ITEM_ELIXIR},
    [ROGUE_GYM_TIER_EXPERT]       = {ITEM_ELIXIR, ITEM_ELIXIR, ITEM_MAX_ELIXIR},
    [ROGUE_GYM_TIER_CHAMPION]     = {ITEM_ELIXIR, ITEM_MAX_ELIXIR, ITEM_MAX_ELIXIR},
};

// Held items that can appear as rewards (competitive items)
static const u16 sHeldItemRewards[] = {
    ITEM_LEFTOVERS,
    ITEM_CHOICE_BAND,
    ITEM_CHOICE_SPECS,
    ITEM_CHOICE_SCARF,
    ITEM_LIFE_ORB,
    ITEM_FOCUS_SASH,
    ITEM_ASSAULT_VEST,
    ITEM_ROCKY_HELMET,
    ITEM_EVIOLITE,
    ITEM_WEAKNESS_POLICY,
    ITEM_BRIGHT_POWDER,
    ITEM_SCOPE_LENS,
    ITEM_SHELL_BELL,
    ITEM_QUICK_CLAW,
    ITEM_KINGS_ROCK,
    ITEM_WIDE_LENS,
    ITEM_MUSCLE_BAND,
    ITEM_WISE_GLASSES,
    ITEM_EXPERT_BELT,
    ITEM_MENTAL_HERB,
};

#define NUM_HELD_ITEM_REWARDS ARRAY_COUNT(sHeldItemRewards)

// TMs that can appear as rewards - a curated list of good ones
static const u16 sTMRewards[] = {
    ITEM_TM_EARTHQUAKE,
    ITEM_TM_ICE_BEAM,
    ITEM_TM_THUNDERBOLT,
    ITEM_TM_FLAMETHROWER,
    ITEM_TM_PSYCHIC,
    ITEM_TM_SHADOW_BALL,
    ITEM_TM_CALM_MIND,
    ITEM_TM_TOXIC,
    ITEM_TM_PROTECT,
    ITEM_TM_RETURN,
    ITEM_TM_BRICK_BREAK,
    ITEM_TM_AERIAL_ACE,
    ITEM_TM_DRAGON_CLAW,
    ITEM_TM_IRON_TAIL,
    ITEM_TM_SLUDGE_BOMB,
    ITEM_TM_OVERHEAT,
};

#define NUM_TM_REWARDS ARRAY_COUNT(sTMRewards)

// Trainer classes used for enemy trainers at each tier
static const u8 sTrainerClasses[][4] = {
    [ROGUE_GYM_TIER_BEGINNER]     = {FACILITY_CLASS_YOUNGSTER, FACILITY_CLASS_LASS, FACILITY_CLASS_BUG_CATCHER, FACILITY_CLASS_SCHOOL_KID_M},
    [ROGUE_GYM_TIER_INTERMEDIATE] = {FACILITY_CLASS_HIKER, FACILITY_CLASS_BEAUTY, FACILITY_CLASS_FISHERMAN, FACILITY_CLASS_GUITARIST},
    [ROGUE_GYM_TIER_ADVANCED]     = {FACILITY_CLASS_COOLTRAINER_M, FACILITY_CLASS_COOLTRAINER_F, FACILITY_CLASS_EXPERT_M, FACILITY_CLASS_EXPERT_F},
    [ROGUE_GYM_TIER_EXPERT]       = {FACILITY_CLASS_DRAGON_TAMER, FACILITY_CLASS_PSYCHIC_M, FACILITY_CLASS_PKMN_RANGER_M, FACILITY_CLASS_PKMN_RANGER_F},
    [ROGUE_GYM_TIER_CHAMPION]     = {FACILITY_CLASS_CHAMPION_WALLACE, FACILITY_CLASS_STEVEN, FACILITY_CLASS_ELITE_FOUR_SIDNEY, FACILITY_CLASS_ELITE_FOUR_PHOEBE},
};

// Level bonus per tier
static const u8 sLevelBonus[ROGUE_GYM_NUM_TIERS] = {
    [ROGUE_GYM_TIER_BEGINNER]     = ROGUE_GYM_LEVEL_BONUS_TIER0,
    [ROGUE_GYM_TIER_INTERMEDIATE] = ROGUE_GYM_LEVEL_BONUS_TIER1,
    [ROGUE_GYM_TIER_ADVANCED]     = ROGUE_GYM_LEVEL_BONUS_TIER2,
    [ROGUE_GYM_TIER_EXPERT]       = ROGUE_GYM_LEVEL_BONUS_TIER3,
    [ROGUE_GYM_TIER_CHAMPION]     = ROGUE_GYM_LEVEL_BONUS_TIER4,
};

// Min/max enemy team sizes per tier
static const u8 sEnemyTeamSizes[ROGUE_GYM_NUM_TIERS][2] = {
    [ROGUE_GYM_TIER_BEGINNER]     = {ROGUE_GYM_ENEMY_SIZE_TIER0_MIN, ROGUE_GYM_ENEMY_SIZE_TIER0_MAX},
    [ROGUE_GYM_TIER_INTERMEDIATE] = {ROGUE_GYM_ENEMY_SIZE_TIER1_MIN, ROGUE_GYM_ENEMY_SIZE_TIER1_MAX},
    [ROGUE_GYM_TIER_ADVANCED]     = {ROGUE_GYM_ENEMY_SIZE_TIER2_MIN, ROGUE_GYM_ENEMY_SIZE_TIER2_MAX},
    [ROGUE_GYM_TIER_EXPERT]       = {ROGUE_GYM_ENEMY_SIZE_TIER3_MIN, ROGUE_GYM_ENEMY_SIZE_TIER3_MAX},
    [ROGUE_GYM_TIER_CHAMPION]     = {ROGUE_GYM_ENEMY_SIZE_TIER4_MIN, ROGUE_GYM_ENEMY_SIZE_TIER4_MAX},
};

// Frontier mon ranges per tier [min, max]
static const u16 sMonRanges[ROGUE_GYM_NUM_TIERS][2] = {
    [ROGUE_GYM_TIER_BEGINNER]     = {ROGUE_GYM_MON_RANGE_TIER0_MIN, ROGUE_GYM_MON_RANGE_TIER0_MAX},
    [ROGUE_GYM_TIER_INTERMEDIATE] = {ROGUE_GYM_MON_RANGE_TIER1_MIN, ROGUE_GYM_MON_RANGE_TIER1_MAX},
    [ROGUE_GYM_TIER_ADVANCED]     = {ROGUE_GYM_MON_RANGE_TIER2_MIN, ROGUE_GYM_MON_RANGE_TIER2_MAX},
    [ROGUE_GYM_TIER_EXPERT]       = {ROGUE_GYM_MON_RANGE_TIER3_MIN, ROGUE_GYM_MON_RANGE_TIER3_MAX},
    [ROGUE_GYM_TIER_CHAMPION]     = {ROGUE_GYM_MON_RANGE_TIER4_MIN, ROGUE_GYM_MON_RANGE_TIER4_MAX},
};

// ==================== Utility Functions ====================

static struct RogueGymData *GetRogueGymData(void)
{
    return &gSaveBlock2Ptr->frontier.rogueGym;
}

static void AllocTemp(void)
{
    if (gRogueGymTemp == NULL)
        gRogueGymTemp = AllocZeroed(sizeof(struct RogueGymTemp));
}

static void FreeTemp(void)
{
    if (gRogueGymTemp != NULL)
    {
        Free(gRogueGymTemp);
        gRogueGymTemp = NULL;
    }
}

u8 RogueGym_GetTierForBattle(u8 battleNum)
{
    if (battleNum < ROGUE_GYM_TIER1_START)
        return ROGUE_GYM_TIER_BEGINNER;
    else if (battleNum < ROGUE_GYM_TIER2_START)
        return ROGUE_GYM_TIER_INTERMEDIATE;
    else if (battleNum < ROGUE_GYM_TIER3_START)
        return ROGUE_GYM_TIER_ADVANCED;
    else if (battleNum < ROGUE_GYM_TIER4_START)
        return ROGUE_GYM_TIER_EXPERT;
    else
        return ROGUE_GYM_TIER_CHAMPION;
}

u8 RogueGym_GetMaxTeamSize(u8 battleNum)
{
    if (battleNum < ROGUE_GYM_MAX_TEAM_BATTLE_2)
        return 1;
    else if (battleNum < ROGUE_GYM_MAX_TEAM_BATTLE_3)
        return 2;
    else if (battleNum < ROGUE_GYM_MAX_TEAM_BATTLE_4)
        return 3;
    else if (battleNum < ROGUE_GYM_MAX_TEAM_BATTLE_5)
        return 4;
    else if (battleNum < ROGUE_GYM_MAX_TEAM_BATTLE_6)
        return 5;
    else
        return 6;
}

u8 RogueGym_GetPlayerAvgLevel(void)
{
    u32 totalLevel = 0;
    u8 count = 0;
    u8 i;

    for (i = 0; i < PARTY_SIZE; i++)
    {
        if (GetMonData(&gPlayerParty[i], MON_DATA_SPECIES) != SPECIES_NONE)
        {
            totalLevel += GetMonData(&gPlayerParty[i], MON_DATA_LEVEL);
            count++;
        }
    }

    if (count == 0)
        return ROGUE_GYM_STARTER_LEVEL;

    return totalLevel / count;
}

bool8 RogueGym_IsActive(void)
{
    return GetRogueGymData()->status == ROGUE_GYM_STATUS_ACTIVE;
}

u8 RogueGym_GetPlayerMonCount(void)
{
    u8 count = 0;
    u8 i;

    for (i = 0; i < PARTY_SIZE; i++)
    {
        if (GetMonData(&gPlayerParty[i], MON_DATA_SPECIES) != SPECIES_NONE)
            count++;
    }
    return count;
}

void RogueGym_AddItem(u16 itemId, u8 quantity)
{
    struct RogueGymData *data = GetRogueGymData();
    u8 i;

    // Check if we already have this item, stack it
    for (i = 0; i < data->numItems; i++)
    {
        if (data->items[i] == itemId)
        {
            data->itemQuantities[i] += quantity;
            return;
        }
    }

    // Add new item if room
    if (data->numItems < ROGUE_GYM_MAX_ITEMS)
    {
        data->items[data->numItems] = itemId;
        data->itemQuantities[data->numItems] = quantity;
        data->numItems++;
    }
}

bool8 RogueGym_HasItem(u16 itemId)
{
    struct RogueGymData *data = GetRogueGymData();
    u8 i;

    for (i = 0; i < data->numItems; i++)
    {
        if (data->items[i] == itemId && data->itemQuantities[i] > 0)
            return TRUE;
    }
    return FALSE;
}

static void RogueGym_RemoveItem(u8 index)
{
    struct RogueGymData *data = GetRogueGymData();

    if (index >= data->numItems)
        return;

    data->itemQuantities[index]--;
    if (data->itemQuantities[index] == 0)
    {
        // Shift remaining items down
        u8 i;
        for (i = index; i < data->numItems - 1; i++)
        {
            data->items[i] = data->items[i + 1];
            data->itemQuantities[i] = data->itemQuantities[i + 1];
        }
        data->items[data->numItems - 1] = ITEM_NONE;
        data->itemQuantities[data->numItems - 1] = 0;
        data->numItems--;
    }
}

// Get a random species suitable as a starter (basic stage, reasonable BST)
static u16 GetRandomStarterSpecies(void)
{
    u16 species;
    u16 bst;
    u32 attempts = 0;

    do
    {
        species = (Random() % (NUM_SPECIES - 1)) + 1; // 1 to NUM_SPECIES-1
        attempts++;

        // Skip invalid species
        if (gSpeciesInfo[species].baseHP == 0)
            continue;

        // Skip legendaries, mythicals, megas, etc.
        if (gSpeciesInfo[species].isRestrictedLegendary
            || gSpeciesInfo[species].isSubLegendary
            || gSpeciesInfo[species].isMythical
            || gSpeciesInfo[species].isUltraBeast
            || gSpeciesInfo[species].isMegaEvolution
            || gSpeciesInfo[species].isPrimalReversion
            || gSpeciesInfo[species].isGigantamax
            || gSpeciesInfo[species].isFrontierBanned)
            continue;

        // Calculate base stat total
        bst = gSpeciesInfo[species].baseHP
            + gSpeciesInfo[species].baseAttack
            + gSpeciesInfo[species].baseDefense
            + gSpeciesInfo[species].baseSpeed
            + gSpeciesInfo[species].baseSpAttack
            + gSpeciesInfo[species].baseSpDefense;

        // Starters should be basic Pokemon with moderate BST (250-450)
        // Not too weak, not too strong
        if (bst >= 250 && bst <= 450)
        {
            // Prefer Pokemon that can evolve (have evolutions defined)
            // But don't require it - some standalone mons are fun too
            return species;
        }

    } while (attempts < 5000);

    // Fallback: just return a classic starter
    return SPECIES_TORCHIC;
}

// Get a random Pokemon for rewards (appropriate to current tier)
static u16 GetRandomRewardSpecies(u8 tier)
{
    u16 species;
    u16 bst;
    u16 minBst, maxBst;
    u32 attempts = 0;

    // BST ranges per tier for reward Pokemon
    switch (tier)
    {
    case ROGUE_GYM_TIER_BEGINNER:
        minBst = 250; maxBst = 400;
        break;
    case ROGUE_GYM_TIER_INTERMEDIATE:
        minBst = 350; maxBst = 500;
        break;
    case ROGUE_GYM_TIER_ADVANCED:
        minBst = 400; maxBst = 550;
        break;
    case ROGUE_GYM_TIER_EXPERT:
        minBst = 450; maxBst = 600;
        break;
    case ROGUE_GYM_TIER_CHAMPION:
    default:
        minBst = 500; maxBst = 700;
        break;
    }

    do
    {
        species = (Random() % (NUM_SPECIES - 1)) + 1;
        attempts++;

        if (gSpeciesInfo[species].baseHP == 0)
            continue;

        if (gSpeciesInfo[species].isMegaEvolution
            || gSpeciesInfo[species].isPrimalReversion
            || gSpeciesInfo[species].isGigantamax
            || gSpeciesInfo[species].isFrontierBanned)
            continue;

        // Allow sub-legendaries in expert+ tiers
        if (tier < ROGUE_GYM_TIER_EXPERT)
        {
            if (gSpeciesInfo[species].isRestrictedLegendary
                || gSpeciesInfo[species].isSubLegendary
                || gSpeciesInfo[species].isMythical
                || gSpeciesInfo[species].isUltraBeast)
                continue;
        }
        else
        {
            // Still no box legendaries or mythicals
            if (gSpeciesInfo[species].isRestrictedLegendary
                || gSpeciesInfo[species].isMythical)
                continue;
        }

        bst = gSpeciesInfo[species].baseHP
            + gSpeciesInfo[species].baseAttack
            + gSpeciesInfo[species].baseDefense
            + gSpeciesInfo[species].baseSpeed
            + gSpeciesInfo[species].baseSpAttack
            + gSpeciesInfo[species].baseSpDefense;

        if (bst >= minBst && bst <= maxBst)
        {
            // Check it's not already in the player's party
            u8 i;
            bool8 duplicate = FALSE;
            for (i = 0; i < PARTY_SIZE; i++)
            {
                if (GetMonData(&gPlayerParty[i], MON_DATA_SPECIES) == species)
                {
                    duplicate = TRUE;
                    break;
                }
            }
            if (!duplicate)
                return species;
        }
    } while (attempts < 5000);

    // Fallback
    return SPECIES_EEVEE;
}

// ==================== Main Entry Point ====================

void CallRogueGymFunction(void)
{
    if (gSpecialVar_0x8004 < ROGUE_GYM_FUNC_COUNT)
        sRogueGymFunctions[gSpecialVar_0x8004]();
}

// ==================== Function Implementations ====================

static void RogueGym_InitFunc(void)
{
    struct RogueGymData *data = GetRogueGymData();

    // Clear run data
    memset(data, 0, sizeof(struct RogueGymData));
    data->status = ROGUE_GYM_STATUS_ACTIVE;
    data->currentBattle = 0;
    data->tier = ROGUE_GYM_TIER_BEGINNER;
    data->seed = Random();

    // Clear the player party for the roguelike run
    ZeroPlayerPartyMons();

    // Allocate temp working data
    AllocTemp();

    // Give the player a starting Potion
    RogueGym_AddItem(ITEM_POTION, 2);
    RogueGym_AddItem(ITEM_ETHER, 1);

    gSpecialVar_Result = TRUE;
}

static void RogueGym_GenerateStartersFunc(void)
{
    u8 i, j;
    bool8 duplicate;

    AllocTemp();

    // Generate 3 unique random starters
    for (i = 0; i < ROGUE_GYM_NUM_STARTER_CHOICES; i++)
    {
        do
        {
            duplicate = FALSE;
            gRogueGymTemp->starterChoices[i] = GetRandomStarterSpecies();

            for (j = 0; j < i; j++)
            {
                if (gRogueGymTemp->starterChoices[j] == gRogueGymTemp->starterChoices[i])
                {
                    duplicate = TRUE;
                    break;
                }
            }
        } while (duplicate);
    }

    // Store choices in script vars for display
    gSpecialVar_0x8005 = gRogueGymTemp->starterChoices[0];
    gSpecialVar_0x8006 = gRogueGymTemp->starterChoices[1];
    gSpecialVar_Result = gRogueGymTemp->starterChoices[2];
}

static void RogueGym_PickStarterFunc(void)
{
    u8 choice = gSpecialVar_0x8005; // 0, 1, or 2
    u16 species;
    struct RogueGymData *data = GetRogueGymData();

    AllocTemp();

    if (choice >= ROGUE_GYM_NUM_STARTER_CHOICES)
        choice = 0;

    species = gRogueGymTemp->starterChoices[choice];

    // Create the starter Pokemon at level 5
    CreateRandomMon(&gPlayerParty[0], species, ROGUE_GYM_STARTER_LEVEL);
    GiveMonInitialMoveset(&gPlayerParty[0]);

    // Record in save data
    data->currentBattle = 1;

    gSpecialVar_Result = species;
}

static void RogueGym_SetupNextBattleFunc(void)
{
    struct RogueGymData *data = GetRogueGymData();
    u8 tier = RogueGym_GetTierForBattle(data->currentBattle);
    u8 avgLevel = RogueGym_GetPlayerAvgLevel();
    u8 enemyLevel;
    u8 enemyTeamSize;
    u8 fixedIV;
    u8 i, j;
    u16 monId;
    u16 minRange, maxRange, range;
    u32 otID;
    u16 usedSpecies[PARTY_SIZE];
    u16 usedItems[PARTY_SIZE];

    AllocTemp();

    data->tier = tier;
    enemyLevel = avgLevel + sLevelBonus[tier];
    if (enemyLevel > MAX_LEVEL)
        enemyLevel = MAX_LEVEL;

    // Determine enemy team size
    {
        u8 minSize = sEnemyTeamSizes[tier][0];
        u8 maxSize = sEnemyTeamSizes[tier][1];
        enemyTeamSize = minSize + (Random() % (maxSize - minSize + 1));
    }

    // Set fixed IVs based on tier (0-31 scale)
    switch (tier)
    {
    case ROGUE_GYM_TIER_BEGINNER:     fixedIV = 3;  break;
    case ROGUE_GYM_TIER_INTERMEDIATE: fixedIV = 9;  break;
    case ROGUE_GYM_TIER_ADVANCED:     fixedIV = 15; break;
    case ROGUE_GYM_TIER_EXPERT:       fixedIV = 21; break;
    case ROGUE_GYM_TIER_CHAMPION:     fixedIV = 31; break;
    default:                          fixedIV = 3;  break;
    }

    // Pick trainer class
    gRogueGymTemp->enemyTrainerClass = sTrainerClasses[tier][Random() % 4];
    gRogueGymTemp->enemyTeamSize = enemyTeamSize;

    // Generate enemy team from frontier mons pool
    ZeroEnemyPartyMons();
    minRange = sMonRanges[tier][0];
    maxRange = sMonRanges[tier][1];
    range = maxRange - minRange + 1;

    for (i = 0; i < PARTY_SIZE; i++)
    {
        usedSpecies[i] = SPECIES_NONE;
        usedItems[i] = ITEM_NONE;
    }

    otID = Random32();
    i = 0;
    while (i < enemyTeamSize)
    {
        bool8 duplicate = FALSE;

        monId = minRange + (Random() % range);

        // Skip Unown
        if (gBattleFrontierMons[monId].species == SPECIES_UNOWN)
            continue;

        // Check for duplicate species
        for (j = 0; j < i; j++)
        {
            if (usedSpecies[j] == gBattleFrontierMons[monId].species)
            {
                duplicate = TRUE;
                break;
            }
        }
        if (duplicate)
            continue;

        // Check for duplicate held items
        for (j = 0; j < i; j++)
        {
            if (usedItems[j] != ITEM_NONE && usedItems[j] == gBattleFrontierMons[monId].heldItem)
            {
                duplicate = TRUE;
                break;
            }
        }
        if (duplicate)
            continue;

        usedSpecies[i] = gBattleFrontierMons[monId].species;
        usedItems[i] = gBattleFrontierMons[monId].heldItem;
        gRogueGymTemp->enemySpecies[i] = gBattleFrontierMons[monId].species;
        gRogueGymTemp->enemyLevels[i] = enemyLevel;

        // Create the enemy mon using the frontier mon data
        CreateFacilityMon(&gBattleFrontierMons[monId], enemyLevel, fixedIV, otID, 0, &gEnemyParty[i]);

        i++;
    }

    // Set script vars for the battle system
    // VAR_RESULT = enemy team size for script to reference
    gSpecialVar_Result = enemyTeamSize;
}

static void RogueGym_ProcessBattleWinFunc(void)
{
    struct RogueGymData *data = GetRogueGymData();

    // Count defeated enemy mons
    u8 i;
    for (i = 0; i < PARTY_SIZE; i++)
    {
        u16 species = GetMonData(&gEnemyParty[i], MON_DATA_SPECIES);
        if (species != SPECIES_NONE)
            data->totalDefeated++;
    }

    // Give a free random healing item after each win
    {
        u8 tier = data->tier;
        u16 freeItem = sHealingRewards[tier][Random() % 3];
        RogueGym_AddItem(freeItem, 1);
    }

    gSpecialVar_Result = data->currentBattle;
}

static void RogueGym_ProcessFaintedFunc(void)
{
    u8 i;
    u8 faintedCount = 0;

    // Check each party member - if fainted, remove permanently (nuzlocke)
    for (i = 0; i < PARTY_SIZE; i++)
    {
        u16 species = GetMonData(&gPlayerParty[i], MON_DATA_SPECIES);
        if (species != SPECIES_NONE)
        {
            u16 hp = GetMonData(&gPlayerParty[i], MON_DATA_HP);
            if (hp == 0)
            {
                // This Pokemon has fallen - remove it permanently
                ZeroMonData(&gPlayerParty[i]);
                faintedCount++;
            }
        }
    }

    // Compact the party (remove gaps)
    if (faintedCount > 0)
    {
        u8 writeIdx = 0;
        struct Pokemon tempParty[PARTY_SIZE];

        memcpy(tempParty, gPlayerParty, sizeof(gPlayerParty));
        ZeroPlayerPartyMons();

        for (i = 0; i < PARTY_SIZE; i++)
        {
            if (GetMonData(&tempParty[i], MON_DATA_SPECIES) != SPECIES_NONE)
            {
                memcpy(&gPlayerParty[writeIdx], &tempParty[i], sizeof(struct Pokemon));
                writeIdx++;
            }
        }
    }

    // Return number of fainted Pokemon
    gSpecialVar_Result = faintedCount;
}

static void RogueGym_CheckGameOverFunc(void)
{
    struct RogueGymData *data = GetRogueGymData();
    u8 aliveCount = RogueGym_GetPlayerMonCount();

    if (aliveCount == 0)
    {
        // Game over!
        data->status = ROGUE_GYM_STATUS_LOST;

        // Update high score
        if (data->currentBattle > data->highScore)
            data->highScore = data->currentBattle;

        gSpecialVar_Result = TRUE; // Game over
    }
    else if (data->currentBattle > ROGUE_GYM_MAX_BATTLES)
    {
        // Victory! Completed all battles
        data->status = ROGUE_GYM_STATUS_WON;

        if (data->currentBattle > data->highScore)
            data->highScore = data->currentBattle;

        gSpecialVar_Result = 2; // Victory
    }
    else
    {
        gSpecialVar_Result = FALSE; // Continue playing
    }
}

static void RogueGym_GenerateRewardsFunc(void)
{
    struct RogueGymData *data = GetRogueGymData();
    u8 tier = data->tier;
    u8 battleNum = data->currentBattle;
    u8 maxTeam = RogueGym_GetMaxTeamSize(battleNum);
    u8 currentTeam = RogueGym_GetPlayerMonCount();
    u8 i;
    u8 rewardTypes[ROGUE_GYM_NUM_REWARD_CHOICES];
    bool8 pokemonOffered = FALSE;

    AllocTemp();

    // Determine reward types
    // If team can grow and it's time for a new mon, guarantee one Pokemon reward
    if (currentTeam < maxTeam)
    {
        rewardTypes[0] = ROGUE_REWARD_POKEMON;
        pokemonOffered = TRUE;

        // Fill remaining slots randomly
        for (i = 1; i < ROGUE_GYM_NUM_REWARD_CHOICES; i++)
        {
            u8 roll = Random() % 5;
            switch (roll)
            {
            case 0: rewardTypes[i] = ROGUE_REWARD_HEALING; break;
            case 1: rewardTypes[i] = ROGUE_REWARD_PP_RESTORE; break;
            case 2: rewardTypes[i] = ROGUE_REWARD_TM; break;
            case 3: rewardTypes[i] = ROGUE_REWARD_HELD_ITEM; break;
            case 4: rewardTypes[i] = ROGUE_REWARD_RARE_CANDY; break;
            }
        }
    }
    else
    {
        // Random rewards, no Pokemon since team is full for this tier
        for (i = 0; i < ROGUE_GYM_NUM_REWARD_CHOICES; i++)
        {
            u8 roll = Random() % 6;
            switch (roll)
            {
            case 0: rewardTypes[i] = ROGUE_REWARD_HEALING; break;
            case 1: rewardTypes[i] = ROGUE_REWARD_PP_RESTORE; break;
            case 2: rewardTypes[i] = ROGUE_REWARD_TM; break;
            case 3: rewardTypes[i] = ROGUE_REWARD_HELD_ITEM; break;
            case 4: rewardTypes[i] = ROGUE_REWARD_RARE_CANDY; break;
            case 5:
                // Sometimes offer Pokemon even when at max for current tier
                // (they still have room in party up to 6)
                if (currentTeam < PARTY_SIZE && (Random() % 4 == 0))
                    rewardTypes[i] = ROGUE_REWARD_POKEMON;
                else
                    rewardTypes[i] = ROGUE_REWARD_HEALING;
                break;
            }
        }
    }

    // Try to avoid all 3 being the same type
    if (rewardTypes[0] == rewardTypes[1] && rewardTypes[1] == rewardTypes[2] && !pokemonOffered)
    {
        rewardTypes[2] = (rewardTypes[0] + 1 + (Random() % (ROGUE_REWARD_TYPE_COUNT - 1))) % ROGUE_REWARD_TYPE_COUNT;
    }

    // Generate specific rewards
    for (i = 0; i < ROGUE_GYM_NUM_REWARD_CHOICES; i++)
    {
        gRogueGymTemp->rewardChoices[i].type = rewardTypes[i];

        switch (rewardTypes[i])
        {
        case ROGUE_REWARD_POKEMON:
            gRogueGymTemp->rewardChoices[i].itemOrMon = GetRandomRewardSpecies(tier);
            gRogueGymTemp->rewardChoices[i].quantity = 1;
            break;

        case ROGUE_REWARD_HEALING:
            gRogueGymTemp->rewardChoices[i].itemOrMon = sHealingRewards[tier][Random() % 3];
            gRogueGymTemp->rewardChoices[i].quantity = 1;
            break;

        case ROGUE_REWARD_PP_RESTORE:
            gRogueGymTemp->rewardChoices[i].itemOrMon = sPPRewards[tier][Random() % 3];
            gRogueGymTemp->rewardChoices[i].quantity = 1;
            break;

        case ROGUE_REWARD_TM:
            gRogueGymTemp->rewardChoices[i].itemOrMon = sTMRewards[Random() % NUM_TM_REWARDS];
            gRogueGymTemp->rewardChoices[i].quantity = 1;
            break;

        case ROGUE_REWARD_HELD_ITEM:
            gRogueGymTemp->rewardChoices[i].itemOrMon = sHeldItemRewards[Random() % NUM_HELD_ITEM_REWARDS];
            gRogueGymTemp->rewardChoices[i].quantity = 1;
            break;

        case ROGUE_REWARD_RARE_CANDY:
            gRogueGymTemp->rewardChoices[i].itemOrMon = ITEM_RARE_CANDY;
            gRogueGymTemp->rewardChoices[i].quantity = 1;
            break;
        }
    }

    // Store first reward info in script vars for display
    gSpecialVar_0x8005 = gRogueGymTemp->rewardChoices[0].type;
    gSpecialVar_0x8006 = gRogueGymTemp->rewardChoices[0].itemOrMon;
    gSpecialVar_Result = ROGUE_GYM_NUM_REWARD_CHOICES;
}

static void RogueGym_PickRewardFunc(void)
{
    u8 choice = gSpecialVar_0x8005; // 0, 1, or 2
    struct RogueGymData *data = GetRogueGymData();
    struct RogueGymReward *reward;

    AllocTemp();

    if (choice >= ROGUE_GYM_NUM_REWARD_CHOICES)
        choice = 0;

    reward = &gRogueGymTemp->rewardChoices[choice];

    switch (reward->type)
    {
    case ROGUE_REWARD_POKEMON:
    {
        // Add a new Pokemon to the party
        u8 slot = RogueGym_GetPlayerMonCount();
        if (slot < PARTY_SIZE)
        {
            u8 level = RogueGym_GetPlayerAvgLevel();
            CreateRandomMon(&gPlayerParty[slot], reward->itemOrMon, level);
            GiveMonInitialMoveset(&gPlayerParty[slot]);
        }
        break;
    }

    case ROGUE_REWARD_HEALING:
    case ROGUE_REWARD_PP_RESTORE:
    case ROGUE_REWARD_TM:
    case ROGUE_REWARD_HELD_ITEM:
    case ROGUE_REWARD_RARE_CANDY:
        // Add item to roguelike inventory
        RogueGym_AddItem(reward->itemOrMon, reward->quantity);
        break;
    }

    // Advance to next battle
    data->currentBattle++;

    gSpecialVar_Result = reward->type;
}

static void RogueGym_GetStatusFunc(void)
{
    gSpecialVar_Result = GetRogueGymData()->status;
}

static void RogueGym_GetBattleNumFunc(void)
{
    gSpecialVar_Result = GetRogueGymData()->currentBattle;
}

static void RogueGym_GetTierFunc(void)
{
    gSpecialVar_Result = GetRogueGymData()->tier;
}

static void RogueGym_UseItemFunc(void)
{
    u8 itemIndex = gSpecialVar_0x8005;
    u8 partySlot = gSpecialVar_0x8006;
    struct RogueGymData *data = GetRogueGymData();
    u16 itemId;
    u16 species;
    u16 maxHp, curHp, newHp;
    u32 status;
    u8 level;

    if (itemIndex >= data->numItems || partySlot >= PARTY_SIZE)
    {
        gSpecialVar_Result = FALSE;
        return;
    }

    itemId = data->items[itemIndex];
    species = GetMonData(&gPlayerParty[partySlot], MON_DATA_SPECIES);

    if (species == SPECIES_NONE)
    {
        gSpecialVar_Result = FALSE;
        return;
    }

    switch (itemId)
    {
    case ITEM_POTION:
        maxHp = GetMonData(&gPlayerParty[partySlot], MON_DATA_MAX_HP);
        curHp = GetMonData(&gPlayerParty[partySlot], MON_DATA_HP);
        newHp = curHp + 20;
        if (newHp > maxHp) newHp = maxHp;
        SetMonData(&gPlayerParty[partySlot], MON_DATA_HP, &newHp);
        break;
    case ITEM_SUPER_POTION:
        maxHp = GetMonData(&gPlayerParty[partySlot], MON_DATA_MAX_HP);
        curHp = GetMonData(&gPlayerParty[partySlot], MON_DATA_HP);
        newHp = curHp + 60;
        if (newHp > maxHp) newHp = maxHp;
        SetMonData(&gPlayerParty[partySlot], MON_DATA_HP, &newHp);
        break;
    case ITEM_HYPER_POTION:
        maxHp = GetMonData(&gPlayerParty[partySlot], MON_DATA_MAX_HP);
        curHp = GetMonData(&gPlayerParty[partySlot], MON_DATA_HP);
        newHp = curHp + 120;
        if (newHp > maxHp) newHp = maxHp;
        SetMonData(&gPlayerParty[partySlot], MON_DATA_HP, &newHp);
        break;
    case ITEM_FULL_RESTORE:
        maxHp = GetMonData(&gPlayerParty[partySlot], MON_DATA_MAX_HP);
        SetMonData(&gPlayerParty[partySlot], MON_DATA_HP, &maxHp);
        status = STATUS1_NONE;
        SetMonData(&gPlayerParty[partySlot], MON_DATA_STATUS, &status);
        break;
    case ITEM_MAX_REVIVE:
        // In nuzlocke mode this acts as a full restore for living mons
        maxHp = GetMonData(&gPlayerParty[partySlot], MON_DATA_MAX_HP);
        SetMonData(&gPlayerParty[partySlot], MON_DATA_HP, &maxHp);
        status = STATUS1_NONE;
        SetMonData(&gPlayerParty[partySlot], MON_DATA_STATUS, &status);
        break;
    case ITEM_ETHER:
    {
        u8 pp = GetMonData(&gPlayerParty[partySlot], MON_DATA_PP1);
        pp += 10;
        SetMonData(&gPlayerParty[partySlot], MON_DATA_PP1, &pp);
        break;
    }
    case ITEM_ELIXIR:
    {
        u8 j;
        for (j = 0; j < MAX_MON_MOVES; j++)
        {
            u8 pp = GetMonData(&gPlayerParty[partySlot], MON_DATA_PP1 + j);
            pp += 10;
            SetMonData(&gPlayerParty[partySlot], MON_DATA_PP1 + j, &pp);
        }
        break;
    }
    case ITEM_MAX_ELIXIR:
    {
        u8 j;
        for (j = 0; j < MAX_MON_MOVES; j++)
        {
            u16 move = GetMonData(&gPlayerParty[partySlot], MON_DATA_MOVE1 + j);
            if (move != MOVE_NONE)
            {
                u8 maxPP = gMovesInfo[move].pp;
                SetMonData(&gPlayerParty[partySlot], MON_DATA_PP1 + j, &maxPP);
            }
        }
        break;
    }
    case ITEM_RARE_CANDY:
        level = GetMonData(&gPlayerParty[partySlot], MON_DATA_LEVEL);
        if (level < MAX_LEVEL)
        {
            level++;
            SetMonData(&gPlayerParty[partySlot], MON_DATA_LEVEL, &level);
            CalculateMonStats(&gPlayerParty[partySlot]);
        }
        break;
    default:
        // For held items and TMs, handle differently
        // Check if it's a held item (not a consumable)
        if (gItemsInfo[itemId].pocket == POCKET_TM_HM)
        {
            // TM - don't consume here, teaching is handled by script
            gSpecialVar_Result = TRUE;
            return; // Don't remove the item yet
        }
        else
        {
            // Held item - give to Pokemon
            SetMonData(&gPlayerParty[partySlot], MON_DATA_HELD_ITEM, &itemId);
        }
        break;
    }

    // Remove the used item from inventory
    RogueGym_RemoveItem(itemIndex);
    gSpecialVar_Result = TRUE;
}

static void RogueGym_SaveFunc(void)
{
    // The save is handled by the script system
    // This just ensures our data is in a consistent state
    gSpecialVar_Result = TRUE;
}

static void RogueGym_CleanupFunc(void)
{
    struct RogueGymData *data = GetRogueGymData();

    // Keep high score, clear run data
    u16 highScore = data->highScore;
    memset(data, 0, sizeof(struct RogueGymData));
    data->highScore = highScore;
    data->status = ROGUE_GYM_STATUS_NONE;

    FreeTemp();

    gSpecialVar_Result = TRUE;
}
