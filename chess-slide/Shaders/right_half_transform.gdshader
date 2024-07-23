shader_type canvas_item;
render_mode blend_mix, skip_vertex_transform;

void vertex() {
	// Check if the vertex is in the positive x half of the circle
    if (VERTEX.x > 0.0) {
		VERTEX = (MODEL_MATRIX * vec4(VERTEX, 0.0, 1.0)).xy;
    } else {
	    // Extract translation
	    vec2 translation = MODEL_MATRIX[3].xy;

	    // Extract scale (length of the columns)
	    float scale_x = length(MODEL_MATRIX[0].xy);
	    float scale_y = length(MODEL_MATRIX[1].xy);

	    // Extract rotation (normalized columns)
	    vec2 rotation_x = MODEL_MATRIX[0].xy / scale_x;
	    vec2 rotation_y = MODEL_MATRIX[1].xy / scale_y;

	    // Build the rotation matrix
	    mat2 rotation_matrix = mat2(rotation_x, rotation_y);

		VERTEX = VERTEX * vec2(1.0, scale_y);

		// Apply rotation
		VERTEX = rotation_matrix * VERTEX;

		// Apply translation
		VERTEX = VERTEX + translation;
	}
}
