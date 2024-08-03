class_name WallSelection extends Node2D

signal wall_selected(file: int, rank: int)

var _container := Node2D.new()
@export var _theme : ChessTheme

func _create_hollow_square(top_left: Vector2, width: float, height: float, hollowWidth: float, hollowHeight: float) -> Polygon2D:
	var square := Polygon2D.new()
	square.position = top_left
	square.polygon =  PackedVector2Array([
		Vector2(0, 0),
		Vector2(width, 0),
		Vector2(width, height),
		Vector2(0, height),
		Vector2(width / 2.0 - hollowWidth / 2.0, height / 2.0 - hollowHeight / 2.0),
		Vector2(width / 2.0 + hollowWidth / 2.0, height / 2.0 - hollowHeight / 2.0),
		Vector2(width / 2.0 + hollowWidth / 2.0, height / 2.0 + hollowHeight / 2.0),
		Vector2(width / 2.0 - hollowWidth / 2.0, height / 2.0 + hollowHeight / 2.0),
	])
	square.polygons = [
		PackedInt32Array([0, 4, 5, 1]),
		PackedInt32Array([1, 5, 6, 2]),
		PackedInt32Array([2, 6, 7, 3]),
		PackedInt32Array([3, 7, 4, 0]),
	]
	square.self_modulate = Color(1, 1, 1, .9)
	square.antialiased = true

	return square

func _create_wall_remove_polygon(wX: int, wY: int) -> Node2D:
	var pivot := Node2D.new()
	pivot.position = Vector2.ONE * _theme.square_size
	pivot.scale = Vector2(.75, .75)
	pivot.modulate = Color(1, 1, 1, .75)

	var rootPolygon := _create_hollow_square(
		Vector2.ONE * -.9 * _theme.square_size,
		_theme.square_size * 1.8,
		_theme.square_size * 1.8,
		_theme.square_size * 1.75,
		_theme.square_size * 1.8 * 2 / 4 - _theme.square_size * .9 * .50 * .1,
	)
	pivot.add_child(rootPolygon)

	for y in range(1, 3):
		for x in range(0, 4):
			if x == wX and y == wY:
				rootPolygon.add_child(
					_create_hollow_square(
						Vector2(
							x * _theme.square_size * .9 * .50 + _theme.square_size * .9 * .05,
							2 * _theme.square_size * .9 * .50 + (y - 2) * _theme.square_size * .81 * .50 + _theme.square_size * .81 * .05,
						),
						_theme.square_size * .81 * .45,
						_theme.square_size * .81 * .45,
						_theme.square_size * .81 * .45 * .8,
						_theme.square_size * .81 * .45 * .8,
					)
				);
			else:
				rootPolygon.add_child(
					_create_hollow_square(
						Vector2(
							x * _theme.square_size * .9 * .50 + _theme.square_size * .9 * .05,
							2 * _theme.square_size * .9 * .50 + (y - 2) * _theme.square_size * .81 * .50 + _theme.square_size * .81 * .05,
						),
						_theme.square_size * .81 * .45,
						_theme.square_size * .81 * .45,
						0,
						0,
					)
				);
	return pivot;

func _create_x(center: Vector2) -> Node2D:
	var pivot := Node2D.new()
	pivot.position = center
	pivot.modulate = Color.TRANSPARENT

	var offsets := Vector2.ONE * _theme.square_size * .1

	var criss_line := Line2D.new()
	criss_line.width = 8
	var criss_points := PackedVector2Array()
	criss_points.push_back(-offsets)
	criss_points.push_back(offsets)
	criss_line.points = criss_points
	criss_line.self_modulate = Color.ORANGE_RED
	criss_line.antialiased = true
	pivot.add_child(criss_line)

	var cross_line := Line2D.new()
	cross_line.width = 8
	var cross_points := PackedVector2Array()
	cross_points.push_back(Vector2(offsets.x, -offsets.y))
	cross_points.push_back(Vector2(-offsets.x, offsets.y))
	cross_line.points = cross_points
	cross_line.self_modulate = Color.ORANGE_RED
	cross_line.antialiased = true
	pivot.add_child(cross_line)

	return pivot

func _ready() -> void:
	add_child(_container);
	var shape := RectangleShape2D.new()
	shape.size = Vector2.ONE * _theme.square_size * 2

	for y in range(1, 3):
		for x in range(0, 4):
			var collision_shape := CollisionShape2D.new()
			collision_shape.shape = shape
			collision_shape.position = Vector2.ONE * _theme.square_size

			var area := Area2D.new()
			area.position = Vector2(x, y) * _theme.square_size * 2 - Vector2.ONE * 4 * _theme.square_size
			area.input_pickable = true

			var selected := Vector2i(x, 3 - y)
			area.input_event.connect(func(viewport: Node, event: InputEvent, shape_idx: int) -> void:
				var mouseButton := event as InputEventMouseButton

				if mouseButton && mouseButton.pressed && (mouseButton.button_mask & MOUSE_BUTTON_MASK_LEFT) == MOUSE_BUTTON_MASK_LEFT:
					wall_selected.emit(selected.x * 2, selected.y * 2)
			)

			var center = Vector2(
				(x - 1) * _theme.square_size * .9 * .50 - _theme.square_size / 4 + _theme.square_size * .9 * .025,
				(y - 2) * _theme.square_size * .9 * .50 + _theme.square_size / 4 - _theme.square_size * .9 * .025,
			)

			var polygon := _create_wall_remove_polygon(x, y)
			var x_node := _create_x(center)
			polygon.add_child(x_node)
			area.mouse_entered.connect(func() -> void:
				var tween := polygon.create_tween();
				tween.tween_property(polygon, "modulate:a", .9, .5).set_trans(Tween.TRANS_CUBIC);
				tween.parallel()
				tween.tween_property(polygon, "scale", Vector2(.9, .9), .5).set_trans(Tween.TRANS_CUBIC).set_ease(Tween.EASE_OUT)
				tween.parallel();
				tween.tween_property(x_node, "modulate:a", 1.0, .5).set_trans(Tween.TRANS_ELASTIC).set_ease(Tween.EASE_OUT)
			)
			area.mouse_exited.connect(func() -> void:
				var tween := polygon.create_tween();
				tween.tween_property(polygon, "modulate:a", .75, .5).set_trans(Tween.TRANS_CUBIC);
				tween.parallel();
				tween.tween_property(polygon, "scale", Vector2(.75, .75), .5).set_trans(Tween.TRANS_CUBIC).set_ease(Tween.EASE_IN)
				tween.parallel();
				tween.tween_property(x_node, "modulate:a", 0.0, .5).set_trans(Tween.TRANS_ELASTIC).set_ease(Tween.EASE_IN)
			)

			area.add_child(collision_shape);
			area.add_child(polygon);
			_container.add_child(area);
