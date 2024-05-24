#ifndef CHESS2D_H
#define CHESS2D_H

#include <godot_cpp/classes/node2d.hpp>

namespace godot {

class Chess2D : public Node2D {
	GDCLASS(Chess2D, Node2D)

private:
	String fen;

protected:
	static void _bind_methods();

public:
	Chess2D();
	~Chess2D();

	void _process(double delta) override;
	void _draw() override;

	String get_fen() const;
	void set_fen(const String& fen);
};

}

#endif
