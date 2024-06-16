#ifndef CHESSTHEME_H
#define CHESSTHEME_H

#include "canvas_item_util.h"
#include "batch_multi_mesh.h"

#include <godot_cpp/classes/font.hpp>
#include <godot_cpp/classes/mesh.hpp>
#include <godot_cpp/classes/resource.hpp>
#include <godot_cpp/classes/shader_material.hpp>
#include <godot_cpp/classes/texture.hpp>
#include <godot_cpp/classes/multi_mesh.hpp>
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

	Ref<godot::ShaderMaterial> slide_hint_material;

	Color white_square_color;
	Color black_square_color;

	real_t square_size;

	Ref<Font> font;

	std::array<Ref<Mesh>, 64> annotation_meshes;
	Color annotation_color;

	Ref<Texture> flourish;
	Ref<Mesh> flourish_mesh;

	Ref<Mesh> highlight_mesh;

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

	Ref<Texture> get_flourish() const {
		return flourish;
	}

	void set_flourish(const Ref<Texture> &texture);

	const Ref<Mesh> &get_annotation_mesh(phase4::engine::common::Square from, phase4::engine::common::Square to);

	// The position as if the board is not centered, origin (0, 0)
	Vector2 to_local(const phase4::engine::common::FieldIndex &field) const {
		return godot::Vector2(field.x, 7 - field.y) * godot::Vector2(square_size, square_size);
	}

	// The position as if the board is centered
	Vector2 to_global(const phase4::engine::common::FieldIndex &field) const {
		return to_local(field) - godot::Vector2(1, 1) * square_size * 4;
	}

	godot::Transform2D get_annotation_transform(phase4::engine::common::Square from, phase4::engine::common::Square to) const {
		using namespace phase4::engine::common;

		ERR_FAIL_COND_V(square_size <= 0, godot::Transform2D());

		const godot::Vector2 begin = to_local(from.asFieldIndex());
		if (from == to) {
			return godot::Transform2D().translated(begin);
		}

		const godot::Vector2 end = to_local(to.asFieldIndex());
		const godot::Vector2 diff = end - begin;

		return godot::Transform2D()
				.scaled(Vector2(diff.x == 0 ? 1 : diff.x, diff.y == 0 ? 1 : diff.y).sign())
				.translated(begin);
	}

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

	Ref<MultiMesh> get_square_mesh(bool use_colors = true) const;

	const Ref<Mesh> &get_highlight_mesh() const {
		return highlight_mesh;
	}

	Ref<ShaderMaterial> get_slide_hint_material() const {
		return slide_hint_material;
	}

	void set_slide_hint_material(const Ref<godot::ShaderMaterial> &material) {
		this->slide_hint_material = material;
	}

	const Mesh &get_flourish_mesh() const {
		return *flourish_mesh.ptr();
	}

	BatchMultiMesh<2> create_circle();

	CanvasItemUtil slide_hint_canvas_item_create(const Vector2 &direction) {
		CanvasItemUtil canvas_item;
		canvas_item.instantiate();

		if (slide_hint_material.is_valid()) {
			const godot::Ref<godot::ShaderMaterial> &material = slide_hint_material->duplicate();
			material->set_shader_parameter("direction", direction);
			canvas_item.set_material(material);
		}
		return canvas_item;
	}
};

} //namespace godot

#endif
