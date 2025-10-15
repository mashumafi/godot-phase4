#include "slide_puzzle.h"

#include <godot_cpp/core/class_db.hpp>
#include <godot_cpp/templates/hash_set.hpp>
#include <godot_cpp/templates/local_vector.hpp>
#include <godot_cpp/templates/sort_array.hpp>

using namespace godot;

namespace {

enum class Direction {
	NONE,
	NORTH,
	EAST,
	SOUTH,
	WEST
};

class TileStateCompact {
public:
	inline uint8_t get(uint8_t index) const {
		return (state >> (4 * index)) & 0xF;
	}

	inline static uint64_t on_nibble(uint8_t index) {
		return 0xFLL << (4 * index);
	}

	inline void set(uint8_t index, uint64_t value) {
		state = (state & ~on_nibble(index)) | (value << (4 * index));
	}

	inline uint8_t find(uint8_t value) const {
		int index = 0;
		size_t itr = state;
		while (itr > 0) {
			uint8_t result = itr & 0xF;
			if (result == value) {
				return index;
			}
			itr >>= 4;
			++index;
		}

		ERR_FAIL_V(-1);
	}

	inline void swap(uint8_t x, uint8_t y) {
		const uint8_t right = get(y);
		set(y, get(x));
		set(x, right);
	}

	static constexpr int64_t size() {
		return 16;
	}

	inline const bool operator==(const TileStateCompact &other) const {
		return state == other.state;
	}

	static inline uint32_t hash(const TileStateCompact &p_state) { return HashMapHasherDefault::hash(p_state.state); }

private:
	uint64_t state;
};

class TileStateArray {
public:
	inline uint8_t get(uint8_t index) const {
		return state_i[index];
	}

	inline void set(uint8_t index, uint64_t value) {
		state_i[index] = value;
	}

	inline uint8_t find(uint8_t value) const {
		for (size_t i = 0; i < 16; ++i) {
			if (state_i[i] == value) {
				return i;
			}
		}

		ERR_FAIL_V(-1);
	}

	inline void swap(uint16_t x, uint16_t y) {
		std::swap(state_i[x], state_i[y]);
	}

	static constexpr int64_t size() {
		return 16;
	}

	inline const bool operator==(const TileStateArray &other) const {
		return state_l[0] == other.state_l[0] && state_l[1] == other.state_l[1];
	}

	static inline uint32_t hash(const TileStateArray &p_state) {
		uint32_t h = hash_murmur3_one_32(uint32_t(p_state.state_l[0]));
		h = hash_murmur3_one_32(uint32_t(p_state.state_l[1]), h);
		return hash_fmix32(h);
	}

private:
	union {
		uint8_t state_i[16];
		uint64_t state_l[2];
	};
};

template <typename TileState>
struct Neighbor {
	TileState state;
	uint8_t empty_tile_index;
	Direction move;
};

template <typename TileState>
struct TileNode {
	TileState state;
	uint8_t empty_tile_index;
	int32_t graph;
	int32_t heuristic;
	Direction direction;
	TileNode *prev;
};

template <typename TileState>
struct SortTiles {
	inline bool operator()(const TileNode<TileState> *A, const TileNode<TileState> *B) const {
		return A->graph + A->heuristic > B->graph + B->heuristic;
	}
};

template <typename TileState>
struct SortTilesGraph {
	inline bool operator()(const TileNode<TileState> *A, const TileNode<TileState> *B) const {
		return A->graph < B->graph;
	}
};

template <typename T, typename Comparator>
class PriorityQueue {
public:
	PriorityQueue() {
		open.reserve(256);
	}

	inline void insert(const T &value) {
		open.push_back(value);
		sorted.push_heap(0, open.size() - 1, 0, value, open.ptr());
	}

	inline T pop() {
		T current = open[0];
		sorted.pop_heap(0, open.size(), open.ptr());
		open.remove_at(open.size() - 1);
		return current;
	}

