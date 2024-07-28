extends Node

@export var move_buttons: Container
@export var chess_board : Chess2D

var _move_button_group := ButtonGroup.new()

func _ready() -> void:
	_piece_moved("*", "*", 0)

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
