#include <godot_cpp/classes/object.hpp>
#include <godot_cpp/classes/random_number_generator.hpp>
#include <godot_cpp/classes/ref_counted.hpp>

#include <godot_cpp/variant/array.hpp>

namespace godot {

class SlidePuzzle : public Object {
	GDCLASS(SlidePuzzle, Object)

protected:
	static void _bind_methods();

public:
	static PackedVector2Array shuffle(uint8_t p_complexity, Array p_squares, int64_t p_moves, const Ref<RandomNumberGenerator> &p_rng);
	static bool is_solvable(uint8_t p_complexity, const PackedInt32Array &p_squares);
	static PackedVector2Array solve(uint8_t p_complexity, const PackedInt32Array &p_squares, const PackedInt32Array &p_goal = {});
};

} //namespace godot