class_name SlidePuzzle2D
extends Node2D

signal solved()

@export var texture: Texture2D

@export_range(3, 4, 1) var complexity := 3
@export var line_color := Color.GRAY
@export var background_color := Color.DIM_GRAY

class Square:
	var canvas_item_id: RID
	var index: int


	func _init(index: int) -> void:
		self.index = index
		self.canvas_item_id = RenderingServer.canvas_item_create()


	func _notification(what: int) -> void:
		match what:
			NOTIFICATION_PREDELETE:
				RenderingServer.free_rid(canvas_item_id)


var squares : Array[Square] = []
var empty_square := -1
var solution : PackedVector2Array


func get_size() -> Vector2:
	return texture.get_size() / complexity


func _ready() -> void:
	var size := get_size()
	var line_width := -1
	var total_complexity := complexity * complexity

	for i in total_complexity:
		squares.push_back(Square.new(i))

	for i in total_complexity:
		var j := squares[i].index
		if j == total_complexity - 1:
			empty_square = i
		else:
			var dst_top_left := Vector2(i % complexity, i / complexity) * size
			var src_top_left := Vector2(j % complexity, j / complexity) * size
			RenderingServer.canvas_item_set_parent(squares[i].canvas_item_id, get_canvas_item())
			RenderingServer.canvas_item_set_transform(squares[i].canvas_item_id, Transform2D.IDENTITY.translated(dst_top_left))
			RenderingServer.canvas_item_add_rect(squares[i].canvas_item_id, Rect2(Vector2(0, 0), size), background_color)
			RenderingServer.canvas_item_add_texture_rect_region(squares[i].canvas_item_id, Rect2(Vector2(0, 0), size), texture.get_rid(), Rect2(src_top_left, size))
			RenderingServer.canvas_item_add_line(squares[i].canvas_item_id, Vector2(0, 0), Vector2(size.x, 0), line_color, line_width)
			RenderingServer.canvas_item_add_line(squares[i].canvas_item_id, Vector2(size.x, 0), Vector2(size.x, size.y), line_color, line_width)
			RenderingServer.canvas_item_add_line(squares[i].canvas_item_id, Vector2(size.x, size.y), Vector2(0, size.y), line_color, line_width)
			RenderingServer.canvas_item_add_line(squares[i].canvas_item_id, Vector2(0, size.y), Vector2(0, 0), line_color, line_width)


func reset() -> void:
	solution.clear()

	var size := get_size()
	var copy : Array[Square] = squares.duplicate()
	for i in squares.size():
		var dst_index := copy[i].index
		squares[dst_index] = copy[i]
		var offset := Vector2i(dst_index % complexity, dst_index / complexity)
		var translation := Vector2(offset.x * size.x, offset.y * size.y)
		RenderingServer.canvas_item_set_transform(
			squares[dst_index].canvas_item_id,
			Transform2D.IDENTITY.translated(translation)
		)

	empty_square = squares.size() - 1


func solve() -> void:
	solution = SlidePuzzle.solve(complexity, PackedInt32Array(squares.map(func (square: Square) -> int: return square.index)))


func shuffle(moves: int) -> void:
	reset()
	var rng := RandomNumberGenerator.new()
	solution = SlidePuzzle.shuffle(complexity, squares, moves, rng)

	var size := get_size()
	for i in squares.size():
		var dst_index := squares[i].index
		var offset := Vector2i(dst_index % complexity, dst_index / complexity)
		var translation := Vector2(offset.x * size.x, offset.y * size.y)
		RenderingServer.canvas_item_set_transform(
			squares[dst_index].canvas_item_id,
			Transform2D.IDENTITY.translated(translation)
		)

	var empty_value := squares.size() - 1
	for i in squares.size():
		if squares[i].index == empty_value:
			empty_square = i
			break


func _process(delta: float) -> void:
	set_process(false)

	if solution.is_empty():
		solved.emit()
		return

	var next_move := solution[0]
	solution.remove_at(0)

	var neighbor := int(empty_square + next_move.x + next_move.y * complexity)
	var tween := create_tween()
	var size := get_size()
	var src_top_left := Vector2(neighbor % complexity, neighbor / complexity) * size
	var dst_top_left := Vector2(empty_square % complexity, empty_square / complexity) * size
	var tmp_square := squares[neighbor]
	squares[neighbor] = squares[empty_square]
	squares[empty_square] = tmp_square
	tween.tween_method(func(target: Vector2):
		RenderingServer.canvas_item_set_transform(tmp_square.canvas_item_id, Transform2D.IDENTITY.translated(target))
	, src_top_left, dst_top_left, .5) \
		.set_trans(Tween.TRANS_CUBIC) \
		.set_ease(Tween.EASE_OUT) \
		.finished.connect(set_process.bind(true))

	empty_square = neighbor
