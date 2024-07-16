#ifndef PHASE4_ENGINE_BOARD_POSITION_VIEW_H
#define PHASE4_ENGINE_BOARD_POSITION_VIEW_H

#include <phase4/engine/board/position_moves.h>
#include <phase4/engine/board/position_state.h>
#include <phase4/engine/board/session.h>

#include <phase4/engine/common/math.h>
#include <phase4/engine/common/wall_operations.h>

#include <cstdint>
#include <cwchar>

namespace phase4::engine::board {

constexpr std::array<wchar_t, 12> unicode_pieces{
	L'♙',
	L'♘',
	L'♗',
	L'♖',
	L'♕',
	L'♔',
	L'♟',
	L'♞',
	L'♝',
	L'♜',
	L'♛',
	L'♚'
};

using AlgebraicNotation = std::array<wchar_t, 42>;
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

	AlgebraicPieceAndSquareOffset() :
			PieceAndSquareOffset() {
		swprintf(move.data(), move.size(), L"");
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

		for (size_t i = 0; i < result.moved.size(); ++i) {
			const size_t fromId = maps.square_id[result.moved[i].from];
			const size_t toId = maps.square_id[result.moved[i].to];

			// Update the square for the moved piece ID
			maps.id_square[fromId] = result.moved[i].to;
			maps.square_id[result.moved[i].to] = fromId;

			// Remove the captured piece ID
			if (toId != -1) {
				maps.id_square[toId] = Square::INVALID;
			}

			// Remove the moved piece ID
			maps.square_id[result.moved[i].from] = -1;
		}

		if (result.slide && result.slide != FieldIndex::ZERO) {
			Bitboard walls = position.walls();
			while (walls > 0) {
				const Square wall(walls);
				walls = walls.popLsb();

				const size_t fromId = maps.square_id[wall];
				if (fromId != -1) {
					// Update the square for the moved piece ID
					const Square wallOffset(wall.get_raw_value() - result.slide->offset());
					maps.id_square[fromId] = wallOffset;
					maps.square_id[wallOffset] = fromId;

					// Remove the moved piece ID
					maps.square_id[wall] = -1;
				}
			}
		}
	}
};

class PositionView {
public:
	PositionView() {
		reset(PositionState::DEFAULT);
	}

	size_t size() const {
		return m_details.size();
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
		return m_details[m_current].position;
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

		const PieceType pieceType = m_session.position().pieceTable(from);
		const std::array<char, 3> &toBuffer = to.asBuffer();

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
		detail.position = m_session.position();
		detail.move = *realMove;
		detail.maps = m_details.peek().maps;
		detail.update_maps(moveResult);
		m_details.push_back(detail);
		++m_current;

		computeValidMoves();

		if (current().colorToMove() == common::PieceColor::BLACK) {
			// White just moved
			const uint16_t moveNumber = (current().movesCount() / 2) + 1;
			const int rc = swprintf(result.move.data(), result.move.size(), L"%d. ", moveNumber);
			if (rc > 0) {
				result.move[rc] = unicode_pieces[pieceType.get_raw_value() + 6];
				const int offset = rc + 1;
				swprintf(result.move.data() + offset, result.move.size() - offset, L"%s", toBuffer.data());
			}
		} else {
			result.move[0] = unicode_pieces[pieceType.get_raw_value()];
			swprintf(result.move.data() + 1, result.move.size() - 1, L"%s", toBuffer.data());
		}

		return result;
	}

	PieceAndSquareOffset undo() {
		if (m_details.size() <= 1) {
			return PieceAndSquareOffset();
		}

		m_session.undoMove(m_details.peek().move);
		computeValidMoves();

		const PieceAndSquareOffset& result = calculateOffsets(m_details[m_details.size() - 1], m_details[m_details.size() - 2]);

		m_details.pop_back();
		m_current = m_details.size() - 1;

		return result;
	}

	PieceAndSquareOffset seek(size_t index) {
		using namespace common;

		if (index >= m_details.size()) {
			return PieceAndSquareOffset();
		}

		const Detail &fromDetail = m_details[m_current];
		m_current = index;
		return calculateOffsets(fromDetail, m_details[index]);
	}

	const moves::Moves &validMoves() const {
		return m_validMoves;
	}

	const common::FastVector<moves::Move, 21> &validMoves(common::Square square) const {
		return m_validMovesMap[square];
	}

private:
	PieceAndSquareOffset calculateOffsets(const Detail &fromDetail, const Detail &toDetail) {
		using namespace common;

		PieceAndSquareOffset result;

		for (Square square = Square::BEGIN; square != Square::INVALID; ++square) {
			if (fromDetail.maps.id_square[square] != Square::INVALID && toDetail.maps.id_square[square] != common::Square::INVALID) {
				result.pieces[toDetail.maps.id_square[square]] = fromDetail.maps.id_square[square];
			}
		}

		const Bitboard fromWall = fromDetail.position.walls();
		const Bitboard toWall = toDetail.position.walls();
		const FieldIndex slideDir = WallOperations::SLIDE_DIR[Square(fromWall)][Square(toWall)];
		if (slideDir != FieldIndex::ZERO) {
			Bitboard walls = toWall;
			while (walls > 0) {
				const Square wall(Square(walls).asFieldIndex() + FieldIndex(slideDir.x, -slideDir.y));
				result.squares[wall] = Square(walls);
				walls = walls.popLsb();
			}
		}

		return result;
	}

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