#ifndef CHESSTHEME_H
#define CHESSTHEME_H

#include <godot_cpp/classes/resource.hpp>
#include <godot_cpp/classes/texture.hpp>

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

    Color white_square_color;
    Color black_square_color;

protected:
	static void _bind_methods();

public:
    Ref<Texture> get_white_pawn_texture() const {
        return white_pawn_texture;
    }

    void set_white_pawn_texture(const Ref<Texture>& texture) {
        return white_pawn_texture = texture;
    }
};

}

#endif
