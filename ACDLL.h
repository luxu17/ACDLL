#pragma once

#include <Windows.h>
#include <string>
#include <format>
#include <thread>
#include <map>
#include <memory>
#include <type_traits>

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

	byte padding_1[0x76];

								// 0x28 -> coordinates

	/* Puts player into non-playable state.
	*  Does not appear to "kill" them per se.
	*/
	char dead;					// 0x76 -> dead

	byte padding_2[0x75];

	/* Health. */
	int health;					// 0xEC -> health

	byte padding_3[0x115];

	/* Name. */
	char name[0x18];			// 0x205 -> name

	byte padding_4[0x147];

								// 0x318 -> ghost

	/* Current gun. */
	Gun* gun;					// 0x364 -> gun

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
	template<typename T1>
	char* GetAliasValue(T1 alias);

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

protected:

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

	/* Temporary values for reference initialization. */
	int gm_intTemp = 0;

	template<typename T1, typename T2>
	void Shell(T1 command, T2 arguments);

	/* This runs the CubeScript necessary to generate the ACDLL menu. */
	void LoadACDLLMenu();

	/* Retrieves the player reference. */
	Entity* GetPlayer();

	/* Retrieves a reference to the players gun. */
	Gun* GetPlayerGun();

	class NPCManager {

	public:

		NPCManager(GameManager& GM);

		~NPCManager() = default;

		void LoadNPCManagerMenu();
		void ShowNPCManagerMenu();

		void DepleteAmmo(char* name);
		void DepleteAmmo(std::string name);
		void DepleteAllAmmo();

		void DepleteHealth(char* name);
		void DepleteHealth(std::string name);
		void DepleteAllHealth();

		GameManager& gm;

	protected:

		int GetNPCCount();

		Entity** GetPlayers();

		int& nm_playerCount;

		std::map<std::string, uintptr_t> nm_offsets;

	private:

		Entity** nm_players;

		int nm_intTemp = 0;

	};

	std::unique_ptr<NPCManager> gm_npcManager;

};

template<typename T1>
char* GameManager::GetAliasValue(T1 alias) {

	/* The AssaultCube function aliasLookup is expecting an unsigned char**.
	*  Needed to add a second variable to make sure it's dereferenced correctly.
	*/
	unsigned char* aliasNamePtr = nullptr;

	if constexpr (std::is_same_v<T1, char*> || std::is_same_v<T1, unsigned char*> || std::is_same_v<T1, const char*> || std::is_same_v<T1, const unsigned char*> || std::is_same_v<T1, char const*>)
	{

		aliasNamePtr = reinterpret_cast<unsigned char*>(const_cast<char*>(reinterpret_cast<const char*>(alias)));

	}
	else if constexpr (std::is_same_v<T1, std::string>) {

		aliasNamePtr = alias.c_str();
	}
	else {

		this->ErrorMessage("Invalid datatype passed to GameManager::GetAliasValue");

		return nullptr;

	}

	uintptr_t aliasMetadata = this->gm_aliasLookup(&aliasNamePtr);

	/* If the function returns 0 then it means that alias isn't in the hash table. */
	if (aliasMetadata != 0) {

		/* TODO: Add this offset to gm_offsets. */
		char* aliasValue = *reinterpret_cast<char**>(aliasMetadata + 0x1C);

		return aliasValue;
	}

	return nullptr;
}