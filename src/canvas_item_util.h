#ifndef CANVAS_ITEM_UTIL_H
#define CANVAS_ITEM_UTIL_H

#include <godot_cpp/classes/material.hpp>
#include <godot_cpp/classes/mesh.hpp>
#include <godot_cpp/classes/ref.hpp>
#include <godot_cpp/classes/rendering_server.hpp>
#include <godot_cpp/classes/texture.hpp>
#include <godot_cpp/classes/multi_mesh.hpp>
#include <godot_cpp/variant/color.hpp>
#include <godot_cpp/variant/rect2.hpp>
#include <godot_cpp/variant/transform2d.hpp>
#include <godot_cpp/variant/utility_functions.hpp>

#include <memory>

class CanvasItemUtil {
public:
	CanvasItemUtil() = default;
	CanvasItemUtil(CanvasItemUtil &&) = default;
	CanvasItemUtil &operator=(CanvasItemUtil &&) = default;
	CanvasItemUtil(const CanvasItemUtil &) = delete;
	CanvasItemUtil &operator=(const CanvasItemUtil &) = delete;

	inline void instantiate() {
		ERR_FAIL_COND_MSG(canvas_item.is_valid(), "Already instantiated");

		canvas_item = godot::RenderingServer::get_singleton()->canvas_item_create();
	}

	inline const godot::RID &operator*() const {
		return canvas_item;
	}

	inline void set_material(const godot::Ref<godot::Material> &material) {
		ERR_FAIL_COND(material.is_null());
		ERR_FAIL_COND(!canvas_item.is_valid());

		this->material = material;

		godot::RenderingServer::get_singleton()->canvas_item_set_material(canvas_item, this->material->get_rid());
	}

	inline void set_parent(godot::RID parent) const {
		ERR_FAIL_COND(!canvas_item.is_valid());

		godot::RenderingServer::get_singleton()->canvas_item_set_parent(canvas_item, parent);
	}

	inline void set_draw_index(int32_t index) const {
		ERR_FAIL_COND(!canvas_item.is_valid());

		godot::RenderingServer::get_singleton()->canvas_item_set_draw_index(canvas_item, index);
	}

	inline void set_self_modulate(const godot::Color &color) const {
		ERR_FAIL_COND(!canvas_item.is_valid());

		godot::RenderingServer::get_singleton()->canvas_item_set_self_modulate(canvas_item, color);
	}

	inline void set_transform(const godot::Transform2D &transform) const {
		ERR_FAIL_COND(!canvas_item.is_valid());

		godot::RenderingServer::get_singleton()->canvas_item_set_transform(canvas_item, transform);
	}

	inline void add_rect(const godot::Rect2 rect, const godot::Color &color) const {
		ERR_FAIL_COND(!canvas_item.is_valid());

		godot::RenderingServer::get_singleton()->canvas_item_add_rect(canvas_item, rect, color);
	}

	inline void add_texture_rect(const godot::Rect2 &rect, const godot::Texture &texture, bool tile = false, const godot::Color &modulate = godot::Color(1, 1, 1, 1), bool transpose = false) const {
		ERR_FAIL_COND(!canvas_item.is_valid());

		godot::RenderingServer::get_singleton()->canvas_item_add_texture_rect(canvas_item, rect, texture.get_rid(), tile, modulate, transpose);
	}

	inline void add_mesh(const godot::Mesh &mesh, const godot::Transform2D &transform = godot::Transform2D(), const godot::Color &modulate = godot::Color(1, 1, 1, 1)) const {
		ERR_FAIL_COND(!canvas_item.is_valid());

		godot::RenderingServer::get_singleton()->canvas_item_add_mesh(canvas_item, mesh.get_rid(), transform, modulate);
	}

	inline void add_multimesh(const godot::MultiMesh& mesh) {
		ERR_FAIL_COND(!canvas_item.is_valid());

		godot::RenderingServer::get_singleton()->canvas_item_add_multimesh(canvas_item, mesh.get_rid());
	}

	inline void add_multimesh(const godot::MultiMesh& mesh, const godot::Texture& texture) {
		ERR_FAIL_COND(!canvas_item.is_valid());

		godot::RenderingServer::get_singleton()->canvas_item_add_multimesh(canvas_item, mesh.get_rid(), texture.get_rid());
	}

	inline void add_mesh(const godot::Mesh &mesh, const godot::Transform2D &transform, const godot::Color &modulate, const godot::Texture &texture) const {
		ERR_FAIL_COND(!canvas_item.is_valid());

		godot::RenderingServer::get_singleton()->canvas_item_add_mesh(canvas_item, mesh.get_rid(), transform, modulate, texture.get_rid());
	}

	inline void clear() const {
		ERR_FAIL_COND(!canvas_item.is_valid());

		godot::RenderingServer::get_singleton()->canvas_item_clear(canvas_item);
	}

	~CanvasItemUtil() {
		if (canvas_item.is_valid()) {
			godot::RenderingServer::get_singleton()->free_rid(canvas_item);
			canvas_item = godot::RID();
		}
	}

private:
	godot::RID canvas_item;
	godot::Ref<godot::Material> material;
};

#endif
