#ifndef CHESS2D_H
#define CHESS2D_H

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

private:
	String fen = "rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1";
	Ref<ChessTheme> theme = nullptr;
	bool is_flipped = false;

	std::optional<Vector2i> highlighted_square;
	std::optional<Vector2i> selected_square;
	std::optional<Vector2i> annotation_begin_square;
	std::unordered_set<uint16_t> annotations;

	std::unique_ptr<phase4::engine::board::Session> session;

	RID demo_canvas_item;

protected:
	static void _bind_methods();

public:
	Chess2D() {
		annotations.reserve(64 * 64);
		using namespace phase4::engine::common;
		toggle_annotation(Square::A1, Square::F4);
		toggle_annotation(Square::D3, Square::D3);

		RenderingServer* RS = RenderingServer::get_singleton();
		demo_canvas_item = RS->canvas_item_create();
	}

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
