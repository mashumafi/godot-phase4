#include "chess_theme.h"

#include <godot_cpp/classes/array_mesh.hpp>
#include <godot_cpp/core/class_db.hpp>
#include <godot_cpp/core/math.hpp>

#include <phase4/engine/common/field_index.h>

using namespace godot;

namespace {
const Color WHITE = Color::hex(0xFFFFFFFF);
const Color WHITE_TRANSPARENT = Color::hex(0xFFFFFF00);

static Ref<Mesh> create_strip_mesh(const PackedVector2Array &vertices, const Color &evenColor = WHITE, const Color &oddColor = WHITE) {
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
	Color *colors_prt = colors.ptrw();
	for (size_t i = 0; i < colors.size(); ++i) {
		colors_prt[i] = i % 2 == 0 ? evenColor : oddColor;
	}
	mesh_arrays[Mesh::ARRAY_COLOR] = colors;

	PackedVector3Array normals;
	normals.resize(vertices.size());
	mesh_arrays[Mesh::ARRAY_NORMAL] = normals;

	PackedInt32Array indices;
	indices.resize(vertices.size());
	int32_t *indices_ptr = indices.ptrw();
	for (size_t i = 0; i < vertices.size(); ++i) {
		indices_ptr[i] = i;
	}
	mesh_arrays[Mesh::ARRAY_INDEX] = indices;

	mesh->add_surface_from_arrays(Mesh::PrimitiveType::PRIMITIVE_TRIANGLE_STRIP, mesh_arrays);

	return mesh;
}

static Ref<Mesh> create_arrow_polygon(real_t square_size, godot::Vector2i target) {
	using namespace phase4::engine::common;

	const Vector2 begin(0, 0);
	const Vector2 end = target * square_size;

	const real_t angle = end.angle();

	const Vector2 adjusted_begin = begin.move_toward(end, square_size / 4);
	const Vector2 adjusted_end = end.move_toward(begin, square_size / 2);

	const Vector2 top = Vector2(0, -square_size / 8).rotated(angle);
	const Vector2 bottom = Vector2(0, square_size / 8).rotated(angle);

	PackedVector2Array polygon;
	polygon.resize(7);
	Vector2 *polygon_ptr = polygon.ptrw();
	polygon_ptr[0] = adjusted_begin + top;
	polygon_ptr[1] = adjusted_begin + bottom;
	polygon_ptr[2] = adjusted_end + top;
	polygon_ptr[3] = adjusted_end + bottom;
	polygon_ptr[4] = adjusted_end + Vector2(0, -square_size / 4).rotated(angle);
	polygon_ptr[5] = adjusted_end + Vector2(square_size / 4, 0).rotated(angle);
	polygon_ptr[6] = adjusted_end + Vector2(0, square_size / 4).rotated(angle);

	return create_strip_mesh(polygon);
}

static Ref<Mesh> create_ring_polygon(const real_t innerRadius, const real_t &outterRadius, const Color &oddColor = WHITE, const Color &evenColor = WHITE) {
	using namespace phase4::engine::common;

	const size_t size = 65;
	const real_t circle_point_step = Math_TAU / (size - 1);

	PackedVector2Array circle_polygon;
	circle_polygon.resize(size * 2);
	Vector2 *circle_polygon_ptr = circle_polygon.ptrw();
	for (size_t r = 0; r < size; ++r) {
		const real_t angle = r * circle_point_step;
		circle_polygon_ptr[r * 2 + 0] = Vector2(outterRadius, 0).rotated(angle);
		circle_polygon_ptr[r * 2 + 1] = Vector2(innerRadius, 0).rotated(angle);
	}

	return create_strip_mesh(circle_polygon, oddColor, evenColor);
}

static Ref<Mesh> create_hollow_square_polygon(real_t square_size, real_t width) {
	using namespace phase4::engine::common;

	const Rect2 outterRect(0, 0, square_size, square_size);
	const Rect2 innerRect = outterRect.grow(square_size * width);

	PackedVector2Array square_polygon;
	square_polygon.resize(10);
	Vector2 *square_polygon_ptr = square_polygon.ptrw();

	square_polygon_ptr[0] = outterRect.get_position();
	square_polygon_ptr[1] = innerRect.get_position();
	square_polygon_ptr[2] = Vector2(outterRect.get_position().x + outterRect.get_size().x, outterRect.get_position().y);
	square_polygon_ptr[3] = Vector2(innerRect.get_position().x + innerRect.get_size().x, innerRect.get_position().y);

	square_polygon_ptr[4] = Vector2(outterRect.get_position().x + outterRect.get_size().x, outterRect.get_position().y + outterRect.get_size().y);
	square_polygon_ptr[5] = Vector2(innerRect.get_position().x + innerRect.get_size().x, innerRect.get_position().y + innerRect.get_size().y);

	square_polygon_ptr[6] = Vector2(outterRect.get_position().x, outterRect.get_position().y + outterRect.get_size().y);
	square_polygon_ptr[7] = Vector2(innerRect.get_position().x, innerRect.get_position().y + innerRect.get_size().y);

	square_polygon_ptr[8] = outterRect.get_position();
	square_polygon_ptr[9] = innerRect.get_position();

	return create_strip_mesh(square_polygon);
}

static Ref<Mesh> create_centered_square_polygon(real_t square_size) {
	using namespace phase4::engine::common;

	Ref<ArrayMesh> mesh;
	mesh.instantiate();

	Array mesh_arrays;
	mesh_arrays.resize(Mesh::ARRAY_MAX);

	PackedVector2Array vertices;
	vertices.resize(4);
	Vector2 *vertices_ptr = vertices.ptrw();
	vertices_ptr[0] = Vector2(-square_size, -square_size) / 2.0;
	vertices_ptr[1] = Vector2(square_size, -square_size) / 2.0;
	vertices_ptr[2] = Vector2(square_size, square_size) / 2.0;
	vertices_ptr[3] = Vector2(-square_size, square_size) / 2.0;
	mesh_arrays[Mesh::ARRAY_VERTEX] = vertices;

	PackedVector2Array uvs;
	uvs.resize(4);
	Vector2 *uvs_ptr = uvs.ptrw();
	uvs_ptr[0] = Vector2(0, 0);
	uvs_ptr[1] = Vector2(1, 0);
	uvs_ptr[2] = Vector2(1, 1);
	uvs_ptr[3] = Vector2(0, 1);
	mesh_arrays[Mesh::ARRAY_TEX_UV] = uvs;

	PackedColorArray colors;
	colors.resize(vertices.size());
	colors.fill(WHITE);
	mesh_arrays[Mesh::ARRAY_COLOR] = colors;

	PackedVector3Array normals;
	normals.resize(vertices.size());
	mesh_arrays[Mesh::ARRAY_NORMAL] = normals;

	PackedInt32Array indices;
	indices.resize(6);
	int32_t *indices_ptr = indices.ptrw();
	indices_ptr[0] = 0;
	indices_ptr[1] = 1;
	indices_ptr[2] = 2;
	indices_ptr[3] = 2;
	indices_ptr[4] = 3;
	indices_ptr[5] = 0;
	mesh_arrays[Mesh::ARRAY_INDEX] = indices;

	mesh->add_surface_from_arrays(Mesh::PrimitiveType::PRIMITIVE_TRIANGLES, mesh_arrays);

	return mesh;
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

	{ // Piece Texture
		const StringName get_piece_texture_method = "get_piece_texture";
		ClassDB::bind_method(D_METHOD(get_piece_texture_method, "color", "type"), &ChessTheme::get_piece_texture);
	}

	BIND_ENUM_CONSTANT(PIECE_COLOR_WHITE);
	BIND_ENUM_CONSTANT(PIECE_COLOR_BLACK);
	BIND_ENUM_CONSTANT(PIECE_COLOR_MAX);
	BIND_ENUM_CONSTANT(PIECE_TYPE_PAWN);
	BIND_ENUM_CONSTANT(PIECE_TYPE_KNIGHT);
	BIND_ENUM_CONSTANT(PIECE_TYPE_BISHOP);
	BIND_ENUM_CONSTANT(PIECE_TYPE_ROOK);
	BIND_ENUM_CONSTANT(PIECE_TYPE_QUEEN);
	BIND_ENUM_CONSTANT(PIECE_TYPE_KING);
	BIND_ENUM_CONSTANT(PIECE_TYPE_MAX);

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

	{ // Slide Hint
		const StringName get_slide_hint_material_method = "get_slide_hint_material";
		const StringName set_slide_hint_material_method = "set_slide_hint_material";
		const StringName slide_hint_material_property = "slide_hint_material";
		ClassDB::bind_method(D_METHOD(get_slide_hint_material_method), &ChessTheme::get_slide_hint_material);
		ClassDB::bind_method(D_METHOD(set_slide_hint_material_method, slide_hint_material_property), &ChessTheme::set_slide_hint_material);
		ClassDB::add_property(class_name, PropertyInfo(Variant::OBJECT, slide_hint_material_property, PROPERTY_HINT_NONE), set_slide_hint_material_method, get_slide_hint_material_method);
	}

	{ // King Danger
		const StringName get_king_danger_material_method = "get_king_danger_material";
		const StringName set_king_danger_material_method = "set_king_danger_material";
		const StringName king_danger_material_property = "king_danger_material";
		ClassDB::bind_method(D_METHOD(get_king_danger_material_method), &ChessTheme::get_king_danger_material);
		ClassDB::bind_method(D_METHOD(set_king_danger_material_method, king_danger_material_property), &ChessTheme::set_king_danger_material);
		ClassDB::add_property(class_name, PropertyInfo(Variant::OBJECT, king_danger_material_property, PROPERTY_HINT_NONE), set_king_danger_material_method, get_king_danger_material_method);
	}

	{ // Florish
		const StringName get_flourish_method = "get_flourish";
		const StringName set_flourish_method = "set_flourish";
		const StringName flourish_property = "flourish";
		ClassDB::bind_method(D_METHOD(get_flourish_method), &ChessTheme::get_flourish);
		ClassDB::bind_method(D_METHOD(set_flourish_method, flourish_property), &ChessTheme::set_flourish);
		ClassDB::add_property(class_name, PropertyInfo(Variant::OBJECT, flourish_property, PROPERTY_HINT_NONE), set_flourish_method, get_flourish_method);
	}

	{ // Trail Texture2D
		const StringName get_trail_texture_method = "get_trail_texture";
		const StringName set_trail_texture_method = "set_trail_texture";
		const StringName trail_texture_property = "trail_texture";
		ClassDB::bind_method(D_METHOD(get_trail_texture_method), &ChessTheme::get_trail_texture);
		ClassDB::bind_method(D_METHOD(set_trail_texture_method, trail_texture_property), &ChessTheme::set_trail_texture);
		ClassDB::add_property(class_name, PropertyInfo(Variant::OBJECT, trail_texture_property, PROPERTY_HINT_NONE), set_trail_texture_method, get_trail_texture_method);
	}

	{ // Trail Material
		const StringName get_trail_material_method = "get_trail_material";
		const StringName set_trail_material_method = "set_trail_material";
		const StringName trail_material_property = "trail_material";
		ClassDB::bind_method(D_METHOD(get_trail_material_method), &ChessTheme::get_trail_material);
		ClassDB::bind_method(D_METHOD(set_trail_material_method, trail_material_property), &ChessTheme::set_trail_material);
		ClassDB::add_property(class_name, PropertyInfo(Variant::OBJECT, trail_material_property, PROPERTY_HINT_NONE), set_trail_material_method, get_trail_material_method);
	}

	{ // Create Trail Transform
		const StringName transform_trail_method = "transform_trail";
		ClassDB::bind_method(D_METHOD(transform_trail_method, "begin", "end", "width"), &ChessTheme::transform_trail);
	}

	{ // Create Square Mesh
		const StringName create_square_mesh_method = "create_square_mesh";
		ClassDB::bind_method(D_METHOD(create_square_mesh_method), &ChessTheme::create_square_mesh);
	}

	{ // Create Circle Polygon
		const StringName create_circle_polygon_method = "create_circle_polygon";
		ClassDB::bind_method(D_METHOD(create_circle_polygon_method, "radius"), &ChessTheme::create_circle_polygon);
	}
}

const Ref<Mesh> &ChessTheme::get_annotation_mesh(phase4::engine::common::Square from, phase4::engine::common::Square to) {
	using namespace phase4::engine::common;

	static const godot::Ref<godot::Mesh> empty_mesh;
	ERR_FAIL_COND_V(square_size <= 0, empty_mesh);

	const FieldIndex begin = from.asFieldIndex();
	const FieldIndex end = to.asFieldIndex();
	const Vector2i line = Vector2i(end.x - begin.x, (7 - end.y) - (7 - begin.y)).abs();

	const uint64_t square = Square(FieldIndex(line.x, line.y));
	Ref<Mesh> &annotation = annotation_meshes[square];
	if (annotation.is_null()) {
		annotation = create_arrow_polygon(square_size, line);
	}
	return annotation;
}

void ChessTheme::set_square_size(real_t size) {
	using namespace phase4::engine::common;

	square_size = size;
	annotation_meshes.fill(nullptr);

	ERR_FAIL_COND(square_size <= 0);

	const double innerRadius = square_size / 2.0f * .8f;
	const double outterRadius = square_size / 2.0f * .9f;
	annotation_meshes[Square::A1.get_raw_value()] = create_ring_polygon(innerRadius, outterRadius);
	highlight_mesh = create_hollow_square_polygon(square_size, -.05);
}

void ChessTheme::set_annotation_color(Color color) {
	this->annotation_color = color;
	emit_changed();
}

void ChessTheme::set_flourish(const Ref<Texture2D> &texture) {
	flourish = texture;
	flourish_mesh = create_centered_square_polygon(square_size * 2);
	emit_changed();
}

void ChessTheme::set_trail_texture(const Ref<Texture2D> &texture) {
	trail_texture = texture;
	emit_changed();
}

void ChessTheme::set_trail_material(const Ref<Material> &material) {
	trail_material = material;
	emit_changed();
}

Transform2D ChessTheme::transform_trail(const Vector2 &begin, const Vector2 &end, real_t width) {
	const float rotated = (end - begin).angle();
	const float length = begin.distance_to(end);
	return Transform2D().scaled(Vector2(length, width * square_size)).rotated(rotated).translated(begin);
}

Ref<Mesh> ChessTheme::create_square_mesh() const {
	return create_centered_square_polygon(square_size);
}

Ref<MultiMesh> ChessTheme::create_square_multimesh(bool use_colors) const {
	Ref<MultiMesh> mesh;
	mesh.instantiate();
	mesh->set_use_colors(use_colors);
	mesh->set_transform_format(MultiMesh::TRANSFORM_2D);
	mesh->set_instance_count(64);
	mesh->set_visible_instance_count(0);
	mesh->set_mesh(create_square_mesh());
	return mesh;
}

Ref<Mesh> ChessTheme::create_circle_polygon(const real_t radius) const {
	static const int circle_segments = 64;

	PackedVector2Array points;
	points.resize(circle_segments + 2);
	Vector2 *points_ptr = points.ptrw();
	// Store circle center in the last point.
	points_ptr[circle_segments + 1] = Vector2(0, 0);

	PackedVector2Array uvs;
	uvs.resize(points.size());
	Vector2 *uvs_ptr = uvs.ptrw();
	uvs_ptr[circle_segments + 1] = Vector2(.5, .5);

	const real_t circle_point_step = Math_TAU / circle_segments;

	for (int i = 0; i < circle_segments + 1; i++) {
		const real_t angle = i * circle_point_step;
		const Vector2 rotated = Vector2(1, 0).rotated(angle);
		points_ptr[i] = rotated * radius;

		uvs_ptr[i] = rotated / 2.0 + Vector2(.5, .5);
	}

	PackedInt32Array indices;
	indices.resize(circle_segments * 3);
	int *indices_ptr = indices.ptrw();

	for (int i = 0; i < circle_segments; i++) {
		indices_ptr[i * 3 + 0] = circle_segments + 1;
		indices_ptr[i * 3 + 1] = i;
		indices_ptr[i * 3 + 2] = i + 1;
	}

	Array mesh_arrays;
	mesh_arrays.resize(Mesh::ARRAY_MAX);
	mesh_arrays[Mesh::ARRAY_INDEX] = indices;
	mesh_arrays[Mesh::ARRAY_VERTEX] = points;
	mesh_arrays[Mesh::ARRAY_TEX_UV] = uvs;

	Ref<ArrayMesh> mesh;
	mesh.instantiate();
	mesh->add_surface_from_arrays(Mesh::PrimitiveType::PRIMITIVE_TRIANGLES, mesh_arrays);
	return mesh;
}

BatchMultiMesh<2> ChessTheme::create_circle() const {
	BatchMultiMesh<2> batch;
	const real_t radius = square_size / 7.0f;
	batch.set_transform_format(MultiMesh::TRANSFORM_2D);
	batch.set_instance_count(21);
	batch.set_visible_instance_count(0);
	batch.set_meshes({ create_circle_polygon(radius), create_ring_polygon(radius, radius + 1.25, WHITE_TRANSPARENT, WHITE) });

	return batch;
}

Ref<MultiMesh> ChessTheme::create_trail() const {
	Ref<MultiMesh> mesh;
	mesh.instantiate();
	mesh->set_transform_format(MultiMesh::TRANSFORM_2D);
	mesh->set_instance_count(64);
	mesh->set_visible_instance_count(0);
	mesh->set_mesh(create_circle_polygon(1));

	return mesh;
}
