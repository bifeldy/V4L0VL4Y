#pragma once

#include "memory.h"
#include "driver.h"
#include "base.h"

#include "vector.h"
#include "defs.h"
#include "offset.h"

#include <vector>

using namespace base;

namespace game
{

    template <typename T> static T read(uintptr_t pid, uintptr_t address) {
        return driver::read<T>(pid, address);
    }

    template <typename T> static void write(uintptr_t pid, uintptr_t address, T& buffer) {
        return driver::write<T>(pid, address, buffer);
    }

    static bool g_esp_enabled{ true };
    static bool g_esp_dormantcheck{ true };
    static bool g_headesp{ true };
    static bool g_boneesp{ true };
    static bool g_boxesp{ true };

    static int g_local_team_id;

    static uintptr_t g_local_player_controller;
    static uintptr_t g_local_player_pawn;
    static uintptr_t g_local_damage_handler;
    static uintptr_t g_camera_manager;

    static std::vector<Enemy> enemy_collection;

    static ImU32 g_esp_color = ImGui::ColorConvertFloat4ToU32(ImVec4(1, 0, 0.4F, 1));
    static ImU32 g_color_white = ImGui::ColorConvertFloat4ToU32(ImVec4(1, 1, 1, 1));

    static HANDLE handleOffset;

    Vector3 getBonePosition(Enemy enemy, int index)
    {
        size_t size = sizeof(FTransform);
        FTransform firstBone = read<FTransform>(g_pid, enemy.bone_array_ptr + (size * index));
        FTransform componentToWorld = read<FTransform>(g_pid, enemy.mesh_ptr + offset::component_to_world);
        D3DMATRIX matrix = MatrixMultiplication(firstBone.ToMatrixWithScale(), componentToWorld.ToMatrixWithScale());
        return Vector3(matrix._41, matrix._42, matrix._43);
    }

    void renderBoneLine(Vector3 first_bone_position, Vector3 second_bone_position, Vector3 position, Vector3 rotation, float fov)
    {
        Vector2 first_bone_screen_position = worldToScreen(first_bone_position, position, rotation, fov);
        ImVec2 fist_screen_position = ImVec2(first_bone_screen_position.x, first_bone_screen_position.y);
        Vector2 second_bone_screen_position = worldToScreen(second_bone_position, position, rotation, fov);
        ImVec2 second_screen_position = ImVec2(second_bone_screen_position.x, second_bone_screen_position.y);
        ImGui::GetOverlayDrawList()->AddLine(fist_screen_position, second_screen_position, g_color_white);
    }

    void renderBox(Vector2 head_at_screen, float distance_modifier)
    {
        int head_x = head_at_screen.x;
        int head_y = head_at_screen.y;
        int start_x = head_x - 35 / distance_modifier;
        int start_y = head_y - 15 / distance_modifier;
        int end_x = head_x + 35 / distance_modifier;
        int end_y = head_y + 155 / distance_modifier;
        ImGui::GetOverlayDrawList()->AddRect(ImVec2(start_x, start_y), ImVec2(end_x, end_y), g_esp_color);
    }

