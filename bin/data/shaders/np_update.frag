#version 330

// ping pong inputs
uniform sampler2DRect particles0;
uniform sampler2DRect particles1;
uniform sampler2DRect particles2;
uniform sampler2DRect particles3;

uniform vec3 leftPos;
uniform vec3 rightPos;
uniform vec2 resolution;
uniform float radiusSquared;
uniform float elapsed;

uniform float noise_scale;
uniform float noise_frequency;

in vec2 texCoordVarying;

layout(location = 0) out vec4 posOut;
layout(location = 1) out vec4 velOut;
layout(location = 2) out vec4 colOut;
layout(location = 3) out vec4 spdOut;

#define TWO_PI 6.283185307179586476925286766559


float rand(vec2 n) { 
    return fract(sin(dot(n, vec2(12.9898, 4.1414))) * 43758.5453);
}

vec3 hash33(vec3 p) { 
    float n = sin(dot(p, vec3(7, 157, 113)));    
    return fract(vec3(2097152, 262144, 32768)*n)*2. - 1.; // return fract(vec3(64, 8, 1)*32768.0*n)*2.-1.; 
}

float noise(in vec3 p)
{
    vec3 i = floor(p + dot(p, vec3(0.333333)) );  p -= i - dot(i, vec3(0.166666)) ;
    vec3 i1 = step(p.yzx, p), i2 = max(i1, 1.0-i1.zxy); i1 = min(i1, 1.0-i1.zxy);    
    
    vec3 p1 = p - i1 + 0.166666, p2 = p - i2 + 0.333333, p3 = p - 0.5;
  
    vec4 v = max(0.5 - vec4(dot(p,p), dot(p1,p1), dot(p2,p2), dot(p3,p3)), 0.0);
    
    vec4 d = vec4(dot(p, hash33(i)), dot(p1, hash33(i + i1)), dot(p2, hash33(i + i2)), dot(p3, hash33(i + 1.)));
    
    return clamp(dot(d, v*v*v*8.)*1.732 + .5, 0., 1.); 
}

void main()
{
    vec3 pos = texture(particles0, texCoordVarying.st).xyz;
    vec3 vel = texture(particles1, texCoordVarying.st).xyz;
    vec4 col = texture(particles2, texCoordVarying.st).xyzw;

    float speed = texture(particles3, texCoordVarying.st).x;

    //  Noise
    float angle = noise(vec3(pos.x/noise_scale, pos.y/noise_scale, 1.0)) * TWO_PI * noise_frequency;
    vec2 dir = vec2(cos(angle), sin(angle));

    vel.xy = dir * speed;

    //  mass
    float mass =  12.0 * col.r + 8.0 * col.g + 4.0 * col.b;
	mass = clamp(mass, 8.0, 24.0);

    //  Attraction / repulsion
    vec3 direction = rightPos - pos.xyz;
    float distSquared = dot(direction, direction);
    float magnitude = 2000.0 * (1.0 - distSquared / radiusSquared);
    vec3 force = step(distSquared, radiusSquared) * magnitude * normalize(direction) / mass;

    vel += elapsed * force;

    direction = leftPos - pos.xyz;
    distSquared = dot(direction, direction);
    magnitude = 2000.0 * (1.0 - distSquared / radiusSquared);
    force = step(distSquared, radiusSquared) * magnitude * normalize(direction) / mass;

    vel += elapsed * force;

    //  Clamp
    //if(length(vel) > 100.0)
    //    vel *= 100.0 / length(vel);

    // Damping
    //vel *= 0.999;

    //  Update position
    pos.x += vel.x;
    pos.y += vel.y;

    if(pos.x < -50 || pos.x > resolution.x + 50 || pos.y < -50 || pos.y > resolution.y + 50){
            pos.xy = vec2(resolution.x * rand(resolution.xy * elapsed), resolution.y * rand(resolution.yx * elapsed));
        }


    posOut = vec4(pos, 1.0);
    velOut = vec4(vel, 0.0);
    colOut = vec4(col);
    spdOut = texture(particles3, texCoordVarying.st).xyzw;
}