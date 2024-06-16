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
		VALID_MOVES = 0b100000,
		DRAG_PIECE = 0b1000000,
		FILE_RANK = 0b10000000,

		BOARD = SQUARES | PIECES | FILE_RANK,
		ALL = 0b111111,
	};
	int64_t draw_flags = DrawFlags::ALL;

private:
	bool make_move(phase4::engine::common::Square from, phase4::engine::common::Square to) {
		using namespace phase4::engine::common;
		using namespace phase4::engine::moves;

		const Move move(from, to, MoveFlags::QUIET);
		auto realMove = phase4::engine::board::PositionMoves::findRealMove(session.position(), move);
		if (!realMove) {
			return false;
		}

		clear_animation_offsets();
		const phase4::engine::board::PositionMoves::MakeMoveResult &result = session.makeMove(*realMove);
		if (result.slide) {
			const FieldIndex fixedSlide(result.slide->x, -result.slide->y);
			const Square slideTo = Square(fixedSlide + to.asFieldIndex());
			piece_animation_offsets[slideTo] = get_square_position(result.moved[0].from) - get_square_position(result.moved[0].to);

			Bitboard walls = session.position().walls();
			while (walls > 0) {
				const Square wall(Square(walls).asFieldIndex() + fixedSlide);
				walls = walls.popLsb();
				square_animation_offsets[wall] = (is_flipped ? -1 : 1) * Vector2(-result.slide->x, -result.slide->y) * theme->get_square_size();
			}

			draw_flags |= DrawFlags::FILE_RANK;
		} else {
			piece_animation_offsets[result.moved[0].to] = get_square_position(result.moved[0].from) - get_square_position(result.moved[0].to);
		}

		compute_valid_moves();
		draw_flags |= DrawFlags::BOARD;
		queue_redraw();

		return true;
	}

	Ref<ChessTheme> theme = nullptr;
	bool is_flipped = false;

	std::optional<Vector2i> highlighted_square;
	std::optional<Vector2i> selected_square;
	std::optional<Vector2i> drag_piece;
	std::optional<Vector2i> annotation_begin_square;
	std::optional<Vector2i> annotation_end_square;
	std::unordered_set<uint16_t> annotations;

	phase4::engine::board::Session session;

	CanvasItemUtil flourish_canvas_item;
	CanvasItemUtil squares_canvas_item;
	CanvasItemUtil valid_move_squares_canvas_item;
	CanvasItemUtil right_slide_hint_canvas_item;
	CanvasItemUtil up_slide_hint_canvas_item;
	CanvasItemUtil left_slide_hint_canvas_item;
	CanvasItemUtil down_slide_hint_canvas_item;
	CanvasItemUtil file_rank_canvas_item;
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

	phase4::engine::moves::Moves valid_moves;
	std::array<phase4::engine::common::FastVector<phase4::engine::moves::Move, 21>, 64> valid_moves_map;

	std::array<Vector2, 64> square_animation_offsets;
	std::array<Vector2, 64> piece_animation_offsets;

	std::array<std::array<Ref<MultiMesh>, 6>, 2> piece_meshes;

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

		draw_flags |= DrawFlags::BOARD;
		queue_redraw();
	}
};

} //namespace godot

#endif
