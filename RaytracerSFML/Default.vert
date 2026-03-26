#version 400

layout(location = 0) in vec2 position;
layout(location = 1) in vec2 texCoord;

uniform mat4 sfTransform;
uniform mat4 sfProjection;

out vec2 vTexCoord;

void main()
{
    vTexCoord = texCoord;
    gl_Position = sfProjection * sfTransform * vec4(position, 0.0, 1.0);
}