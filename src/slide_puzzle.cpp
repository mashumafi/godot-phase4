#include "slide_puzzle.h"

#include <godot_cpp/core/class_db.hpp>
#include <godot_cpp/templates/hash_set.hpp>
#include <godot_cpp/templates/local_vector.hpp>
#include <godot_cpp/templates/sort_array.hpp>

using namespace godot;

namespace {

using TileState = uint64_t;

uint8_t get_nibble(uint64_t p_state, uint8_t index) {
	return (p_state >> (4 * index)) & 0xF;
}

uint64_t on_nibble(uint8_t index) {
	return 0xFLL << (4 * index);
}

uint64_t set_nibble(uint64_t p_state, uint8_t index, uint64_t value) {
	return (p_state & ~on_nibble(index)) | (value << (4 * index));
}

uint8_t find_nibble(uint64_t p_state, uint8_t value) {
	int index = 0;
	while (p_state > 0) {
		uint8_t result = p_state & 0xF;
		if (result == value) {
			return index;
		}
		p_state >>= 4;
		++index;
	}

	ERR_FAIL_COND_V(index >= 16, -1);
	return index;
}

uint64_t swap_nibbles(uint64_t p_state, uint8_t x, uint8_t y) {
	const uint8_t left = get_nibble(p_state, x);
	const uint8_t right = get_nibble(p_state, y);
	p_state = set_nibble(p_state, x, right);
	return set_nibble(p_state, y, left);
}

struct Neighbor {
	TileState state;
	int empty_tile_index;
	Vector2 move;
};

struct TileNode {
	TileState state;
	int empty_tile_index;
	int g;
	int h;
	Vector2 move;
	TileNode *prev;
	TileNode *next;
};

struct SortTiles {
	_FORCE_INLINE_ bool operator()(const TileNode *A, const TileNode *B) const {
		return A->g + A->h > B->g + B->h;
	}
};

struct SortTilesGraph {
	_FORCE_INLINE_ bool operator()(const TileNode *A, const TileNode *B) const {
		return A->g < B->g;
	}
};

template <typename T, typename Comparator>
class PriorityQueue {
public:
	_FORCE_INLINE_ void insert(const T &value) {
		open.push_back(value);
		sorted.push_heap(0, open.size() - 1, 0, value, open.ptr());
	}

	_FORCE_INLINE_ T pop() {
		T current = open[0];
		sorted.pop_heap(0, open.size(), open.ptr());
		open.remove_at(open.size() - 1);
		return current;
	}

	_FORCE_INLINE_ bool is_empty() const {
		return open.is_empty();
	}

private:
	LocalVector<T> open;
	SortArray<T, Comparator> sorted;
};

template <typename Comparator>
class TileNodes {
public:
	TileNodes() :
			nodes(nullptr) {
	}

	~TileNodes() {
		while (nodes) {
			TileNode *next = nodes->next;
			memfree(nodes);
			nodes = next;
		}
	}

	_FORCE_INLINE_ void alloc(TileState state, int empty_tile_index, int g, int h, const Vector2 &move, TileNode *prev) {
		TileNode *node = memnew(TileNode);
		node->state = state;
		node->empty_tile_index = empty_tile_index;
		node->g = g;
		node->h = h;
		node->move = move;
		node->prev = prev;

		queue.insert(node);

		node->next = nodes;
		nodes = node;
	}

	_FORCE_INLINE_ TileNode *next() {
		if (queue.is_empty()) {
			return nullptr;
		}

		return queue.pop();
	}

private:
	PriorityQueue<TileNode *, Comparator> queue;

	TileNode *nodes;
};

TileState unpack(const PackedInt32Array &p_state) {
	TileState state = 0;
	const int size = p_state.size();
	for (int64_t i = 0; i < size; ++i) {
		state = set_nibble(state, i, p_state[i]);
	}
	return state;
}

TileState create_goal(int p_size) {
	TileState goal = 0;
	for (int i = 0; i < p_size; ++i) {
		goal = set_nibble(goal, i, i);
	}
	return goal;
}

class SlideUtil {
public:
	SlideUtil(int p_complexity) :
			complexity(p_complexity), total_complexity(complexity * complexity), empty_tile(total_complexity - 1) {
	}

	_FORCE_INLINE_ int manhattan_distance(TileState p_state) const {
		int distance = 0;
		for (size_t i = 0; i < total_complexity; ++i) {
			int tile = get_nibble(p_state, i);
			if (tile == empty_tile)
				continue;
			int goal_x = tile % complexity;
			int goal_y = tile / complexity;
			int cur_x = i % complexity;
			int cur_y = i / complexity;
			distance += Math::abs(goal_x - cur_x) + Math::abs(goal_y - cur_y);
		}
		return distance;
	}

