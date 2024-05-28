#include "chess_theme.h"

#include <godot_cpp/classes/array_mesh.hpp>
#include <godot_cpp/core/class_db.hpp>
#include <godot_cpp/core/math.hpp>

#include <phase4/engine/common/field_index.h>

using namespace godot;

namespace {
static Ref<Mesh> create_strip_mesh(const PackedVector2Array &vertices, Color color) {
	Ref<ArrayMesh> mesh;
	mesh.instantiate();

	Array mesh_arrays;
	mesh_arrays.resize(Mesh::ARRAY_MAX);

	mesh_arrays[Mesh::ARRAY_VERTEX] = vertices;

	PackedVector2Array uvs;
	uvs.resize(vertices.size());
	mesh_arrays[Mesh::ARRAY_TEX_UV] = uvs;

	PackedColorArray colors;
	colors.resize(vertices.size());
	colors.fill(color);
	mesh_arrays[Mesh::ARRAY_COLOR] = colors;

	PackedVector3Array normals;
	normals.resize(vertices.size());
	mesh_arrays[Mesh::ARRAY_NORMAL] = normals;

	PackedInt32Array indices;
	for (size_t i = 0; i < vertices.size(); ++i) {
		indices.push_back(i);
	}
	mesh_arrays[Mesh::ARRAY_INDEX] = indices;

	mesh->add_surface_from_arrays(Mesh::PrimitiveType::PRIMITIVE_TRIANGLE_STRIP, mesh_arrays);

	return mesh;
}

static Ref<Mesh> create_arrow_polygon(real_t square_size, Color color, phase4::engine::common::Square from, phase4::engine::common::Square to) {
	using namespace phase4::engine::common;

	const Vector2 half_square = Vector2(.5, .5) * square_size;

	const real_t offset = -square_size * 4;
	const Vector2 start_position(offset, offset);

	const FieldIndex start_field = from.asFieldIndex();
	const FieldIndex end_field = to.asFieldIndex();

	const Vector2 begin = start_position + Vector2(start_field.x, 7 - start_field.y) * square_size + half_square;
	const Vector2 end = start_position + Vector2(end_field.x, 7 - end_field.y) * square_size + half_square;

	const Vector2 line = end - begin;
	const real_t angle = line.angle();

	const Vector2 adjusted_begin = begin.move_toward(end, square_size / 4);
	const Vector2 adjusted_end = end.move_toward(begin, square_size / 2);

	const Vector2 top = Vector2(0, -square_size / 8).rotated(angle);
	const Vector2 bottom = Vector2(0, square_size / 8).rotated(angle);

	PackedVector2Array polygon;
	polygon.resize(7);
	polygon[0] = adjusted_begin + top;
	polygon[1] = adjusted_begin + bottom;
	polygon[2] = adjusted_end + top;
	polygon[3] = adjusted_end + bottom;
	polygon[4] = adjusted_end + Vector2(0, -square_size / 4).rotated(angle);
	polygon[5] = adjusted_end + Vector2(square_size / 4, 0).rotated(angle);
	polygon[6] = adjusted_end + Vector2(0, square_size / 4).rotated(angle);

	return create_strip_mesh(polygon, color);
}

static Ref<Mesh> create_circle_polygon(real_t square_size, Color color, phase4::engine::common::Square square) {
	using namespace phase4::engine::common;

	const size_t size = 32;
	const real_t delta = Math_TAU / (size - 1);
	const Vector2 half_square = Vector2(.5, .5) * square_size;

	const real_t offset = -square_size * 4;
	const Vector2 start_position(offset, offset);

	const FieldIndex field = square.asFieldIndex();

	const Vector2 center = start_position + Vector2(field.x, 7 - field.y) * square_size + half_square;

	const Vector2 outterRadius(square_size / 2.0f * .9f, 0);
	const Vector2 innerRadius(square_size / 2.0f * .8f, 0);

	PackedVector2Array circlePolygon;
	circlePolygon.resize(size * 2);
	for (size_t r = 0; r < size; ++r) {
		circlePolygon[r * 2] = center + outterRadius.rotated(r * delta);
		circlePolygon[r * 2 + 1] = center + innerRadius.rotated(r * delta);
	}

	return create_strip_mesh(circlePolygon, color);
}
} //namespace

