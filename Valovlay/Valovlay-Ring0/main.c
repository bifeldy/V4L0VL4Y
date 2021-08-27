#include "definitions.h"
#include "dummy.h"

// Defines used to check if call is really coming from client
#define VARIABLE_NAME L"Bifeldy-Driver"
#define baseOperation 0x81
#define COMMAND_MAGIC baseOperation * 0x45 * 0x01

/*
 *  https://www.guidgen.com
 *  https://forums.codeguru.com/showthread.php?258399-How-to-assign-GUID-into-a-constant-variable-in-C
 */

// Our protocol GUID (should be different for every driver)
EFI_GUID ProtocolGuid = { 0x2fac7380, 0x0c4f, 0x4c04, {0xa1, 0x5c, 0xcf, 0x00, 0x94, 0x87, 0xed, 0x1b} };

// VirtualAddressMap GUID (gEfiEventVirtualAddressChangeGuid)
EFI_GUID VirtualGuid = { 0x13FA7698, 0xC831, 0x49C7, { 0x87, 0xEA, 0x8F, 0x43, 0xFC, 0xC2, 0x51, 0x96 } };
// ExitBootServices GUID (gEfiEventExitBootServicesGuid)
EFI_GUID ExitGuid = { 0x27ABF055, 0xB1B8, 0x4C26, { 0x80, 0x48, 0x74, 0x8F, 0x37, 0xBA, 0xA2, 0xDF } };

// Pointers to original functions
EFI_GET_TIME oGetTime = NULL;
EFI_SET_TIME oSetTime = NULL;
EFI_GET_WAKEUP_TIME oGetWakeupTime = NULL;
EFI_SET_WAKEUP_TIME oSetWakeupTime = NULL;
EFI_SET_VIRTUAL_ADDRESS_MAP oSetVirtualAddressMap = NULL;
EFI_CONVERT_POINTER oConvertPointer = NULL;
EFI_GET_VARIABLE oGetVariable = NULL;
EFI_SET_VARIABLE oSetVariable = NULL;
EFI_GET_NEXT_VARIABLE_NAME oGetNextVariableName = NULL;
EFI_GET_NEXT_HIGH_MONO_COUNT oGetNextHighMonotonicCount = NULL;
EFI_RESET_SYSTEM oResetSystem = NULL;
EFI_UPDATE_CAPSULE oUpdateCapsule = NULL;
EFI_QUERY_CAPSULE_CAPABILITIES oQueryCapsuleCapabilities = NULL;
EFI_QUERY_VARIABLE_INFO oQueryVariableInfo = NULL;

// Global declarations
EFI_EVENT NotifyEvent = NULL;
EFI_EVENT ExitEvent = NULL;
BOOLEAN Virtual = FALSE;
BOOLEAN Runtime = FALSE;

PsLookupProcessByProcessId GetProcessByPid = (PsLookupProcessByProcessId)NULL;
PsGetProcessSectionBaseAddress GetBaseAddress = (PsGetProcessSectionBaseAddress)NULL;
MmCopyVirtualMemory MCopyVirtualMemory = (MmCopyVirtualMemory)NULL;

// Function that actually performs the r/w
EFI_STATUS RunCommand(MemoryCommand* cmd) {

    // Check if the command has right magic (just to be sure again)
    if (cmd->magic != COMMAND_MAGIC) {
        return EFI_ACCESS_DENIED;
    }

    // Initialize
    if (cmd->operation == baseOperation * 0x45 * 0x2) {
        GetProcessByPid = (PsLookupProcessByProcessId)cmd->data[0];
        GetBaseAddress = (PsGetProcessSectionBaseAddress)cmd->data[1];
        MCopyVirtualMemory = (MmCopyVirtualMemory)cmd->data[2];
        ptr64 resultAddr = cmd->data[3];
        *(ptr64*)resultAddr = 1;

        return EFI_SUCCESS;
    }

    // Copy operation
    if (cmd->operation == baseOperation * 0x45 * 0x3) {
        void* src_process_id = (void*)cmd->data[0];
        void* src_address = (void*)cmd->data[1];
        void* dest_process_id = (void*)cmd->data[2];
        void* dest_address = (void*)cmd->data[3];
        ptr64 size = cmd->data[4];
        void* resultAddr = (void*)cmd->data[5];

        if (src_process_id == (void*)4ULL) {
            CopyMem(dest_address, src_address, size);
        }
        else {
            void* SrcProc = 0;
            void* DstProc = 0;
            ptr64 size_out = 0;
            int status = 0;
            status = GetProcessByPid(src_process_id, &SrcProc);
            if (status < 0) {
                *(ptr64*)resultAddr = status;
                return EFI_SUCCESS;
            }
            status = GetProcessByPid(dest_process_id, &DstProc);
            if (status < 0) {
                *(ptr64*)resultAddr = status;
                return EFI_SUCCESS;
            }
            *(ptr64*)resultAddr = MCopyVirtualMemory(SrcProc, src_address, DstProc, dest_address, size, 1, &size_out);
        }

        return EFI_SUCCESS;
    }

    // Get Process Base Address
    if (cmd->operation == baseOperation * 0x45 * 0x4) {
        void* pid = (void*)cmd->data[0];
        void* resultAddr = (void*)cmd->data[1];
        void* ProcessPtr = 0;

        // Find process by ID or Base Address
        if (GetProcessByPid(pid, &ProcessPtr) < 0 || ProcessPtr == 0) {
            *(ptr64*)resultAddr = 0;
            return EFI_SUCCESS;
        }
        else {
            *(ptr64*)resultAddr = (ptr64)GetBaseAddress(ProcessPtr);
            return EFI_SUCCESS;
        }
    }

    // Invalid command
    return EFI_UNSUPPORTED;
}

