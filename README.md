I made a pretty simple hack for AssaultCube that took some time to make. Learned a lot though. It's a dll that's injected into the game. I learned a bit of CubeScript (AssaultCube's scripting language) in the process, reverse engineered a couple of functions for the games internal scripting system using Ghidra and Cheat Engine. Also reversed some of the games structures.

Essentially it does a few things:

1) Locates and maps an Entity structure over the player in memory to access the players health and Gun (which has a pointer to the ammo).
 Injects some CubeScript to create a menu in game using the games shell function that interprets CubeScript functions and their parameters. Such as shell(2, "echo", "Hello, World!") and various other CubeScript functions such as newmenu, menuitem and menuitemcheckbox. The three of which I used for my custom menu. If you press L it will show the menu.

2) Once the menu is created it has checkboxes to enable invincibility and infinite ammo. CubeScript has variables it calls alias's. So I create an alias for invincible and infiniteAmmo. When a box is checked they're either set to 1 for true or 0 for false.

3) The problem then becomes being able to check the respective alias's value to enable/disable invincibility or infinite ammo. After all, they're internal to AssaultCube's script engine which I only have access to through functions from the game. This took me a bit to workout. But it has an aliasLookup function that uses a variant of djb2 hashing to look through a hash table for the alias. If it's not there it returns 0. Otherwise it returns a pointer to the alias's metadata and at offset 0x1C is its value.

4) A loop runs constantly, checking whether or not either the invincible or infiniteAmmo alias has been set to true. If so it enables said cheat.

Had a lot of fun with this. Probably gonna keep playing with it. I mean, it's a game from like 2008 I think? So no harm no foul. It's been dead for decades. Visual Studio 2022, C++20.

![ACDLLMenu Picture](https://github.com/luxu17/ACDLL/blob/main/ACDLLMenu2.png?raw=true)
