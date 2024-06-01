shader_type canvas_item;

uniform sampler2D gradient;

float luminance(vec3 color) {
	return 0.2126*color.r + 0.7152*color.g + 0.0722*color.b;
}

void fragment() {
	vec4 t = texture(TEXTURE, UV);
	float l = luminance(t.xyz) * t.a;
	COLOR.rgb *= texture(gradient, vec2(l , 0)).rgb;
	COLOR.a *= t.a;
}
