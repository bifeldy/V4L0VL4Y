#pragma once

#define GLFW_EXPOSE_NATIVE_WIN32

#include "GL/glew.h"
#include "imgui/imgui.h"
#include "imgui/imgui_impl_glfw.h"
#include "imgui/imgui_impl_opengl3.h"

#include "memory.h"
#include "driver.h"

#include <iostream>
#include <GLFW/glfw3.h>
#include <GLFW/glfw3native.h>
#include <windows.h>
#include <tlHelp32.h>

namespace base
{

    static int g_width;
    static int g_height;
    static bool g_overlay_visible{ false };

    static bool enableVsync = false;

    static uintptr_t g_pid;
    static uintptr_t g_base_address;

    static GLFWmonitor* monitor;
    static GLFWwindow* g_window;

    static HWND valorantWindow;

    std::wstring s2ws(const std::string& str)
    {
        int size_needed = MultiByteToWideChar(CP_UTF8, 0, &str[0], (int)str.size(), NULL, 0);
        std::wstring wstrTo(size_needed, 0);
        MultiByteToWideChar(CP_UTF8, 0, &str[0], (int)str.size(), &wstrTo[0], size_needed);
        return wstrTo;
    }

    static uintptr_t getValorantPid()
    {
        BYTE target_name[] = { 'V','A','L','O','R','A','N','T','-','W','i','n','6','4','-','S','h','i','p','p','i','n','g','.','e','x','e',0 };
        std::wstring process_name = s2ws(std::string((char*)target_name));
        HANDLE snapshot = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0);
        PROCESSENTRY32W entry;
        entry.dwSize = sizeof(entry);
        if (!Process32First(snapshot, &entry)) {
            return 0;
        }
        while (Process32Next(snapshot, &entry)) {
            if (std::wstring(entry.szExeFile) == process_name) {
                return entry.th32ProcessID;
            }
        }
        return 0;
    }

    static uintptr_t getBaseAddress(uintptr_t pid)
    {
        memory::Unprotect(driver::GetBaseAddress);
        uintptr_t BaseAddr = driver::GetBaseAddress(pid);
        memory::Protect(driver::GetBaseAddress);
        return BaseAddr;
    }

    void glfwErrorCallback(int error, const char* description)
    {
        std::cout << "Glfw Error :: " << error << " :: " << description << std::endl;
    }

    bool setupWindow()
    {
        glfwSetErrorCallback(glfwErrorCallback);
        if (!glfwInit()) {
            std::cout << "glfwInit Not Work!" << std::endl;
            return false;
        }

        monitor = glfwGetPrimaryMonitor();
        if (!monitor) {
            std::cout << "Failed To Get Primary Monitor!" << std::endl;
            return false;
        }

        g_width = glfwGetVideoMode(monitor)->width;
        g_height = glfwGetVideoMode(monitor)->height;

        glfwWindowHint(GLFW_FLOATING, true);
        glfwWindowHint(GLFW_RESIZABLE, false);
        glfwWindowHint(GLFW_MAXIMIZED, true);
        glfwWindowHint(GLFW_TRANSPARENT_FRAMEBUFFER, true);
        glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
        glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
        glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

        g_window = glfwCreateWindow(g_width, g_height, "Word", NULL, NULL);
        if (g_window == NULL) {
            std::cout << "Could Not Create Window!" << std::endl;
            return false;
        }

        glfwSetWindowAttrib(g_window, GLFW_DECORATED, false);
        glfwSetWindowAttrib(g_window, GLFW_MOUSE_PASSTHROUGH, true);
        glfwSetWindowMonitor(g_window, NULL, 0, 0, g_width, g_height + 1, 0);

        glfwMakeContextCurrent(g_window);
        if (enableVsync)
        {
            glfwSwapInterval(1);
        }

        if (glewInit() != GLEW_OK)
        {
            std::cout << "Failed To Initialize OpenGL Loader!" << std::endl;
            return false;
        }

        IMGUI_CHECKVERSION();
        ImGui::CreateContext();
        ImGuiIO& io = ImGui::GetIO();
        (void)io;

        ImGui::StyleColorsLight();

        ImGui_ImplGlfw_InitForOpenGL(g_window, true);
        ImGui_ImplOpenGL3_Init("#version 130");

        std::cout << "Loading Font Roboto-Light.ttf ..." << std::endl;
        ImFont* font = io.Fonts->AddFontFromFileTTF("Roboto-Light.ttf", 18.0f, NULL, io.Fonts->GetGlyphRangesJapanese());

        return true;
    }

    void showValorantWindow()
    {
        SetForegroundWindow(valorantWindow);
        mouse_event(MOUSEEVENTF_LEFTDOWN, 0, 0, 0, 0);
        mouse_event(MOUSEEVENTF_LEFTUP, 0, 0, 0, 0);
    }

    HWND getOverlayWindow()
    {
        return glfwGetWin32Window(g_window);
    }

    void overlayStart()
    {
        glfwPollEvents();
        ImGui_ImplOpenGL3_NewFrame();
        ImGui_ImplGlfw_NewFrame();
        ImGui::NewFrame();
    }

    void overlayEnd()
    {
        ImGui::Render();
        int display_w, display_h;
        glfwGetFramebufferSize(g_window, &display_w, &display_h);
        glViewport(0, 0, display_w, display_h);
        glClear(GL_COLOR_BUFFER_BIT);
        ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
        glfwSwapBuffers(g_window);
    }

    BOOL CALLBACK getValorantWindow(HWND hwnd, LPARAM lparam)
    {
        DWORD pid;
        GetWindowThreadProcessId(hwnd, &pid);
        if (pid == g_pid) {
            valorantWindow = hwnd;
            return FALSE;
        }
        return TRUE;
    }

    bool initialize()
    {
        std::cout << "Getting Valorant Process Id ..." << std::endl;
        g_pid = getValorantPid();
        if (!g_pid) {
            std::cout << "Could Not Find Valorant Process Id!" << std::endl;
            return false;
        }
        std::cout << "-> g_pid :: " << g_pid << std::endl;

        std::cout << "Getting Valorant Game Window ..." << std::endl;
        EnumWindows(getValorantWindow, NULL);
        if (!valorantWindow) {
            std::cout << "Could Not Find Valorant Window!" << std::endl;
            return false;
        }
        std::cout << "-> valorantWindow :: " << valorantWindow << std::endl;

        std::cout << "Getting Base Address ..." << std::endl;
        g_base_address = getBaseAddress(g_pid);
        if (!g_base_address) {
            std::cout << "Could Not Get Base Address!" << std::endl;
            return false;
        }
        std::cout << "-> g_base_address :: " << g_base_address << std::endl;

        std::cout << "Setting Up ImgUI Overlay ..." << std::endl;
        setupWindow();
        if (!g_window) {
            std::cout << "Could Not Setup Window Overlay!" << std::endl;
            return false;
        }
        std::cout << "-> g_window :: " << g_window << std::endl;

        return true;
    }

    void cleanupWindow()
    {
        ImGui_ImplOpenGL3_Shutdown();
        ImGui_ImplGlfw_Shutdown();
        ImGui::DestroyContext();

        glfwDestroyWindow(g_window);
        glfwTerminate();
    }
};
