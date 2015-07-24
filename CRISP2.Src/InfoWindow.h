#pragma once

// if inCommandOnClick is not 0, sends a WM_COMMAND with inCommandOnClick as the command ID
HWND CreateInfoWindow(const TCHAR* inMessage, DWORD inCommandOnClick = 0, bool inChild = true);