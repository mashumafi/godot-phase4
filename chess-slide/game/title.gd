extends Node2D

enum PieceColor {
	WHITE,
	BLACK
}

enum Pattern {
	Stripes,
	Checkered,
}

const PackedSlidePuzzle2D := preload("res://game/slide_puzzle.tscn")
const COLOR_KEY := &"color"
var RNG := RandomNumberGenerator.new()


class PuzzleWrapper:
	extends Node2D

	var _puzzle : SlidePuzzle2D

	func _init(chess_theme: ChessTheme, color: PieceColor, rng: RandomNumberGenerator) -> void:
		_puzzle = PackedSlidePuzzle2D.instantiate() as SlidePuzzle2D
		_puzzle.complexity = 3
		_puzzle.texture = chess_theme.get_piece_texture(
			rng.randi_range(0, ChessTheme.PIECE_COLOR_MAX - 1),
			rng.randi_range(0, ChessTheme.PIECE_TYPE_MAX - 1)
		)
		add_child(_puzzle)
		set_meta(COLOR_KEY, color)


	func shuffle() -> void:
		_puzzle.shuffle(45)


	func solve() -> void:
		_puzzle.solve()


@export var chess_theme : ChessTheme

var black_puzzle_cache : Array[PuzzleWrapper] = []
var white_puzzle_cache : Array[PuzzleWrapper] = []


func _ready() -> void:
	var puzzle := _alloc_slide_puzzle(0)
	add_child(puzzle)
	puzzle.shuffle()
	puzzle.solve()


func _notification(what: int) -> void:
	match what:
		NOTIFICATION_PREDELETE:
			for puzzle in black_puzzle_cache:
				puzzle.free()
			for puzzle in white_puzzle_cache:
				puzzle.free()


func _alloc_slide_puzzle(color: PieceColor) -> PuzzleWrapper:
	assert(chess_theme)
	match color:
		PieceColor.WHITE:
			if not white_puzzle_cache.is_empty():
				return white_puzzle_cache.pop_back()
		PieceColor.BLACK:
			if not black_puzzle_cache.is_empty():
				return black_puzzle_cache.pop_back()

	return PuzzleWrapper.new(chess_theme, color, RNG)


func _free_slide_puzzle(puzzle: PuzzleWrapper) -> void:
	puzzle.get_parent().remove_child(puzzle)
	match puzzle.get_meta(COLOR_KEY):
		PieceColor.WHITE:
			return white_puzzle_cache.push_back(puzzle)
		PieceColor.BLACK:
			return black_puzzle_cache.push_back(puzzle)
