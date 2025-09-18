extends Node

@export var board : Chess2D
@export var overlay : Control
@export var flourish_label : Label

const fen = "rnbqkbnr/pppppppp/8/8/6PP/6NR/PPPPPP**/RNBQKB** w Qkq - 0 1"
var solution := PackedVector2Array()

var off_screen_factor := 16

@onready var HORIZONTAL_CRISS_CROSS_PATTERN := PackedVector2Array([
	Vector2(board.theme.square_size * off_screen_factor, 0), Vector2(board.theme.square_size * off_screen_factor, 0), Vector2(board.theme.square_size * off_screen_factor, 0), Vector2(board.theme.square_size * off_screen_factor, 0),
	Vector2(board.theme.square_size * -off_screen_factor, 0), Vector2(board.theme.square_size * -off_screen_factor, 0), Vector2(board.theme.square_size * -off_screen_factor, 0), Vector2(board.theme.square_size * -off_screen_factor, 0),
	Vector2(board.theme.square_size * off_screen_factor, 0), Vector2(board.theme.square_size * off_screen_factor, 0), Vector2(board.theme.square_size * off_screen_factor, 0), Vector2(board.theme.square_size * off_screen_factor, 0),
	Vector2(board.theme.square_size * -off_screen_factor, 0), Vector2(board.theme.square_size * -off_screen_factor, 0), Vector2(board.theme.square_size * -off_screen_factor, 0), Vector2(board.theme.square_size * -off_screen_factor, 0),
])

func _ready() -> void:
	board.fen = fen
	_shuffle()

	board.set_target_offsets(HORIZONTAL_CRISS_CROSS_PATTERN)
	board.clear_animation_offsets()
	board.set_target_offsets(ChessSession.ZERO_PATTERN)

func set_flourish_text(text: String):
	overlay.modulate = Color.WHITE
	flourish_label.text = text
	var tween := create_tween()
	tween.tween_property(overlay, "modulate", Color.TRANSPARENT, 1.0).set_delay(1.2).set_trans(Tween.TRANS_CUBIC).set_ease(Tween.EASE_OUT)

func _shuffle():
	var rng := RandomNumberGenerator.new()
	var squares := [
		 0,  1,  2,  3,
		 4,  5,  6,  7,
		 8,  9, 10, 11,
		12, 13, 14, 15,
	]
	solution = SlidePuzzle.shuffle(4, squares, 100, rng)
	solution = SlidePuzzle.solve(4, squares)
	var shuffle = solution.duplicate()
	shuffle.reverse()
	for move in shuffle:
		board.slide_squares(move * 2)
	solution.push_back(Vector2(0, -1))
	solution.push_back(Vector2(-1, 0))

func solve():
	while not solution.is_empty():
		Engine.time_scale = min(Engine.time_scale * 1.4, 4.75)
		board.slide_squares(solution[0] * -2)
		solution.remove_at(0)
		await get_tree().create_timer(.5).timeout

	Engine.time_scale = 1.0
