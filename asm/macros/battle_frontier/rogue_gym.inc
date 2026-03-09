	@ Initialize a new Roguelike Gym run
	.macro roguegym_init
	setvar VAR_0x8004, ROGUE_GYM_FUNC_INIT
	special CallRogueGymFunction
	.endm

	@ Generate 3 random starter choices. Species stored in VAR_0x8005, VAR_0x8006, VAR_RESULT
	.macro roguegym_generatestarters
	setvar VAR_0x8004, ROGUE_GYM_FUNC_GENERATE_STARTERS
	special CallRogueGymFunction
	.endm

	@ Pick a starter. Set VAR_0x8005 to the choice index (0-2) before calling
	.macro roguegym_pickstarter choice:req
	setvar VAR_0x8004, ROGUE_GYM_FUNC_PICK_STARTER
	setvar VAR_0x8005, \choice
	special CallRogueGymFunction
	.endm

	@ Set up the next battle (generates enemy team in gEnemyParty)
	.macro roguegym_setupbattle
	setvar VAR_0x8004, ROGUE_GYM_FUNC_SETUP_NEXT_BATTLE
	special CallRogueGymFunction
	.endm

	@ Process a battle win (count defeated, give free healing item)
	.macro roguegym_processwin
	setvar VAR_0x8004, ROGUE_GYM_FUNC_PROCESS_BATTLE_WIN
	special CallRogueGymFunction
	.endm

	@ Process fainted Pokemon (nuzlocke removal). VAR_RESULT = number of mons lost
	.macro roguegym_processfainted
	setvar VAR_0x8004, ROGUE_GYM_FUNC_PROCESS_FAINTED
	special CallRogueGymFunction
	.endm

	@ Check if game is over. VAR_RESULT: 0 = continue, 1 = lost, 2 = won
	.macro roguegym_checkgameover
	setvar VAR_0x8004, ROGUE_GYM_FUNC_CHECK_GAME_OVER
	special CallRogueGymFunction
	.endm

	@ Generate 3 reward choices
	.macro roguegym_generaterewards
	setvar VAR_0x8004, ROGUE_GYM_FUNC_GENERATE_REWARDS
	special CallRogueGymFunction
	.endm

	@ Pick a reward. Set VAR_0x8005 to choice index (0-2) before calling
	.macro roguegym_pickreward choice:req
	setvar VAR_0x8004, ROGUE_GYM_FUNC_PICK_REWARD
	setvar VAR_0x8005, \choice
	special CallRogueGymFunction
	.endm

	@ Get the current run status. VAR_RESULT = ROGUE_GYM_STATUS_*
	.macro roguegym_getstatus
	setvar VAR_0x8004, ROGUE_GYM_FUNC_GET_STATUS
	special CallRogueGymFunction
	.endm

	@ Get the current battle number. VAR_RESULT = battle number
	.macro roguegym_getbattlenum
	setvar VAR_0x8004, ROGUE_GYM_FUNC_GET_BATTLE_NUM
	special CallRogueGymFunction
	.endm

	@ Get the current tier. VAR_RESULT = ROGUE_GYM_TIER_*
	.macro roguegym_gettier
	setvar VAR_0x8004, ROGUE_GYM_FUNC_GET_TIER
	special CallRogueGymFunction
	.endm

	@ Use an item from roguelike inventory on a party mon
	@ Set VAR_0x8005 = item index, VAR_0x8006 = party slot
	.macro roguegym_useitem itemindex:req, partyslot:req
	setvar VAR_0x8004, ROGUE_GYM_FUNC_USE_ITEM
	setvar VAR_0x8005, \itemindex
	setvar VAR_0x8006, \partyslot
	special CallRogueGymFunction
	.endm

	@ Save the roguelike gym state
	.macro roguegym_save
	setvar VAR_0x8004, ROGUE_GYM_FUNC_SAVE
	special CallRogueGymFunction
	.endm

	@ Clean up after a run ends (preserves high score)
	.macro roguegym_cleanup
	setvar VAR_0x8004, ROGUE_GYM_FUNC_CLEANUP
	special CallRogueGymFunction
	.endm
