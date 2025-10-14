extends MainLoop

var child_instances := PackedInt64Array()

class MovieArgs:
	var _output := ""
	var _resolution := ""
	var _fixed_fps := ""
	var _quit_after := ""
	var _scene : PackedScene

	func set_output(output: String) -> MovieArgs:
		_output = output
		return self

	func set_resolution(size: Vector2i) -> MovieArgs:
		_resolution = "{0}x{1}".format([size.x, size.y])
		return self

	func set_fixed_fps(fps: int) -> MovieArgs:
		_fixed_fps = String.num(fps)
		return self

	func set_quit_after(n: int) -> MovieArgs:
		_quit_after = String.num(n)
		return self

	func set_scene(scene: PackedScene) -> MovieArgs:
		_scene = scene
		return self

	func build() -> PackedStringArray:
		var args := PackedStringArray()
		
		assert(_output != "")
		args.append(_output)

		if _scene:
			args.append("--scene")
			args.append(_scene.resource_path)

		if _resolution != "":
			args.append("--resolution")
			args.append(_resolution)

		if _fixed_fps != "":
			args.append("--fixed-fps")
			args.append(_fixed_fps)

		if _quit_after != "":
			args.append("--quit-after")
			args.append(_quit_after)

		return args

func _make_screenshot():
	pass

func _make_movie(movie: MovieArgs):
	var args := PackedStringArray()
	args.append("--write-movie")
	args.append_array(movie.build())
	args.append("--always-on-top")
	print(args)
	var pid := OS.create_instance(args)
	assert(pid != -1)
	child_instances.push_back(pid)

func _initialize() -> void:
	_make_movie(MovieArgs.new()
		.set_output("res://promo/trailer.ogv")
		.set_scene(preload("res://promo/trailer.tscn")))

func _process(_delta: float) -> bool:
	for pid in child_instances:
		if OS.is_process_running(pid):
			return false

	
	for pid in child_instances:
		var rc := OS.get_process_exit_code(pid)
		if rc != 0:
			printerr("rc: ", rc)

	return true
