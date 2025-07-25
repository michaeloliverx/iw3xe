#include "common.h"

enum GameTitleId
{
	GAME_TITLE_ID_IW2 = 0x415607D1, // Call of Duty 2
	GAME_TITLE_ID_IW3 = 0x415607E6, // Call of Duty 4: Modern Warfare
	GAME_TITLE_ID_T4 = 0x4156081C,	// Call of Duty: World at War
	GAME_TITLE_ID_IW4 = 0X41560817, // Call of Duty: Modern Warfare 2
};

void monitor_title_id()
{
	// Xenia loads extremely fast
	// but we need to wait a bit for the game to load on real hardware
	// otherwise the game will crash
	bool in_xenia = xbox::InXenia();

	for (;;)
	{
		uint32_t current_title_id = xbox::XamGetCurrentTitleId();
		if (current_title_id == GAME_TITLE_ID_IW2)
		{
			if (strncmp((char *)0x82059FE0, "startSingleplayer", 17) == 0)
			{
				if (!in_xenia)
					Sleep(1000);
				xbox::show_notification(L"CODxe - IW2 Singleplayer Detected");
				iw2::sp::init();
			}
			else if (strncmp((char *)0x820410E4, "multiplayer", 11) == 0)
			{
				if (!in_xenia)
					Sleep(1000);
				xbox::show_notification(L"CODxe - IW2 Multiplayer Detected");
				iw2::mp::init();
			}
			else
			{
				xbox::show_notification(L"CODxe - IW2 Unsupported Executable");
			}
			return;
		}
		else if (current_title_id == GAME_TITLE_ID_IW3)
		{
			if (strncmp((char *)0x82032AC4, "multiplayer", 11) == 0)
			{
				if (!in_xenia)
					Sleep(1000);
				xbox::show_notification(L"CODxe - IW3 Multiplayer Detected");
				iw3::mp::init();
			}
			else if (strncmp((char *)0x82065E48, "startSingleplayer", 17) == 0)
			{
				if (!in_xenia)
					Sleep(1000);
				xbox::show_notification(L"CODxe - IW3 Singleplayer Detected");
				iw3::sp::init();
			}
			else
			{
				xbox::show_notification(L"CODxe - IW3 Unsupported Executable");
			}
			return;
		}
		else if (current_title_id == GAME_TITLE_ID_T4)
		{
			if (strncmp((char *)0x820024CC, "multiplayer", 11) == 0)
			{
				if (!in_xenia)
					Sleep(1000);
				xbox::show_notification(L"CODxe - T4 Multiplayer Detected");
				t4::mp::init();
			}
			else if (strncmp((char *)0x82035A94, "startSingleplayer", 17) == 0)
			{
				if (!in_xenia)
					Sleep(1000);
				xbox::show_notification(L"CODxe - T4 Singleplayer Detected");
				t4::sp::init();
			}
			else
			{
				xbox::show_notification(L"CODxe - T4 Unsupported Executable");
			}
			return;
		}
		else if (current_title_id == GAME_TITLE_ID_IW4)
		{
			if (strncmp((char *)0x8203C924, "startSingleplayer", 17) == 0)
			{
				if (!in_xenia)
					Sleep(1000);
				xbox::show_notification(L"CODxe - IW4 Singleplayer Detected");
				iw4::sp::init();
			}
			else
			{
				xbox::show_notification(L"CODxe - IW4 Unsupported Executable");
			}
			return;
		}

		Sleep(50);
	}
}

int DllMain(HANDLE hModule, DWORD reason, void *pReserved)
{
	switch (reason)
	{
	case DLL_PROCESS_ATTACH:
		xbox::ExCreateThread(nullptr, 0, nullptr, nullptr, reinterpret_cast<PTHREAD_START_ROUTINE>(monitor_title_id), nullptr, 2);
		break;
	case DLL_PROCESS_DETACH:
		break;
	}

	return TRUE;
}
