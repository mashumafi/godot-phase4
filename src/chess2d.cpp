#include "chess2d.h"

#include <godot_cpp/core/class_db.hpp>
#include <godot_cpp/classes/rendering_server.hpp>

using namespace godot;

void Chess2D::_bind_methods() {
	const StringName class_name = "Chess2D";
	const StringName get_fen_method = "get_fen";
	const StringName set_fen_method = "set_fen";
	const StringName fen_property = "fen";

	ClassDB::bind_method(D_METHOD(get_fen_method), &Chess2D::get_fen);
	ClassDB::bind_method(D_METHOD(set_fen_method, fen_property), &Chess2D::set_fen);
	ClassDB::add_property(class_name, PropertyInfo(Variant::STRING, fen_property), set_fen_method, get_fen_method);
}

Chess2D::Chess2D() {
}

Chess2D::~Chess2D() {
}

void Chess2D::_process(double delta) {
}

void Chess2D::_draw() {
	RenderingServer* gfx = RenderingServer::get_singleton();
	
}

String Chess2D::get_fen() const {
	return this->fen;
}

void Chess2D::set_fen(const String& fen) {
	this->fen = fen;
}
