extends Node2D

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
	
	var test_solution = SlidePuzzle.solve(4, PackedInt32Array([
		0, 1, 2, 3,
		4, 5, 6, 7,
		8, 9, 15, 10,
		12, 13, 14, 11
	]))

	for i in total_complexity:
		squares.push_back(Square.new(i))
	while solution.is_empty():
		print_debug("shuffling")
		var rng := RandomNumberGenerator.new()
		solution = SlidePuzzle.shuffle(complexity, squares, rng)
		#solution = SlidePuzzle.solve(complexity, PackedInt32Array(squares.map(func (square: Square) -> int: return square.index)))

	for i in total_complexity:
		var j = squares[i].index
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


func _get_square_position(square: int) -> Vector2i:
	return Vector2i(square % complexity, square / complexity)


func _get_empty_square_position() -> Vector2i:
	return _get_square_position(empty_square)


func _get_neighbors() -> Array[int]:
	assert(empty_square != -1)
	assert(empty_square < complexity * complexity)

	var neighbors : Array[int] = []
	var empty_square_position := _get_empty_square_position()
	if empty_square_position.x > 0:
		neighbors.push_back(empty_square - 1)
	if empty_square_position.x < complexity - 1:
		neighbors.push_back(empty_square + 1)
	if empty_square_position.y > 0:
		neighbors.push_back(empty_square - complexity)
	if empty_square_position.y < complexity - 1:
		neighbors.push_back(empty_square + complexity)
	
	return neighbors


func _random_move() -> int:
	var neighbors := _get_neighbors()
	assert(not neighbors.is_empty())
	return neighbors.pick_random()


func _find_square(square: int) -> int:
	for i in range(squares.size()):
		if square == squares[i].index:
			return i

	return -1


func _get_unsolved_square() -> int:
	for i in range(squares.size()):
		if i != squares[i].index:
			return _find_square(i)

	return -1


func _is_solved() -> bool:
	return solution.is_empty()
	#return _get_unsolved_square() == -1


func _solve() -> int:
	var target_square := _get_unsolved_square()
	assert(target_square != -1)
	
	var target_square_position := _get_square_position(target_square)
	var empty_square_position := _get_empty_square_position()



	return _random_move()


func _process(delta: float) -> void:
	set_process(false)

	if _is_solved():
		# TODO: Play an animation
		return

	prints(solution.size(), " moves remaining.")
	var next_move := solution[0]
	solution.remove_at(0)

	var neighbor := int(empty_square + next_move.x + next_move.y * complexity)
	var tween := create_tween()
	var size := get_size()
	var src_top_left := Vector2(neighbor % complexity, neighbor / complexity) * size
	var dst_top_left := Vector2(empty_square % complexity, empty_square / complexity) * size
	var tmp_square = squares[neighbor]
	squares[neighbor] = squares[empty_square]
	squares[empty_square] = tmp_square
	tween.tween_method(func(target: Vector2):
		RenderingServer.canvas_item_set_transform(tmp_square.canvas_item_id, Transform2D.IDENTITY.translated(target))
	, src_top_left, dst_top_left, .1) \
		.set_trans(Tween.TRANS_CUBIC) \
		.set_ease(Tween.EASE_OUT) \
		.finished.connect(set_process.bind(true))

	empty_square = neighbor
