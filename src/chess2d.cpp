#include "chess2d.h"

#include <godot_cpp/classes/input_event_mouse_button.hpp>
#include <godot_cpp/classes/input_event_mouse_motion.hpp>
#include <godot_cpp/classes/ref.hpp>
#include <godot_cpp/core/class_db.hpp>
#include <godot_cpp/core/property_info.hpp>
#include <godot_cpp/variant/color.hpp>
#include <godot_cpp/variant/rect2.hpp>
#include <godot_cpp/variant/string_name.hpp>
#include <godot_cpp/variant/transform2d.hpp>
#include <godot_cpp/variant/vector2.hpp>

#include <phase4/engine/fen/fen_to_position.h>
#include <phase4/engine/fen/position_to_fen.h>

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
	session = std::make_unique<phase4::engine::board::Session>();
	annotations.reserve(64 * 64);
}

void Chess2D::_ready() {
	compute_valid_moves();

	ERR_FAIL_COND_MSG(theme.is_null(), "Chess Theme is not provided.");

	const Vector2 square_size = Vector2(1, 1) * theme->get_square_size();
	const Vector2 half_square = square_size / 2;
	const real_t offset = -theme->get_square_size() * 4;
	const Vector2 start_position(offset, offset);

	squares_canvas_item.instantiate();
	squares_canvas_item.set_parent(get_canvas_item());

	file_rank_canvas_item.instantiate();
	file_rank_canvas_item.set_parent(get_canvas_item());

	const float slide_hint_speed = .25;
	right_slide_hint_canvas_item = theme->slide_hint_canvas_item_create(Vector2(slide_hint_speed, 0));
	right_slide_hint_canvas_item.set_parent(get_canvas_item());
	up_slide_hint_canvas_item = theme->slide_hint_canvas_item_create(Vector2(0, -slide_hint_speed));
	up_slide_hint_canvas_item.set_parent(get_canvas_item());
	left_slide_hint_canvas_item = theme->slide_hint_canvas_item_create(Vector2(-slide_hint_speed, 0));
	left_slide_hint_canvas_item.set_parent(get_canvas_item());
	down_slide_hint_canvas_item = theme->slide_hint_canvas_item_create(Vector2(0, slide_hint_speed));
	down_slide_hint_canvas_item.set_parent(get_canvas_item());

	pieces_canvas_item.instantiate();
	pieces_canvas_item.set_parent(get_canvas_item());

	valid_hover_canvas_item.instantiate();
	valid_hover_canvas_item.set_parent(get_canvas_item());
	valid_hover_canvas_item.set_self_modulate(Color("GREEN"));

	invalid_hover_canvas_item.instantiate();
	invalid_hover_canvas_item.set_parent(get_canvas_item());
	invalid_hover_canvas_item.set_self_modulate(Color("RED"));

	selected_canvas_item.instantiate();
	selected_canvas_item.set_parent(get_canvas_item());
	invalid_hover_canvas_item.set_self_modulate(Color("YELLOW"));

	annotations_canvas_item.instantiate();
	annotations_canvas_item.set_parent(get_canvas_item());
	annotations_canvas_item.set_self_modulate(theme->get_annotation_color());
	annotations_canvas_item.set_transform(godot::Transform2D().translated(start_position + half_square));
}

void Chess2D::_process(double delta) {
}

