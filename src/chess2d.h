#ifndef CHESS2D_H
#define CHESS2D_H

#include "canvas_item_util.h"
#include "chess_theme.h"
#include "position_view.h" // TODO: Move into phase4::engine::board

#include <godot_cpp/classes/input_event.hpp>
#include <godot_cpp/classes/node2d.hpp>
#include <godot_cpp/classes/rendering_server.hpp>

#include <phase4/engine/moves/result.h>

#include <memory>
#include <optional>
#include <unordered_set>

namespace godot {

class Chess2D : public Node2D {
	GDCLASS(Chess2D, Node2D)

	enum DrawFlags : int64_t {
		NONE = 0,
		FLOURISH = 0b1,
		SQUARES = 0b10,
		PIECES = 0b100,
		ANNOTATIONS = 0b1000,
		HIGHLIGHT = 0b10000,
		VALID_MOVES = 0b100000,
		DRAG_PIECE = 0b1000000,
		FILE_RANK = 0b10000000,

		BOARD = SQUARES | PIECES | FILE_RANK,
		ALL = 0b11111111,
	};
	int64_t draw_flags = DrawFlags::ALL;

private:
	inline static const char *SIGNAL_PIECE_MOVED = "piece_moved";

	bool make_move(phase4::engine::common::Square from, phase4::engine::common::Square to) {
		using namespace phase4::engine::moves;
		using namespace phase4::engine::board;

		const AlgebraicPieceAndSquareOffset &result = position.makeMove(from, to);
		if (wcscmp(result.move.data(), L"") == 0) {
			return false;
		}

		update_animation_offsets(result);
		emit_signal(StringName(SIGNAL_PIECE_MOVED), String(result.move.data()), static_cast<uint64_t>(position.size() - 1));
		return true;
	}

	void update_animation_offsets(const phase4::engine::board::PieceAndSquareOffset &result) {
		using namespace phase4::engine::common;

		for (size_t i = 0; i < result.squares.size(); ++i) {
			const Square square(i);
			square_animation_offsets[i] = Vector2(0, 0);
			const Vector2 offset = get_square_position(result.squares[i]) - get_square_position(square);
			const FieldIndex field = result.squares[i].asFieldIndex();
			int field_mod = is_flipped ? 0 : 1;
			if (!offset.is_zero_approx() && field.x % 2 == field_mod && field.y % 2 != field_mod) {
				slide_trail_end = get_square_position(result.squares[i]) + (Vector2(theme->get_square_size(), 0.0)).rotated(offset.angle());
			}
			square_animation_offsets[i] = offset;
		}
		for (size_t i = 0; i < result.pieces.size(); ++i) {
			piece_animation_offsets[i] = get_square_position(result.pieces[i]) - get_square_position(Square(i));
			piece_trail_ends[Square(i)] = get_square_position(result.pieces[i]) + Vector2(.5, .5) * theme->get_square_size();
		}

		draw_flags |= DrawFlags::BOARD;
		queue_redraw();
	}

	Ref<ChessTheme> theme = nullptr;
	bool is_flipped = false;

	std::optional<Vector2i> highlighted_square;
	std::optional<Vector2i> selected_square;
	std::optional<Vector2i> drag_piece;
	std::optional<Vector2i> annotation_begin_square;
	std::optional<Vector2i> annotation_end_square;
	std::unordered_set<uint16_t> annotations;

	phase4::engine::board::PositionView position;

	CanvasItemUtil flourish_canvas_item;
	CanvasItemUtil square_trail_canvas_item;
	CanvasItemUtil squares_canvas_item;
	CanvasItemUtil valid_move_squares_canvas_item;
	CanvasItemUtil right_slide_hint_canvas_item;
	CanvasItemUtil up_slide_hint_canvas_item;
	CanvasItemUtil left_slide_hint_canvas_item;
	CanvasItemUtil down_slide_hint_canvas_item;
	CanvasItemUtil king_danger_canvas_item;
	CanvasItemUtil file_rank_canvas_item;
	CanvasItemUtil piece_trails_canvas_item;
	CanvasItemUtil pieces_canvas_item;
	CanvasItemUtil valid_move_circles_canvas_item;
	CanvasItemUtil valid_hover_canvas_item;
	CanvasItemUtil invalid_hover_canvas_item;
	CanvasItemUtil selected_canvas_item;
	CanvasItemUtil annotations_canvas_item;
	CanvasItemUtil drag_piece_canvas_item;