    void renderBones(Enemy enemy, Vector3 position, Vector3 rotation, float fov)
    {
        Vector3 head_position = getBonePosition(enemy, 8);
        Vector3 neck_position;
        Vector3 chest_position = getBonePosition(enemy, 6);
        Vector3 l_upper_arm_position;
        Vector3 l_fore_arm_position;
        Vector3 l_hand_position;
        Vector3 r_upper_arm_position;
        Vector3 r_fore_arm_position;
        Vector3 r_hand_position;
        Vector3 stomach_position = getBonePosition(enemy, 4);
        Vector3 pelvis_position = getBonePosition(enemy, 3);
        Vector3 l_thigh_position;
        Vector3 l_knee_position;
        Vector3 l_foot_position;
        Vector3 r_thigh_position;
        Vector3 r_knee_position;
        Vector3 r_foot_position;
        if (enemy.bone_count == 102) { // MALE
            neck_position = getBonePosition(enemy, 19);

            l_upper_arm_position = getBonePosition(enemy, 21);
            l_fore_arm_position = getBonePosition(enemy, 22);
            l_hand_position = getBonePosition(enemy, 23);

            r_upper_arm_position = getBonePosition(enemy, 47);
            r_fore_arm_position = getBonePosition(enemy, 48);
            r_hand_position = getBonePosition(enemy, 49);

            l_thigh_position = getBonePosition(enemy, 75);
            l_knee_position = getBonePosition(enemy, 76);
            l_foot_position = getBonePosition(enemy, 78);

            r_thigh_position = getBonePosition(enemy, 82);
            r_knee_position = getBonePosition(enemy, 83);
            r_foot_position = getBonePosition(enemy, 85);
        }
        else if (enemy.bone_count == 99) { // FEMALE
            neck_position = getBonePosition(enemy, 19);

            l_upper_arm_position = getBonePosition(enemy, 21);
            l_fore_arm_position = getBonePosition(enemy, 40);
            l_hand_position = getBonePosition(enemy, 42);

            r_upper_arm_position = getBonePosition(enemy, 46);
            r_fore_arm_position = getBonePosition(enemy, 65);
            r_hand_position = getBonePosition(enemy, 67);

            l_thigh_position = getBonePosition(enemy, 78);
            l_knee_position = getBonePosition(enemy, 75);
            l_foot_position = getBonePosition(enemy, 77);

            r_thigh_position = getBonePosition(enemy, 80);
            r_knee_position = getBonePosition(enemy, 82);
            r_foot_position = getBonePosition(enemy, 84);
        }
        else if (enemy.bone_count == 103) { // BOT
            neck_position = getBonePosition(enemy, 9);

            l_upper_arm_position = getBonePosition(enemy, 33);
            l_fore_arm_position = getBonePosition(enemy, 30);
            l_hand_position = getBonePosition(enemy, 32);

            r_upper_arm_position = getBonePosition(enemy, 58);
            r_fore_arm_position = getBonePosition(enemy, 55);
            r_hand_position = getBonePosition(enemy, 57);

            l_thigh_position = getBonePosition(enemy, 63);
            l_knee_position = getBonePosition(enemy, 65);
            l_foot_position = getBonePosition(enemy, 69);

            r_thigh_position = getBonePosition(enemy, 77);
            r_knee_position = getBonePosition(enemy, 79);
            r_foot_position = getBonePosition(enemy, 83);
        }
        else {
            return;
        }

        renderBoneLine(head_position, neck_position, position, rotation, fov);
        renderBoneLine(neck_position, chest_position, position, rotation, fov);
        renderBoneLine(neck_position, l_upper_arm_position, position, rotation, fov);
        renderBoneLine(l_upper_arm_position, l_fore_arm_position, position, rotation, fov);
        renderBoneLine(l_fore_arm_position, l_hand_position, position, rotation, fov);
        renderBoneLine(neck_position, r_upper_arm_position, position, rotation, fov);
        renderBoneLine(r_upper_arm_position, r_fore_arm_position, position, rotation, fov);
        renderBoneLine(r_fore_arm_position, r_hand_position, position, rotation, fov);
        renderBoneLine(chest_position, stomach_position, position, rotation, fov);
        renderBoneLine(stomach_position, pelvis_position, position, rotation, fov);
        renderBoneLine(pelvis_position, l_thigh_position, position, rotation, fov);
        renderBoneLine(l_thigh_position, l_knee_position, position, rotation, fov);
        renderBoneLine(l_knee_position, l_foot_position, position, rotation, fov);
        renderBoneLine(pelvis_position, r_thigh_position, position, rotation, fov);
        renderBoneLine(r_thigh_position, r_knee_position, position, rotation, fov);
        renderBoneLine(r_knee_position, r_foot_position, position, rotation, fov);
    }