// Hooked EFI function SetVariable can be called from Windows with NtSetSystemEnvironmentValueEx
EFI_STATUS EFIAPI mySetVariable(IN CHAR16* VariableName, IN EFI_GUID* VendorGuid, IN UINT32 Attributes, IN UINTN DataSize, IN VOID* Data) {

    // Use our hook only after we are in virtual address-space
    if (Virtual && Runtime) {

        // Check of input is not null
        if (VariableName != NULL && VariableName[0] != CHAR_NULL && VendorGuid != NULL) {

            // Check if variable name is same as our declared one this is used to check if call is really from our program running in the OS (client)
            if (StrnCmp(VariableName, VARIABLE_NAME, (sizeof(VARIABLE_NAME) / sizeof(CHAR16)) - 1) == 0) {

                // Skip no data
                if (DataSize == 0 && Data == NULL) {
                    return EFI_SUCCESS;
                }

                // Check if the data size is correct
                if (DataSize == sizeof(MemoryCommand)) {

                    // We did it! Now we can call the magic function
                    MemoryCommand* cmd = (MemoryCommand*)Data;
                    if (cmd->magic == COMMAND_MAGIC) {
                        return RunCommand(cmd);
                    }
                }
            }
        }
    }

    // Call the original SetVariable() function
    return oSetVariable(VariableName, VendorGuid, Attributes, DataSize, Data);
}

// Event callback when SetVitualAddressMap() is called by OS
VOID EFIAPI SetVirtualAddressMapEvent(IN EFI_EVENT Event, IN VOID* Context) {

    // Convert original SetVariable address
    RT->ConvertPointer(0, (VOID**) &oSetVariable);

    // Convert all other addresses
    RT->ConvertPointer(0, (VOID**)&oGetTime);
    RT->ConvertPointer(0, (VOID**)&oSetTime);
    RT->ConvertPointer(0, (VOID**)&oGetWakeupTime);
    RT->ConvertPointer(0, (VOID**)&oSetWakeupTime);
    RT->ConvertPointer(0, (VOID**)&oSetVirtualAddressMap);
    RT->ConvertPointer(0, (VOID**)&oConvertPointer);
    RT->ConvertPointer(0, (VOID**)&oGetVariable);
    RT->ConvertPointer(0, (VOID**)&oGetNextVariableName);
    // RT->ConvertPointer(0, (VOID**)&oSetVariable);
    RT->ConvertPointer(0, (VOID**)&oGetNextHighMonotonicCount);
    RT->ConvertPointer(0, (VOID**)&oResetSystem);
    RT->ConvertPointer(0, (VOID**)&oUpdateCapsule);
    RT->ConvertPointer(0, (VOID**)&oQueryCapsuleCapabilities);
    RT->ConvertPointer(0, (VOID**)&oQueryVariableInfo);

    // Convert runtime services pointer
    RtLibEnableVirtualMappings();

    // Null and close the event so it does not get called again
    NotifyEvent = NULL;

    // We are now working in virtual address-space
    Virtual = TRUE;
}