	BatchMultiMesh<2> valid_circle_multimeshes;
	Ref<MultiMesh> valid_square_multimesh;
	Ref<Mesh> drag_piece_mesh;
	Ref<MultiMesh> piece_trail_multimesh;
	Ref<MultiMesh> square_trail_multimesh;

	std::array<Vector2, 64> square_animation_offsets;
	std::array<Vector2, 64> piece_animation_offsets;

	std::array<Vector2, 64> piece_trail_ends;
	Vector2 slide_trail_begin;
	Vector2 slide_trail_end;

	std::array<std::array<Ref<MultiMesh>, 6>, 2> piece_meshes;

protected:
	static void _bind_methods();

public:
	Chess2D();

	void _ready() override;
	void _process(double delta) override;
	void _draw() override;
	void _input(const Ref<InputEvent> &event) override;

	String get_fen() const;
	void set_fen(const String &fen);

	bool get_flipped() const;
	void set_flipped(bool flipped);

	void undo_last_move();
	void seek_position(uint64_t index);

	Ref<ChessTheme> get_theme() const;
	void set_theme(const Ref<ChessTheme> &theme);

	Vector2 get_square_position(phase4::engine::common::Square square) {
		using namespace phase4::engine::common;

		ERR_FAIL_COND_V_MSG(theme.is_null(), Vector2(), "Chess Theme is not provided.");

		const real_t offset = -theme->get_square_size() * 4;
		const Vector2 start_position(offset, offset);

		const FieldIndex field = (is_flipped ? square.flipped() : square).asFieldIndex();
		return start_position + theme->get_square_size() * Vector2(field.x, 7 - field.y) + square_animation_offsets[square];
	}

	std::optional<phase4::engine::common::Square> get_selected() {
		using namespace phase4::engine::common;

		if (!selected_square) {
			return {};
		}

		Square square(FieldIndex(selected_square->x, 7 - selected_square->y));

		return is_flipped ? square.flipped() : square;
	}

	phase4::engine::common::Square get_dragged() {
		using namespace phase4::engine::common;

		if (!drag_piece) {
			return Square::INVALID;
		}

		Square square(FieldIndex(drag_piece->x, 7 - drag_piece->y));

		return is_flipped ? square.flipped() : square;
	}

	Vector2 get_mouse_coordinate() const {
		const real_t offset = -theme->get_square_size() * 4;
		const Vector2 square_size = Vector2(1, 1) * theme->get_square_size();
		const Vector2 start_position = Vector2(offset, offset) + get_global_position();
		return (get_global_mouse_position() - start_position) / theme->get_square_size();
	}

	std::optional<phase4::engine::common::Square> get_mouse_square() const {
		using namespace phase4::engine::common;

		const Vector2i mouse_coordinate = get_mouse_coordinate();
		if (!Rect2(0, 0, 8, 8).has_point(mouse_coordinate)) {
			return {};
		}

		const Square square(FieldIndex(mouse_coordinate.x, 7 - mouse_coordinate.y));
		return is_flipped ? square.flipped() : square;
	}

	void toggle_annotation(phase4::engine::common::Square from, phase4::engine::common::Square to) {
		int16_t value = from.get_raw_value() + to.get_raw_value() * 64;
		if (annotations.find(value) != annotations.end()) {
			annotations.erase(value);
		} else {
			annotations.insert(value);
		}
	}

	void clear_animation_offsets() {
		square_animation_offsets.fill(Vector2(0, 0));
		piece_animation_offsets.fill(Vector2(0, 0));

		for (size_t i = 0; i < piece_trail_ends.size(); ++i) {
			piece_trail_ends[i] = get_square_position(phase4::engine::common::Square(i)) + Vector2(.5, .5) * theme->get_square_size();
		}

		draw_flags |= DrawFlags::BOARD;
		queue_redraw();
	}
};

} //namespace godot

#endif
