@ =============================================================================
@ Roguelike Gym Trainer Mode - Script File
@ =============================================================================
@ This script handles the entire roguelike gym battle loop.
@ Triggered by talking to the Roguelike Gym NPC in a Battle Frontier lounge.
@ =============================================================================

RogueGym_EventScript_NPC::
	lock
	faceplayer
	roguegym_getstatus
	goto_if_eq VAR_RESULT, ROGUE_GYM_STATUS_ACTIVE, RogueGym_EventScript_ResumeRun
	goto_if_eq VAR_RESULT, ROGUE_GYM_STATUS_PAUSED, RogueGym_EventScript_ResumeRun
	msgbox RogueGym_Text_Welcome, MSGBOX_DEFAULT
	msgbox RogueGym_Text_WantToChallenge, MSGBOX_YESNO
	goto_if_eq VAR_RESULT, NO, RogueGym_EventScript_Decline
	goto RogueGym_EventScript_StartNewRun

RogueGym_EventScript_Decline::
	msgbox RogueGym_Text_ComeBackAnytime, MSGBOX_DEFAULT
	release
	end

@ =============================================================================
@ Start a new roguelike run
@ =============================================================================
RogueGym_EventScript_StartNewRun::
	msgbox RogueGym_Text_Excellent, MSGBOX_DEFAULT
	special SavePlayerParty
	roguegym_init
	@ Generate 3 starter choices
	roguegym_generatestarters
	@ VAR_0x8005 = species 1, VAR_0x8006 = species 2, VAR_RESULT = species 3
	@ Store species 3 in VAR_TEMP_0 so we can use it later
	copyvar VAR_TEMP_0, VAR_RESULT
	msgbox RogueGym_Text_ChooseStarter, MSGBOX_DEFAULT
	@ Present starter choices as a multichoice
	@ For now, use a simple yes/no/info style - pick 1, 2, or 3
	@ We show species names using bufferspeciesname
	bufferspeciesname STR_VAR_1, VAR_0x8005
	bufferspeciesname STR_VAR_2, VAR_0x8006
	bufferspeciesname STR_VAR_3, VAR_TEMP_0
	msgbox RogueGym_Text_StarterChoice1, MSGBOX_DEFAULT
	msgbox RogueGym_Text_WhichStarter, MSGBOX_DEFAULT
	@ Use multichoice for selection (we need a 3-option choice)
	@ Option 0 = first starter, 1 = second, 2 = third
	multichoice 0, 0, MULTI_FRONTIER_GAMBLER_BET, FALSE
	@ MULTI_FRONTIER_GAMBLER_BET has options 1/2/3 which maps to our starters
	switch VAR_RESULT
	case 0, RogueGym_EventScript_PickStarter0
	case 1, RogueGym_EventScript_PickStarter1
	case 2, RogueGym_EventScript_PickStarter2
	case MULTI_B_PRESSED, RogueGym_EventScript_PickStarter0
	end

RogueGym_EventScript_PickStarter0::
	roguegym_pickstarter 0
	goto RogueGym_EventScript_StarterChosen
RogueGym_EventScript_PickStarter1::
	roguegym_pickstarter 1
	goto RogueGym_EventScript_StarterChosen
RogueGym_EventScript_PickStarter2::
	roguegym_pickstarter 2
	goto RogueGym_EventScript_StarterChosen

RogueGym_EventScript_StarterChosen::
	bufferspeciesname STR_VAR_1, VAR_RESULT
	msgbox RogueGym_Text_ReceivedStarter, MSGBOX_DEFAULT
	msgbox RogueGym_Text_LetsBegin, MSGBOX_DEFAULT
	goto RogueGym_EventScript_BattleLoop

@ =============================================================================
@ Resume an existing run
@ =============================================================================
RogueGym_EventScript_ResumeRun::
	msgbox RogueGym_Text_WelcomeBack, MSGBOX_DEFAULT
	goto RogueGym_EventScript_BattleLoop

@ =============================================================================
@ Main battle loop
@ =============================================================================
RogueGym_EventScript_BattleLoop::
	@ Get current battle number for display
	roguegym_getbattlenum
	buffernumberstring STR_VAR_1, VAR_RESULT
	roguegym_gettier
	@ Set up the next battle (generates enemy team)
	roguegym_setupbattle
	@ Show pre-battle message
	msgbox RogueGym_Text_ChallengerApproaches, MSGBOX_DEFAULT
	closemessage
	@ Start the battle!
	@ The enemy party has already been populated by setupbattle
	dofacilitytrainerbattle FACILITY_BATTLE_ROGUE_GYM
	waitstate
	@ Check battle result: 1 = won, 0 = lost
	goto_if_eq VAR_RESULT, 1, RogueGym_EventScript_BattleWon
	@ Player lost the battle
	goto RogueGym_EventScript_BattleLost

