// Valovlay.cpp : This file contains the 'main' function. Program execution begins and ends there.
//

#include "memory.h"
#include "driver.h"
#include "game.h"

#include <iostream>
#include <cstdint>

bool CheckDriverStatus() {
    std::cout << "Checking Driver Status ..." << std::endl;

    memory::Unprotect(driver::GetBaseAddress);
    uintptr_t BaseAddr = driver::GetBaseAddress(driver::currentProcessId);
    std::cout << "-> BaseAddr :: " << BaseAddr << std::endl;
    if (BaseAddr == 0) {
        return false;
    }
    memory::Protect(driver::GetBaseAddress);

    int icheck = 29;
    NTSTATUS status = 0;
    int checked = driver::read<int>(driver::currentProcessId, (uintptr_t) &icheck, &status);
    std::cout << "-> checked :: " << checked << std::endl;
    std::cout << "-> icheck :: " << icheck << std::endl;
    if (checked != icheck) {
        return false;
    }

    return true;
}

int main()
{
    std::cout << "Welcome To Valorant Overlay!" << std::endl;
    std::cout << "Last Build By Bifeldy :: 17-08-2021" << std::endl;
    std::cout << "Have Fun~" << std::endl;

    memory::Unprotect(driver::initialize);
    bool ready = false;
    if (driver::initialize())
    {
        memory::Unprotect(CheckDriverStatus);
        if (CheckDriverStatus())
        {
            ready = true;
        }
        else
        {
            wchar_t VarName[] = { 'F','a','s','t','B','o','o','t','O','p','t','i','o','n','\0' };
            UNICODE_STRING FVariableName = UNICODE_STRING();
            FVariableName.Buffer = VarName;
            FVariableName.Length = 28;
            FVariableName.MaximumLength = 30;
            // UNICODE_STRING VariableName = RTL_CONSTANT_STRING(VARIABLE_NAME);
            driver::myNtSetSystemEnvironmentValueEx(&FVariableName, &driver::DummyGuid, 0, 0, ATTRIBUTES);
            memset(VarName, 0, sizeof(VarName));
            // memset(VariableName.Buffer, 0, VariableName.Length);
            // VariableName.Length = 0;
            Beep(1250, 500);
            Beep(1250, 500);
            std::cout << "No EFI Driver Found ..." << std::endl;
        }
        memory::Protect(CheckDriverStatus);
    }
    else
    {
        std::cout << "Connection To The Driver Failed ..." << std::endl;
        Beep(1250, 500);
    }
    memory::Protect(driver::initialize);

    if (ready)
    {
        Beep(1250, 250);
        game::run();
    }

    std::cout << "Good Bye~" << std::endl;
    system("pause");
    return 0;
}
