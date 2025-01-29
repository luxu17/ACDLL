#include "ACDLL.h"

GameManager::GameManager() : gm_player(nullptr), gm_playerHealth(gm_intTemp), gm_playerGun(nullptr), gm_playerAmmo(gm_intTemp), gm_npcManager(nullptr) {

	/* Get our base address to calculate function offsets from. */
	this->gm_gameBaseAddress = reinterpret_cast<uintptr_t>(LoadLibraryA("ac_client.exe"));

	/* Define our various offsets from the games base executable. */
	this->gm_offsets["shell"] = 0xD44C0;
	this->gm_offsets["aliasLookup"] = 0x26800;
	this->gm_offsets["player"] = 0x18AC00;
	this->gm_offsets["playerCount"] = 0x18AC0C;
	this->gm_offsets["playerArray"] = 0x18AC04;
	this->gm_offsets["unsure"] = 0x17E0A8; // unsure->+0x77 = showBody

	/* AssaultCube's internal shell input function. */
	this->gm_shell = (cmdProcessorType)(this->gm_gameBaseAddress + this->gm_offsets["shell"]);

	/* AssaultCube's internal aliasLookup function. */
	this->gm_aliasLookup = (aliasLookupType)(this->gm_gameBaseAddress + this->gm_offsets["aliasLookup"]);

	this->gm_player = this->GetPlayer();

	this->gm_playerGun = this->GetPlayerGun();

	this->LoadACDLLMenu();

	this->ShowMenu();

	this->gm_npcManager = std::make_unique<NPCManager>(*this);

}

GameManager& GameManager::getInstance() {

	static GameManager instance;

	return instance;

}

template<typename T1, typename T2>
void GameManager::Shell(T1 command, T2 arguments) {

	/* Check the data type of command arguments then pass them to the wrapped function appropriately. */
	if constexpr (std::is_same_v<T1, std::string> && std::is_same_v<T2, std::string>) {

		this->gm_shell(2, command.c_str(), arguments.c_str());

	}
	else if constexpr ((std::is_same_v<T1, const char*> || std::is_same_v<T1, char*>) && (std::is_same_v<T2, const char*> || std::is_same_v<T2, char*>)) {

		this->gm_shell(2, const_cast<char*>(command), const_cast<char*>(arguments));

	}
	else if constexpr ((std::is_same_v<T1, const char*> || std::is_same_v<T1, char*>) && std::is_same_v<T2, std::string>) {

		this->gm_shell(2, const_cast<char*>(command), const_cast<char*>(arguments.c_str()));

	}
	else if constexpr (std::is_same_v<T1, std::string> && (std::is_same_v<T2, const char*> || std::is_same_v<T2, char*>)) {

		this->gm_shell(2, const_cast<char*>(command.c_str()), const_cast<char*>(arguments));

	}
	else {

		this->ErrorMessage("Shell was given an invalid argument type");

	}

}

void GameManager::ShowMenu() {

	/* Show our previously defined ACDLL menu. */
	this->Shell("showmenu", "ACDLL");

}

void GameManager::LoadACDLLMenu() {

	this->Shell("exec", "./config/ACDLL.cfg");

}

Entity* GameManager::GetPlayer() {

	/* Add the games executable address ac_client.exe to player offset
	*  which is a pointer to another pointer that points to the player
	*  Entity structure.
	*/
	Entity** playerPointer = reinterpret_cast<Entity**>(this->gm_gameBaseAddress + this->gm_offsets["player"]);

	Entity* player = *playerPointer;

	/* Tried to have it return a reference to the Entity in memory.
	*  This did not work at all. It just created a copy.
	*  So now it returns a pointer to the player Entity structure in memory.
	*/
	return player;
}

int& GameManager::GetPlayerHealth() {

	return this->gm_player->health;

}

Gun* GameManager::GetPlayerGun() {

	/* Return a pointer to the players *currently* equipped gun. */
	return this->gm_player->gun;

}

int& GameManager::GetPlayerAmmo() {

	/* The players gun needs to be updated each time to ensure
	*  we're passing the ammo for the *currently* equipped gun.
	*/
	this->gm_playerGun = this->GetPlayerGun();

	return *this->gm_playerGun->ammo;

}

void GameManager::Message(const char* msg) {

	/* \f4 Makes the message grey. */

	auto formatting = std::format("\f4ACDLL \t\"\ff{}\"", msg);

	/* Echo our message in the games feed. */
	this->Shell("echo", formatting);

}


