#ifndef PHASE4_ENGINE_BOARD_POSITION_VIEW_H
#define PHASE4_ENGINE_BOARD_POSITION_VIEW_H

#include <phase4/engine/board/position_moves.h>
#include <phase4/engine/board/session.h>

namespace phase4::engine::board {

using AlgebraicNotation = char[8];
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
	AlgebraicNotation move = "";
};

class PositionView {
public:
	PositionView() {
		computeValidMoves();
	}

	void reset(const Position &position) {
		m_session.setPosition(position);
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

		m_moveHistory.push_back(*realMove);

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

		computeValidMoves();

        strcpy(result.move, "TODO");

		return result;
	}

	PieceAndSquareOffset undo() {
		PieceAndSquareOffset result;
		if (m_moveHistory.is_empty()) {
			return result;
		}

		m_session.undoMove(m_moveHistory.peek());
		computeValidMoves();
		m_moveHistory.pop_back();

		return result;
	}

	PieceAndSquareOffset prev() {
		PieceAndSquareOffset result;
		return result;
	}

	PieceAndSquareOffset next() {
		PieceAndSquareOffset result;
		return result;
	}

	PieceAndSquareOffset seek(size_t index) {
		PieceAndSquareOffset result;
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

	common::FastVector<moves::Move, 1024> m_moveHistory;
};

} //namespace phase4::engine::board

#endif