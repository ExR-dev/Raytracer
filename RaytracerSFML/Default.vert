#version 400

layout(location = 0) in vec2 position;

out vec2 vTexCoord;

void main()
{
    mat4 orthoProj = mat4(
        1.0, 0.0, 0.0, 0.0,
        0.0, 1.0, 0.0, 0.0,
        0.0, 0.0, 1.0, 0.0,
        -1.0, -1.0, 0.0, 1.0
    );

    vTexCoord = position * 0.5;

    gl_Position = orthoProj * vec4(position, 0.0, 1.0);
}