#pragma once

#include <windows.h>
#include <winternl.h>

#define BASE_OPERATION 0x7980
#define COMMAND_MAGIC BASE_OPERATION * 0x5478

#define STRING_XOR_KEY 0x6F

#define EFI_VARIABLE_NON_VOLATILE 0x00000001
#define EFI_VARIABLE_BOOTSERVICE_ACCESS 0x00000002
#define EFI_VARIABLE_RUNTIME_ACCESS 0x00000004
#define EFI_VARIABLE_HARDWARE_ERROR_RECORD 0x00000008
#define EFI_VARIABLE_AUTHENTICATED_WRITE_ACCESS 0x00000010
#define EFI_VARIABLE_TIME_BASED_AUTHENTICATED_WRITE_ACCESS 0x00000020
#define EFI_VARIABLE_APPEND_WRITE 0x00000040

#define SE_SYSTEM_ENVIRONMENT_PRIVILEGE (22L)

#define ATTRIBUTES (EFI_VARIABLE_NON_VOLATILE | EFI_VARIABLE_BOOTSERVICE_ACCESS | EFI_VARIABLE_RUNTIME_ACCESS | EFI_VARIABLE_HARDWARE_ERROR_RECORD | EFI_VARIABLE_AUTHENTICATED_WRITE_ACCESS | EFI_VARIABLE_TIME_BASED_AUTHENTICATED_WRITE_ACCESS | EFI_VARIABLE_APPEND_WRITE)

#define M_PI 3.1415926535

typedef NTSTATUS(*mNtQuerySystemInformation) (
    SYSTEM_INFORMATION_CLASS SystemInformationClass,
    PVOID SystemInformation,
    ULONG SystemInformationLength,
    PULONG ReturnLength
);

typedef NTSTATUS(*mNtSetSystemEnvironmentValueEx) (
    PUNICODE_STRING VariableName,
    LPGUID VendorGuid,
    PVOID Value,
    ULONG ValueLength,
    ULONG Attributes
);

typedef NTSTATUS(*mRtlAdjustPrivilege) (
    ULONG Privilege, BOOLEAN Enable,
    BOOLEAN Client,
    PBOOLEAN WasEnabled
);

struct ProtectedFunction {
    void* address;
    size_t size;
    BYTE lastXor;
    bool crypted;
};

typedef struct _MemoryCommand {
    int magic;
    int operation;
    unsigned long long data[6];
} MemoryCommand;

typedef struct _RTL_PROCESS_MODULE_INFORMATION
{
    HANDLE Section;
    PVOID MappedBase;
    PVOID ImageBase;
    ULONG ImageSize;
    ULONG Flags;
    USHORT LoadOrderIndex;
    USHORT InitOrderIndex;
    USHORT LoadCount;
    USHORT OffsetToFileName;
    UCHAR FullPathName[256];
} RTL_PROCESS_MODULE_INFORMATION, * PRTL_PROCESS_MODULE_INFORMATION;

typedef struct _RTL_PROCESS_MODULES
{
    ULONG NumberOfModules;
    RTL_PROCESS_MODULE_INFORMATION Modules[1];
} RTL_PROCESS_MODULES, * PRTL_PROCESS_MODULES;

struct State {
    uintptr_t keys[7];
};

typedef struct {
    uintptr_t actor_ptr;
    uintptr_t damage_handler_ptr;
    uintptr_t player_state_ptr;
    uintptr_t root_component_ptr;
    uintptr_t mesh_ptr;
    uintptr_t bone_array_ptr;
    int bone_count;
    bool is_visible;
} Enemy;
