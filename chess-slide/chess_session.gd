extends Node

@export var move_buttons: Container
@export var chess_board : Chess2D

var _move_button_group := ButtonGroup.new()

func _ready() -> void:
	_piece_moved("*", 0)

func _flip_board() -> void:
	chess_board.is_flipped = not chess_board.is_flipped


func _piece_moved(notation: String, index: int) -> void:
	var button := Button.new()
	button.text = notation
	button.button_group = _move_button_group
	button.toggle_mode = true
	move_buttons.add_child(button)
	button.button_pressed = true

	button.pressed.connect(func():
		print("Preview index:", index)
		chess_board.seek_position(index)
	)


func _undo_last_move() -> void:
	chess_board.undo_last_move()
	# TODO: Delete last piece moved button
