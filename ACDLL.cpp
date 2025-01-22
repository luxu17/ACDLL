#include "ACDLL.h"


GameManager::GameManager() : gm_player(nullptr), gm_playerHealth(gm_intTemp), gm_playerGun(nullptr), gm_playerAmmo(gm_intTemp) {

	/* Get our base address to calculate function offsets from. */
	this->gm_gameBaseAddress = reinterpret_cast<uintptr_t>(LoadLibraryA("ac_client.exe"));

	/* Define our various offsets from the games base executable. */
	this->gm_offsets["shell"] = 0xD44C0;
	this->gm_offsets["aliasLookup"] = 0x26800;
	this->gm_offsets["player"] = 0x18AC00;
	this->gm_offsets["playerCount"] = 0x18AC0C;
	this->gm_offsets["playerArray"] = 0x18AC04;

	/* AssaultCube's internal shell input function. */
	this->gm_shell = (cmdProcessorType)(this->gm_gameBaseAddress + this->gm_offsets["shell"]);

	/* AssaultCube's internal aliasLookup function. */
	this->gm_aliasLookup = (aliasLookupType)(this->gm_gameBaseAddress + this->gm_offsets["aliasLookup"]);

	this->gm_player = this->GetPlayer();

	this->gm_playerGun = this->GetPlayerGun();

	this->LoadACDLLMenu();

	this->ShowMenu();

}

GameManager& GameManager::getInstance() {

	static GameManager instance;

	return instance;

}

void GameManager::ShowMenu() {

	/* Show our previously defined ACDLL menu. */
	this->gm_shell(2, const_cast<char*>("showmenu"), const_cast<char*>("ACDLL"));

}

void GameManager::LoadACDLLMenu() {

	/* Use the games internal script interpreter to build our menu. 
	* 
	* Makes a new menu named ACDLL.
	* 
	*	newmenu ACDLL;
	* 
	* Displays text with the message about pressing L.
	* 
	*	menuitem [Press L to make this menu appear] [-1] [-1];
	* 
	* Create variables to hold the state of the invincibility and infinite ammo checkbox's respectively.
	* These get checked by the aliasLookup function to check their value and enable/disable the corresponding cheat.
	* 
	*	alias [invincible] [0];
	* 
	*	alias [infiniteAmmo] [0];
	* 
	* Create the checkboxes to toggle on/off our cheats.
	* 
	*	menuitemcheckbox [Invincible] [invincible] [invincible = $arg1] [1];
	* 
	*	menuitemcheckbox [Infinite Ammo] [infiniteAmmo] [infiniteAmmo = $arg1] [1];
	* 
	*/
	this->gm_shell(2, const_cast<char*>("newmenu"), const_cast<char*>("ACDLL"));
	this->gm_shell(2, const_cast<char*>("menuitem"), const_cast<char*>("[Press L to make this menu appear] [-1] [-1]"));
	this->gm_shell(2, const_cast<char*>("alias"), const_cast<char*>("[invincible] [0]"));
	this->gm_shell(2, const_cast<char*>("alias"), const_cast<char*>("[infiniteAmmo] [0]"));
	this->gm_shell(2, const_cast<char*>("menuitemcheckbox"), const_cast<char*>("[Invincible] [invincible] [invincible = $arg1] [1]"));
	this->gm_shell(2, const_cast<char*>("menuitemcheckbox"), const_cast<char*>("[Infinite Ammo] [infiniteAmmo] [infiniteAmmo = $arg1] [1]"));

}

char* GameManager::GetAliasValue(unsigned char* alias) {

	/* The AssaultCube function aliasLookup is expecting an unsigned char**.
	*  Needed to add a second variable to make sure it's dereferenced correctly.
	*/
	unsigned char* aliasNamePtr = alias;

	uintptr_t aliasMetadata = this->gm_aliasLookup(&aliasNamePtr);

	/* If the function returns 0 then it means that alias isn't in the hash table. */
	if (aliasMetadata != 0) {

		/* TODO: Add this offset to gm_offsets. */
		char* aliasValue = *reinterpret_cast<char**>(aliasMetadata + 0x1C);

		return aliasValue;
	}

	return nullptr;
}

char* GameManager::GetAliasValue(std::string alias) {

	return this->GetAliasValue((unsigned char*)alias.c_str());

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
	std::string formatting = "\f4ACDLL \t";

	formatting += "\"\ff";
	formatting += msg;
	formatting += "\"";

	/* Echo our message in the games feed. */
	unsigned int result = this->gm_shell(2, const_cast<char*>("echo"), const_cast<char*>(formatting.c_str()));

}


void GameManager::Message(std::string msg) {

	this->Message(msg.c_str());

}

void GameManager::InfoMessage(const char* infoMsg) {

	/* \f1 Makes the message blue. 
	*  \t Terminates the previously applied color.
	*/
	std::string msg = "\f1INFO: \ff";

	msg += infoMsg;

	this->Message(msg);


}

void GameManager::InfoMessage(std::string infoMsg) {

	this->InfoMessage(infoMsg.c_str());

}

void GameManager::ErrorMessage(const char* errorMsg) {

	/* \f3 Makes the message red.
	*  \t Terminates the previously applied color.
	*/
	std::string msg = "\f3ERROR: \ff";

	msg += errorMsg;

	this->Message(msg);

}

void GameManager::ErrorMessage(std::string errorMsg) {

	this->ErrorMessage(errorMsg.c_str());

}