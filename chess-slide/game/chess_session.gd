extends Node

@export var move_buttons: Container
@export var chess_board : Chess2D
@export var wall_selection : WallSelection

var _move_button_group := ButtonGroup.new()

func _ready() -> void:
	_piece_moved("*", "*", 0)
	if "rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1" == chess_board.fen:
		wall_selection.modulate = Color.WHITE
		chess_board.modulate = Color.DARK_GRAY
		chess_board.process_mode = Node.PROCESS_MODE_DISABLED
	else:
		wall_selection.modulate = Color.TRANSPARENT
		chess_board.modulate = Color.WHITE
		chess_board.process_mode = Node.PROCESS_MODE_INHERIT

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
	for child: Button in move_buttons.get_children():
		child.set_pressed_no_signal(false)
	move_buttons.get_child(move_buttons.get_child_count() - 1).set_pressed_no_signal(true)


func _wall_selected(file: int, rank: int) -> void:
	var tween := create_tween()
	tween.tween_property(wall_selection, "modulate", Color.TRANSPARENT, .25).set_trans(Tween.TRANS_CUBIC).set_ease(Tween.EASE_IN)
	tween.parallel()
	tween.tween_property(chess_board, "modulate", Color.WHITE, .25).set_trans(Tween.TRANS_CUBIC).set_ease(Tween.EASE_IN)
	var square := Chess2D.field_to_square(file, rank, chess_board.is_flipped)
	print(square)
	chess_board.break_square(square)
	chess_board.process_mode = Node.PROCESS_MODE_INHERIT
