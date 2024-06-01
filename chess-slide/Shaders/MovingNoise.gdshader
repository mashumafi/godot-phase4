shader_type canvas_item;
render_mode blend_mix;

uniform vec2 Scale;
uniform float Offset;
uniform sampler2D Noise;

const vec2 minBounds = vec2(0.0, 0.0);
const vec2 maxBounds = vec2(1.0, 1.0);

void fragment() {
	float offset = TIME + Offset;
	vec2 uv1 = mix(minBounds, maxBounds, fract(UV + vec2(Scale.x * cos(offset), Scale.y * sin(offset))));
	vec2 uv2 = mix(minBounds, maxBounds, fract(UV + vec2(Scale.x * sin(-TIME * Scale.x), Scale.y * cos(TIME * Scale.y))));

	float a = texture(Noise, uv1).x * texture(Noise, uv2).x;
	float s = step(.2, a);

	COLOR.a = mix(a * s, COLOR.a, .5);
}