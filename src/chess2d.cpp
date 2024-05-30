#include "chess2d.h"

#include <godot_cpp/classes/array_mesh.hpp>
#include <godot_cpp/classes/gradient_texture2d.hpp>
#include <godot_cpp/classes/input_event_mouse_button.hpp>
#include <godot_cpp/classes/input_event_mouse_motion.hpp>
#include <godot_cpp/classes/rendering_server.hpp>
#include <godot_cpp/core/class_db.hpp>
#include <godot_cpp/variant/utility_functions.hpp>

#include <phase4/engine/fen/fen_to_position.h>

#include <array>

using namespace godot;

void Chess2D::_bind_methods() {
	const StringName class_name = "Chess2D";

	{
		const StringName get_fen_method = "get_fen";
		const StringName set_fen_method = "set_fen";
		const StringName fen_property = "fen";
		ClassDB::bind_method(D_METHOD(get_fen_method), &Chess2D::get_fen);
		ClassDB::bind_method(D_METHOD(set_fen_method, fen_property), &Chess2D::set_fen);
		ClassDB::add_property(class_name, PropertyInfo(Variant::STRING, fen_property), set_fen_method, get_fen_method);
	}

	{
		const StringName get_flipped_method = "get_flipped";
		const StringName set_flipped_method = "set_flipped";
		const StringName is_flipped_property = "is_flipped";
		ClassDB::bind_method(D_METHOD(get_flipped_method), &Chess2D::get_flipped);
		ClassDB::bind_method(D_METHOD(set_flipped_method, is_flipped_property), &Chess2D::set_flipped);
		ClassDB::add_property(class_name, PropertyInfo(Variant::BOOL, is_flipped_property), set_flipped_method, get_flipped_method);
	}

	{
		const StringName get_theme_method = "get_theme";
		const StringName set_theme_method = "set_theme";
		const StringName theme_property = "theme";
		ClassDB::bind_method(D_METHOD(get_theme_method), &Chess2D::get_theme);
		ClassDB::bind_method(D_METHOD(set_theme_method, theme_property), &Chess2D::set_theme);
		ClassDB::add_property(class_name, PropertyInfo(Variant::OBJECT, theme_property), set_theme_method, get_theme_method);
	}
}

Chess2D::Chess2D() {
	annotations.reserve(64 * 64);
	using namespace phase4::engine::common;
	//toggle_annotation(Square::A1, Square::F4);
	toggle_annotation(Square::D3, Square::D3);
	toggle_annotation(Square::A8, Square::C6);
	toggle_annotation(Square::A8, Square::B4);
}

void Chess2D::_ready() {
	auto position = phase4::engine::fen::FenToPosition::parse(fen.ascii().get_data());
	ERR_FAIL_COND_MSG(!position, "Invalid fen: " + fen);
	session = std::make_unique<phase4::engine::board::Session>(*position);

	ERR_FAIL_COND_MSG(theme.is_null(), "Chess Theme is not provided.");

	squares_canvas_item.instantiate();
	squares_canvas_item.set_parent(get_canvas_item());

	right_slide_hint_canvas_item = theme->slide_hint_canvas_item_create();
	up_slide_hint_canvas_item = theme->slide_hint_canvas_item_create();
	left_slide_hint_canvas_item = theme->slide_hint_canvas_item_create();
	down_slide_hint_canvas_item = theme->slide_hint_canvas_item_create();

	file_rank_canvas_item.instantiate();
	file_rank_canvas_item.set_parent(get_canvas_item());

	pieces_canvas_item.instantiate();
	pieces_canvas_item.set_parent(get_canvas_item());

	annotations_canvas_item.instantiate();
	annotations_canvas_item.set_parent(get_canvas_item());
	annotations_canvas_item.set_self_modulate(theme->get_annotation_color());
}

void Chess2D::_process(double delta) {
}

