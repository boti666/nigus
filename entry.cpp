#include "includes.h"
#include <thread>
#include <chrono>
#include <tchar.h>
#include <algorithm>
#include <iterator>

// outside of func shouldnt appear in ida
std::map<DWORD, std::string> ids = {
    {674666432, XOR("admin")}
};

int __stdcall DllMain(HMODULE self, ulong_t reason, void* reserved) {
    TCHAR volumeName[MAX_PATH + 1] = { 0 };
    TCHAR fileSystemName[MAX_PATH + 1] = { 0 };
    DWORD serialNumber = 0;
    DWORD maxComponentLen = 0;
    DWORD fileSystemFlags = 0;

    if (reason == DLL_PROCESS_ATTACH) {
        if (GetVolumeInformation(
            _T("C:\\"),
            volumeName,
            ARRAYSIZE(volumeName),
            &serialNumber,
            &maxComponentLen,
            &fileSystemFlags,
            fileSystemName,
            ARRAYSIZE(fileSystemName)))
        {

            if (auto search = ids.find(serialNumber); search != ids.end()) {
                for (auto& id : ids) {
                    int hwid = id.first;
                    std::string name = id.second;

                    if (hwid == serialNumber) {
                        g_cl.m_user = name;
                        g_cl.id = id.first;
                        g_cl.m_hwid_found = true;
                        continue;
                    }
                }

                HANDLE thread = CreateThread(nullptr, 0, Client::init, nullptr, 0, nullptr);
                if (!thread)
                    return 0;

                return 1;
            }
            else
                exit(0); // goodbye people who dont use
        }
    }
}