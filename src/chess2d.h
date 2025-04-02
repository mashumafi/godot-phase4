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

	bool _make_move(phase4::engine::moves::Move move);

	void update_animation_offsets(const phase4::engine::board::PieceAndSquareOffset &result);

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

	std::array<Vector2, 64> square_target_offsets;
	std::array<Vector2, 64> square_animation_offsets;
	std::array<Vector2, 64> piece_animation_offsets;

	std::array<Vector2, 64> piece_trail_ends;
	std::array<Vector2, 64> slide_trail_end;

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
	void theme_changed();

	Vector2 get_square_position(phase4::engine::common::Square square);

	std::optional<phase4::engine::common::Square> get_selected();

	phase4::engine::common::Square get_dragged();

	Vector2 get_mouse_coordinate() const;

	std::optional<phase4::engine::common::Square> get_mouse_square() const;

	void toggle_annotation(phase4::engine::common::Square from, phase4::engine::common::Square to);

	void clear_animation_offsets();

	void break_square(const godot::String &square_name);

	void slide_squares(const Vector2i &direction);

	void set_target_offsets(const PackedVector2Array& p_offsets);

	static godot::String field_to_square(int file, int rank, bool flip) ;
	static Vector2i square_to_field(const godot::String &square_name, bool flip);
};

} //namespace godot

#endif