void GameManager::Message(std::string msg) {

	this->Message(msg.c_str());

}

void GameManager::InfoMessage(const char* infoMsg) {

	/* \f1 Makes the message blue.
	*  \ff Terminates the previously applied color.
	*/

	this->Message(std::format("\f1INFO: \ff{}", infoMsg));

}

void GameManager::InfoMessage(std::string infoMsg) {

	this->InfoMessage(infoMsg.c_str());

}

void GameManager::ErrorMessage(const char* errorMsg) {

	/* \f3 Makes the message red.
	*  \ff Terminates the previously applied color.
	*/

	this->Message(std::format("\f3ERROR: \ff{}", errorMsg));

}

void GameManager::ErrorMessage(std::string errorMsg) {

	this->ErrorMessage(errorMsg.c_str());

}

GameManager::NPCManager::NPCManager(GameManager& GM) : gm(GM), nm_playerCount(nm_intTemp) {

	this->nm_offsets["playerCount"] = 0x18AC0C;
	this->nm_offsets["playerArray"] = 0x18AC04;

	this->nm_playerCount = this->GetNPCCount();

	this->nm_players = this->GetPlayers();

	this->LoadNPCManagerMenu();

}

void GameManager::NPCManager::LoadNPCManagerMenu() {

	this->nm_playerCount = this->GetNPCCount();

	this->gm.Shell("newmenu", "depletehealth");

	for (int i = 0; i < this->nm_playerCount; i++) {

		Entity* npc = this->nm_players[i];

		std::string arguments = std::format("[{}] [] [] [Deplete this NPC's health]", npc->name);

		this->gm.Shell("menuitem", arguments);

	}

	this->gm.Shell("newmenu", "depleteammo");

	for (int i = 0; i < this->nm_playerCount; i++) {

		Entity* npc = this->nm_players[i];

		std::string arguments = std::format("[{}] [] [] [Deplete this NPC's ammo]", npc->name);

		this->gm.Shell("menuitem", arguments);

	}

	this->gm.Shell("exec", "./config/NPCManager.cfg");

}

void GameManager::NPCManager::ShowNPCManagerMenu() {

	this->gm.Shell("showmenu", "npcmanager");

}

int GameManager::NPCManager::GetNPCCount() {

	return *reinterpret_cast<int*>(this->gm.gm_gameBaseAddress + this->nm_offsets["playerCount"]) - 1;

}

Entity** GameManager::NPCManager::GetPlayers() {

	uintptr_t baseOfPlayerArray = *reinterpret_cast<uintptr_t*>(this->gm.gm_gameBaseAddress + this->nm_offsets["playerArray"]);

	return reinterpret_cast<Entity**>(baseOfPlayerArray + 4);

}

void GameManager::NPCManager::DepleteAmmo(char* name) {

	this->nm_playerCount = this->GetNPCCount();

	for (int i = 0; i < this->nm_playerCount; i++) {

		Entity* npc = this->nm_players[i];

		if (!std::strcmp(npc->name, name)) {

			*npc->gun->ammo = 0;

		}

	}

}


void GameManager::NPCManager::DepleteAmmo(std::string name) {

	this->DepleteAmmo(const_cast<char*>(name.c_str()));

}

void GameManager::NPCManager::DepleteAllAmmo() {

	this->nm_playerCount = this->GetNPCCount();

	for (int i = 0; i < this->nm_playerCount; i++) {

		Entity* npc = this->nm_players[i];

		if (npc != this->gm.gm_player) {

			*npc->gun->ammo = 0;

		}

	}

}

void GameManager::NPCManager::DepleteHealth(char* name) {

	this->nm_playerCount = this->GetNPCCount();

	for (int i = 0; i < this->nm_playerCount; i++) {

		Entity* npc = this->nm_players[i];

		if (!std::strcmp(npc->name, name)) {

			npc->health = 0;

		}

	}

}

void GameManager::NPCManager::DepleteHealth(std::string name) {

	this->DepleteHealth(const_cast<char*>(name.c_str()));

}

void GameManager::NPCManager::DepleteAllHealth() {

	this->nm_playerCount = this->GetNPCCount();

	for (int i = 0; i < this->nm_playerCount; i++) {

		Entity* npc = this->nm_players[i];

		if (npc != this->gm.gm_player) {

			npc->health = 0;

		}

	}

}