	inline bool is_empty() const {
		return open.is_empty();
	}

private:
	LocalVector<T> open;
	SortArray<T, Comparator> sorted;
};

template <typename T>
struct Arena {
	Arena *next;
	T memory[];
};

template <typename T>
class ArenaAllocator {
public:
	ArenaAllocator() :
			size(0), capacity(256), arenas(create_arena(capacity)) {
	}

	template <typename... Args>
	inline T *alloc(Args &&...args) {
		if (unlikely(size == capacity)) {
			size = 0;
			capacity *= 2;
			Arena<T> *arena = create_arena(capacity);
			arena->next = arenas;
			arenas = arena;
		}

		T *mem = arenas->memory + size;
		size++;
		memnew_placement(mem, T(args...));
		return mem;
	}

	inline void free(T *mem) {
		mem->~T();
	}

	~ArenaAllocator() {
		while (arenas != nullptr) {
			Arena<T> *next = arenas->next;
			memfree(arenas);
			arenas = next;
		}
	}

private:
	static Arena<T> *create_arena(size_t capacity) {
		Arena<T> *arena = (Arena<T> *)DefaultAllocator::alloc(sizeof(Arena<T>) + capacity * sizeof(T));
		arena->next = nullptr;
		return arena;
	}

	size_t size;
	size_t capacity;
	Arena<T> *arenas;
};

template <typename TileState, typename Comparator>
class TileNodes {
public:
	inline void alloc(TileState state, uint8_t empty_tile_index, int32_t graph, int32_t heuristic, Direction direction, TileNode<TileState> *prev) {
		TileNode<TileState> *node = mem.alloc();
		node->state = state;
		node->empty_tile_index = empty_tile_index;
		node->graph = graph;
		node->heuristic = heuristic;
		node->direction = direction;
		node->prev = prev;

		queue.insert(node);
	}

	inline TileNode<TileState> *next() {
		if (queue.is_empty()) {
			return nullptr;
		}

		return queue.pop();
	}

private:
	ArenaAllocator<TileNode<TileState>> mem;
	PriorityQueue<TileNode<TileState> *, Comparator> queue;
};

template <typename TileState>
inline TileState unpack(const PackedInt32Array &p_state) {
	TileState state;
	const int size = p_state.size();
	for (int64_t i = 0; i < size; ++i) {
		state.set(i, p_state[i]);
	}
	for (int64_t i = size; i < TileState::size(); ++i) {
		state.set(i, -1);
	}
	return state;
}

bool is_valid_goal(Array &p_tiles) {
	for (int i = 0; i < p_tiles.size(); ++i) {
		if (p_tiles[i].get_type() != Variant::INT)
			return false;
	}
	return true;
}

uint8_t find_empty_tile(Array &p_tiles) {
	const uint8_t empty_tile_value = p_tiles.size() - 1;

	if (!is_valid_goal(p_tiles)) {
		return empty_tile_value;
	}

	for (uint8_t i = 0; i < p_tiles.size(); ++i) {
		if ((int)p_tiles[i] == empty_tile_value) {
			return i;
		}
	}

	ERR_FAIL_V_MSG(empty_tile_value, "Using last tile as empty tile.");
}

template <typename TileState>
inline TileState create_goal(int p_size) {
	TileState goal;
	for (int i = 0; i < p_size; ++i) {
		goal.set(i, i);
	}
	for (int64_t i = p_size; i < TileState::size(); ++i) {
		goal.set(i, -1);
	}
	return goal;
}

template <typename TileState>
inline TileState create_goal(uint8_t p_complexity, const PackedInt32Array &p_goal) {
	uint8_t total_complexity = p_complexity * p_complexity;
	if (total_complexity != p_goal.size()) {
		return create_goal<TileState>(total_complexity);
	}

	TileState goal;
	for (int64_t i = 0; i < p_goal.size(); ++i) {
		goal.set(i, p_goal[i]);
	}
	for (int64_t i = p_goal.size(); i < TileState::size(); ++i) {
		goal.set(i, -1);
	}

	return goal;
}

template <typename TileState>
inline TileState create_goal(Array &p_tiles) {
	if (!is_valid_goal(p_tiles)) {
		return create_goal<TileState>(p_tiles.size());
	}

	TileState goal;
	for (int i = 0; i < p_tiles.size(); ++i) {
		goal.set(i, p_tiles[i]);
	}
	for (int64_t i = p_tiles.size(); i < TileState::size(); ++i) {
		goal.set(i, -1);
	}
	return goal;
}

template <typename TileState>
class SlideUtil {
public:
	SlideUtil(uint8_t p_complexity) :
			complexity(p_complexity), total_complexity(complexity * complexity), empty_tile(total_complexity - 1) {
	}

