#version 330

uniform mat4 modelViewProjectionMatrix;
uniform sampler2DRect particles0;
uniform sampler2DRect particles4;

uniform float radius;

in vec4  position;
in vec2  texcoord;

out vec2 texCoordVarying;


float rand(vec2 n) { 
    return fract(sin(dot(n, vec2(12.9898, 4.1414))) * 43758.5453);
}

void main()
{
    texCoordVarying = texcoord;
    gl_Position = modelViewProjectionMatrix * vec4(texture(particles0, texCoordVarying).xyz, 1.0);

    //float radius = texture(particles4, texCoordVarying.st).x;
    gl_PointSize = radius + radius * rand(texture(particles0, texCoordVarying).xy);
}