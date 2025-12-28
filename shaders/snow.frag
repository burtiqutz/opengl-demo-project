#version 410 core
out vec4 fragColor;
in vec2 fragTexCoords;

uniform sampler2D snowTexture;
uniform float time;

void main() {
    // Scroll texture
    vec2 newCoords = vec2(fragTexCoords.x, fragTexCoords.y + (time * 0.3f));

    vec4 color = texture(snowTexture, newCoords);

    // Discard transparent parts
    if (color.a < 0.1)
        discard;

    fragColor = color;
}