	_FORCE_INLINE_ int linear_conflict(TileState p_state) const {
		int conflict = 0;

		// Check rows
		for (int row = 0; row < complexity; ++row) {
			for (int i = 0; i < complexity; ++i) {
				int idx_i = row * complexity + i;
				int tile_i = get_nibble(p_state, idx_i);
				if (tile_i == empty_tile || tile_i / complexity != row) {
					continue;
				}

				for (int j = i + 1; j < complexity; ++j) {
					int idx_j = row * complexity + j;
					int tile_j = get_nibble(p_state, idx_j);
					if (tile_j == empty_tile || tile_j / complexity != row) {
						continue;
					}

					if (tile_i > tile_j) {
						++conflict;
					}
				}
			}
		}

		// Check columns
		for (int col = 0; col < complexity; ++col) {
			for (int i = 0; i < complexity; ++i) {
				int idx_i = i * complexity + col;
				int tile_i = get_nibble(p_state, idx_i);
				if (tile_i == empty_tile || tile_i % complexity != col) {
					continue;
				}

				for (int j = i + 1; j < complexity; ++j) {
					int idx_j = j * complexity + col;
					int tile_j = get_nibble(p_state, idx_j);
					if (tile_j == empty_tile || tile_j % complexity != col) {
						continue;
					}

					if (tile_i > tile_j) {
						++conflict;
					}
				}
			}
		}

		return conflict * 2;
	}

	_FORCE_INLINE_ int heuristic(TileState p_state) const {
		return manhattan_distance(p_state) + linear_conflict(p_state);
	}

	_FORCE_INLINE_ int get_neighbors(TileState p_state, int empty_tile_index, Neighbor p_neighbors[4]) const {
		int count = 0;

		const int y = empty_tile_index / complexity;
		const int x_offsets[2] = { -1, 1 };
		const Vector2 x_moves[2] = { Vector2(-1, 0), Vector2(1, 0) };

		for (int i = 0; i < 2; ++i) {
			const int target = empty_tile_index + x_offsets[i];
			if (0 <= target && target < total_complexity) {
				const int ty = target / complexity;

				if (ty == y) {
					p_neighbors[count++] = { swap_nibbles(p_state, empty_tile_index, target), target, x_moves[i] };
				}
			}
		}

		const int y_offsets[2] = { -complexity, complexity };
		const Vector2 y_moves[2] = { Vector2(0, -1), Vector2(0, 1) };

		for (int i = 0; i < 2; ++i) {
			const int target = empty_tile_index + y_offsets[i];

			if (0 <= target && target < total_complexity) {
				p_neighbors[count++] = { swap_nibbles(p_state, empty_tile_index, target), target, y_moves[i] };
			}
		}

		return count;
	}

protected:
	int complexity;
	int total_complexity;
	int empty_tile;
};

PackedVector2Array get_moves(TileNode *current) {
	PackedVector2Array moves;
	int size = current->g;
	moves.resize(size);
	Vector2 *moves_ptrw = moves.ptrw();
	while (current != nullptr) {
		if (current->move != Vector2(0, 0)) {
			moves_ptrw[--size] = current->move;
		}
		current = current->prev;
	}
	ERR_FAIL_COND_V(size != 0, {});
	return moves;
}

class Solver : public SlideUtil {
public:
	Solver(int p_complexity, const PackedInt32Array &p_state) :
			SlideUtil(p_complexity),
			state(unpack(p_state)),
			goal(create_goal(total_complexity)) {
		nodes.alloc(state, find_nibble(state, empty_tile), 0, heuristic(state), Vector2(), nullptr);
	}

	PackedVector2Array solve() {
		for (TileNode *current = nodes.next(); current != nullptr; current = nodes.next()) {
			if (visited.has(current->state)) {
				continue;
			}

			visited.insert(current->state);

			if (current->state == goal) {
				return get_moves(current);
			}

			Neighbor neighbors[4];
			int n = get_neighbors(current->state, current->empty_tile_index, neighbors);

			for (int i = 0; i < n; ++i) {
				const Neighbor &neighbor = neighbors[i];
				if (!visited.has(neighbor.state)) {
					nodes.alloc(neighbor.state, neighbor.empty_tile_index, current->g + 1, heuristic(neighbor.state), neighbor.move, current);
				}
			}
		}

		return {};
	}

private:
	TileState state;
	TileState goal;