    void renderEsp()
    {
        std::vector<Enemy> local_enemy_collection = enemy_collection;
        if (local_enemy_collection.empty()) {
            return;
        }

        Vector3 camera_position = read<Vector3>(g_pid, g_camera_manager + offset::camera_position);
        Vector3 camera_rotation = read<Vector3>(g_pid, g_camera_manager + offset::camera_rotation);
        float camera_fov = read<float>(g_pid, g_camera_manager + offset::camera_fov);

        for (int i = 0; i < local_enemy_collection.size(); i++) {
            Enemy enemy = local_enemy_collection[i];
            float health = read<float>(g_pid, enemy.damage_handler_ptr + offset::health);
            if (enemy.actor_ptr == g_local_player_pawn || health <= 0 || !enemy.mesh_ptr) {
                continue;
            }

            Vector3 head_position = getBonePosition(enemy, 8); // 8 = head bone
            Vector3 root_position = read<Vector3>(g_pid, enemy.root_component_ptr + offset::root_position);
            if (head_position.z <= root_position.z) {
                continue;
            }

            if (g_esp_dormantcheck) {
                float last_render_time = read<float>(g_pid, enemy.mesh_ptr + offset::last_render_time);
                float last_submit_time = read<float>(g_pid, enemy.mesh_ptr + offset::last_submit_time);
                bool is_visible = last_render_time + 0.06F >= last_submit_time;
                bool dormant = read<bool>(g_pid, enemy.actor_ptr + offset::dormant);
                if (!dormant || !is_visible) {
                    continue;
                }
            }

            Vector2 head_at_screen_vec = worldToScreen(head_position, camera_position, camera_rotation, camera_fov);
            ImVec2 head_at_screen = ImVec2(head_at_screen_vec.x, head_at_screen_vec.y);
            float distance_modifier = camera_position.Distance(head_position) * 0.001F;

            if (g_boneesp) {
                renderBones(enemy, camera_position, camera_rotation, camera_fov);
            }
            if (g_headesp) {
                ImGui::GetOverlayDrawList()->AddCircle(head_at_screen, 7 / distance_modifier, g_esp_color, 0, 3);
            }
            if (g_boxesp) {
                renderBox(head_at_screen_vec, distance_modifier);
            }
        }
    }

    void handleOtherKeyPresses()
    {
        if (GetAsyncKeyState(VK_INSERT) & 1) {
            g_overlay_visible = !g_overlay_visible;
            glfwSetWindowAttrib(g_window, GLFW_MOUSE_PASSTHROUGH, !g_overlay_visible);
            if (g_overlay_visible) {
                SetForegroundWindow(getOverlayWindow());
            }
            else {
                showValorantWindow();
            }
        }
    }

    void runRenderTick()
    {
        overlayStart();

        if (g_esp_enabled) {
            renderEsp();
        }

        if (g_overlay_visible) {
            {
                ImGui::Begin("Visuals", nullptr, ImGuiWindowFlags_NoResize);
                ImGui::SetWindowSize("Visuals", ImVec2(200, 176));
                ImGui::Checkbox("ESP", &g_esp_enabled);
                ImGui::Checkbox("ESP Dormant Check", &g_esp_dormantcheck);
                ImGui::Checkbox("Head ESP", &g_headesp);
                ImGui::Checkbox("Bone ESP", &g_boneesp);
                ImGui::Checkbox("Box ESP", &g_boxesp);
                ImGui::End();
            }
        }

        overlayEnd();
    }

