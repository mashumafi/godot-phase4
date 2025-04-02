extends Control

@export var move_buttons: Container
@export var chess_board : Chess2D
@export var wall_selection : WallSelection

var _move_button_group := ButtonGroup.new()

var ZERO_PATTERN := PackedVector2Array([
	Vector2.ZERO, Vector2.ZERO, Vector2.ZERO, Vector2.ZERO,
	Vector2.ZERO, Vector2.ZERO, Vector2.ZERO, Vector2.ZERO,
	Vector2.ZERO, Vector2.ZERO, Vector2.ZERO, Vector2.ZERO,
	Vector2.ZERO, Vector2.ZERO, Vector2.ZERO, Vector2.ZERO,
])

@onready var square_size := chess_board.theme.square_size

@onready var off_screen_factor := 16

@onready var HORIZONTAL_CRISS_CROSS_PATTERN := PackedVector2Array([
	Vector2(square_size * off_screen_factor, 0), Vector2(square_size * off_screen_factor, 0), Vector2(square_size * off_screen_factor, 0), Vector2(square_size * off_screen_factor, 0),
	Vector2(square_size * -off_screen_factor, 0), Vector2(square_size * -off_screen_factor, 0), Vector2(square_size * -off_screen_factor, 0), Vector2(square_size * -off_screen_factor, 0),
	Vector2(square_size * off_screen_factor, 0), Vector2(square_size * off_screen_factor, 0), Vector2(square_size * off_screen_factor, 0), Vector2(square_size * off_screen_factor, 0),
	Vector2(square_size * -off_screen_factor, 0), Vector2(square_size * -off_screen_factor, 0), Vector2(square_size * -off_screen_factor, 0), Vector2(square_size * -off_screen_factor, 0),
])

class Squares:
	extends Node2D

	const gravity := 988.

	var chess_theme: ChessTheme

	var trail := MeshInstance2D.new()

	@onready var square_mesh := chess_theme.create_square_mesh()
	@onready var square_size := chess_theme.square_size
	@onready var white_square_color := chess_theme.white_square_color
	@onready var black_square_color := chess_theme.black_square_color
	@onready var trail_end := global_position

	var velocity := Vector2(-5400, 0)
	var anguler_velocity := -9.8

	func _ready() -> void:
		trail.texture = chess_theme.trail_texture
		trail.material = chess_theme.trail_material
		trail.mesh = chess_theme.create_circle_polygon(1)
		add_child(trail)

		var half_square := square_size / 2.0

		var multimesh_instance := MultiMeshInstance2D.new()
		var multimesh := MultiMesh.new()
		multimesh.mesh = square_mesh
		multimesh.transform_format = MultiMesh.TRANSFORM_2D
		multimesh.use_colors = true
		multimesh.instance_count = 4
		multimesh.color_array = PackedColorArray([
			black_square_color,
			white_square_color,
			white_square_color,
			black_square_color,
		])
		
		var top_left := Transform2D.IDENTITY.translated(Vector2(-half_square, -half_square))
		var top_right := Transform2D.IDENTITY.translated(Vector2(half_square, -half_square))
		var bottom_left := Transform2D.IDENTITY.translated(Vector2(-half_square, half_square))
		var bottom_right := Transform2D.IDENTITY.translated(Vector2(half_square, half_square))
		multimesh.transform_2d_array = PackedVector2Array([
			top_left.x, top_left.y, top_left.origin, 
			top_right.x, top_right.y, top_right.origin, 
			bottom_left.x, bottom_left.y, bottom_left.origin, 
			bottom_right.x, bottom_right.y, bottom_right.origin, 
		])
		multimesh_instance.multimesh = multimesh
		add_child(multimesh_instance)
		
		var vis_notifier := VisibleOnScreenNotifier2D.new()
		var square_vec := Vector2(square_size, square_size)
		vis_notifier.rect = Rect2(-square_vec, square_vec * 2)
		vis_notifier.screen_exited.connect(queue_free)
		add_child(vis_notifier)


	func _process(delta: float) -> void:
		rotate(anguler_velocity * delta)
		velocity.y += gravity * delta
		translate(velocity * delta)
		trail_end = trail_end.lerp(global_position, minf(delta * 3, 1.0))
		trail.global_transform = chess_theme.transform_trail(global_position, trail_end, 1)


