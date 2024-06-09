#ifndef CHESS2D_H
#define CHESS2D_H

#include "canvas_item_util.h"
#include "chess_theme.h"

#include <godot_cpp/classes/input_event.hpp>
#include <godot_cpp/classes/node2d.hpp>
#include <godot_cpp/classes/rendering_server.hpp>

#include <phase4/engine/board/session.h>

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

		BOARD = 0b110,
		ALL = 0b11111,
	};
	int64_t draw_flags = DrawFlags::ALL;

private:
	Ref<ChessTheme> theme = nullptr;
	bool is_flipped = false;

	std::optional<Vector2i> highlighted_square;
	std::optional<Vector2i> selected_square;
	std::optional<Vector2i> annotation_begin_square;
	std::optional<Vector2i> annotation_end_square;
	std::unordered_set<uint16_t> annotations;

	phase4::engine::board::Session session;

	CanvasItemUtil squares_canvas_item;
	CanvasItemUtil right_slide_hint_canvas_item;
	CanvasItemUtil up_slide_hint_canvas_item;
	CanvasItemUtil left_slide_hint_canvas_item;
	CanvasItemUtil down_slide_hint_canvas_item;
	CanvasItemUtil flourish_canvas_item;
	CanvasItemUtil file_rank_canvas_item;
	CanvasItemUtil pieces_canvas_item;
	CanvasItemUtil valid_hover_canvas_item;
	CanvasItemUtil invalid_hover_canvas_item;
	CanvasItemUtil selected_canvas_item;
	CanvasItemUtil annotations_canvas_item;

	phase4::engine::moves::Moves valid_moves;
	std::array<phase4::engine::common::FastVector<phase4::engine::moves::Move, 21>, 64> valid_moves_map;

	void compute_valid_moves() {
		valid_moves.clear();
		for (size_t i = 0; i < valid_moves_map.size(); ++i) {
			valid_moves_map[i].clear();
		}

		phase4::engine::board::PositionMoves::getValidMoves(session.position(), valid_moves);
		for (size_t i = 0; i < valid_moves.size(); ++i) {
			valid_moves_map[valid_moves[i].from()].push_back(valid_moves[i]);
		}
	}

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

	Ref<ChessTheme> get_theme() const;
	void set_theme(const Ref<ChessTheme> &theme);

	Vector2 get_square_position(phase4::engine::common::Square square) {
		using namespace phase4::engine::common;

		ERR_FAIL_COND_V_MSG(theme.is_null(), Vector2(), "Chess Theme is not provided.");

		const real_t offset = -theme->get_square_size() * 4;
		const Vector2 start_position(offset, offset);

		const FieldIndex field = (is_flipped ? square.flipped() : square).asFieldIndex();
		return start_position + theme->get_square_size() * Vector2(field.x, 7 - field.y);
	}

	std::optional<phase4::engine::common::Square> get_selected() {
		using namespace phase4::engine::common;

		if (!selected_square) {
			return {};
		}

		Square square(FieldIndex(selected_square->x, 7 - selected_square->y));

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
};

} //namespace godot

#endif
