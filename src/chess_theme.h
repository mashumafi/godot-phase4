#ifndef CHESSTHEME_H
#define CHESSTHEME_H

#include "batch_multi_mesh.h"
#include "canvas_item_util.h"

#include <godot_cpp/classes/font.hpp>
#include <godot_cpp/classes/mesh.hpp>
#include <godot_cpp/classes/multi_mesh.hpp>
#include <godot_cpp/classes/resource.hpp>
#include <godot_cpp/classes/shader_material.hpp>
#include <godot_cpp/classes/texture2d.hpp>
#include <godot_cpp/core/math.hpp>

#include <phase4/engine/common/field_index.h>
#include <phase4/engine/common/piece_color.h>
#include <phase4/engine/common/piece_type.h>
#include <phase4/engine/common/square.h>

#include <memory>

namespace godot {

class ChessTheme : public Resource {
	GDCLASS(ChessTheme, Resource)

private:
	Ref<Texture2D> white_pawn_texture;
	Ref<Texture2D> white_knight_texture;
	Ref<Texture2D> white_bishop_texture;
	Ref<Texture2D> white_rook_texture;
	Ref<Texture2D> white_queen_texture;
	Ref<Texture2D> white_king_texture;

	Ref<Texture2D> black_pawn_texture;
	Ref<Texture2D> black_knight_texture;
	Ref<Texture2D> black_bishop_texture;
	Ref<Texture2D> black_rook_texture;
	Ref<Texture2D> black_queen_texture;
	Ref<Texture2D> black_king_texture;

	Ref<ShaderMaterial> slide_hint_material;
	Ref<ShaderMaterial> king_danger_material;

	Color white_square_color;
	Color black_square_color;

	real_t square_size;

	Ref<Font> font;

	std::array<Ref<Mesh>, 64> annotation_meshes;
	Color annotation_color;

	Ref<Texture2D> flourish;
	Ref<Texture2D> trail_texture;
	Ref<Material> trail_material;
	Ref<Mesh> flourish_mesh;

	Ref<Mesh> highlight_mesh;

protected:
	static void _bind_methods();

public:
	enum PieceColor {
		PIECE_COLOR_WHITE = phase4::engine::common::PieceColor::WHITE.get_raw_value(),
		PIECE_COLOR_BLACK = phase4::engine::common::PieceColor::BLACK.get_raw_value(),
		PIECE_COLOR_MAX = phase4::engine::common::PieceColor::INVALID.get_raw_value(),
	};

	enum PieceType {
		PIECE_TYPE_PAWN = phase4::engine::common::PieceType::PAWN.get_raw_value(),
		PIECE_TYPE_KNIGHT = phase4::engine::common::PieceType::KNIGHT.get_raw_value(),
		PIECE_TYPE_BISHOP = phase4::engine::common::PieceType::BISHOP.get_raw_value(),
		PIECE_TYPE_ROOK = phase4::engine::common::PieceType::ROOK.get_raw_value(),
		PIECE_TYPE_QUEEN = phase4::engine::common::PieceType::QUEEN.get_raw_value(),
		PIECE_TYPE_KING = phase4::engine::common::PieceType::KING.get_raw_value(),
		PIECE_TYPE_MAX = phase4::engine::common::PieceType::INVALID.get_raw_value(),
	};

	Ref<Texture2D> get_white_pawn_texture() const {
		return white_pawn_texture;
	}

	void set_white_pawn_texture(const Ref<Texture2D> &texture) {
		white_pawn_texture = texture;
		emit_changed();
	}

	Ref<Texture2D> get_white_knight_texture() const {
		return white_knight_texture;
	}

	void set_white_knight_texture(const Ref<Texture2D> &texture) {
		white_knight_texture = texture;
		emit_changed();
	}

	Ref<Texture2D> get_white_bishop_texture() const {
		return white_bishop_texture;
	}

	void set_white_bishop_texture(const Ref<Texture2D> &texture) {
		white_bishop_texture = texture;
		emit_changed();
	}

	Ref<Texture2D> get_white_rook_texture() const {
		return white_rook_texture;
	}

	void set_white_rook_texture(const Ref<Texture2D> &texture) {
		white_rook_texture = texture;
		emit_changed();
	}

	Ref<Texture2D> get_white_queen_texture() const {
		return white_queen_texture;
	}

	void set_white_queen_texture(const Ref<Texture2D> &texture) {
		white_queen_texture = texture;
		emit_changed();
	}

	Ref<Texture2D> get_white_king_texture() const {
		return white_king_texture;
	}

	void set_white_king_texture(const Ref<Texture2D> &texture) {
		white_king_texture = texture;
		emit_changed();
	}

	Ref<Texture2D> get_black_pawn_texture() const {
		return black_pawn_texture;
	}

	void set_black_pawn_texture(const Ref<Texture2D> &texture) {
		black_pawn_texture = texture;
		emit_changed();
	}

	Ref<Texture2D> get_black_knight_texture() const {
		return black_knight_texture;
	}

	void set_black_knight_texture(const Ref<Texture2D> &texture) {
		black_knight_texture = texture;
		emit_changed();
	}

	Ref<Texture2D> get_black_bishop_texture() const {
		return black_bishop_texture;
	}

	void set_black_bishop_texture(const Ref<Texture2D> &texture) {
		black_bishop_texture = texture;
		emit_changed();
	}

	Ref<Texture2D> get_black_rook_texture() const {
		return black_rook_texture;
	}

	void set_black_rook_texture(const Ref<Texture2D> &texture) {
		black_rook_texture = texture;
		emit_changed();
	}

