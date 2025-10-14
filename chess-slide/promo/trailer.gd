extends Node

@export var board : Chess2D
@export var overlay : Control
@export var flourish_label : Label
@export var canvas_layer : CanvasLayer

const Title := preload("res://game/title.tscn")
var title := Title.instantiate()

var rng := RandomNumberGenerator.new()
var solution := PackedVector2Array()

func _ready() -> void:
	Engine.time_scale = 1.75
	board.fen = "rnbqkbnr/pppppppp/8/8/4**2/4**2/PPPPPPPP/RNBQKBNR w KQkq - 0 1"
	_shuffle()

	set_and_clear_target_offsets()
	await set_target_offsets(board.theme.make_zero_pattern())
	await set_flourish_text("Chess")
	await sleep(1)
	await solve()

	await set_flourish_text("Familiar Rules")
	Engine.time_scale = 1.0

	await sleep(1.0)
	board.make_move("g1h3")
	await board.animation_finished
	await sleep(.5)
	board.make_move("b8c6")
	await board.animation_finished
	await sleep(.5)
	board.make_move("d2d4")
	await board.animation_finished
	await sleep(.5)
	board.make_move("e7e5")
	await board.animation_finished
	await sleep(.5)
	board.make_move("e2e4")
	await board.animation_finished
	await sleep(1.5)

	Engine.time_scale = 2.6
	await set_target_offsets()

	# Castling
	# r1bq1r1k/pp1nbp1n/7Q/4P1N1/2B1**1P/2p1**2/PP4P1/R3K2R b KQ - 1 16 d7f6 (e1g1) d8d4 g1h1 d4e5 f1f6


	# Promotion

	# En Passant
	board.fen = "3rk1r1/p2n1p2/qpQP2p1/4P2p/2P1**1P/4**PB/P7/2R3K1 b - - 2 33"
	set_target_offsets(board.theme.make_zero_pattern())
	await set_flourish_text("En Passant")
	Engine.time_scale = 1.0
	await sleep(.5)
	board.make_move("f7f5")
	await sleep(.5)
	board.make_move("e3")
	await sleep(1.5)
	board.make_move("e3f4")
	Engine.time_scale = .7
	await sleep(.5)
	Engine.time_scale = 2.6
	await set_target_offsets()
	
	board.fen = "8/1p4kp/p1**2p1/P1**P1P1/1P2K2n/8/8/8 b - - 1 44"
	await set_target_offsets(board.theme.make_zero_pattern())
	Engine.time_scale = 1.0
	await sleep(.5)
	board.make_move("b7b5")
	await sleep(.5)
	board.make_move("e5")
	await sleep(1.5)
	board.make_move("e5d6")
	await sleep(.5)
	Engine.time_scale = 1.0

	await set_flourish_text("New Tactics")
	
	# Show exposing king
	# Grab and go
	# Fortress?

	# Versus
	# Show AI

	# Puzzles
	# await set_flourish_text("Solve Puzzles")

	# Show title
	set_target_offsets()
	await sleep(.5)
	var tween := create_tween()
	tween.tween_property(title, "modulate", Color.WHITE, 1).set_ease(Tween.EASE_OUT).set_trans(Tween.TRANS_CUBIC)
	canvas_layer.add_child(title)
	await sleep(10)

	print("Total time (seconds): ", Time.get_ticks_msec() / 1000.0)
	get_tree().quit()


func set_target_offsets(offsets := board.theme.make_random_pattern(rng)):
	board.set_target_offsets(offsets)
	await board.animation_finished

func set_and_clear_target_offsets(offsets := board.theme.make_random_pattern(rng)):
	set_target_offsets(offsets)
	board.clear_animation_offsets()

func set_flourish_text(text: String):
	var tween := create_tween()
	tween.tween_property(overlay, "modulate", Color.WHITE, 0.25 * Engine.time_scale).set_trans(Tween.TRANS_CUBIC).set_ease(Tween.EASE_OUT)
	flourish_label.text = text

	tween.tween_property(overlay, "modulate", Color.TRANSPARENT, 0.5 * Engine.time_scale).set_delay(1.8 * Engine.time_scale).set_trans(Tween.TRANS_CUBIC).set_ease(Tween.EASE_OUT)
	await tween.finished

func _shuffle():
	var squares := [
		 0,  1,  2,  3,
		 4,  5,  6,  7,
		 8,  9, 15, 10,
		12, 13, 14, 11,
	]
	var goal := PackedInt32Array(squares)
	solution = SlidePuzzle.shuffle(4, squares, 30, rng)
	solution = SlidePuzzle.solve(4, squares, goal)
	var shuffle = solution.duplicate()
	shuffle.reverse()
	for move in shuffle:
		board.slide_squares(move * 2)

func sleep(time_sec: float):
	await get_tree().create_timer(time_sec).timeout

func solve():
	while not solution.is_empty():
		Engine.time_scale = min(Engine.time_scale * 1.4, 4.75)
		board.slide_squares(solution[0] * -2)
		solution.remove_at(0)
		await board.animation_finished
		await sleep(.4)