    std::vector<Enemy> getValidEnemies(uintptr_t actor_array, int actor_count)
    {
        std::vector<Enemy> temp_enemy_collection{};
        size_t size = sizeof(uintptr_t);

        for (int i = 0; i < actor_count; i++) {

            uintptr_t actor = read<uintptr_t>(g_pid, actor_array + (i * size));
            if (actor == 0x00) {
                continue;
            }
            uintptr_t unique_id = read<uintptr_t>(g_pid, actor + offset::unique_id);
            if (unique_id != 18743553) {
                continue;
            }
            uintptr_t mesh = read<uintptr_t>(g_pid, actor + offset::mesh_component);
            if (!mesh) {
                continue;
            }

            uintptr_t player_state = read<uintptr_t>(g_pid, actor + offset::player_state);
            uintptr_t team_component = read<uintptr_t>(g_pid, player_state + offset::team_component);
            int team_id = read<int>(g_pid, team_component + offset::team_id);
            int bone_count = read<int>(g_pid, mesh + offset::bone_count);
            bool is_bot = bone_count == 103;
            if (team_id == g_local_team_id && !is_bot) {
                continue;
            }

            uintptr_t damage_handler = read<uintptr_t>(g_pid, actor + offset::damage_handler);
            uintptr_t root_component = read<uintptr_t>(g_pid, actor + offset::root_component);
            uintptr_t bone_array = read<uintptr_t>(g_pid, mesh + offset::bone_array);

            Enemy enemy{
                actor,
                damage_handler,
                player_state,
                root_component,
                mesh,
                bone_array,
                bone_count,
                true
            };

            temp_enemy_collection.push_back(enemy);
        }

        return temp_enemy_collection;
    }

    uintptr_t decryptWorld(uintptr_t pid, uintptr_t base_address)
    {
        const auto key = read<uintptr_t>(pid, base_address + offset::uworld_key);
        const auto state = read<State>(pid, base_address + offset::uworld_state);
        const auto uworld_ptr = decrypt_uworld(key, (uintptr_t*)&state);
        return read<uintptr_t>(pid, uworld_ptr);
    }

    void getOffset(LPVOID lpParameter)
    {
        while (true) {

            uintptr_t world = decryptWorld(g_pid, g_base_address);
            uintptr_t game_instance = read<uintptr_t>(g_pid, world + offset::game_instance);
            uintptr_t persistent_level = read<uintptr_t>(g_pid, world + offset::persistent_level);
            uintptr_t local_player_array = read<uintptr_t>(g_pid, game_instance + offset::local_player_array);
            uintptr_t local_player = read<uintptr_t>(g_pid, local_player_array);
            uintptr_t local_player_controller = read<uintptr_t>(g_pid, local_player + offset::local_player_controller);
            uintptr_t local_player_pawn = read<uintptr_t>(g_pid, local_player_controller + offset::local_player_pawn);
            uintptr_t local_damage_handler = read<uintptr_t>(g_pid, local_player_pawn + offset::damage_handler);
            uintptr_t local_player_state = read<uintptr_t>(g_pid, local_player_pawn + offset::player_state);
            uintptr_t local_team_component = read<uintptr_t>(g_pid, local_player_state + offset::team_component);
            uintptr_t camera_manager = read<uintptr_t>(g_pid, local_player_controller + offset::camera_manager);
            uintptr_t actor_array = read<uintptr_t>(g_pid, persistent_level + offset::actor_array);
            int local_team_id = read<int>(g_pid, local_team_component + offset::team_id);
            int actor_count = read<int>(g_pid, persistent_level + offset::actor_count);

            g_local_player_controller = local_player_controller;
            g_local_player_pawn = local_player_pawn;
            g_local_damage_handler = local_damage_handler;
            g_camera_manager = camera_manager;
            g_local_team_id = local_team_id;
            enemy_collection = getValidEnemies(actor_array, actor_count);

            Sleep(2500);
        }
    }

    void stop()
    {
        if (handleOffset) {
            CloseHandle(handleOffset);
        }
        cleanupWindow();
    }

    void run()
    {
        if (initialize()) {

            handleOffset = CreateThread(nullptr, NULL, (LPTHREAD_START_ROUTINE)getOffset, nullptr, NULL, nullptr);
            if (handleOffset) {
                CloseHandle(handleOffset);
            }

            while (!glfwWindowShouldClose(g_window))
            {
                if (GetAsyncKeyState(VK_F12) & 1) {
                    break;
                }

                handleOtherKeyPresses();
                runRenderTick();
            }
            stop();
        }
    }
};

