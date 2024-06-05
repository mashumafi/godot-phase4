extends Node

func _flip_board() -> void:
	$Chess2D.is_flipped = not $Chess2D.is_flipped