@ =============================================================================
@ Battle won
@ =============================================================================
RogueGym_EventScript_BattleWon::
	roguegym_processwin
	buffernumberstring STR_VAR_1, VAR_RESULT
	@ Check for fainted Pokemon (nuzlocke)
	roguegym_processfainted
	goto_if_ne VAR_RESULT, 0, RogueGym_EventScript_PokemonFainted
	goto RogueGym_EventScript_PostBattleRewards

RogueGym_EventScript_PokemonFainted::
	buffernumberstring STR_VAR_1, VAR_RESULT
	msgbox RogueGym_Text_PokemonFallen, MSGBOX_DEFAULT
	@ Check if all Pokemon are gone
	roguegym_checkgameover
	goto_if_eq VAR_RESULT, TRUE, RogueGym_EventScript_GameOver
	goto_if_eq VAR_RESULT, 2, RogueGym_EventScript_Victory
	goto RogueGym_EventScript_PostBattleRewards

@ =============================================================================
@ Post-battle rewards
@ =============================================================================
RogueGym_EventScript_PostBattleRewards::
	@ Check if we've won the whole thing
	roguegym_checkgameover
	goto_if_eq VAR_RESULT, 2, RogueGym_EventScript_Victory
	@ Generate rewards
	roguegym_generaterewards
	msgbox RogueGym_Text_ChooseReward, MSGBOX_DEFAULT
	@ For now, use a simple 3-option multichoice
	multichoice 0, 0, MULTI_FRONTIER_GAMBLER_BET, FALSE
	switch VAR_RESULT
	case 0, RogueGym_EventScript_PickReward0
	case 1, RogueGym_EventScript_PickReward1
	case 2, RogueGym_EventScript_PickReward2
	case MULTI_B_PRESSED, RogueGym_EventScript_PickReward0

RogueGym_EventScript_PickReward0::
	roguegym_pickreward 0
	goto RogueGym_EventScript_RewardChosen
RogueGym_EventScript_PickReward1::
	roguegym_pickreward 1
	goto RogueGym_EventScript_RewardChosen
RogueGym_EventScript_PickReward2::
	roguegym_pickreward 2
	goto RogueGym_EventScript_RewardChosen

RogueGym_EventScript_RewardChosen::
	@ VAR_RESULT = reward type
	goto_if_eq VAR_RESULT, ROGUE_REWARD_POKEMON, RogueGym_EventScript_GotPokemon
	msgbox RogueGym_Text_ReceivedReward, MSGBOX_DEFAULT
	goto RogueGym_EventScript_BetweenBattles

RogueGym_EventScript_GotPokemon::
	msgbox RogueGym_Text_NewPokemonJoined, MSGBOX_DEFAULT
	goto RogueGym_EventScript_BetweenBattles

@ =============================================================================
@ Between battles menu
@ =============================================================================
RogueGym_EventScript_BetweenBattles::
	roguegym_getbattlenum
	buffernumberstring STR_VAR_1, VAR_RESULT
	msgbox RogueGym_Text_BetweenBattles, MSGBOX_DEFAULT
	@ Save the game
	roguegym_save
	msgbox RogueGym_Text_ReadyForNext, MSGBOX_YESNO
	goto_if_eq VAR_RESULT, YES, RogueGym_EventScript_BattleLoop
	@ Player wants to take a break
	msgbox RogueGym_Text_TakeBreak, MSGBOX_DEFAULT
	release
	end

@ =============================================================================
@ Battle lost
@ =============================================================================
RogueGym_EventScript_BattleLost::
	@ Process fainted (all of them since we lost)
	roguegym_processfainted
	roguegym_checkgameover
	goto RogueGym_EventScript_GameOver

@ =============================================================================
@ Game Over
@ =============================================================================
RogueGym_EventScript_GameOver::
	roguegym_getbattlenum
	buffernumberstring STR_VAR_1, VAR_RESULT
	msgbox RogueGym_Text_GameOver, MSGBOX_DEFAULT
	roguegym_cleanup
	special LoadPlayerParty
	callnative UpdateFollowingPokemon
	msgbox RogueGym_Text_BetterLuckNextTime, MSGBOX_DEFAULT
	release
	end

