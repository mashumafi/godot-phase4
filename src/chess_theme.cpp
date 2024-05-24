#include "chess_theme.h"

#include <godot_cpp/core/class_db.hpp>
#include <godot_cpp/classes/rendering_server.hpp>

using namespace godot;

void ChessTheme::_bind_methods() {
    const StringName class_name = "ChessTheme";
    const StringName texture_name = "Texture";

    const StringName get_white_pawn_texture_method = "get_white_pawn_texture";
    const StringName set_white_pawn_texture_method = "set_white_pawn_texture";
    const StringName white_pawn_texture_property = "white_pawn_texture";
	ClassDB::bind_method(D_METHOD(get_white_pawn_texture_method), &ChessTheme::get_white_pawn_texture);
	ClassDB::bind_method(D_METHOD(set_white_pawn_texture_method, white_pawn_texture_property), &ChessTheme::set_white_pawn_texture);
	ClassDB::add_property(class_name, PropertyInfo(Variant::OBJECT, white_pawn_texture_property, PROPERTY_HINT_NONE, texture_name, PROPERTY_USAGE_DEFAULT), set_white_pawn_texture_method, get_white_pawn_texture_method);
}
