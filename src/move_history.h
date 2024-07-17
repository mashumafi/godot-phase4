#ifndef MOVE_HISTORY_H
#define MOVE_HISTORY_H

#include <phase4/engine/board/position.h>

#include <phase4/engine/moves/result.h>

#include <phase4/engine/common/fast_vector.h>

class MoveHistory {
private:
	struct Maps {
		std::array<size_t, 64> square_id; // Which piece id is on a square
		std::array<phase4::engine::common::Square, 64> id_square; // Which square owns this piece id

		Maps() {
			square_id.fill(-1);
			id_square.fill(phase4::engine::common::Square::INVALID);
		}
	};
	struct Detail {
		phase4::engine::board::Position position;
		phase4::engine::moves::Move move;

		Maps maps;

		Detail update_maps(const phase4::engine::moves::Result &result) {
			using namespace phase4::engine::common;

			if (result.slide) {
				for (size_t i = 0; i < result.moved.size(); ++i) {
				}
				if ((position.occupancySummary() & Square(*result.slide)) != 0) {
				}

				Bitboard walls = position.walls();
				while (walls > 0) {
					const Square wall(walls);
					walls = walls.popLsb();
				}
			} else {
				for (size_t i = 0; i < result.moved.size(); ++i) {
					maps.id_square[maps.square_id[result.moved[i].from]] = result.moved[i].to;
					const size_t to_id = maps.square_id[result.moved[i].to];
					if (to_id != -1) {
						maps.id_square[to_id] = Square::INVALID;
					}
					maps.square_id[result.moved[i].from] = -1;
				}
			}
		}
	};

public:
	struct Offsets {
		std::array<phase4::engine::common::Square, 64> pieces;
		std::array<phase4::engine::common::Square, 64> walls;
	};

	void reset(const phase4::engine::board::Position &position) {
		using namespace phase4::engine::common;

		index = 0;

		begin_position = position;
		begin_maps.square_id.fill(-1);
		begin_maps.id_square.fill(phase4::engine::common::Square::INVALID);
		details.clear();

		Bitboard occupancySummary = position.occupancySummary();
		size_t id = 0;
		while (occupancySummary != 0) {
			const Square square(occupancySummary);
			occupancySummary = occupancySummary.popLsb();

			begin_maps.square_id[square] = id;
			begin_maps.id_square[id] = square;
			++id;
		}
	}

	void add_detail(const phase4::engine::board::Position &position, const phase4::engine::moves::Move move, const phase4::engine::moves::Result &result) {
		if (details.is_empty()) {
			details.push_back(Detail{ position, move, begin_maps }.update_maps(result));
		} else {
			details.push_back(Detail{ position, move, details.peek().maps }.update_maps(result));
		}
		index = details.size();
	}

	Offsets seek(int64_t position) {
		Offsets offsets;
		return offsets;
	}

	Offsets undo() {
		Offsets offsets;
		index = details.size();
		return offsets;
	}

	const phase4::engine::board::Position &position() const {
		return index == 0 ? begin_position : details[index - 1].position;
	}

private:
	size_t index;

	phase4::engine::board::Position begin_position;
	Maps begin_maps;

	phase4::engine::common::FastVector<Detail> details;
};

#endif
