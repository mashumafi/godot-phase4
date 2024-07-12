#include "chess2d.h"

#include <godot_cpp/classes/input_event_mouse_button.hpp>
#include <godot_cpp/classes/input_event_mouse_motion.hpp>
#include <godot_cpp/classes/ref.hpp>
#include <godot_cpp/classes/time.hpp>
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

	{
		const StringName undo_last_move_method = "undo_last_move";
		ClassDB::bind_method(D_METHOD(undo_last_move_method), &Chess2D::undo_last_move);
	}
}

Chess2D::Chess2D() {
	annotations.reserve(64 * 64);
}

void Chess2D::_ready() {
	ERR_FAIL_COND_MSG(theme.is_null(), "Chess Theme is not provided.");

	const Vector2 square_size = Vector2(1, 1) * theme->get_square_size();
	const Vector2 half_square = square_size / 2;
	const real_t offset = -theme->get_square_size() * 4;
	const Vector2 start_position(offset, offset);

	flourish_canvas_item.instantiate();
	flourish_canvas_item.set_parent(get_canvas_item());

	squares_canvas_item.instantiate();
	squares_canvas_item.set_parent(get_canvas_item());

	valid_square_multimesh = theme->create_square_multimesh(false);
	valid_move_squares_canvas_item.instantiate();
	valid_move_squares_canvas_item.set_parent(get_canvas_item());
	valid_move_squares_canvas_item.set_self_modulate(Color(.75, .55, .05, .15));
	valid_move_squares_canvas_item.set_transform(godot::Transform2D().translated(half_square));

	king_danger_canvas_item.instantiate();
	king_danger_canvas_item.set_parent(get_canvas_item());
	king_danger_canvas_item.set_material(theme->get_king_danger_material());

	file_rank_canvas_item.instantiate();
	file_rank_canvas_item.set_parent(get_canvas_item());

	const float slide_hint_speed = .25;
	right_slide_hint_canvas_item = theme->create_slide_hint_canvas_item(Vector2(slide_hint_speed, 0));
	right_slide_hint_canvas_item.set_parent(get_canvas_item());
	up_slide_hint_canvas_item = theme->create_slide_hint_canvas_item(Vector2(0, -slide_hint_speed));
	up_slide_hint_canvas_item.set_parent(get_canvas_item());
	left_slide_hint_canvas_item = theme->create_slide_hint_canvas_item(Vector2(-slide_hint_speed, 0));
	left_slide_hint_canvas_item.set_parent(get_canvas_item());
	down_slide_hint_canvas_item = theme->create_slide_hint_canvas_item(Vector2(0, slide_hint_speed));
	down_slide_hint_canvas_item.set_parent(get_canvas_item());

	pieces_canvas_item.instantiate();
	pieces_canvas_item.set_parent(get_canvas_item());

	valid_circle_multimeshes = theme->create_circle();
	valid_move_circles_canvas_item.instantiate();
	valid_move_circles_canvas_item.set_parent(get_canvas_item());
	valid_move_circles_canvas_item.set_self_modulate(Color::hex(0x666666BA));
	valid_move_circles_canvas_item.set_transform(godot::Transform2D().translated(half_square));

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

	drag_piece_mesh = theme->create_square_mesh();
	drag_piece_canvas_item.instantiate();
	drag_piece_canvas_item.set_parent(get_canvas_item());
}

void Chess2D::_process(double delta) {
	using namespace phase4::engine::common;

	const Bitboard walls = position.current().walls();
	if (walls != 0) {
		draw_flags |= DrawFlags::FLOURISH;
		queue_redraw();
	}

	// Process piece offsets
	auto piece_offset = piece_animation_offsets.begin();
	for (; piece_offset != piece_animation_offsets.end(); ++piece_offset) {
		if (*piece_offset != Vector2(0, 0)) {
			break;
		}
	}

	if (piece_offset != piece_animation_offsets.end()) {
		draw_flags |= DrawFlags::PIECES;
		queue_redraw();

		for (; piece_offset != piece_animation_offsets.end(); ++piece_offset) {
			if (*piece_offset != Vector2(0, 0)) {
				*piece_offset = piece_offset->move_toward(Vector2(0, 0), delta * Math::clamp(piece_offset->length_squared() / 2, theme->get_square_size(), theme->get_square_size() * 12));
			}
		}
	} else {
		auto square_offset = square_animation_offsets.begin();
		for (; square_offset != square_animation_offsets.end(); ++square_offset) {
			if (*square_offset != Vector2(0, 0)) {
				break;
			}
		}

		if (square_offset != square_animation_offsets.end()) {
			draw_flags |= DrawFlags::BOARD;
			queue_redraw();
		}

		for (; square_offset != square_animation_offsets.end(); ++square_offset) {
			if (*square_offset != Vector2(0, 0)) {
				*square_offset = square_offset->move_toward(Vector2(0, 0), delta * Math::clamp(square_offset->length_squared() / 2, theme->get_square_size(), theme->get_square_size() * 12));
			}
		}
	}

	if (drag_piece) {
		draw_flags |= DrawFlags::DRAG_PIECE;
		queue_redraw();
	}
}

