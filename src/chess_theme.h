#ifndef CHESSTHEME_H
#define CHESSTHEME_H

#include "canvas_item_util.h"

#include <godot_cpp/classes/font.hpp>
#include <godot_cpp/classes/resource.hpp>
#include <godot_cpp/classes/texture.hpp>
#include <godot_cpp/classes/mesh.hpp>
#include <godot_cpp/classes/rendering_server.hpp>
#include <godot_cpp/classes/material.hpp>

#include <phase4/engine/common/piece_color.h>
#include <phase4/engine/common/piece_type.h>
#include <phase4/engine/common/square.h>

#include <memory>

namespace godot {

class ChessTheme : public Resource {
	GDCLASS(ChessTheme, Resource)

private:
	Ref<Texture> white_pawn_texture;
	Ref<Texture> white_knight_texture;
	Ref<Texture> white_bishop_texture;
	Ref<Texture> white_rook_texture;
	Ref<Texture> white_queen_texture;
	Ref<Texture> white_king_texture;

	Ref<Texture> black_pawn_texture;
	Ref<Texture> black_knight_texture;
	Ref<Texture> black_bishop_texture;
	Ref<Texture> black_rook_texture;
	Ref<Texture> black_queen_texture;
	Ref<Texture> black_king_texture;

    Ref<Material> slide_hint_material;

	Color white_square_color;
	Color black_square_color;

	real_t square_size;

	Ref<Font> font;

    std::array<Ref<Mesh>, 64> annotation_meshes;
    Color annotation_color;

protected:
	static void _bind_methods();

public:
	Ref<Texture> get_white_pawn_texture() const {
		return white_pawn_texture;
	}

	void set_white_pawn_texture(const Ref<Texture> &texture) {
		white_pawn_texture = texture;
	}

	Ref<Texture> get_white_knight_texture() const {
		return white_knight_texture;
	}

	void set_white_knight_texture(const Ref<Texture> &texture) {
		white_knight_texture = texture;
	}

	Ref<Texture> get_white_bishop_texture() const {
		return white_bishop_texture;
	}

	void set_white_bishop_texture(const Ref<Texture> &texture) {
		white_bishop_texture = texture;
	}

	Ref<Texture> get_white_rook_texture() const {
		return white_rook_texture;
	}

	void set_white_rook_texture(const Ref<Texture> &texture) {
		white_rook_texture = texture;
	}

	Ref<Texture> get_white_queen_texture() const {
		return white_queen_texture;
	}

	void set_white_queen_texture(const Ref<Texture> &texture) {
		white_queen_texture = texture;
	}

	Ref<Texture> get_white_king_texture() const {
		return white_king_texture;
	}

	void set_white_king_texture(const Ref<Texture> &texture) {
		white_king_texture = texture;
	}

	Ref<Texture> get_black_pawn_texture() const {
		return black_pawn_texture;
	}

	void set_black_pawn_texture(const Ref<Texture> &texture) {
		black_pawn_texture = texture;
	}

	Ref<Texture> get_black_knight_texture() const {
		return black_knight_texture;
	}

	void set_black_knight_texture(const Ref<Texture> &texture) {
		black_knight_texture = texture;
	}

	Ref<Texture> get_black_bishop_texture() const {
		return black_bishop_texture;
	}

	void set_black_bishop_texture(const Ref<Texture> &texture) {
		black_bishop_texture = texture;
	}

	Ref<Texture> get_black_rook_texture() const {
		return black_rook_texture;
	}

	void set_black_rook_texture(const Ref<Texture> &texture) {
		black_rook_texture = texture;
	}

	Ref<Texture> get_black_queen_texture() const {
		return black_queen_texture;
	}

	void set_black_queen_texture(const Ref<Texture> &texture) {
		black_queen_texture = texture;
	}

	Ref<Texture> get_black_king_texture() const {
		return black_king_texture;
	}

	void set_black_king_texture(const Ref<Texture> &texture) {
		black_king_texture = texture;
	}

	Color get_white_square_color() const {
		return white_square_color;
	}

	void set_white_square_color(Color color) {
		white_square_color = color;
	}

	Color get_black_square_color() const {
		return black_square_color;
	}

	void set_black_square_color(Color color) {
		black_square_color = color;
	}

	real_t get_square_size() const {
		return square_size;
	}

	void set_square_size(real_t size);

	Ref<Font> get_font() const {
		return font;
	}

	void set_font(const Ref<Font> &font) {
		this->font = font;
	}

	Color get_annotation_color() const {
		return annotation_color;
	}

	void set_annotation_color(Color color);

	const Ref<Mesh>& get_annotation_mesh(phase4::engine::common::Square from, phase4::engine::common::Square to);

	const Ref<Texture> &get_piece_texture(phase4::engine::common::PieceColor color, phase4::engine::common::PieceType piece) {
		static const Ref<Texture> invalid_texture;
		if (color == phase4::engine::common::PieceColor::WHITE) {
			switch (piece.get_raw_value()) {
				case phase4::engine::common::PieceType::PAWN.get_raw_value():
					return white_pawn_texture;
				case phase4::engine::common::PieceType::KNIGHT.get_raw_value():
					return white_knight_texture;
				case phase4::engine::common::PieceType::BISHOP.get_raw_value():
					return white_bishop_texture;
				case phase4::engine::common::PieceType::ROOK.get_raw_value():
					return white_rook_texture;
				case phase4::engine::common::PieceType::QUEEN.get_raw_value():
					return white_queen_texture;
				case phase4::engine::common::PieceType::KING.get_raw_value():
					return white_king_texture;
				default:
					ERR_FAIL_V_MSG(invalid_texture, "Invalid Piece Type");
			}
		} else if (color == phase4::engine::common::PieceColor::BLACK) {
			switch (piece.get_raw_value()) {
				case phase4::engine::common::PieceType::PAWN.get_raw_value():
					return black_pawn_texture;
				case phase4::engine::common::PieceType::KNIGHT.get_raw_value():
					return black_knight_texture;
				case phase4::engine::common::PieceType::BISHOP.get_raw_value():
					return black_bishop_texture;
				case phase4::engine::common::PieceType::ROOK.get_raw_value():
					return black_rook_texture;
				case phase4::engine::common::PieceType::QUEEN.get_raw_value():
					return black_queen_texture;
				case phase4::engine::common::PieceType::KING.get_raw_value():
					return black_king_texture;
				default:
					ERR_FAIL_V_MSG(invalid_texture, "Invalid Piece Type");
			}
		}
		ERR_FAIL_V_MSG(invalid_texture, "Invalid Piece Color");
	}

    CanvasItemUtil slide_hint_canvas_item_create() {
        CanvasItemUtil canvas_item;
        canvas_item.instantiate();

        if (slide_hint_material.is_valid()) {
            Ref<Material> material = slide_hint_material->duplicate();
            canvas_item.set_material(material);
        }
        return canvas_item;
    }
};

} //namespace godot

#endif