func _ready() -> void:
	_piece_moved("*", "*", 0)
	_show_wall_selection()

	chess_board.set_target_offsets(HORIZONTAL_CRISS_CROSS_PATTERN)
	chess_board.clear_animation_offsets();
	chess_board.set_target_offsets(ZERO_PATTERN)


func _show_wall_selection() -> void:
	# TODO: Check that black is a local player
	var is_black_local_human := true
	chess_board.input_mode = Chess2D.INPUT_MODE_NONE
	if is_black_local_human and "rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1" == chess_board.fen:
		wall_selection.modulate = Color.WHITE
		wall_selection.process_mode = Node.PROCESS_MODE_INHERIT
		chess_board.modulate = Color.DARK_GRAY
	else:
		wall_selection.modulate = Color.TRANSPARENT
		wall_selection.process_mode = Node.PROCESS_MODE_DISABLED
		chess_board.modulate = Color.WHITE


func _flip_board() -> void:
	chess_board.is_flipped = not chess_board.is_flipped


func _piece_moved(uci_notation: String, algebraic_notation: String, index: int) -> void:
	var button := Button.new()
	button.text = algebraic_notation
	button.button_group = _move_button_group
	button.toggle_mode = true
	move_buttons.add_child(button)
	button.button_pressed = true

	button.pressed.connect(func():
		chess_board.seek_position(index)
	)


func _undo_last_move() -> void:
	if move_buttons.get_child_count() <= 1:
		return

	chess_board.undo_last_move()
	move_buttons.get_child(move_buttons.get_child_count() - 1).free()

	# Change the view to the latest by pressing the last button
	# Need to unpress all buttons first
	for child: Button in move_buttons.get_children():
		child.set_pressed_no_signal(false)
	move_buttons.get_child(move_buttons.get_child_count() - 1).set_pressed_no_signal(true)

	_show_wall_selection()


func _wall_selected(file: int, rank: int) -> void:
	_break_square(Chess2D.field_to_square(file, rank, chess_board.is_flipped))


func _break_square(square: String) -> void:
	_piece_moved("X", "X", 1)
	var tween := create_tween()
	tween.tween_property(wall_selection, "modulate", Color.TRANSPARENT, .1).set_trans(Tween.TRANS_CUBIC).set_ease(Tween.EASE_IN)
	tween.parallel()
	tween.tween_property(chess_board, "modulate", Color.WHITE, .1).set_trans(Tween.TRANS_CUBIC).set_ease(Tween.EASE_IN)
	wall_selection.process_mode = Node.PROCESS_MODE_DISABLED
	var field := Chess2D.square_to_field(square)
	chess_board.break_square(Chess2D.field_to_square(0, field.y))
	var direction := Vector2i(-field.x / 2 * 2 if chess_board.is_flipped else -field.x, 0)
	chess_board.slide_squares(direction)
	chess_board.input_mode = Chess2D.INPUT_MODE_STANDARD

	var squares := Squares.new()
	squares.chess_theme = chess_board.theme
	var x := square_size * 4  if chess_board.is_flipped else -square_size * 3
	var y := square_size * (field.y - 4) if chess_board.is_flipped else square_size * (3 - field.y)
	squares.velocity.x = -squares.velocity.x if chess_board.is_flipped else squares.velocity.x
	squares.anguler_velocity = -squares.anguler_velocity if chess_board.is_flipped else squares.anguler_velocity
	squares.position = Vector2(x, y)
	chess_board.add_child(squares)