// Event callback after boot process is started
VOID EFIAPI ExitBootServicesEvent(IN EFI_EVENT Event, IN VOID* Context) {

    // This event is called only once so close it
    BS->CloseEvent(ExitEvent);
    ExitEvent = NULL;

    // Boot services are now not avaible
    BS = NULL;

    // We are booting the OS now
    Runtime = TRUE;

    // Print some text so we know it works (300iq)
    ST->ConOut->SetAttribute(ST->ConOut, EFI_WHITE | EFI_BACKGROUND_BLUE);
    ST->ConOut->ClearScreen(ST->ConOut);
    CHAR16* str = L"Driver seems to be working as expected! Windows is booting now...\n";
    Print(str);
    SetMem(str, 67 * sizeof(short), 0);
}

// Replaces service table pointer with desired one returns original
VOID* SetServicePointer(IN OUT EFI_TABLE_HEADER* ServiceTableHeader, IN OUT VOID** ServiceTableFunction, IN VOID* NewFunction) {

    // We don't want to fuck up the system
    if (ServiceTableFunction == NULL || NewFunction == NULL)
        return NULL;

    // Make sure boot services pointers are not null
    ASSERT(BS != NULL);
    ASSERT(BS->CalculateCrc32 != NULL);

    // Raise task priority level
    CONST EFI_TPL Tpl = BS->RaiseTPL(TPL_HIGH_LEVEL);

    // Swap the pointers GNU-EFI and InterlockedCompareExchangePointer are not friends
    VOID* OriginalFunction = *ServiceTableFunction;
    *ServiceTableFunction = NewFunction;

    // Change the table CRC32 signature
    ServiceTableHeader->CRC32 = 0;
    BS->CalculateCrc32((UINT8*)ServiceTableHeader, ServiceTableHeader->HeaderSize, &ServiceTableHeader->CRC32);

    // Restore task priority level
    BS->RestoreTPL(Tpl);

    return OriginalFunction;
}

// EFI driver unload routine
EFI_STATUS EFI_FUNCTION efi_unload(IN EFI_HANDLE ImageHandle) {

    // We don't want our driver to be unloaded until complete reboot
    return EFI_ACCESS_DENIED;
}