void Chess2D::_draw() {
	using namespace phase4::engine::common;
	using namespace phase4::engine::moves;
	using namespace phase4::engine::board;

	static constexpr std::array<int64_t, 8> FILES{ 'A', 'B', 'C', 'D', 'E', 'F', 'G', 'H' };
	static constexpr std::array<int64_t, 8> RANKS{ '1', '2', '3', '4', '5', '6', '7', '8' };

	ERR_FAIL_COND_MSG(theme.is_null(), "Chess Theme is not provided.");

	const Vector2 square_size = Vector2(1, 1) * theme->get_square_size();
	const Vector2 half_square_size = Vector2(.5, .5) * theme->get_square_size();
	const real_t offset = -theme->get_square_size() * 4;
	const Vector2 start_position(offset, offset);

	if (draw_flags & DrawFlags::FLOURISH) {
		flourish_canvas_item.clear();
		const Bitboard walls = position.current().walls();
		if (walls != 0) {
			const Square square(walls);
			const Vector2 flip_offset = is_flipped ? square_size : Vector2(0, 0);
			const Vector2 translation = get_square_position(square) + flip_offset;
			const double time = Math::sin(Time::get_singleton()->get_ticks_msec() / 250.0);
			const double scaledTime = Math::remap(time, -1, 1, .9, 1.0);
			const Vector2 scale(scaledTime, scaledTime);
			flourish_canvas_item.add_mesh(theme->get_flourish_mesh(), Transform2D().scaled(scale).translated(translation), Color("WHITE"), *theme->get_flourish().ptr());
			flourish_canvas_item.add_mesh(theme->get_flourish_mesh(), Transform2D().scaled(scale).rotated(Math_PI / 2).translated(translation), Color("WHITE"), *theme->get_flourish().ptr());
			flourish_canvas_item.add_mesh(theme->get_flourish_mesh(), Transform2D().scaled(scale).rotated(Math_PI).translated(translation), Color("WHITE"), *theme->get_flourish().ptr());
			flourish_canvas_item.add_mesh(theme->get_flourish_mesh(), Transform2D().scaled(scale).rotated(3 * Math_PI / 2).translated(translation), Color("WHITE"), *theme->get_flourish().ptr());
		}
	}

	if (draw_flags & DrawFlags::SQUARES) {
		squares_canvas_item.clear();
		size_t circle_count = 0;

		for (Square square = Square::BEGIN; square != Square::INVALID; ++square) {
			const Square flippedSquare = is_flipped ? square.flipped() : square;
			if ((flippedSquare.asBitboard() & position.current().walls()) != 0) {
				continue;
			}

			const FieldIndex field = square.asFieldIndex();
			const Color color = (field.x + field.y) % 2 == 0 ? theme->get_black_square_color() : theme->get_white_square_color();

			squares_canvas_item.add_rect(Rect2(get_square_position(flippedSquare), square_size), color);
		}
	}

	if (draw_flags & DrawFlags::VALID_MOVES) {
		valid_move_circles_canvas_item.clear();
		valid_move_squares_canvas_item.clear();

		if (const std::optional<Square> &from = get_selected()) {
			const FastVector<Move, 21> &moves = position.valid_moves(*from);
			for (size_t i = 0; i < moves.size(); ++i) {
				const Move move = moves[i];
				const Transform2D transform = Transform2D().translated(get_square_position(move.to()));
				valid_circle_multimeshes.set_instance_transform_2d(i, transform);
				valid_square_multimesh->set_instance_transform_2d(i, transform);
			}

			valid_circle_multimeshes.set_visible_instance_count(moves.size());
			valid_circle_multimeshes.add_multimesh(*valid_move_circles_canvas_item);

			valid_square_multimesh->set_visible_instance_count(moves.size());
			valid_move_squares_canvas_item.add_multimesh(*valid_square_multimesh.ptr());
		}
	}

	if (draw_flags & DrawFlags::FILE_RANK) {
		file_rank_canvas_item.clear();
		const real_t font_size = theme->get_square_size() / 5;
		const int32_t fixed_field = is_flipped ? 0 : 7;
		const int32_t color_mod = is_flipped ? 0 : 1;

		auto next_square = [&position = position.current(), is_flipped = this->is_flipped](const FieldIndex &field_index, const Square::Direction &normal_direction, const Square::Direction &flipped_direction) -> Square {
			const Square square(field_index);
			if ((square.asBitboard() & position.walls()) == 0) {
				return square;
			}

			return is_flipped ? (square.*flipped_direction)(2) : (square.*normal_direction)(2);
		};

		// The vertical numbers
		for (size_t rank = 0; rank < RANKS.size(); rank++) {
			const Square square(next_square(FieldIndex(7 - fixed_field, rank), &Square::east, &Square::west));
			const Color color = rank % 2 == color_mod ? theme->get_black_square_color() : theme->get_white_square_color();
			theme->get_font()->draw_char(
					*file_rank_canvas_item,
					get_square_position(square) + Vector2(0, theme->get_square_size()),
					RANKS[rank],
					font_size,
					color);
		}

		// The horizontal letters
		const double ascent = theme->get_font()->get_ascent(font_size);
		for (size_t file = 0; file < FILES.size(); file++) {
			const Square square(next_square(FieldIndex(file, fixed_field), &Square::south, &Square::north));
			const Color color = file % 2 != color_mod ? theme->get_black_square_color() : theme->get_white_square_color();
			theme->get_font()->draw_char(
					*file_rank_canvas_item,
					get_square_position(square) + Vector2(0, ascent),
					FILES[file],
					font_size,
					color);
		}
	}

	if (draw_flags & DrawFlags::SQUARES && position.current().walls() != 0) {
		right_slide_hint_canvas_item.clear();
		up_slide_hint_canvas_item.clear();
		left_slide_hint_canvas_item.clear();
		down_slide_hint_canvas_item.clear();
		const std::array<FieldIndex, 64> &slide_dir = WallOperations::SLIDE_DIR[position.current().walls().fastBitScan()];
		for (Square square = Square::BEGIN; square != Square::INVALID; ++square) {
			const Rect2 rect(get_square_position(square), square_size);
			const Color color = Color(1.1, 1.25, 1.25, .25);

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
	}

	if (draw_flags & DrawFlags::PIECES) {
		pieces_canvas_item.clear();
		king_danger_canvas_item.clear();
		const Square dragged_square = get_dragged();
		for (PieceColor piece_color = PieceColor::WHITE; piece_color != PieceColor::INVALID; ++piece_color) {
			if (position.current().colorToMove() == piece_color && position.current().isKingChecked(piece_color)) {
				king_danger_canvas_item.add_rect(Rect2(get_square_position(Square(position.current().colorPieceMask(piece_color, PieceType::KING))), square_size), Color(1.0, 1.0, 1.0, 1.0));
			}
			for (PieceType piece_type = PieceType::PAWN; piece_type != PieceType::INVALID; ++piece_type) {
				const Ref<MultiMesh> &mesh = piece_meshes[piece_color.get_raw_value()][piece_type.get_raw_value()];
				const Ref<Texture> &texture = theme->get_piece_texture(piece_color, piece_type);
				ERR_CONTINUE_MSG(texture.is_null(), "Invalid texture");

				Bitboard squares = position.current().colorPieceMask(piece_color, piece_type);
				int32_t instance = 0;
				while (squares != 0) {
					const Square square(squares);
					squares = squares.popLsb();
					if (square == dragged_square) {
						continue;
					}

					const Color color = position.valid_moves(square).is_empty() ? Color("GRAY") : Color("WHITE");
					mesh->set_instance_color(instance, color);
					const Vector2 piece_offset = piece_animation_offsets[square] + half_square_size;
					mesh->set_instance_transform_2d(instance, Transform2D().translated(get_square_position(square) + piece_offset));
					++instance;
				}

				if (likely(instance > 0)) {
					mesh->set_visible_instance_count(instance);
					pieces_canvas_item.add_multimesh(*mesh.ptr(), *texture.ptr());
				}
			}
		}
	}

	if (draw_flags & DrawFlags::HIGHLIGHT) {
		valid_hover_canvas_item.clear();
		invalid_hover_canvas_item.clear();
		if (!annotation_begin_square) {
			if (highlighted_square) {
				const Square square(FieldIndex(highlighted_square->x, 7 - highlighted_square->y));
				const Square flippedSquare = is_flipped ? square.flipped() : square;
				CanvasItemUtil &hover_canvas_item = position.valid_moves(flippedSquare).is_empty() ? invalid_hover_canvas_item : valid_hover_canvas_item;
				hover_canvas_item.add_mesh(*theme->get_highlight_mesh().ptr(), godot::Transform2D().translated(start_position + highlighted_square.value() * theme->get_square_size()));
			}
		}

		selected_canvas_item.clear();
		if (const std::optional<Square> &selected = get_selected()) {
			selected_canvas_item.add_mesh(*theme->get_highlight_mesh().ptr(), godot::Transform2D().translated(get_square_position(*selected)));
		}
	}

	if (draw_flags & DrawFlags::ANNOTATIONS) {
		annotations_canvas_item.clear();
		for (uint16_t annotation : annotations) {
			const Square from(annotation % 64);
			const Square to(annotation / 64);
			const Square flippedFrom = is_flipped ? from.flipped() : from;
			const Square flippedTo = is_flipped ? to.flipped() : to;
			annotations_canvas_item.add_mesh(*theme->get_annotation_mesh(flippedFrom, flippedTo).ptr(), theme->get_annotation_transform(flippedFrom, flippedTo));
		}

		if (annotation_begin_square && annotation_end_square) {
			const Vector2 mouse_square_transform = ((get_global_mouse_position() - start_position) / theme->get_square_size()).floor();
			const Square from(FieldIndex(annotation_begin_square->x, 7 - annotation_begin_square->y));
			const Square to(FieldIndex(mouse_square_transform.x, 7 - mouse_square_transform.y));
			annotations_canvas_item.add_mesh(*theme->get_annotation_mesh(from, to).ptr(), theme->get_annotation_transform(from, to));
		}
	}

	if (draw_flags & DrawFlags::DRAG_PIECE) {
		drag_piece_canvas_item.clear();

		Square drag_square = get_dragged();
		if (drag_square < Square::INVALID) {
			const Vector2 scale(1.25, 1.25);
			const Transform2D transform = Transform2D().scaled(scale).translated(get_global_mouse_position());
			const std::optional<std::tuple<PieceColor, PieceType>> &pieceTypeAndColor = position.current().getPiece(drag_square);
			assert(pieceTypeAndColor); // This square should have a valid piece and color
			const auto &[piece_color, piece_type] = *pieceTypeAndColor;
			const Ref<Texture> &texture = theme->get_piece_texture(piece_color, piece_type);
			assert(texture.is_valid()); // A texture should be found since everything is valid above
			drag_piece_canvas_item.add_mesh(*drag_piece_mesh.ptr(), transform, Color(1, 1, 1, 1), *texture.ptr());
		}
	}

	draw_flags = DrawFlags::NONE;
}

void Chess2D::_input(const Ref<InputEvent> &event) {
	using namespace phase4::engine::common;
	using namespace phase4::engine::moves;
	using namespace phase4::engine::moves;

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
					draw_flags |= DrawFlags::ANNOTATIONS;
					queue_redraw();
				} else if (annotation_end_square) {
					annotation_end_square.reset();
					draw_flags |= DrawFlags::ANNOTATIONS;
					queue_redraw();
				}
			}
		} else {
			if (Rect2(0, 0, 8, 8).has_point(mouse_square_transform)) {
				if (highlighted_square != mouse_square_transform) {
					highlighted_square = mouse_square_transform;
					draw_flags |= DrawFlags::HIGHLIGHT;
					queue_redraw();
				}
			} else if (highlighted_square) {
				highlighted_square.reset();
				draw_flags |= DrawFlags::HIGHLIGHT;
				queue_redraw();
			}
		}
	}

	const Ref<InputEventMouseButton> mouse_button = event;
	if (mouse_button.is_valid()) {
		const real_t offset = -theme->get_square_size() * 4;
		const Vector2 square_size = Vector2(1, 1) * theme->get_square_size();
		const Vector2 start_position = Vector2(offset, offset) + get_global_position();
		const Vector2 mouse_square_transform = ((get_global_mouse_position() - start_position) / theme->get_square_size()).floor();
		if (mouse_button->get_button_index() == MOUSE_BUTTON_LEFT) {
			if (mouse_button->is_pressed()) {
				if (const std::optional<Square> &to = get_mouse_square()) {
					if (const std::optional<Square> &from = get_selected()) {
						make_move(*from, *to);
					}
					if (position.valid_moves(*to).is_empty()) {
						selected_square.reset();
						draw_flags |= DrawFlags::HIGHLIGHT | DrawFlags::VALID_MOVES;
						queue_redraw();
					} else {
						selected_square = mouse_square_transform;
						drag_piece = mouse_square_transform;

						draw_flags |= DrawFlags::HIGHLIGHT | DrawFlags::VALID_MOVES | DrawFlags::DRAG_PIECE | DrawFlags::PIECES;
						queue_redraw();
					}
				} else {
					selected_square.reset();
					draw_flags |= DrawFlags::HIGHLIGHT | DrawFlags::VALID_MOVES;
					queue_redraw();
				}
			} else { // Mouse Released
				drag_piece.reset();
				draw_flags |= DrawFlags::DRAG_PIECE | DrawFlags::PIECES;
				queue_redraw();

				if (const std::optional<Square> &to = get_mouse_square()) {
					if (const std::optional<Square> &from = get_selected()) {
						if (make_move(*from, *to)) {
							selected_square.reset();
							draw_flags |= DrawFlags::HIGHLIGHT | DrawFlags::VALID_MOVES;
							queue_redraw();
						}
					}
				}
			}
		} else if (mouse_button->get_button_index() == MOUSE_BUTTON_RIGHT) {
			if (Rect2(0, 0, 8, 8).has_point(mouse_square_transform)) {
				if (mouse_button->is_pressed()) {
					annotation_begin_square = mouse_square_transform;
					annotation_end_square = mouse_square_transform;
					draw_flags |= DrawFlags::ANNOTATIONS;
					queue_redraw();
				} else if (annotation_begin_square) {
					if (annotation_end_square) {
						const Square from(FieldIndex(annotation_begin_square->x, 7 - annotation_begin_square->y));
						const Square to(FieldIndex(annotation_end_square->x, 7 - annotation_end_square->y));
						toggle_annotation(is_flipped ? from.flipped() : from, is_flipped ? to.flipped() : to);
						highlighted_square = annotation_end_square;
						annotation_end_square.reset();
					}
					annotation_begin_square.reset();
					draw_flags |= DrawFlags::ANNOTATIONS;
					queue_redraw();
				}
			} else if (annotation_begin_square) {
				annotation_begin_square.reset();
			}
		}
	}
}

