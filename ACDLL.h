#pragma once

#include <Windows.h>
#include <string>
#include <format>
#include <thread>
#include <map>

/*
*
* Struct: Gun
*
* Can be directly mapped onto the memory location of a gun.
*
*/
struct Gun {

	byte padding_1[0x14];

	/* Ammo in the gun. */
	int* ammo;

};

/*
*
* Struct: Entity
*
* Can be mapped onto the player or an NPC in memory.
*
*/
struct Entity {

	byte padding_1[0xEC];

	/* Health. */
	int health;

	byte padding_2[0x115];

	/* Name. */
	char name[0x18];

	byte padding_3[0x147];

	/* Currently equipped gun. */
	Gun* gun;

};

/*
*
* Class: GameManager
*
* This is a singleton class used to manage ACDLL as well as access and manage various
* parts of AssaultCube's game state.
*
*/
class GameManager {

public:

	GameManager();

	~GameManager() = default;

	/* Returns a reference to our singleton class. */
	static GameManager& getInstance();

	GameManager(const GameManager&) = delete;
	GameManager& operator=(const GameManager&) = delete;

	/* This shows our menu in game using CubeScript. */
	void ShowMenu();

	/* Return the value of an alias in AssaultCubes scripting engine. */
	char* GetAliasValue(unsigned char* alias);

	char* GetAliasValue(std::string alias);

	/* Getter: Returns a reference to the players health. */
	int& GetPlayerHealth();

	/* Getter: Returns a reference to the players ammo. */
	int& GetPlayerAmmo();

	/* Echo's a message into the games feed. */
	void Message(const char* msg);

	void Message(std::string);

	/* Provide user with an information message. */
	void InfoMessage(const char* infoMsg);

	void InfoMessage(std::string infoMsg);

	/* Show an error message in the games message feed. */
	void ErrorMessage(const char* errorMsg);

	void ErrorMessage(std::string errorMsg);

private:

	/* Passess a command to the games internal scripting system. */
	typedef unsigned int(*cmdProcessorType)(
		/* TODO: Unsure what this argument is. Perhaps privilege level. */
		unsigned int param1,
		/* The command (i.e. 'echo') */
		char* command,
		/* Arguments passed to our command. */
		char* arguments
	);

	/* This allows us to look up aliases or variables within the games scripting system. */
	typedef int(__stdcall* aliasLookupType)(unsigned char** aliasNamePtrPtr);

	/* Used for calculating offsets to various functions and variables within the game. */
	uintptr_t gm_gameBaseAddress;

	cmdProcessorType gm_shell;

	aliasLookupType gm_aliasLookup;

	/* This is our reference to the player structure in memory. */
	Entity* gm_player;

	/* Player health. */
	int& gm_playerHealth;

	/* Reference to the players gun. */
	Gun* gm_playerGun;

	/* Player gun ammo. */
	int& gm_playerAmmo;

	/* Various offsets that can be accessed via name. */
	std::map<std::string, uintptr_t> gm_offsets;

	/* This runs the CubeScript necessary to generate the ACDLL menu. */
	void LoadACDLLMenu();

	/* Retrieves the player reference. */
	Entity* GetPlayer();

	/* Retrieves a reference to the players gun. */
	Gun* GetPlayerGun();

	/* Temporary values for reference initialization. */
	int gm_intTemp = 0;

};