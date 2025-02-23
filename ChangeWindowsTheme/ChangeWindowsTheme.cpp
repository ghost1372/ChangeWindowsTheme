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
const std::wstring valueNameAccent = L"ColorPrevalence";
const std::wstring backupValueName = L"ColorPrevalenceBackup"; // Custom key to store previous value

int WINAPI wWinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, PWSTR pCmdLine, int nCmdShow)
{
    HKEY hKey;
    LONG result = RegOpenKeyExW(HKEY_CURRENT_USER, keyPath.c_str(), 0, KEY_READ | KEY_WRITE, &hKey);

    if (result == ERROR_SUCCESS) {
        DWORD currentValueSystem, currentValueApps, currentValueAccent;
        DWORD size = sizeof(DWORD);

        // Get the current theme values
        result = RegQueryValueExW(hKey, valueNameSystem.c_str(), 0, NULL, (LPBYTE)&currentValueSystem, &size);
        result = RegQueryValueExW(hKey, valueNameApps.c_str(), 0, NULL, (LPBYTE)&currentValueApps, &size);
        result = RegQueryValueExW(hKey, valueNameAccent.c_str(), 0, NULL, (LPBYTE)&currentValueAccent, &size);

        if (result == ERROR_SUCCESS) {
            // Ensure both values are the same
            if (currentValueApps != currentValueSystem) {
                currentValueApps = currentValueSystem;
                RegSetValueExW(hKey, valueNameApps.c_str(), 0, REG_DWORD, (const BYTE*)&currentValueApps, sizeof(DWORD));
            }

            // Determine new theme
            int newTheme = (currentValueSystem == 1) ? 0 : 1;

            // Handle accent color settings
            if (newTheme == 1) { // Switching to Light theme
                // Always store the current ColorPrevalence value before changing it
                RegSetValueExW(hKey, backupValueName.c_str(), 0, REG_DWORD, (const BYTE*)&currentValueAccent, sizeof(DWORD));

                if (currentValueAccent == 1) {
                    // Disable accent color on Start and Taskbar
                    DWORD newAccentValue = 0;
                    RegSetValueExW(hKey, valueNameAccent.c_str(), 0, REG_DWORD, (const BYTE*)&newAccentValue, sizeof(DWORD));
                }
            }
            else { // Switching to Dark theme
                DWORD storedAccentValue;
                size = sizeof(DWORD);
                result = RegQueryValueExW(hKey, backupValueName.c_str(), 0, NULL, (LPBYTE)&storedAccentValue, &size);

                if (result == ERROR_SUCCESS) {
                    // Restore the stored accent color setting
                    RegSetValueExW(hKey, valueNameAccent.c_str(), 0, REG_DWORD, (const BYTE*)&storedAccentValue, sizeof(DWORD));
                }
            }

            // Apply the new theme
            RegSetValueExW(hKey, valueNameSystem.c_str(), 0, REG_DWORD, (const BYTE*)&newTheme, sizeof(DWORD));
            RegSetValueExW(hKey, valueNameApps.c_str(), 0, REG_DWORD, (const BYTE*)&newTheme, sizeof(DWORD));

            std::wcout << (newTheme == 1 ? L"Light theme set" : L"Dark theme set") << std::endl;

            // Broadcast the change
            DWORD_PTR resultMessage;
            SendMessageTimeoutW(HWND_BROADCAST, WM_SETTINGCHANGE, 0, (LPARAM)L"ImmersiveColorSet", SMTO_ABORTIFHUNG, 50, &resultMessage);
        }
        else {
            std::wcout << L"Failed to read registry values." << std::endl;
        }

        // Close the registry key
        RegCloseKey(hKey);
    }
    else {
        std::wcout << L"Failed to open registry key." << std::endl;
    }

    return 0;
}