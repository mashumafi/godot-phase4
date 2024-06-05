shader_type canvas_item;

uniform vec2 direction = vec2(0.0, 0.0);
uniform float frequency = .1;
uniform sampler2D Noise1;
uniform sampler2D Noise2;

const vec2 minBounds = vec2(0.0, 0.0);
const vec2 maxBounds = vec2(1.0, 1.0);

void fragment() {
	vec2 uv = UV;

	vec2 uv1 = mix(minBounds, maxBounds, fract(uv - direction * TIME));
	vec2 uv2 = mix(minBounds, maxBounds, fract(uv + frequency * vec2(cos(direction.y * TIME), sin(direction.x * TIME))));

	vec4 noise = texture(Noise1, uv1) * texture(Noise2, uv2);
	float hint = step(.3, noise.x);

	COLOR.xyz = mix(COLOR.xyz, COLOR.xyz * mix(1.05, 1.15, noise.x), hint);
	COLOR.a *= hint;
}