// EFI entry point
EFI_STATUS efi_main(IN EFI_HANDLE ImageHandle, IN EFI_SYSTEM_TABLE* SystemTable) {

    // Initialize internal GNU-EFI functions
    InitializeLib(ImageHandle, SystemTable);

    // Get handle to this image
    EFI_LOADED_IMAGE* LoadedImage = NULL;
    EFI_STATUS status = BS->OpenProtocol(ImageHandle, &LoadedImageProtocol, (void**)&LoadedImage, ImageHandle, NULL, EFI_OPEN_PROTOCOL_GET_PROTOCOL);

    CHAR16* fstr = L"Can't open protocol: %d\n";

    // Return if protocol failed to open
    if (EFI_ERROR(status)) {
        Print(fstr, status);
        return status;
    }

    // Randomize protocol GUID
    EFI_TIME time = { 0 };
    SetMem(&time, sizeof(EFI_TIME), 0);
    RT->GetTime(&time, NULL);
    ptr64 num = time.Nanosecond + time.Second;
    if (num == 0) {
        num = (ptr64)&ProtocolGuid;
    }
    unsigned char* gdata = (unsigned char*)&ProtocolGuid;
    for (int i = 0; i < 16; i++) {
        gdata[i] = num * gdata[i];
    }

    CHAR16 str[0x100] = { 0 };
    SetMem(str, 0x100, 0);
    GuidToString(str, &ProtocolGuid);
    Print(L"GUID: ");
    Print(str);
    Print(L"\n");

    // Install our protocol interface this is needed to keep our driver loaded
    DummyProtocalData dummy = { 0 };
    status = LibInstallProtocolInterfaces(&ImageHandle, &ProtocolGuid, &dummy, NULL);

    // Return if interface failed to register
    if (EFI_ERROR(status)) {
        Print(L"Can't register interface: %d\n", status);
        return status;
    }

    // Set our image unload routine
    LoadedImage->Unload = (EFI_IMAGE_UNLOAD)efi_unload;

    // Create global event for VirtualAddressMap
    status = BS->CreateEventEx(EVT_NOTIFY_SIGNAL, TPL_NOTIFY, SetVirtualAddressMapEvent, NULL, VirtualGuid, &NotifyEvent);

    // Return if event create failed
    if (EFI_ERROR(status)) {
        Print(L"Can't create event (SetVirtualAddressMapEvent): %d\n", status);
        return status;
    }

    // Create global event for ExitBootServices
    status = BS->CreateEventEx(EVT_NOTIFY_SIGNAL, TPL_NOTIFY, ExitBootServicesEvent, NULL, ExitGuid, &ExitEvent);

    // Return if event create failed (yet again)
    if (EFI_ERROR(status)) {
        Print(L"Can't create event (ExitBootServicesEvent): %d\n", status);
        return status;
    }

    // Hook SetVariable (should not fail)
    oSetVariable = (EFI_SET_VARIABLE)SetServicePointer(&RT->Hdr, (VOID**)&RT->SetVariable, (VOID**)&mySetVariable);

    // Hook all the other runtime services functions
    oGetTime = (EFI_GET_TIME)SetServicePointer(&RT->Hdr, (VOID**)&RT->GetTime, (VOID**)&HookedGetTime);
    oSetTime = (EFI_SET_TIME)SetServicePointer(&RT->Hdr, (VOID**)&RT->SetTime, (VOID**)&HookedSetTime);
    oGetWakeupTime = (EFI_GET_WAKEUP_TIME)SetServicePointer(&RT->Hdr, (VOID**)&RT->GetWakeupTime, (VOID**)&HookedGetWakeupTime);
    oSetWakeupTime = (EFI_SET_WAKEUP_TIME)SetServicePointer(&RT->Hdr, (VOID**)&RT->SetWakeupTime, (VOID**)&HookedSetWakeupTime);
    oSetVirtualAddressMap = (EFI_SET_VIRTUAL_ADDRESS_MAP)SetServicePointer(&RT->Hdr, (VOID**)&RT->SetVirtualAddressMap, (VOID**)&HookedSetVirtualAddressMap);
    oConvertPointer = (EFI_CONVERT_POINTER)SetServicePointer(&RT->Hdr, (VOID**)&RT->ConvertPointer, (VOID**)&HookedConvertPointer);
    oGetVariable = (EFI_GET_VARIABLE)SetServicePointer(&RT->Hdr, (VOID**)&RT->GetVariable, (VOID**)&HookedGetVariable);
    oGetNextVariableName = (EFI_GET_NEXT_VARIABLE_NAME)SetServicePointer(&RT->Hdr, (VOID**)&RT->GetNextVariableName, (VOID**)&HookedGetNextVariableName);
    // oSetVariable = (EFI_SET_VARIABLE)SetServicePointer(&RT->Hdr, (VOID**)&RT->SetVariable, (VOID**)&HookedSetVariable);
    oGetNextHighMonotonicCount = (EFI_GET_NEXT_HIGH_MONO_COUNT)SetServicePointer(&RT->Hdr, (VOID**)&RT->GetNextHighMonotonicCount, (VOID**)&HookedGetNextHighMonotonicCount);
    oResetSystem = (EFI_RESET_SYSTEM)SetServicePointer(&RT->Hdr, (VOID**)&RT->ResetSystem, (VOID**)&HookedResetSystem);
    oUpdateCapsule = (EFI_UPDATE_CAPSULE)SetServicePointer(&RT->Hdr, (VOID**)&RT->UpdateCapsule, (VOID**)&HookedUpdateCapsule);
    oQueryCapsuleCapabilities = (EFI_QUERY_CAPSULE_CAPABILITIES)SetServicePointer(&RT->Hdr, (VOID**)&RT->QueryCapsuleCapabilities, (VOID**)&HookedQueryCapsuleCapabilities);
    oQueryVariableInfo = (EFI_QUERY_VARIABLE_INFO)SetServicePointer(&RT->Hdr, (VOID**)&RT->QueryVariableInfo, (VOID**)&HookedQueryVariableInfo);

    // Print confirmation text
    Print(L"\n");
    Print(L"       __ _                                  \n");
    Print(L"  ___ / _(_)___ _ __  ___ _ __  ___ _ _ _  _ \n");
    Print(L" / -_)  _| |___| '  \\/ -_) '  \\/ _ \\ '_| || |\n");
    Print(L" \\___|_| |_|   |_|_|_\\___|_|_|_\\___/_|  \\_, |\n");
    Print(L"                                        |__/ \n");
    Print(L"Rewrite and modified by Bifeldy\n");
    Print(L"Developed and improved by TheCruZ\n");
    Print(L"Based in efi-memory of Samuel Tulach\n");
    Print(L"Thanks to: @Mattiwatti (EfiGuard), Roderick W. Smith (rodsbooks.com)\n\n");
    Print(L"Driver has been loaded successfully. You can now boot to the OS.\n");
    CHAR16* pos2 = L"If you don't see a blue screen while booting disable Secure Boot!.\n";
    Print(pos2);
    SetMem(fstr, ((ptr64)pos2 - (ptr64)fstr) + (68 * sizeof(short)), 0);

    return EFI_SUCCESS;
}
