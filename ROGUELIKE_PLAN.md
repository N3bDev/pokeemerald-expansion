# Roguelike Gym Trainer Mode - Implementation Plan

## Overview
A new roguelike game mode where the player is a Gym Trainer defending their gym against
progressively stronger challengers. Uses the Battle Frontier area as the setting.
Nuzlocke rules: fainted Pokemon are gone forever. Game over when all Pokemon faint.

## Architecture

### Phase 1: Core Data Structures & Constants
**Files to create/modify:**
- `include/constants/rogue_gym.h` - Constants (tiers, max battles, reward types, etc.)
- `include/rogue_gym.h` - Header for the rogue gym system
- `src/rogue_gym.c` - Main roguelike gym logic

**New save data** (added to `struct BattleFrontier` in `include/global.h`):
```c
struct RogueGymData {
    u8 active;                    // Is a run in progress?
    u8 currentBattle;             // Current battle number (0-49)
    u8 tier;                      // Current difficulty tier (0-4)
    u8 teamSize;                  // How many Pokemon the player currently has
    u16 starterSpecies;           // Which starter was picked
    u16 seed;                     // Run seed for reproducibility
    u8 rewardsPending;            // Rewards to choose
    u16 itemBag[20];              // Roguelike item inventory (potions, ethers, TMs, held items)
    u8 itemQuantities[20];        // Quantities for consumables
    u8 numItems;                  // Number of distinct items
    u16 defeatedCount;            // Total Pokemon defeated
    u8 pokemonFaintedFlags;       // Bitmask of which party slots have been permanently lost
};
```

### Phase 2: Pokemon Generation & Starter Selection
**Core functions in `src/rogue_gym.c`:**
- `RogueGym_GenerateStarterChoices()` - Roll 3 random Pokemon from all species (basic stage only, reasonable BST for starters). Store in temp vars for the selection screen.
- `RogueGym_CreateStarterMon()` - Create the chosen Pokemon at level 5 with appropriate starting moves.
- `RogueGym_GenerateEnemyTeam()` - Generate opponent team based on current tier:
  - **Tier 0 (Battles 1-5):** 1-2 Pokemon, basic stages, random movesets, level = player level
  - **Tier 1 (Battles 6-12):** 2-3 Pokemon, can include stage 1 evolutions, level = player+1
  - **Tier 2 (Battles 13-20):** 3-4 Pokemon, any evolution stage, decent moves, level = player+2
  - **Tier 3 (Battles 21-30):** 4-5 Pokemon, good movesets + held items, level = player+2
  - **Tier 4 (Battles 31+):** Full team of 6, competitive movesets from frontier mons pool, level = player+3, EVs

We leverage the existing `gBattleFrontierMons[NUM_FRONTIER_MONS]` (882 mons!) for higher tiers - these already have competitive movesets, held items, EVs, and natures defined.

### Phase 3: Reward System
**After each battle, player picks 1 of 3 random rewards:**
- `RogueGym_GenerateRewards()` - Creates 3 reward options weighted by tier
- Reward types:
  - `ROGUE_REWARD_POKEMON` - New Pokemon to add to team (offered every 3-4 battles, or when team < max for tier)
  - `ROGUE_REWARD_HEALING` - Potion/Super Potion/Hyper Potion/Full Restore (scales with tier)
  - `ROGUE_REWARD_PP_RESTORE` - Ether/Elixir/Max Elixir
  - `ROGUE_REWARD_TM` - Random TM to teach between battles
  - `ROGUE_REWARD_HELD_ITEM` - Battle items (Leftovers, Choice Band, Life Orb, etc.)
  - `ROGUE_REWARD_RARE_CANDY` - Level up one Pokemon

**Team growth schedule:**
- Battles 1-4: Max team of 1
- Battles 5-8: Max team of 2
- Battles 9-13: Max team of 3
- Battles 14-18: Max team of 4
- Battles 19-24: Max team of 5
- Battles 25+: Max team of 6

