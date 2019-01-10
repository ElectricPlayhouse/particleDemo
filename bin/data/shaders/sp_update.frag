#version 330

// ping pong inputs
uniform sampler2DRect particles0;
uniform sampler2DRect particles1;
uniform sampler2DRect particles2;

uniform vec3 leftPos;
uniform vec3 rightPos;
uniform vec2 resolution;
uniform float radiusSquared;
uniform float elapsed;

in vec2 texCoordVarying;

layout(location = 0) out vec4 posOut;
layout(location = 1) out vec4 velOut;
layout(location = 2) out vec4 colOut;

vec3 rgb2hsv(vec3 c)
{
    vec4 K = vec4(0.0, -1.0 / 3.0, 2.0 / 3.0, -1.0);
    vec4 p = mix(vec4(c.bg, K.wz), vec4(c.gb, K.xy), step(c.b, c.g));
    vec4 q = mix(vec4(p.xyw, c.r), vec4(c.r, p.yzx), step(p.x, c.r));

    float d = q.x - min(q.w, q.y);
    float e = 1.0e-10;
    return vec3(abs(q.z + (q.w - q.y) / (6.0 * d + e)), d / (q.x + e), q.x);
}

vec3 hsv2rgb(vec3 c)
{
    vec4 K = vec4(1.0, 2.0 / 3.0, 1.0 / 3.0, 3.0);
    vec3 p = abs(fract(c.xxx + K.xyz) * 6.0 - K.www);
    return c.z * mix(K.xxx, clamp(p - K.xxx, 0.0, 1.0), c.y);
}

void main()
{
    vec3 pos = texture(particles0, texCoordVarying.st).xyz;
    vec3 vel = texture(particles1, texCoordVarying.st).xyz;
    vec3 col = texture(particles2, texCoordVarying.st).xyz;

    //  mass
    float mass =  25.0 * col.z;
	mass = clamp(mass, 5.0, 25.0);

    // attraction
    vec3 direction = leftPos - pos.xyz;
    float distSquared = dot(direction, direction);
    float magnitude = 2000.0 * (1.0 - distSquared / radiusSquared) / mass;
    vec3 force = step(distSquared, radiusSquared) * magnitude * normalize(direction);
    
    // accelerate
    vel += elapsed * force;

     // attraction
    direction = rightPos - pos.xyz;
    distSquared = dot(direction, direction);
    magnitude = 2000.0 * (1.0 - distSquared / radiusSquared) / mass;
    force = step(distSquared, radiusSquared) * magnitude * normalize(direction);
    
    // accelerate
    vel += elapsed * force;
    
    // bounce off the sides
    if(pos.x > resolution.x || pos.x < 0) vel.x *= -1.0;
    if(pos.y > resolution.y || pos.y < 0) vel.y *= -1.0;
    
    // damping
    vel *= 0.995;
    
    // move
    pos += elapsed * vel;

        //  Update color
    //vec3 hsv = rgb2hsv(col);
    //hsv.x += sin(0.01 * elapsed);
    //col = hsv2rgb(hsv);
	//col.z = clamp(col.z, 0.25, 1.0);
    
    posOut = vec4(pos, 1.0);
    velOut = vec4(vel, 0.0);
    colOut = vec4(col, 1.0);
}