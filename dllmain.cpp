#include <Windows.h>
#include <thread>
#include "ACDLL.h"

GameManager GM;

void GameManagerThread() {

    /* Obtain health and ammo references. */
    int& health = GM.GetPlayerHealth();
    int& ammo = GM.GetPlayerAmmo();

    while (true) {

        /* If the user presses L it shows the ACDLL menu. */
        if (GetAsyncKeyState('L') & 0x8000) {

            GM.ShowMenu();

        }

        /* Check the state of the Invincible checkbox. */
        char* invincible = GM.GetAliasValue("invincible");

        if (invincible != nullptr) {

            /* If Invincible is checked set health to 500 until it's unchecked. */
            if (!std::strcmp(invincible, "1")) health = 500;

        }

        /* Check the state of the Infinite Ammo checkbox. */
        char* infiniteAmmo = GM.GetAliasValue("infiniteAmmo");

        if (infiniteAmmo != nullptr) {

            /* If InfiniteAmmo is checked set ammo to 100 until it's unchecked. */
            if (!std::strcmp(infiniteAmmo, "1")) ammo = 100;

        }

        Sleep(20);

    }

}


void onAttach() {

    std::thread gameManagerThread(GameManagerThread);

    gameManagerThread.detach();

}


BOOL APIENTRY DllMain(HMODULE hModule, DWORD ul_reason_for_call, LPVOID lpReserved)
{

    switch (ul_reason_for_call)
    {

    case DLL_PROCESS_ATTACH: {

        std::thread main(onAttach);

        main.detach();

        break;
    }

    case DLL_THREAD_ATTACH:

        break;

    case DLL_THREAD_DETACH:

        break;

    case DLL_PROCESS_DETACH:

        break;

    }
    return TRUE;

}