	inline int32_t manhattan_distance(const TileState &p_state) const {
		int32_t distance = 0;
		for (size_t i = 0; i < total_complexity; ++i) {
			int32_t tile = p_state.get(i);
			if (tile == empty_tile)
				continue;
			int32_t goal_x = tile % complexity;
			int32_t goal_y = tile / complexity;
			int32_t cur_x = i % complexity;
			int32_t cur_y = i / complexity;
			distance += Math::abs(goal_x - cur_x) + Math::abs(goal_y - cur_y);
		}
		return distance;
	}

	inline int32_t linear_conflict(const TileState &p_state) const {
		int32_t conflict = 0;

		// Check rows
		for (int row = 0; row < complexity; ++row) {
			for (int i = 0; i < complexity; ++i) {
				int idx_i = row * complexity + i;
				int tile_i = p_state.get(idx_i);
				if (tile_i == empty_tile || tile_i / complexity != row) {
					continue;
				}

				for (int j = i + 1; j < complexity; ++j) {
					int idx_j = row * complexity + j;
					int tile_j = p_state.get(idx_j);
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
				int tile_i = p_state.get(idx_i);
				if (tile_i == empty_tile || tile_i % complexity != col) {
					continue;
				}

				for (int j = i + 1; j < complexity; ++j) {
					int idx_j = j * complexity + col;
					int tile_j = p_state.get(idx_j);
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

	inline int32_t heuristic(const TileState &p_state) const {
		return manhattan_distance(p_state) + linear_conflict(p_state);
	}

	inline size_t get_neighbors(const TileState &p_state, uint8_t empty_tile_index, Neighbor<TileState> p_neighbors[4]) const {
		size_t count = 0;

		const uint8_t y = empty_tile_index / complexity;
		const int8_t x_offsets[2] = { -1, 1 };
		const Direction x_moves[2] = { Direction::WEST, Direction::EAST };

		for (int i = 0; i < 2; ++i) {
			const uint8_t target = empty_tile_index + x_offsets[i];
			if (0 <= target && target < total_complexity) {
				const int ty = target / complexity;

				if (ty == y) {
					p_neighbors[count] = { p_state, target, x_moves[i] };
					p_neighbors[count].state.swap(empty_tile_index, target);
					count++;
				}
			}
		}

		const int8_t y_offsets[2] = { static_cast<int8_t>(-complexity), static_cast<int8_t>(complexity) };
		const Direction y_moves[2] = { Direction::NORTH, Direction::SOUTH };

		for (int i = 0; i < 2; ++i) {
			const uint8_t target = empty_tile_index + y_offsets[i];

			if (0 <= target && target < total_complexity) {
				p_neighbors[count] = { p_state, target, y_moves[i] };
				p_neighbors[count].state.swap(empty_tile_index, target);
				count++;
			}
		}

		return count;
	}

protected:
	const uint8_t complexity;
	const uint8_t total_complexity;
	uint8_t empty_tile;
};

inline Vector2 dir_to_move(Direction direction) {
	switch (direction) {
		case Direction::NORTH:
			return Vector2(0, -1);
		case Direction::EAST:
			return Vector2(1, 0);
		case Direction::SOUTH:
			return Vector2(0, 1);
		case Direction::WEST:
			return Vector2(-1, 0);
		case Direction::NONE:
			return Vector2();
	}
}

template <typename TileState>
inline PackedVector2Array get_moves(TileState *current) {
	PackedVector2Array moves;
	int32_t size = current->graph;
	moves.resize(size);
	Vector2 *moves_ptrw = moves.ptrw();
	while (current != nullptr) {
		if (current->direction != Direction::NONE) {
			moves_ptrw[--size] = dir_to_move(current->direction);
		}
		current = current->prev;
	}
	ERR_FAIL_COND_V(size != 0, {});
	return moves;
}

template <typename TileState>
class Solver : public SlideUtil<TileState> {
public:
	Solver(uint8_t p_complexity, const PackedInt32Array &p_state, const PackedInt32Array &p_goal) :
			SlideUtil<TileState>(p_complexity),
			state(unpack<TileState>(p_state)),
			goal(create_goal<TileState>(p_complexity, p_goal)) {
		visited.reserve(256);

		nodes.alloc(state, state.find(SlideUtil<TileState>::empty_tile), 0, SlideUtil<TileState>::heuristic(state), Direction::NONE, nullptr);
	}

	inline PackedVector2Array solve() {
		for (TileNode<TileState> *current = nodes.next(); current != nullptr; current = nodes.next()) {
			if (visited.has(current->state)) {
				continue;
			}

			visited.insert(current->state);

			if (current->state == goal) {
				return get_moves(current);
			}

			Neighbor<TileState> neighbors[4];
			size_t n = SlideUtil<TileState>::get_neighbors(current->state, current->empty_tile_index, neighbors);

			for (size_t i = 0; i < n; ++i) {
				const Neighbor<TileState> &neighbor = neighbors[i];
				if (!visited.has(neighbor.state)) {
					nodes.alloc(neighbor.state, neighbor.empty_tile_index, current->graph + 1, SlideUtil<TileState>::heuristic(neighbor.state), neighbor.move, current);
				}
			}
		}

		return {};
	}

private:
	TileState state;
	TileState goal;

	TileNodes<TileState, SortTiles<TileState>> nodes;
	HashSet<TileState, TileState> visited;
};

template <typename TileState>
class Shuffler : public SlideUtil<TileState> {
public:
	Shuffler(uint8_t p_complexity, Array &p_tiles, int64_t p_moves, const Ref<RandomNumberGenerator> &p_rng) :
			SlideUtil<TileState>(p_complexity),
			tiles(p_tiles),
			moves(p_moves),
			rng(p_rng),
			state(create_goal<TileState>(p_tiles)) {
		visited.reserve(256);

		SlideUtil<TileState>::empty_tile = find_empty_tile(tiles);
		nodes.alloc(state, SlideUtil<TileState>::empty_tile, 0, 0, Direction::NONE, nullptr);
	}

	inline PackedVector2Array shuffle() {
		for (TileNode<TileState> *current = nodes.next(); current != nullptr; current = nodes.next()) {
			if (visited.has(current->state)) {
				continue;
			}

			visited.insert(current->state);

			if (current->graph == moves) {
				return get_moves(current);
			}

			Neighbor<TileState> neighbors[4];
			size_t n = SlideUtil<TileState>::get_neighbors(current->state, current->empty_tile_index, neighbors);

			for (size_t i = 0; i < n; ++i) {
				int index = rng->randi_range(i, n - 1);
				const Neighbor<TileState> &neighbor = neighbors[index];
				if (!visited.has(neighbor.state)) {
					nodes.alloc(neighbor.state, neighbor.empty_tile_index, current->graph + 1, 0, neighbor.move, current);
				}
				neighbors[index] = neighbors[i];
			}
		}

		return {};
	}

private:
	Array tiles;
	int64_t moves;
	Ref<RandomNumberGenerator> rng;

	TileState state;
	TileNodes<TileState, SortTilesGraph<TileState>> nodes;
	HashSet<TileState, TileState> visited;
};

using TileStateDefault = TileStateCompact;
static_assert(std::is_trivially_constructible_v<Neighbor<TileStateDefault>>);

} //namespace

void SlidePuzzle::_bind_methods() {
	StringName class_name = "SlidePuzzle";
	ClassDB::bind_static_method(class_name, D_METHOD("shuffle", "complexity", "squares", "moves", "rng"), &SlidePuzzle::shuffle);
	ClassDB::bind_static_method(class_name, D_METHOD("is_solvable", "complexity", "squares"), &SlidePuzzle::is_solvable);
	ClassDB::bind_static_method(class_name, D_METHOD("solve", "complexity", "squares", "goal"), &SlidePuzzle::solve, PackedInt32Array());
}

PackedVector2Array SlidePuzzle::shuffle(uint8_t p_complexity, Array p_squares, int64_t p_moves, const Ref<RandomNumberGenerator> &p_rng) {
	ERR_FAIL_COND_V(p_rng.is_null(), {});

	uint8_t total_complexity = p_complexity * p_complexity;
	ERR_FAIL_COND_V(total_complexity != p_squares.size(), {});

	uint8_t empty_tile_index = find_empty_tile(p_squares);
	const Variant empty_tile = p_squares[empty_tile_index];

	Shuffler<TileStateDefault> shuffler(p_complexity, p_squares, p_moves, p_rng);
	PackedVector2Array moves = shuffler.shuffle();
	Vector2 *moves_ptrw = moves.ptrw();
	const int64_t size = moves.size();
	for (int64_t i = 0; i < size; ++i) {
		int offset = moves_ptrw[i].x + moves_ptrw[i].y * p_complexity;
		int target = empty_tile_index + offset;
		p_squares[empty_tile_index] = p_squares[target];
		empty_tile_index = target;
		moves_ptrw[i] = -moves_ptrw[i];
	}
	p_squares[empty_tile_index] = empty_tile;
	moves.reverse();
	return moves;
}

bool SlidePuzzle::is_solvable(uint8_t p_complexity, const PackedInt32Array &p_squares) {
	ERR_FAIL_COND_V(p_complexity * p_complexity != p_squares.size(), false);

	const uint8_t total_complexity = p_complexity * p_complexity;
	const uint8_t empty_tile = total_complexity - 1;

	uint8_t inversions = 0;

	for (uint8_t i = 0; i < total_complexity; ++i) {
		if (p_squares[i] == empty_tile)
			continue;

		for (uint8_t j = i + 1; j < total_complexity; ++j) {
			if (p_squares[j] != empty_tile && p_squares[i] > p_squares[j]) {
				++inversions;
			}
		}
	}

	if (p_complexity % 2 == 1) {
		// odd grid
		return inversions % 2 == 0;
	} else {
		// even grid
		uint8_t empty_tile_index = p_squares.find(empty_tile);
		int empty_row_from_bottom = p_complexity - (empty_tile_index / p_complexity);
		if (empty_row_from_bottom % 2 == 0) {
			return inversions % 2 == 1;
		} else {
			return inversions % 2 == 0;
		}
	}

	return false;
}

PackedVector2Array SlidePuzzle::solve(uint8_t p_complexity, const PackedInt32Array &p_squares, const PackedInt32Array &p_goal) {
	ERR_FAIL_COND_V(p_complexity * p_complexity != p_squares.size(), PackedVector2Array());
	ERR_FAIL_COND_V(!is_solvable(p_complexity, p_squares), PackedVector2Array());

	Solver<TileStateDefault> solver(p_complexity, p_squares, p_goal);
	return solver.solve();
}