void Chess2D::_draw() {
	using namespace phase4::engine::common;

	static const std::array<int64_t, 8> FILES{ 'A', 'B', 'C', 'D', 'E', 'F', 'G', 'H' };
	static const std::array<int64_t, 8> RANKS{ '1', '2', '3', '4', '5', '6', '7', '8' };

	ERR_FAIL_COND_MSG(theme.is_null(), "Chess Theme is not provided.");

	const Vector2 square_size = Vector2(1, 1) * theme->get_square_size();
	const real_t offset = -theme->get_square_size() * 4;
	const Vector2 start_position(offset, offset);

	squares_canvas_item.clear();
	for (Square square = Square::BEGIN; square != Square::INVALID; ++square) {
		const Square flippedSquare = is_flipped ? square.flip() : square;
		if ((flippedSquare.asBitboard() & session->position().m_walls) != 0)
			continue;

		const FieldIndex field = flippedSquare.asFieldIndex();
		const Color color = (field.x + field.y) % 2 == 0 ? theme->get_white_square_color() : theme->get_black_square_color();
		squares_canvas_item.add_rect(Rect2(get_square_position(flippedSquare), square_size), color);
	}

	file_rank_canvas_item.clear();
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

	right_slide_hint_canvas_item.clear();
	up_slide_hint_canvas_item.clear();
	left_slide_hint_canvas_item.clear();
	down_slide_hint_canvas_item.clear();
	const std::array<FieldIndex, 64> &slide_dir = WallOperations::SLIDE_DIR[session->position().m_walls.fastBitScan()];
	for (Square square = Square::BEGIN; square != Square::INVALID; ++square) {
		const Rect2 rect(get_square_position(square), square_size);
		const Color color = Color::hex(0xa75cff7b);

		const FieldIndex direction = is_flipped ? -slide_dir[square] : slide_dir[square];
		if (direction == FieldIndex::ZERO)
			continue;

		switch (direction.x + direction.y * 2) {
			case -4:
				up_slide_hint_canvas_item.add_rect(rect, color);
				break;
			case -2:
				left_slide_hint_canvas_item.add_rect(rect, color);
				break;
			case 2:
				right_slide_hint_canvas_item.add_rect(rect, color);
				break;
			case 4:
				down_slide_hint_canvas_item.add_rect(rect, color);
				break;
			default:
				godot::UtilityFunctions::print(Vector2i(direction.x, direction.y));
				break;
		}
	}

	pieces_canvas_item.clear();
	for (PieceType piece_type = PieceType::PAWN; piece_type != PieceType::INVALID; ++piece_type) {
		for (PieceColor piece_color = PieceColor::WHITE; piece_color != PieceColor::INVALID; ++piece_color) {
			const Ref<Texture> &texture = theme->get_piece_texture(piece_color, piece_type);
			ERR_CONTINUE_MSG(texture.is_null(), "Invalid texture");

			Bitboard squares = session->position().colorPieceMask(piece_color, piece_type);
			while (squares != 0) {
				const Square square(squares);
				squares = squares.popLsb();

				pieces_canvas_item.add_texture_rect(Rect2(get_square_position(square), square_size), *texture.ptr());
			}
		}
	}

	valid_hover_canvas_item.clear();
	invalid_hover_canvas_item.clear();
	if (!annotation_begin_square) {
		if (highlighted_square) {
			const Square square(FieldIndex(highlighted_square->x, 7 - highlighted_square->y));
			const Square flippedSquare = is_flipped ? square.flip() : square;
			if (valid_moves_map[flippedSquare].is_empty()) {
				invalid_hover_canvas_item.add_mesh(*theme->get_highlight_mesh().ptr(), godot::Transform2D().translated(start_position + highlighted_square.value() * theme->get_square_size()));
			} else {
				valid_hover_canvas_item.add_mesh(*theme->get_highlight_mesh().ptr(), godot::Transform2D().translated(start_position + highlighted_square.value() * theme->get_square_size()));
			}
		}
	}

	annotations_canvas_item.clear();
	for (uint16_t annotation : annotations) {
		const Square from(annotation % 64);
		const Square to(annotation / 64);
		const Square flippedFrom = is_flipped ? from.flip() : from;
		const Square flippedTo = is_flipped ? to.flip() : to;
		annotations_canvas_item.add_mesh(*theme->get_annotation_mesh(flippedFrom, flippedTo).ptr(), theme->get_annotation_transform(flippedFrom, flippedTo));
	}

	if (annotation_begin_square && annotation_end_square) {
		const Vector2 mouse_square_transform = ((get_global_mouse_position() - start_position) / theme->get_square_size()).floor();
		const Square from(FieldIndex(annotation_begin_square->x, 7 - annotation_begin_square->y));
		const Square to(FieldIndex(mouse_square_transform.x, 7 - mouse_square_transform.y));
		annotations_canvas_item.add_mesh(*theme->get_annotation_mesh(from, to).ptr(), theme->get_annotation_transform(from, to));
	}
}

void Chess2D::_input(const Ref<InputEvent> &event) {
	using namespace phase4::engine::common;

	ERR_FAIL_COND_MSG(theme.is_null(), "Chess Theme is not provided.");

	const Ref<InputEventMouseMotion> &mouse_motion = event;
	if (mouse_motion.is_valid()) {
		const real_t offset = -theme->get_square_size() * 4;
		const Vector2 square_size = Vector2(1, 1) * theme->get_square_size();
		const Vector2 start_position = Vector2(offset, offset) + get_global_position();
		const Vector2 mouse_square_transform = ((get_global_mouse_position() - start_position) / theme->get_square_size()).floor();

		if (annotation_begin_square) {
			if (annotation_end_square != mouse_square_transform) {
				if (Rect2(0, 0, 8, 8).has_point(mouse_square_transform)) {
					annotation_end_square = mouse_square_transform;
					queue_redraw();
				} else if (annotation_end_square) {
					annotation_end_square.reset();
					queue_redraw();
				}
			}
		} else {
			if (Rect2(0, 0, 8, 8).has_point(mouse_square_transform)) {
				if (highlighted_square != mouse_square_transform) {
					highlighted_square = mouse_square_transform;
					queue_redraw();
				}
			} else if (highlighted_square) {
				highlighted_square.reset();
				queue_redraw();
			}
		}
	}

	const Ref<InputEventMouseButton> mouse_button = event;
	if (mouse_button.is_valid()) {
		if (mouse_button->get_button_index() == MOUSE_BUTTON_RIGHT) {
			const real_t offset = -theme->get_square_size() * 4;
			const Vector2 square_size = Vector2(1, 1) * theme->get_square_size();
			const Vector2 start_position = Vector2(offset, offset) + get_global_position();
			const Vector2 mouse_square_transform = ((get_global_mouse_position() - start_position) / theme->get_square_size()).floor();
			if (Rect2(0, 0, 8, 8).has_point(mouse_square_transform)) {
				if (mouse_button->is_pressed()) {
					annotation_begin_square = mouse_square_transform;
					annotation_end_square = mouse_square_transform;
					queue_redraw();
				} else if (annotation_begin_square) {
					if (annotation_end_square) {
						const Square from(FieldIndex(annotation_begin_square->x, 7 - annotation_begin_square->y));
						const Square to(FieldIndex(annotation_end_square->x, 7 - annotation_end_square->y));
						toggle_annotation(is_flipped ? from.flip() : from, is_flipped ? to.flip() : to);
						highlighted_square = annotation_end_square;
						annotation_end_square.reset();
					}
					annotation_begin_square.reset();
					queue_redraw();
				}
			} else if (annotation_begin_square) {
				annotation_begin_square.reset();
			}
		}
	}
}

String Chess2D::get_fen() const {
	const std::string &fen = phase4::engine::fen::PositionToFen::encode(session->position());
	return String(fen.c_str());
}

void Chess2D::set_fen(const String &fen) {
	auto position = phase4::engine::fen::FenToPosition::parse(fen.ascii().get_data());
	ERR_FAIL_COND_MSG(!position, "Invalid fen: " + fen);
	session->setPosition(*position);
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