void Chess2D::_draw() {
	using namespace phase4::engine::common;

	RenderingServer *RS = RenderingServer::get_singleton();

	static const std::array<int64_t, 8> FILES{ 'A', 'B', 'C', 'D', 'E', 'F', 'G', 'H' };
	static const std::array<int64_t, 8> RANKS{ '1', '2', '3', '4', '5', '6', '7', '8' };

	ERR_FAIL_COND_MSG(theme.is_null(), "Chess Theme is not provided.");

	const Vector2 square_size = Vector2(1, 1) * theme->get_square_size();
	const real_t offset = -theme->get_square_size() * 4;
	const Vector2 start_position(offset, offset);

	squares_canvas_item.clear();
	for (Square square = Square::BEGIN; square != Square::INVALID; ++square) {
		const FieldIndex field = square.asFieldIndex();
		const Color color = (field.x + field.y) % 2 == 0 ? theme->get_white_square_color() : theme->get_black_square_color();
		squares_canvas_item.add_rect(Rect2(start_position + theme->get_square_size() * Vector2(field.x, field.y), square_size), color);
	}

	pieces_canvas_item.clear();
	Bitboard occupied = session->position().m_occupancySummary;
	while (occupied != 0) {
		const Square square(occupied);
		occupied = occupied.popLsb();

		const auto piece = session->position().getPiece(square);
		ERR_CONTINUE_MSG(!piece, "Piece expected");

		const auto [piece_color, piece_type] = *piece;
		const Ref<Texture> &texture = theme->get_piece_texture(piece_color, piece_type);
		ERR_CONTINUE_MSG(texture.is_null(), "Invalid texture");

		const FieldIndex field = square.asFieldIndex();
		const Vector2 flippedField = is_flipped ? Vector2(7 - field.x, field.y) : Vector2(field.x, 7 - field.y);
		pieces_canvas_item.add_texture_rect(Rect2(start_position + theme->get_square_size() * flippedField, square_size), *texture.ptr());
	}

	const real_t font_size = theme->get_square_size() / 5;
	for (size_t rank = 0; rank < RANKS.size(); rank++) {
		const size_t rank_index = is_flipped ? rank : RANKS.size() - rank - 1;
		const Color color = rank % 2 == 0 ? theme->get_black_square_color() : theme->get_white_square_color();
		theme->get_font()->draw_char(
				*file_rank_canvas_item,
				start_position + Vector2(0, theme->get_square_size() * (rank + 1)),
				RANKS[rank_index],
				font_size,
				color);
	}

	const real_t ascent = theme->get_font()->get_ascent(font_size);
	for (size_t file = 0; file < FILES.size(); file++) {
		const size_t file_index = is_flipped ? FILES.size() - file - 1 : file;
		const Color color = file % 2 == 0 ? theme->get_black_square_color() : theme->get_white_square_color();
		theme->get_font()->draw_char(
				*file_rank_canvas_item,
				start_position + Vector2(theme->get_square_size() * file, ascent),
				FILES[file_index],
				font_size,
				color);
	}

	const Vector2 mouse_square_transform = (get_global_mouse_position() - start_position) / theme->get_square_size();
	const real_t width = theme->get_square_size() / 16;
	if (highlighted_square) {
		draw_rect(Rect2(start_position + highlighted_square.value() * theme->get_square_size(), square_size).grow(-width / 2), Color("RED"), false, width);
	}

	annotations_canvas_item.clear();
	for (uint16_t annotation : annotations) {
		Square from(annotation % 64);
		Square to(annotation / 64);
		annotations_canvas_item.add_mesh(*theme->get_annotation_mesh(from, to).ptr());
	}
}

void Chess2D::_input(const Ref<InputEvent> &event) {
	ERR_FAIL_COND_MSG(theme.is_null(), "Chess Theme is not provided.");

	const Ref<InputEventMouseMotion> &mouse_motion = event;
	if (mouse_motion.is_valid()) {
		const real_t offset = -theme->get_square_size() * 4;
		const Vector2 square_size = Vector2(1, 1) * theme->get_square_size();
		const Vector2 start_position = Vector2(offset, offset) + get_global_position();

		const Vector2 mouse_square_transform = ((get_global_mouse_position() - start_position) / theme->get_square_size()).floor();
		if (mouse_square_transform.x >= 0 && mouse_square_transform.y >= 0 && mouse_square_transform.x < 8 && mouse_square_transform.y < 8) {
			if (highlighted_square != mouse_square_transform) {
				highlighted_square = mouse_square_transform;
				queue_redraw();
			}
		} else if (highlighted_square) {
			highlighted_square.reset();
			queue_redraw();
		}
	}

	const Ref<InputEventMouseButton> mouse_button = event;
	if (mouse_button.is_valid()) {
		if (mouse_button->get_button_index() == MOUSE_BUTTON_RIGHT) {
			const real_t offset = -theme->get_square_size() * 4;
			const Vector2 square_size = Vector2(1, 1) * theme->get_square_size();
			const Vector2 start_position = Vector2(offset, offset) + get_global_position();
			const Vector2 mouse_square_transform = ((get_global_mouse_position() - start_position) / theme->get_square_size()).floor();
			if (mouse_square_transform.x >= 0 && mouse_square_transform.y >= 0 && mouse_square_transform.x < 8 && mouse_square_transform.y < 8) {
				if (mouse_button->is_pressed()) {
					annotation_begin_square = mouse_square_transform;
				} else {
					annotation_begin_square.reset();
				}
			}
		}
	}
}

String Chess2D::get_fen() const {
	return this->fen;
}

void Chess2D::set_fen(const String &fen) {
	this->fen = fen;
}

Ref<ChessTheme> Chess2D::get_theme() const {
	return theme;
}

bool Chess2D::get_flipped() const {
	return is_flipped;
}

void Chess2D::set_flipped(bool flipped) {
	this->is_flipped = flipped;
	queue_redraw();
}

void Chess2D::set_theme(const Ref<ChessTheme> &theme) {
	if (this->theme.is_valid()) {
		Callable queue_redraw(this, "queue_redraw");
		this->theme->disconnect("changed", queue_redraw);
	}
	this->theme = theme;
	queue_redraw();
	if (this->theme.is_valid()) {
		Callable queue_redraw(this, "queue_redraw");
		const Error rc = this->theme->connect("changed", queue_redraw);
		ERR_FAIL_COND_MSG(rc != Error::OK, "Could not connect changed");
	}
}