	TileNodes<SortTiles> nodes;
	HashSet<uint64_t> visited;
};

class Shuffler : public SlideUtil {
public:
	Shuffler(int p_complexity, Array &p_tiles, int p_goal, const Ref<RandomNumberGenerator> &p_rng) :
			SlideUtil(p_complexity),
			tiles(p_tiles),
			goal(p_goal),
			rng(p_rng),
			state(create_goal(total_complexity)) { // Assume the array is already sorted
		nodes.alloc(state, empty_tile, 0, 0, Vector2(), nullptr);
	}

	PackedVector2Array shuffle() {
		for (TileNode *current = nodes.next(); current != nullptr; current = nodes.next()) {
			if (visited.has(current->state)) {
				continue;
			}

			visited.insert(current->state);

			if (current->g == goal) {
				return get_moves(current);
			}

			Neighbor neighbors[4];
			int n = get_neighbors(current->state, current->empty_tile_index, neighbors);

			for (int i = 0; i < n; ++i) {
				int index = rng->randi_range(i, n - 1);
				const Neighbor &neighbor = neighbors[index];
				if (!visited.has(neighbor.state)) {
					nodes.alloc(neighbor.state, neighbor.empty_tile_index, current->g + 1, 0, neighbor.move, current);
				}
				neighbors[index] = neighbors[i];
			}
		}

		return {};
	}

private:
	Array tiles;
	int goal;
	Ref<RandomNumberGenerator> rng;

	TileState state;
	TileNodes<SortTilesGraph> nodes;
	HashSet<uint64_t> visited;
};

} //namespace

void SlidePuzzle::_bind_methods() {
	StringName class_name = "SlidePuzzle";
	ClassDB::bind_static_method(class_name, D_METHOD("shuffle", "complexity", "squares", "moves", "rng"), &SlidePuzzle::shuffle);
	ClassDB::bind_static_method(class_name, D_METHOD("is_solvable", "complexity", "squares"), &SlidePuzzle::is_solvable);
	ClassDB::bind_static_method(class_name, D_METHOD("solve", "complexity", "squares"), &SlidePuzzle::solve);
}

PackedVector2Array SlidePuzzle::shuffle(int p_complexity, Array p_state, int p_moves, const Ref<RandomNumberGenerator> &p_rng) {
	int total_complexity = p_complexity * p_complexity;
	ERR_FAIL_COND_V(total_complexity != p_state.size(), {});

	const int empty_tile_value = total_complexity - 1;
	int empty_tile_index = empty_tile_value;
	const Variant empty_tile = p_state[empty_tile_index];

	Shuffler shuffler(p_complexity, p_state, p_moves, p_rng);
	PackedVector2Array moves = shuffler.shuffle();
	Vector2 *moves_ptrw = moves.ptrw();
	const int size = moves.size();
	for (int i = 0; i < size; ++i) {
		int offset = moves_ptrw[i].x + moves_ptrw[i].y * p_complexity;
		int target = empty_tile_index + offset;
		p_state[empty_tile_index] = p_state[target];
		empty_tile_index = target;
		moves_ptrw[i] = -moves_ptrw[i];
	}
	p_state[empty_tile_index] = empty_tile;
	moves.reverse();
	return moves;
}

bool SlidePuzzle::is_solvable(int p_complexity, const PackedInt32Array &p_state) {
	ERR_FAIL_COND_V(p_complexity * p_complexity != p_state.size(), false);

	const int total_complexity = p_complexity * p_complexity;
	const int empty_tile = total_complexity - 1;

	int inversions = 0;

	for (int i = 0; i < total_complexity; ++i) {
		if (p_state[i] == empty_tile)
			continue;

		for (int j = i + 1; j < total_complexity; ++j) {
			if (p_state[j] != empty_tile && p_state[i] > p_state[j]) {
				++inversions;
			}
		}
	}

	if (p_complexity % 2 == 1) {
		// odd grid
		return inversions % 2 == 0;
	} else {
		// even grid
		int empty_tile_index = p_state.find(empty_tile);
		int empty_row_from_bottom = p_complexity - (empty_tile_index / p_complexity);
		if (empty_row_from_bottom % 2 == 0) {
			return inversions % 2 == 1;
		} else {
			return inversions % 2 == 0;
		}
	}

	return false;
}

PackedVector2Array SlidePuzzle::solve(int p_complexity, const PackedInt32Array &p_state) {
	ERR_FAIL_COND_V(p_complexity * p_complexity != p_state.size(), PackedVector2Array());
	ERR_FAIL_COND_V(!is_solvable(p_complexity, p_state), PackedVector2Array());

	Solver solver(p_complexity, p_state);
	return solver.solve();
}