@ =============================================================================
@ Victory - completed all battles!
@ =============================================================================
RogueGym_EventScript_Victory::
	roguegym_getbattlenum
	buffernumberstring STR_VAR_1, VAR_RESULT
	msgbox RogueGym_Text_Victory, MSGBOX_DEFAULT
	playfanfare MUS_OBTAIN_SYMBOL
	waitfanfare
	msgbox RogueGym_Text_VictoryCongrats, MSGBOX_DEFAULT
	roguegym_cleanup
	special LoadPlayerParty
	callnative UpdateFollowingPokemon
	release
	end

@ =============================================================================
@ Text Strings
@ =============================================================================

RogueGym_Text_Welcome:
	.string "Welcome to the ROGUELIKE GYM!\p"
	.string "Here, YOU are the GYM TRAINER.\n"
	.string "Challengers will come to face you\l"
	.string "one after another!\p"
	.string "But be warned - if your POKéMON\n"
	.string "faints, it's gone forever!\p"
	.string "How long can you hold your gym?$"

RogueGym_Text_WantToChallenge:
	.string "Would you like to take on the\n"
	.string "ROGUELIKE GYM challenge?$"

RogueGym_Text_ComeBackAnytime:
	.string "Come back anytime you're ready\n"
	.string "to defend your gym!$"

RogueGym_Text_Excellent:
	.string "Excellent! Let's get you set up.\p"
	.string "First, choose your partner POKéMON.\n"
	.string "Three POKéMON have been randomly\l"
	.string "selected for you.$"

RogueGym_Text_ChooseStarter:
	.string "Choose wisely - this POKéMON will\n"
	.string "be your first line of defense!$"

RogueGym_Text_StarterChoice1:
	.string "Your choices are:\n"
	.string "1. {STR_VAR_1}\l"
	.string "2. {STR_VAR_2}\l"
	.string "3. {STR_VAR_3}$"

RogueGym_Text_WhichStarter:
	.string "Which POKéMON will you choose?$"

RogueGym_Text_ReceivedStarter:
	.string "{PLAYER} received {STR_VAR_1}!\p"
	.string "Take good care of it - remember,\n"
	.string "if it faints, it's gone for good!$"

RogueGym_Text_LetsBegin:
	.string "Your first challenger is on\n"
	.string "their way! Get ready!$"

RogueGym_Text_WelcomeBack:
	.string "Welcome back, GYM TRAINER!\n"
	.string "Your next challenger awaits!$"

RogueGym_Text_ChallengerApproaches:
	.string "Battle {STR_VAR_1}!\p"
	.string "A new challenger approaches\n"
	.string "your gym!$"

RogueGym_Text_PokemonFallen:
	.string "{STR_VAR_1} of your POKéMON\n"
	.string "fell in battle…\p"
	.string "They fought bravely, but they\n"
	.string "can no longer continue…$"

RogueGym_Text_ChooseReward:
	.string "You defended your gym!\n"
	.string "Choose your reward:\p"
	.string "1, 2, or 3?$"

RogueGym_Text_ReceivedReward:
	.string "You received your reward!\n"
	.string "Use it wisely.$"

RogueGym_Text_NewPokemonJoined:
	.string "A new POKéMON joined your team!\n"
	.string "Welcome aboard!$"

RogueGym_Text_BetweenBattles:
	.string "Next up: Battle {STR_VAR_1}.\p"
	.string "You can use items from your\n"
	.string "inventory to prepare.$"

RogueGym_Text_ReadyForNext:
	.string "Ready for the next challenger?$"

RogueGym_Text_TakeBreak:
	.string "Take a break.\n"
	.string "Your gym will be here when\l"
	.string "you're ready to return!$"

RogueGym_Text_GameOver:
	.string "All your POKéMON have fallen…\p"
	.string "You made it to Battle {STR_VAR_1}\n"
	.string "before your gym fell.\p"
	.string "Your original team has been\n"
	.string "returned to you.$"

RogueGym_Text_BetterLuckNextTime:
	.string "Don't give up! Train harder and\n"
	.string "try the ROGUELIKE GYM again!$"

RogueGym_Text_Victory:
	.string "Incredible! You survived all\n"
	.string "{STR_VAR_1} battles!\p"
	.string "You are the ultimate\n"
	.string "GYM TRAINER!$"

RogueGym_Text_VictoryCongrats:
	.string "No challenger could defeat you!\p"
	.string "Your gym stands undefeated!\n"
	.string "Congratulations, champion!$"
