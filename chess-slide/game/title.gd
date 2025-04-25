extends Node2D

enum Pattern {
	Stripes,
	Checkered,
}

const PackedSlidePuzzle2D := preload("res://game/slide_puzzle.tscn")
const COLOR_KEY := &"color"
const PADDING := 15.0
const SCALE := 1.5


class PuzzleWrapper:
	extends Node2D

	signal shown()
	signal finished()

	var _puzzle : SlidePuzzle2D
	var _was_shown := false

	func _init(chess_theme: ChessTheme, color: ChessTheme.PieceColor, rng: RandomNumberGenerator) -> void:
		scale = Vector2(SCALE, SCALE)
		_puzzle = PackedSlidePuzzle2D.instantiate() as SlidePuzzle2D
		_puzzle.complexity = 3
		_puzzle.texture = chess_theme.get_piece_texture(
			color,
			rng.randi_range(0, ChessTheme.PIECE_TYPE_MAX - 1)
		)
		add_child(_puzzle)
		set_meta(COLOR_KEY, color)

		var on_screen := VisibleOnScreenNotifier2D.new()
		on_screen.rect = Rect2(Vector2(-PADDING, -PADDING), _puzzle.texture.get_size() * SCALE + 2 * Vector2(PADDING, PADDING))
		on_screen.screen_entered.connect(_screen_entered)
		on_screen.screen_exited.connect(_screen_exited)
		add_child(on_screen)


	func _screen_entered():
		if not _was_shown:
			_was_shown = true
			shown.emit()


	func _screen_exited():
		if _was_shown:
			_was_shown = false
			finished.emit()


	func shuffle() -> void:
		_puzzle.shuffle(45)


	func solve() -> void:
		_puzzle.solve()


class PuzzlePool:
	var RNG := RandomNumberGenerator.new()

	var black_puzzle_cache : Array[PuzzleWrapper] = []
	var white_puzzle_cache : Array[PuzzleWrapper] = []
		
	func _notification(what: int) -> void:
		match what:
			NOTIFICATION_PREDELETE:
				for puzzle in black_puzzle_cache:
					puzzle.free()
				for puzzle in white_puzzle_cache:
					puzzle.free()


	func alloc_slide_puzzle(chess_theme: ChessTheme, color: ChessTheme.PieceColor) -> PuzzleWrapper:
		match color:
			ChessTheme.PIECE_COLOR_WHITE:
				if not white_puzzle_cache.is_empty():
					return white_puzzle_cache.pop_back()
			ChessTheme.PIECE_COLOR_BLACK:
				if not black_puzzle_cache.is_empty():
					return black_puzzle_cache.pop_back()

		return PuzzleWrapper.new(chess_theme, color, RNG)


	func free_slide_puzzle(puzzle: PuzzleWrapper) -> void:
		puzzle.get_parent().remove_child(puzzle)
		match puzzle.get_meta(COLOR_KEY):
			ChessTheme.PIECE_COLOR_WHITE:
				white_puzzle_cache.push_back(puzzle)
			ChessTheme.PIECE_COLOR_BLACK:
				black_puzzle_cache.push_back(puzzle)


