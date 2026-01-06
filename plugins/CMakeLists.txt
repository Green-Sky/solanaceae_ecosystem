cmake_minimum_required(VERSION 3.14...3.24 FATAL_ERROR)

add_library(plugin_transfer_auto_accept MODULE
	./plugin_transfer_auto_accept.cpp
	./transfer_auto_accept.hpp
	./transfer_auto_accept.cpp
)
set_target_properties(plugin_transfer_auto_accept PROPERTIES
	C_VISIBILITY_PRESET hidden
)
target_compile_definitions(plugin_transfer_auto_accept PUBLIC ENTT_API_IMPORT)

target_link_libraries(plugin_transfer_auto_accept PUBLIC
	solanaceae_plugin
	solanaceae_util
	solanaceae_object_store
	solanaceae_message3
	solanaceae_tox_messages # sad, for filekind
)

########################################

add_library(plugin_dice_tool MODULE
	./plugin_dice_tool.cpp
	./dice_tool.hpp
	./dice_tool.cpp
)
set_target_properties(plugin_dice_tool PROPERTIES
	C_VISIBILITY_PRESET hidden
)
target_compile_definitions(plugin_dice_tool PUBLIC ENTT_API_IMPORT)

target_link_libraries(plugin_dice_tool PUBLIC
	solanaceae_plugin
	solanaceae_tox_p2prng
	imgui
)
