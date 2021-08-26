#pragma once

#include <cstdint>

namespace offset
{
	constexpr uint64_t uworld_key = 0x87109B8;
	constexpr uint64_t uworld_state = 0x8710980;
	constexpr uint64_t game_instance = 0x1A8;
	constexpr uint64_t persistent_level = 0x38;
	constexpr uint64_t local_player_array = 0x40;
	constexpr uint64_t local_player_controller = 0x38;
	constexpr uint64_t local_player_pawn = 0x440;
	constexpr uint64_t control_rotation = 0x420;
	constexpr uint64_t camera_manager = 0x458;
	constexpr uint64_t camera_position = 0x1220;
	constexpr uint64_t camera_rotation = 0x122C;
	constexpr uint64_t camera_fov = 0x1238;
	constexpr uint64_t actor_array = 0xA0;
	constexpr uint64_t actor_count = 0xB8;
	constexpr uint64_t unique_id = 0x38;
	constexpr uint64_t mesh_component = 0x410;
	constexpr uint64_t last_render_time = 0x350;
	constexpr uint64_t last_submit_time = 0x358;
	constexpr uint64_t bone_array = 0x558;
	constexpr uint64_t bone_count = 0x560;
	constexpr uint64_t component_to_world = 0x250;
	constexpr uint64_t root_component = 0x210;
	constexpr uint64_t root_position = 0x164;
	constexpr uint64_t damage_handler = 0x968;
	constexpr uint64_t health = 0x1B0;
	constexpr uint64_t dormant = 0x100;
	constexpr uint64_t player_state = 0x3D0;
	constexpr uint64_t team_component = 0x580;
	constexpr uint64_t team_id = 0xF8;
	constexpr auto FresnelIntensity = 0x690;
	constexpr auto FresnelOffset = 0x694;
	constexpr auto CachedFresnelColor = 0x680;
	constexpr auto CachedFresnelOffset = 0x684;
	constexpr auto CachedFresnelIntensity = 0x688;
	constexpr auto CachedLocalFresnelOffset = 0x68c;
}
