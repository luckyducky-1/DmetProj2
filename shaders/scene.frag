#version 330 core

in vec3 worldPosition;
in vec3 worldNormal;
in vec2 texCoord;

out vec4 fragmentColor;

uniform sampler2D diffuseTexture;

uniform vec3 directionalDirection;
uniform vec3 directionalColor;

uniform vec3 pointPosition;
uniform vec3 pointColor;

uniform vec3 spotPosition;
uniform vec3 spotDirection;
uniform vec3 spotColor;

vec3 diffuseLight(vec3 normal, vec3 toLight, vec3 color)
{
    float brightness = max(dot(normal, toLight), 0.0);
    return color * brightness;
}

void main()
{
    vec4 surface = texture(diffuseTexture, texCoord);
    vec3 normal = normalize(worldNormal);

    vec3 lighting = vec3(0.10);

    // Directional light has the same direction across the scene.
    lighting += diffuseLight(
        normal,
        normalize(-directionalDirection),
        directionalColor
    );

    vec3 pointOffset = pointPosition - worldPosition;
    float pointDistance = length(pointOffset);
    float pointFalloff = 1.0 / (
        1.0 + 0.14 * pointDistance +
        0.07 * pointDistance * pointDistance
    );

    lighting += diffuseLight(
        normal,
        normalize(pointOffset),
        pointColor
    ) * pointFalloff;

    vec3 spotOffset = spotPosition - worldPosition;
    float spotDistance = length(spotOffset);
    vec3 toSpot = normalize(spotOffset);

    float alignment = dot(toSpot, normalize(-spotDirection));
    float cone = smoothstep(
        cos(radians(25.0)),
        cos(radians(15.0)),
        alignment
    );

    float spotFalloff = 1.0 / (
        1.0 + 0.09 * spotDistance +
        0.032 * spotDistance * spotDistance
    );

    lighting += diffuseLight(
        normal,
        toSpot,
        spotColor
    ) * cone * spotFalloff;

    fragmentColor = vec4(surface.rgb * lighting, surface.a);
}