Every time the max grows, one of the 3 rewards is guaranteed to be a new Pokemon.

### Phase 4: Between-Battle Menu
A custom menu screen (similar to Battle Factory's swap screen) allowing:
- View team summary
- Use healing items from roguelike inventory
- Use PP restores
- Teach TMs collected as rewards
- Swap held items between Pokemon
- View next opponent info? (maybe show trainer class but not team)
- Continue to next battle

### Phase 5: Nuzlocke & Battle Integration
- Hook into battle end processing to check for fainted Pokemon
- `RogueGym_ProcessBattleResult()`:
  - On WIN: increment battle count, check tier advancement, generate rewards
  - After rewards: auto-save, set up next battle
  - On LOSS (all Pokemon fainted): Game Over screen, record stats, return to lobby
- Fainted Pokemon handling:
  - After each battle, check party for fainted mons
  - Permanently remove fainted Pokemon (set species to NONE, shift party)
  - Display message "X has fallen in battle..."
  - If all Pokemon gone = game over even if battle was "won"

### Phase 6: Map Scripts & Entry Point
**Reuse Battle Frontier Battle Tower map** (or create a Gym-themed variant):
- New NPC in Battle Frontier lobby area to start Roguelike Gym mode
- Map scripts handle the battle loop:
  1. Enter gym → starter selection (first time)
  2. Pre-battle dialogue from challenger
  3. Battle
  4. Post-battle: remove fainted mons, check game over
  5. Reward selection screen
  6. Between-battle menu (items/TMs/management)
  7. Loop back to step 2

**Script flow** (in `data/maps/BattleFrontier_BattleTowerLobby/scripts.inc` or new map):
```
RogueGym_Start:
    → Set facility vars
    → Call RogueGym_Init (C function)
    → Show starter selection
    → Save
    → Enter battle loop

RogueGym_BattleLoop:
    → Generate opponent (C function)
    → Set trainer data
    → Challenger walks in + dialogue
    → Start battle
    → Check result
    → If lost → Game Over
    → Process fainted mons
    → If no mons left → Game Over
    → Show rewards screen
    → Between-battle menu
    → Loop
```

### Phase 7: Enemy Trainer Variety
- Use existing facility trainer classes for visual variety
- Generate trainer names procedurally or pick from a pool
- Trainer dialogue scales with tier:
  - Tier 0: "I just started my Pokemon journey!"
  - Tier 1: "I've been training for a while now!"
  - Tier 2: "I hear this gym is pretty tough!"
  - Tier 3: "I've beaten 7 gyms already!"
  - Tier 4: "I'm a Pokemon Champion, prepare yourself!"

## Implementation Order

1. **Constants & headers** - Define all the constants and data structures
2. **Core C module** - `src/rogue_gym.c` with init, pokemon gen, enemy gen, reward gen
3. **Save data integration** - Add RogueGymData to save block
4. **Starter selection** - Reuse/adapt the party selection UI
5. **Battle loop** - Map scripts + C callbacks for the battle cycle
6. **Nuzlocke logic** - Post-battle faint processing
7. **Reward selection screen** - Pick 1 of 3 rewards UI
8. **Between-battle menu** - Item usage, TM teaching, team management
9. **Enemy scaling & variety** - Trainer classes, movesets, competitive teams
10. **Polish** - Game over screen, stats tracking, dialogue

## Key Design Decisions
- **Starting level:** 5 (allows growth throughout run)
- **Level scaling:** Enemy level = avg party level + tier bonus (0/1/2/2/3)
- **Max battles for full run:** ~40 (defeating the "Champion" tier challenger)
- **Experience:** Pokemon gain XP normally from battles
- **Auto-heal between battles:** NO - managing HP/PP is part of the strategy
- **Free heal per battle:** 1 random healing item added to inventory after each win
- **Location:** Battle Frontier Battle Tower (repurposed with dialogue)
