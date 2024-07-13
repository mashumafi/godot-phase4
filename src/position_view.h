#ifndef PHASE4_ENGINE_BOARD_POSITION_VIEW_H
#define PHASE4_ENGINE_BOARD_POSITION_VIEW_H

#include <phase4/engine/board/position_moves.h>
#include <phase4/engine/board/position_state.h>
#include <phase4/engine/board/session.h>

#include <phase4/engine/common/math.h>

namespace phase4::engine::board {

constexpr wchar_t unicode_pieces[13] = L"♙♘♗♖♕♔♟♞♝♜♛♚";

using AlgebraicNotation = std::array<wchar_t, 13>;
using SquareOffset = std::array<phase4::engine::common::Square, 64>;

struct PieceAndSquareOffset {
	SquareOffset pieces;
	SquareOffset squares;

	PieceAndSquareOffset() {
		reset();
	}

	void reset() {
		reset(pieces);
		reset(squares);
	}

	static void reset(SquareOffset &offsets) {
		for (size_t i = 0; i != common::Square::INVALID.get_raw_value(); ++i) {
			offsets[i] = common::Square(i); // No offset, same square
		}
	}
};

struct AlgebraicPieceAndSquareOffset : public PieceAndSquareOffset {
	AlgebraicNotation move;

    AlgebraicPieceAndSquareOffset() : PieceAndSquareOffset() {
        wcpcpy(move.data(), L"");
    }
};

struct Maps {
	std::array<size_t, 64> square_id; // Which piece id is on a square
	std::array<phase4::engine::common::Square, 64> id_square; // Which square owns this piece id

	Maps() {
		square_id.fill(-1);
		id_square.fill(phase4::engine::common::Square::INVALID);
	}
};
struct Detail {
    Position position;
    moves::Move move;

	Maps maps;

	void update_maps(const moves::Result &result) {
		using namespace common;

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

class PositionView {
public:
	PositionView() {
		reset(PositionState::DEFAULT);
	}

	void reset(const Position &position) {
		m_session.setPosition(position);

        Detail firstDetail;
        firstDetail.position = position;
        firstDetail.move = moves::Move::EMPTY;

		common::Bitboard occupancySummary = position.occupancySummary();
		size_t id = 0;
		while (occupancySummary != 0) {
			const common::Square square(occupancySummary);
			occupancySummary = occupancySummary.popLsb();

			firstDetail.maps.square_id[square] = id;
			firstDetail.maps.id_square[id] = square;
			++id;
		}

		m_details.clear();
        m_details.push_back(firstDetail);

		m_current = 0;
		computeValidMoves();
	}

	const Position &current() const {
		return m_session.position();
	}

	AlgebraicPieceAndSquareOffset makeMove(common::Square from, common::Square to) {
		using namespace moves;
		using namespace common;

		AlgebraicPieceAndSquareOffset result;

		const Move move(from, to, MoveFlags::QUIET);
		const std::optional<Move> &realMove = PositionMoves::findRealMove(m_validMoves, move);
		if (!realMove) {
			return result;
		}

		const Result &moveResult = m_session.makeMove(*realMove);
		if (moveResult.slide && moveResult.slide != FieldIndex::ZERO) {
			Bitboard walls = m_session.position().walls();
			const FieldIndex fixedSlide(moveResult.slide->x, -moveResult.slide->y);
			while (walls > 0) {
				const Square wall(Square(walls).asFieldIndex() + fixedSlide);
				result.squares[wall] = Square(walls);
				walls = walls.popLsb();
			}

			walls = m_session.position().walls();
			for (size_t i = 0; i < moveResult.moved.size(); ++i) {
				if ((walls & moveResult.moved[i].to.asBitboard()) == 0) {
					result.pieces[moveResult.moved[i].to] = moveResult.moved[i].from;
				} else {
					const Square slide_to(Square(moveResult.moved[i].to).asFieldIndex() + fixedSlide);
					result.pieces[slide_to] = moveResult.moved[i].from;
				}
			}
		} else {
			for (size_t i = 0; i < moveResult.moved.size(); ++i) {
				result.pieces[moveResult.moved[i].to] = moveResult.moved[i].from;
			}
		}


        Detail detail;
        detail.move = *realMove;
		m_details.push_back(detail);

		computeValidMoves();

        if (current().colorToMove() == common::PieceColor::BLACK) {
            // White just moved
            swprintf(result.move.data(), result.move.size(), L"%d. TODO", (current().movesCount() / 2) % 1000);
        } else {
            swprintf(result.move.data(), result.move.size(), L"TODO");
        }

		return result;
	}

	PieceAndSquareOffset undo() {
		PieceAndSquareOffset result;
		if (m_details.size() > 1) {
			return result;
		}

		m_session.undoMove(m_details.peek().move);
		computeValidMoves();
		m_details.pop_back();

		return result;
	}

	PieceAndSquareOffset prev() {
		return seek(m_current - 1);
	}

	PieceAndSquareOffset next() {
		return seek(m_current + 1);
	}

	PieceAndSquareOffset seek(size_t index) {
        PieceAndSquareOffset result;
		if (index > m_details.size()) {
			return result;
		}

		// TODO: Is Off By 1?

		const Maps &from = m_details[m_current].maps;
		const Maps &to = m_details[index].maps;

		for (common::Square square = common::Square::BEGIN; square != common::Square::INVALID; ++square) {
			const size_t id = from.square_id[square];
			if (id < to.id_square.size()) {
				result.pieces[square] = to.id_square[id];
			}
		}
		return result;
	}

	const moves::Moves &valid_moves() const {
		return m_validMoves;
	}

	const common::FastVector<moves::Move, 21> &valid_moves(common::Square square) const {
		return m_validMovesMap[square];
	}

private:
	void computeValidMoves() {
		m_validMoves.clear();
		for (size_t i = 0; i < m_validMovesMap.size(); ++i) {
			m_validMovesMap[i].clear();
		}

		phase4::engine::board::PositionMoves::getValidMoves(m_session.position(), m_validMoves);
		for (size_t i = 0; i < m_validMoves.size(); ++i) {
			m_validMovesMap[m_validMoves[i].from()].push_back(m_validMoves[i]);
		}
	}

	Session m_session;
	moves::Moves m_validMoves;
	std::array<common::FastVector<moves::Move, 21>, 64> m_validMovesMap;

	size_t m_current;
	phase4::engine::common::FastVector<Detail> m_details;
};

} //namespace phase4::engine::board

#endif