	Ref<Texture2D> get_black_queen_texture() const {
		return black_queen_texture;
	}

	void set_black_queen_texture(const Ref<Texture2D> &texture) {
		black_queen_texture = texture;
		emit_changed();
	}

	Ref<Texture2D> get_black_king_texture() const {
		return black_king_texture;
	}

	void set_black_king_texture(const Ref<Texture2D> &texture) {
		black_king_texture = texture;
		emit_changed();
	}

	Color get_white_square_color() const {
		return white_square_color;
	}

	void set_white_square_color(Color color) {
		white_square_color = color;
		emit_changed();
	}

	Color get_black_square_color() const {
		return black_square_color;
	}

	void set_black_square_color(Color color) {
		black_square_color = color;
		emit_changed();
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
		emit_changed();
	}

	Color get_annotation_color() const {
		return annotation_color;
	}

	void set_annotation_color(Color color);

	Ref<Texture2D> get_flourish() const {
		return flourish;
	}

	void set_flourish(const Ref<Texture2D> &texture);

	Ref<Texture2D> get_trail_texture() const {
		return trail_texture;
	}

	void set_trail_texture(const Ref<Texture2D> &texture);

	Ref<Material> get_trail_material() const {
		return trail_material;
	}

	void set_trail_material(const Ref<Material> &material);

	Transform2D transform_trail(const Vector2 &begin, const Vector2 &end, real_t width);

	const Ref<Mesh> &get_annotation_mesh(phase4::engine::common::Square from, phase4::engine::common::Square to);

	// The position as if the board is not centered, origin (0, 0)
	Vector2 to_local(const phase4::engine::common::FieldIndex &field) const {
		return Vector2(field.x, 7 - field.y) * Vector2(square_size, square_size);
	}

	// The position as if the board is centered
	Vector2 to_global(const phase4::engine::common::FieldIndex &field) const {
		return to_local(field) - Vector2(1, 1) * square_size * 4;
	}

	Transform2D get_annotation_transform(phase4::engine::common::Square from, phase4::engine::common::Square to) const {
		using namespace phase4::engine::common;

		ERR_FAIL_COND_V(square_size <= 0, Transform2D());

		const Vector2 begin = to_local(from.asFieldIndex());
		if (from == to) {
			return Transform2D().translated(begin);
		}

		const Vector2 end = to_local(to.asFieldIndex());
		const Vector2 diff = end - begin;

		return Transform2D()
				.scaled(Vector2(diff.x == 0 ? 1 : diff.x, diff.y == 0 ? 1 : diff.y).sign())
				.translated(begin);
	}

	Ref<Texture2D> get_piece_texture(PieceColor color, PieceType piece) {
		static const Ref<Texture2D> invalid_texture;
		if (color == PIECE_COLOR_WHITE) {
			switch (piece) {
				case PIECE_TYPE_PAWN:
					return white_pawn_texture;
				case PIECE_TYPE_KNIGHT:
					return white_knight_texture;
				case PIECE_TYPE_BISHOP:
					return white_bishop_texture;
				case PIECE_TYPE_ROOK:
					return white_rook_texture;
				case PIECE_TYPE_QUEEN:
					return white_queen_texture;
				case PIECE_TYPE_KING:
					return white_king_texture;
				default:
					ERR_FAIL_V_MSG(invalid_texture, "Invalid Piece Type");
			}
		} else if (color == PIECE_COLOR_BLACK) {
			switch (piece) {
				case PIECE_TYPE_PAWN:
					return black_pawn_texture;
				case PIECE_TYPE_KNIGHT:
					return black_knight_texture;
				case PIECE_TYPE_BISHOP:
					return black_bishop_texture;
				case PIECE_TYPE_ROOK:
					return black_rook_texture;
				case PIECE_TYPE_QUEEN:
					return black_queen_texture;
				case PIECE_TYPE_KING:
					return black_king_texture;
				default:
					ERR_FAIL_V_MSG(invalid_texture, "Invalid Piece Type");
			}
		}
		ERR_FAIL_V_MSG(invalid_texture, "Invalid Piece Color");
	}

	Ref<MultiMesh> create_square_multimesh(bool use_colors = true) const;
	Ref<Mesh> create_square_mesh() const;

	const Ref<Mesh> &get_highlight_mesh() const {
		return highlight_mesh;
	}

	Ref<ShaderMaterial> get_slide_hint_material() const {
		return slide_hint_material;
	}

	void set_slide_hint_material(const Ref<ShaderMaterial> &material) {
		this->slide_hint_material = material;
		emit_changed();
	}

	Ref<ShaderMaterial> get_king_danger_material() const {
		return king_danger_material;
	}

	void set_king_danger_material(const Ref<ShaderMaterial> &material) {
		this->king_danger_material = material;
		emit_changed();
	}

	const Mesh &get_flourish_mesh() const {
		return *flourish_mesh.ptr();
	}

	Ref<Mesh> create_circle_polygon(const real_t radius) const;

	BatchMultiMesh<2> create_circle() const;
	Ref<MultiMesh> create_trail() const;

	CanvasItemUtil create_slide_hint_canvas_item(const Vector2 &direction) {
		CanvasItemUtil canvas_item;
		canvas_item.instantiate();

		if (slide_hint_material.is_valid()) {
			const Ref<ShaderMaterial> &material = slide_hint_material->duplicate();
			material->set_shader_parameter("direction", direction);
			canvas_item.set_material(material);
		}
		return canvas_item;
	}
};

} //namespace godot

VARIANT_ENUM_CAST(godot::ChessTheme::PieceColor);
VARIANT_ENUM_CAST(godot::ChessTheme::PieceType);

#endif