String Chess2D::get_fen() const {
	using namespace phase4::engine::fen;

	const std::string &fen = PositionToFen::encode(position.current());
	return String(fen.c_str());
}

void Chess2D::set_fen(const String &fen) {
	using namespace phase4::engine::board;
	using namespace phase4::engine::fen;

	const std::optional<Position>& parsedPosition = FenToPosition::parse(fen.ascii().get_data());
	ERR_FAIL_COND_MSG(!parsedPosition, "Invalid fen: " + fen);
	position.reset(*parsedPosition);
	draw_flags |= DrawFlags::BOARD;
	queue_redraw();
}

Ref<ChessTheme> Chess2D::get_theme() const {
	return theme;
}

bool Chess2D::get_flipped() const {
	return is_flipped;
}

void Chess2D::set_flipped(bool flipped) {
	if (is_flipped != flipped && selected_square) {
		// TODO: Avoid flipping the selection
		selected_square = Vector2i(7 - selected_square->x, 7 - selected_square->y);
	}
	this->is_flipped = flipped;
	draw_flags |= DrawFlags::ALL;
	clear_animation_offsets();
}

void Chess2D::undo_last_move() {
	position.undo();
	draw_flags |= DrawFlags::ALL;
	queue_redraw();
}

void Chess2D::set_theme(const Ref<ChessTheme> &theme) {
	using namespace phase4::engine::common;

	if (this->theme.is_valid()) {
		Callable queue_redraw(this, "queue_redraw");
		this->theme->disconnect("changed", queue_redraw);
	}
	this->theme = theme;
	draw_flags |= DrawFlags::ALL;

	for (PieceColor piece_color = PieceColor::WHITE; piece_color != PieceColor::INVALID; ++piece_color) {
		for (PieceType piece_type = PieceType::PAWN; piece_type != PieceType::INVALID; ++piece_type) {
			piece_meshes[piece_color.get_raw_value()][piece_type.get_raw_value()] = theme->create_square_multimesh();
		}
	}
	queue_redraw();
	if (this->theme.is_valid()) {
		Callable queue_redraw(this, "queue_redraw");
		const Error rc = this->theme->connect("changed", queue_redraw);
		ERR_FAIL_COND_MSG(rc != Error::OK, "Could not connect changed");
	}
}
