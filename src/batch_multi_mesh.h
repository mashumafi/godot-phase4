#ifndef BATCH_MULTIMESH_H
#define BATCH_MULTIMESH_H

#include <godot_cpp/classes/multi_mesh.hpp>
#include <godot_cpp/classes/rendering_server.hpp>
#include <godot_cpp/classes/texture.hpp>

#include <array>

template <size_t MESH_COUNT>
class BatchMultiMesh {
public:
	using Meshes = std::array<godot::Ref<godot::MultiMesh>, MESH_COUNT>;

	inline BatchMultiMesh() {
		for (godot::Ref<godot::MultiMesh> &mesh : meshes) {
			mesh.instantiate();
		}
	}

	inline void set_meshes(const std::array<godot::Ref<godot::Mesh>, MESH_COUNT> &meshes) {
		for (size_t i = 0; i < MESH_COUNT; ++i) {
			this->meshes[i]->set_mesh(meshes[i]);
		}
	}

	inline void set_use_colors(bool enable) {
		for (godot::Ref<godot::MultiMesh> &mesh : meshes) {
			mesh->set_use_colors(enable);
		}
	}

	inline void set_transform_format(godot::MultiMesh::TransformFormat format) {
		for (godot::Ref<godot::MultiMesh> &mesh : meshes) {
			mesh->set_transform_format(format);
		}
	}

	inline void set_instance_count(int32_t count) {
		for (godot::Ref<godot::MultiMesh> &mesh : meshes) {
			mesh->set_instance_count(count);
		}
	}

	inline void set_instance_color(int32_t instance, const godot::Color &color) {
		for (godot::Ref<godot::MultiMesh> &mesh : meshes) {
			mesh->set_instance_color(instance, color);
		}
	}

	inline void set_instance_transform_2d(int32_t instance, const godot::Transform2D &transform) {
		for (godot::Ref<godot::MultiMesh> &mesh : meshes) {
			mesh->set_instance_transform_2d(instance, transform);
		}
	}

	inline void set_visible_instance_count(int32_t count) {
		for (godot::Ref<godot::MultiMesh> &mesh : meshes) {
			mesh->set_visible_instance_count(count);
		}
	}

	inline void add_multimesh(const godot::RID &canvas_item) const {
		for (const godot::Ref<godot::MultiMesh> &mesh : meshes) {
			godot::RenderingServer::get_singleton()->canvas_item_add_multimesh(canvas_item, mesh->get_rid());
		}
	}

	inline void add_multimesh(const godot::RID &canvas_item, const godot::Texture &texture) const {
		for (const godot::Ref<godot::MultiMesh> &mesh : meshes) {
			godot::RenderingServer::get_singleton()->canvas_item_add_multimesh(canvas_item, mesh->get_rid(), texture.get_rid());
		}
	}

private:
	Meshes meshes;
};

#endif