void ChessTheme::_bind_methods() {
	const StringName class_name = "ChessTheme";

	{ // White Pawn
		const StringName get_white_pawn_texture_method = "get_white_pawn_texture";
		const StringName set_white_pawn_texture_method = "set_white_pawn_texture";
		const StringName white_pawn_texture_property = "white_pawn_texture";
		ClassDB::bind_method(D_METHOD(get_white_pawn_texture_method), &ChessTheme::get_white_pawn_texture);
		ClassDB::bind_method(D_METHOD(set_white_pawn_texture_method, white_pawn_texture_property), &ChessTheme::set_white_pawn_texture);
		ClassDB::add_property(class_name, PropertyInfo(Variant::OBJECT, white_pawn_texture_property, PROPERTY_HINT_NONE), set_white_pawn_texture_method, get_white_pawn_texture_method);
	}

	{ // White Knight
		const StringName get_white_knight_texture_method = "get_white_knight_texture";
		const StringName set_white_knight_texture_method = "set_white_knight_texture";
		const StringName white_knight_texture_property = "white_knight_texture";
		ClassDB::bind_method(D_METHOD(get_white_knight_texture_method), &ChessTheme::get_white_knight_texture);
		ClassDB::bind_method(D_METHOD(set_white_knight_texture_method, white_knight_texture_property), &ChessTheme::set_white_knight_texture);
		ClassDB::add_property(class_name, PropertyInfo(Variant::OBJECT, white_knight_texture_property, PROPERTY_HINT_NONE), set_white_knight_texture_method, get_white_knight_texture_method);
	}

	{ // White Bishop
		const StringName get_white_bishop_texture_method = "get_white_bishop_texture";
		const StringName set_white_bishop_texture_method = "set_white_bishop_texture";
		const StringName white_bishop_texture_property = "white_bishop_texture";
		ClassDB::bind_method(D_METHOD(get_white_bishop_texture_method), &ChessTheme::get_white_bishop_texture);
		ClassDB::bind_method(D_METHOD(set_white_bishop_texture_method, white_bishop_texture_property), &ChessTheme::set_white_bishop_texture);
		ClassDB::add_property(class_name, PropertyInfo(Variant::OBJECT, white_bishop_texture_property, PROPERTY_HINT_NONE), set_white_bishop_texture_method, get_white_bishop_texture_method);
	}

	{ // White Rook
		const StringName get_white_rook_texture_method = "get_white_rook_texture";
		const StringName set_white_rook_texture_method = "set_white_rook_texture";
		const StringName white_rook_texture_property = "white_rook_texture";
		ClassDB::bind_method(D_METHOD(get_white_rook_texture_method), &ChessTheme::get_white_rook_texture);
		ClassDB::bind_method(D_METHOD(set_white_rook_texture_method, white_rook_texture_property), &ChessTheme::set_white_rook_texture);
		ClassDB::add_property(class_name, PropertyInfo(Variant::OBJECT, white_rook_texture_property, PROPERTY_HINT_NONE), set_white_rook_texture_method, get_white_rook_texture_method);
	}

	{ // White Queen
		const StringName get_white_queen_texture_method = "get_white_queen_texture";
		const StringName set_white_queen_texture_method = "set_white_queen_texture";
		const StringName white_queen_texture_property = "white_queen_texture";
		ClassDB::bind_method(D_METHOD(get_white_queen_texture_method), &ChessTheme::get_white_queen_texture);
		ClassDB::bind_method(D_METHOD(set_white_queen_texture_method, white_queen_texture_property), &ChessTheme::set_white_queen_texture);
		ClassDB::add_property(class_name, PropertyInfo(Variant::OBJECT, white_queen_texture_property, PROPERTY_HINT_NONE), set_white_queen_texture_method, get_white_queen_texture_method);
	}

	{ // White King
		const StringName get_white_king_texture_method = "get_white_king_texture";
		const StringName set_white_king_texture_method = "set_white_king_texture";
		const StringName white_king_texture_property = "white_king_texture";
		ClassDB::bind_method(D_METHOD(get_white_king_texture_method), &ChessTheme::get_white_king_texture);
		ClassDB::bind_method(D_METHOD(set_white_king_texture_method, white_king_texture_property), &ChessTheme::set_white_king_texture);
		ClassDB::add_property(class_name, PropertyInfo(Variant::OBJECT, white_king_texture_property, PROPERTY_HINT_NONE), set_white_king_texture_method, get_white_king_texture_method);
	}

	{ // Black Pawn
		const StringName get_black_pawn_texture_method = "get_black_pawn_texture";
		const StringName set_black_pawn_texture_method = "set_black_pawn_texture";
		const StringName black_pawn_texture_property = "black_pawn_texture";
		ClassDB::bind_method(D_METHOD(get_black_pawn_texture_method), &ChessTheme::get_black_pawn_texture);
		ClassDB::bind_method(D_METHOD(set_black_pawn_texture_method, black_pawn_texture_property), &ChessTheme::set_black_pawn_texture);
		ClassDB::add_property(class_name, PropertyInfo(Variant::OBJECT, black_pawn_texture_property, PROPERTY_HINT_NONE), set_black_pawn_texture_method, get_black_pawn_texture_method);
	}

	{ // Black Knight
		const StringName get_black_knight_texture_method = "get_black_knight_texture";
		const StringName set_black_knight_texture_method = "set_black_knight_texture";
		const StringName black_knight_texture_property = "black_knight_texture";
		ClassDB::bind_method(D_METHOD(get_black_knight_texture_method), &ChessTheme::get_black_knight_texture);
		ClassDB::bind_method(D_METHOD(set_black_knight_texture_method, black_knight_texture_property), &ChessTheme::set_black_knight_texture);
		ClassDB::add_property(class_name, PropertyInfo(Variant::OBJECT, black_knight_texture_property, PROPERTY_HINT_NONE), set_black_knight_texture_method, get_black_knight_texture_method);
	}

	{ // Black Bishop
		const StringName get_black_bishop_texture_method = "get_black_bishop_texture";
		const StringName set_black_bishop_texture_method = "set_black_bishop_texture";
		const StringName black_bishop_texture_property = "black_bishop_texture";
		ClassDB::bind_method(D_METHOD(get_black_bishop_texture_method), &ChessTheme::get_black_bishop_texture);
		ClassDB::bind_method(D_METHOD(set_black_bishop_texture_method, black_bishop_texture_property), &ChessTheme::set_black_bishop_texture);
		ClassDB::add_property(class_name, PropertyInfo(Variant::OBJECT, black_bishop_texture_property, PROPERTY_HINT_NONE), set_black_bishop_texture_method, get_black_bishop_texture_method);
	}

	{ // Black Rook
		const StringName get_black_rook_texture_method = "get_black_rook_texture";
		const StringName set_black_rook_texture_method = "set_black_rook_texture";
		const StringName black_rook_texture_property = "black_rook_texture";
		ClassDB::bind_method(D_METHOD(get_black_rook_texture_method), &ChessTheme::get_black_rook_texture);
		ClassDB::bind_method(D_METHOD(set_black_rook_texture_method, black_rook_texture_property), &ChessTheme::set_black_rook_texture);
		ClassDB::add_property(class_name, PropertyInfo(Variant::OBJECT, black_rook_texture_property, PROPERTY_HINT_NONE), set_black_rook_texture_method, get_black_rook_texture_method);
	}

	{ // Black Queen
		const StringName get_black_queen_texture_method = "get_black_queen_texture";
		const StringName set_black_queen_texture_method = "set_black_queen_texture";
		const StringName black_queen_texture_property = "black_queen_texture";
		ClassDB::bind_method(D_METHOD(get_black_queen_texture_method), &ChessTheme::get_black_queen_texture);
		ClassDB::bind_method(D_METHOD(set_black_queen_texture_method, black_queen_texture_property), &ChessTheme::set_black_queen_texture);
		ClassDB::add_property(class_name, PropertyInfo(Variant::OBJECT, black_queen_texture_property, PROPERTY_HINT_NONE), set_black_queen_texture_method, get_black_queen_texture_method);
	}

	{ // Black King
		const StringName get_black_king_texture_method = "get_black_king_texture";
		const StringName set_black_king_texture_method = "set_black_king_texture";
		const StringName black_king_texture_property = "black_king_texture";
		ClassDB::bind_method(D_METHOD(get_black_king_texture_method), &ChessTheme::get_black_king_texture);
		ClassDB::bind_method(D_METHOD(set_black_king_texture_method, black_king_texture_property), &ChessTheme::set_black_king_texture);
		ClassDB::add_property(class_name, PropertyInfo(Variant::OBJECT, black_king_texture_property, PROPERTY_HINT_NONE), set_black_king_texture_method, get_black_king_texture_method);
	}

	{ // White Square Color
		const StringName get_white_square_color_method = "get_white_square_color";
		const StringName set_white_square_color_method = "set_white_square_color";
		const StringName white_square_color_property = "white_square_color";
		ClassDB::bind_method(D_METHOD(get_white_square_color_method), &ChessTheme::get_white_square_color);
		ClassDB::bind_method(D_METHOD(set_white_square_color_method, white_square_color_property), &ChessTheme::set_white_square_color);
		ClassDB::add_property(class_name, PropertyInfo(Variant::COLOR, white_square_color_property, PROPERTY_HINT_NONE), set_white_square_color_method, get_white_square_color_method);
	}

	{ // Black Square Color
		const StringName get_black_square_color_method = "get_black_square_color";
		const StringName set_black_square_color_method = "set_black_square_color";
		const StringName black_square_color_property = "black_square_color";
		ClassDB::bind_method(D_METHOD(get_black_square_color_method), &ChessTheme::get_black_square_color);
		ClassDB::bind_method(D_METHOD(set_black_square_color_method, black_square_color_property), &ChessTheme::set_black_square_color);
		ClassDB::add_property(class_name, PropertyInfo(Variant::COLOR, black_square_color_property, PROPERTY_HINT_NONE), set_black_square_color_method, get_black_square_color_method);
	}

	{ // Square Size
		const StringName get_square_size_method = "get_square_size";
		const StringName set_square_size_method = "set_square_size";
		const StringName square_size_property = "square_size";
		ClassDB::bind_method(D_METHOD(get_square_size_method), &ChessTheme::get_square_size);
		ClassDB::bind_method(D_METHOD(set_square_size_method, square_size_property), &ChessTheme::set_square_size);
		ClassDB::add_property(class_name, PropertyInfo(Variant::FLOAT, square_size_property, PROPERTY_HINT_NONE), set_square_size_method, get_square_size_method);
	}

	{ // Font
		const StringName get_font_method = "get_font";
		const StringName set_font_method = "set_font";
		const StringName font_property = "font";
		ClassDB::bind_method(D_METHOD(get_font_method), &ChessTheme::get_font);
		ClassDB::bind_method(D_METHOD(set_font_method, font_property), &ChessTheme::set_font);
		ClassDB::add_property(class_name, PropertyInfo(Variant::OBJECT, font_property, PROPERTY_HINT_NONE), set_font_method, get_font_method);
	}

	{ // Annotation Color
		const StringName get_annotation_color_method = "get_annotation_color";
		const StringName set_annotation_color_method = "set_annotation_color";
		const StringName annotation_color_property = "annotation_color";
		ClassDB::bind_method(D_METHOD(get_annotation_color_method), &ChessTheme::get_annotation_color);
		ClassDB::bind_method(D_METHOD(set_annotation_color_method, annotation_color_property), &ChessTheme::set_annotation_color);
		ClassDB::add_property(class_name, PropertyInfo(Variant::COLOR, annotation_color_property, PROPERTY_HINT_NONE), set_annotation_color_method, get_annotation_color_method);
	}
}

const Ref<Mesh> &ChessTheme::get_annotation_mesh(phase4::engine::common::Square from, phase4::engine::common::Square to) {
	using namespace phase4::engine::common;

	Ref<Mesh> &annotation = annotation_meshes[from.get_raw_value() + to.get_raw_value() * 64];
	if (annotation.is_null()) {
		annotation = from == to ? create_circle_polygon(square_size, annotation_color, from) : create_arrow_polygon(square_size, annotation_color, from, to);
	}
	return annotation;
}

void ChessTheme::set_square_size(real_t size) {
	square_size = size;
	annotation_meshes.fill(nullptr);
}

void ChessTheme::set_annotation_color(Color color) {
	this->annotation_color = color;
	annotation_meshes.fill(nullptr);
}
