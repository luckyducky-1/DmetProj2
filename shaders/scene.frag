#version 330 core

in vec2 texCoord;

out vec4 fragmentColor;

uniform sampler2D diffuseTexture;

void main()
{
    fragmentColor = texture(diffuseTexture, texCoord);
}