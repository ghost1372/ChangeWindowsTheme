// ChangeWindowsTheme.cpp : This file contains the 'main' function. Program execution begins and ends there.
//

#include <Windows.h>
#include <iostream>
#include <string>
#include <winreg.h>

// Path to the registry key
const std::wstring keyPath = L"Software\\Microsoft\\Windows\\CurrentVersion\\Themes\\Personalize";
const std::wstring valueNameApps = L"AppsUseLightTheme";
const std::wstring valueNameSystem = L"SystemUsesLightTheme";

int WINAPI wWinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, PWSTR pCmdLine, int nCmdShow)
{
    HKEY hKey;
    LONG result = RegOpenKeyExW(HKEY_CURRENT_USER, keyPath.c_str(), 0, KEY_READ | KEY_WRITE, &hKey);

    if (result == ERROR_SUCCESS) {
        DWORD currentValueSystem, currentValueApps;
        DWORD size = sizeof(DWORD);

        // Get the current value for SystemUsesLightTheme
        result = RegQueryValueExW(hKey, valueNameSystem.c_str(), 0, NULL, (LPBYTE)&currentValueSystem, &size);

        // Get the current value for AppsUseLightTheme
        result = RegQueryValueExW(hKey, valueNameApps.c_str(), 0, NULL, (LPBYTE)&currentValueApps, &size);

        if (result == ERROR_SUCCESS) {
            // Ensure both values are the same to avoid "Custom" setting
            if (currentValueApps != currentValueSystem) {
                currentValueApps = currentValueSystem;
                RegSetValueExW(hKey, valueNameApps.c_str(), 0, REG_DWORD, (const BYTE*)&currentValueApps, sizeof(DWORD));
            }

            // Toggle the theme
            int newTheme = (currentValueSystem == 1) ? 0 : 1;
            RegSetValueExW(hKey, valueNameSystem.c_str(), 0, REG_DWORD, (const BYTE*)&newTheme, sizeof(DWORD));
            RegSetValueExW(hKey, valueNameApps.c_str(), 0, REG_DWORD, (const BYTE*)&newTheme, sizeof(DWORD));

            // Print the new theme
            std::wcout << (newTheme == 1 ? L"Light theme set" : L"Dark theme set") << std::endl;

            // Broadcast the change
            DWORD_PTR resultMessage;
            SendMessageTimeoutW(HWND_BROADCAST, WM_SETTINGCHANGE, 0, (LPARAM)L"ImmersiveColorSet", SMTO_ABORTIFHUNG, 50, &resultMessage);
        }
        else {
            std::wcout << L"One or both registry values do not exist." << std::endl;
        }

        // Close the registry key
        RegCloseKey(hKey);
    }
    else {
        std::wcout << L"The registry key does not exist." << std::endl;
    }

    return 0;
}