class PuzzleSpawner:
	extends Node2D

	var magnitude := Vector2(0, 0)

	var _chess_theme : ChessTheme
	var _puzzles: PuzzlePool

	func _init(chess_theme: ChessTheme, puzzles: PuzzlePool) -> void:
		_chess_theme = chess_theme
		_puzzles = puzzles


	func _ready() -> void:
		_spawn()


	func _get_offset() -> Vector2:
		var size := get_child_count()
		if size == 0:
			if absf(magnitude.x) < absf(magnitude.y):
				var y := 0.0 if magnitude.y < 0 else -225.0 * SCALE
				y += signf(magnitude.y) * (Vector2.ONE.length() - 1) * -225.0 * SCALE
				return Vector2(-225.0 * SCALE / 2.0, y)
			else:
				var x := 0.0 if magnitude.x < 0 else -225.0 * SCALE
				x += signf(magnitude.x) * (Vector2.ONE.length() - 1) * -225.0 * SCALE
				return Vector2(x, 225.0 * SCALE / 2.0)

		var child : PuzzleWrapper = get_child(size - 1)
		return child.position - sign(magnitude) * Vector2(child._puzzle.texture.get_size().x * SCALE + 2 * PADDING, child._puzzle.texture.get_size().y * SCALE + 2 * PADDING)


	func _spawn() -> void:
		var puzzle := _puzzles.alloc_slide_puzzle(_chess_theme, 0)
		puzzle.position = _get_offset()
		puzzle.shown.connect(func():
			_spawn()
		, CONNECT_ONE_SHOT)
		puzzle.finished.connect(func():
			_puzzles.free_slide_puzzle(puzzle)
		, CONNECT_ONE_SHOT)
		add_child(puzzle)
		puzzle.shuffle()
		puzzle.solve()

	var total_delta := 0.0
	const duration := 1.5

	func _process(delta: float) -> void:
		total_delta += delta

		for child: Node2D in get_children():
			if absf(magnitude.x) < absf(magnitude.y):
				child.position.y += delta * magnitude.y if total_delta > duration else delta * sign(magnitude.y) * 1920 * (duration * duration - total_delta * total_delta)
			else:
				child.position.x += delta * magnitude.x if total_delta > duration else delta * sign(magnitude.x) * 1920 * (duration * duration - total_delta * total_delta)


@export var chess_theme : ChessTheme

var _puzzles := PuzzlePool.new()


func length(x: float) -> float:
	return sqrt(x*x + x*x)


func _ready() -> void:
	var SUPPORTED_ROTATIONS : Array[float] = [
		40, 0, -40, # RIGHT (0, 1, 2)
		-50, -90, -130, # UP (3, 4, 5)
		-140, -180, -220, # LEFT (6, 7, 8)
		-230, -270, -310, # DOWN (9, 10, 11)
	]
	var ROTATION := deg_to_rad(SUPPORTED_ROTATIONS[5])
	var slow_mag := 250.0
	var fast_mag := 480.0

	var direction := Vector2.RIGHT.rotated(ROTATION)

	for i in range(5):
		var spawner := PuzzleSpawner.new(chess_theme, _puzzles)

		var prec := absf(direction.x) < absf(direction.y)
		var factor := direction.y if prec else direction.x

		var magnitude := factor * (fast_mag if i % 2 == 0 else slow_mag)
		var theta := ROTATION + (0.0 if magnitude > 0 else PI)

		if prec:
			spawner.magnitude.y = magnitude

			var width := (225 * SCALE + PADDING * 2) / absf(sin(theta))
			var tan_theta := tan(theta)
			if magnitude > 0:
				# DOWN
				spawner.position.x = minf(-1080 / tan_theta + PADDING * 2, PADDING * 2) + (225.0 * SCALE / 2.0)
				spawner.position.x += width * i
				spawner.position.y = 0
				spawner.rotation = -PI / 2 + theta
			else:
				# UP
				spawner.position.x = minf(1080 / tan_theta + PADDING * 2, PADDING * 2) + (225.0 * SCALE / 2.0)
				spawner.position.x += width * i
				spawner.position.y = 1080
				spawner.rotation = PI / 2 + theta
		else:
			spawner.magnitude.x = magnitude

			var height := (225 * SCALE + PADDING * 2) / absf(cos(theta))
			if magnitude > 0:
				# RIGHT
				var tan_theta = tan(theta + PI / 2)
				spawner.position.y = minf(1920 / tan_theta + PADDING * 2, PADDING * 2) - (225.0 * SCALE / 2.0)
				spawner.position.y += height * i
				spawner.position.x = 0
				spawner.rotation = theta
			else:
				# LEFT
				var tan_theta = tan(PI / 2 + theta)
				spawner.position.y = minf(-1920 / tan_theta + PADDING * 2, PADDING * 2) - (225.0 * SCALE / 2.0)
				spawner.position.y += height * i
				spawner.position.x = 1920
				spawner.rotation = PI + theta

		add_child(spawner)
