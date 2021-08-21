#pragma once

#include "data_type.h"

#include <iostream>

#include <intrin.h>
#include <windows.h>

namespace memory
{
    static int funcCount = 0;

    static ProtectedFunction functions[50];

    static int GetFunctionIndex(void* FunctionAddress)
    {
        for (int i = 0; i < funcCount; i++) {
            if ((uintptr_t)functions[i].address <= (uintptr_t)FunctionAddress &&
                (uintptr_t)functions[i].address + functions[i].size >= (uintptr_t)FunctionAddress) {
                return i;
            }
        }
        return -1;
    }

    static void addFunc(ProtectedFunction func)
    {
        functions[funcCount] = func;
        funcCount++;
    }

    static void XOR(BYTE* data, size_t size, BYTE XOR_KEY = STRING_XOR_KEY)
    {
        for (size_t i = 0; i < size; i++) {
            data[i] = data[i] ^ XOR_KEY;
        }
    }

    static void nextLastXor(int index)
    {
        BYTE xorByte = functions[index].lastXor;
        if (xorByte > 0xf3) {
            xorByte = 0x5;
        }
        xorByte += 0x01;
        functions[index].lastXor = xorByte;
    }

    static void unsafe_unprotect(int index)
    {
        XOR((BYTE*)functions[index].address, functions[index].size, functions[index].lastXor);
    }

    static void unsafe_protect(int index)
    {
        nextLastXor(index);
        unsafe_unprotect(index);
    }

    static void Unprotect(void* FunctionAddress)
    {
        int function = GetFunctionIndex(FunctionAddress);
        if (function > -1 && functions[function].crypted == true) {
            unsafe_unprotect(function);
            functions[function].crypted = false;
        }
    }

    static void Protect(void* FunctionAddress)
    {
        int function = GetFunctionIndex(FunctionAddress);
        if (function > -1 && functions[function].crypted == false) {
            unsafe_protect(function);
            functions[function].crypted = true;
        }
    }
};
