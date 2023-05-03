#version 450 core

#extension GL_EXT_buffer_reference : require

struct FireSource
{
    vec2 position;
    vec2 addedVelocity;
    float size;
    float addedFuel;
};

layout(buffer_reference, std430) buffer FireSourcesList
{
    FireSource item[];
};

layout(push_constant) uniform FrameParams
{
    vec2 TexelSize;
    FireSourcesList Sources;
    uint NumSources;
    float DeltaTime;
    float TotalTime;
    float FuelNoise;
    float NoiseBlend;
    float BurnTemp; //velocity dissipation 0.98
    float Cooling;
    //fuel dissipation 0.92
    //temp, noise dissipation = 1 (no dissipation)
};

layout(set=0, binding=0, rgba8) uniform image2D FireFTN;

layout(set=0, binding=1, rg16f) uniform image2D FireVelocity;

#if COMPUTE_SHADER

/*
float uniform3d(vec3 pos)
{
    return fract(sin(dot(pos.xyz ,vec3(12.9898,78.233,144.7272))) * 43758.5453);
}

float noise(vec3 uvt, float frequency)
{
    vec3 gridPos = uvt * frequency;
    vec3 gridPoint = floor(gridPos);
    vec3 gridOffs = fract(gridPos);

    vec4 a = vec4(
        uniform3d(gridPoint + vec3(0.0, 0.0, 0.0)),
        uniform3d(gridPoint + vec3(1.0, 0.0, 0.0)),
        uniform3d(gridPoint + vec3(0.0, 1.0, 0.0)),
        uniform3d(gridPoint + vec3(1.0, 1.0, 0.0))
    );

    vec4 b = vec4(
        uniform3d(gridPoint + vec3(0.0, 0.0, 1.0)),
        uniform3d(gridPoint + vec3(1.0, 0.0, 1.0)),
        uniform3d(gridPoint + vec3(0.0, 1.0, 1.0)),
        uniform3d(gridPoint + vec3(1.0, 1.0, 1.0))
    );

    vec4 c = a + smoothstep(0.0, 1.0, gridOffs.zzzz) * (b - a);
    vec2 d = c.xz + smoothstep(0.0, 1.0, gridOffs.xx) * (c.yw - c.xz);
    return d.x + smoothstep(0.0, 1.0, gridOffs.y) * (d.y - d.x);
}

void mainImage( out vec4 fragColor, in vec2 fragCoord )
{
    vec3 uvt = vec3(fragCoord / iResolution.xy, fract(iTime));
    float value = noise(uvt, 256.0) + noise(uvt, 128.0) + noise(uvt, 64.0) + noise(uvt, 32.0);
    fragColor = vec4(0.25 * value, 0.0, 0.0, 1.0);
}
*/

/*
float gaussianSplat(vec2 uv, vec2 center, float radius, float aspectRatio)
{
    vec2 point = (uv - center) * vec2(aspectRatio, 1.0);
    return exp(-dot(point, point) / radius);
}

void mainImage( out vec4 fragColor, in vec2 fragCoord )
{
    vec2 uv = fragCoord/iResolution.xy;
    float aspectRatio = iResolution.x / iResolution.y;
    float value = gaussianSplat(uv, vec2(0.5, 0.5), 0.001, aspectRatio);
    fragColor = vec4(value, 0.0, 0.0, 1.0);
}

Combustion step: splat for every buffer entry, read fuel and temp, write fuel and temp
Combustion step + add noise!!! FireFTN (fuel + temperature + noise)

*/

vec2 globalUV(ivec2 point)
{   
    return (vec2(point) + 0.5) * TexelSize;
}

float uniform3D(vec3 pos)
{
    return fract(sin(dot(pos.xyz ,vec3(12.9898,78.233,144.7272))) * 43758.5453);
}

float gaussianSplat(vec2 uv, vec2 center, float radius)
{
    vec2 point = (uv - center);
    return exp(-dot(point, point) / radius);
}

float noise(vec3 uvt, float frequency)
{
    vec3 gridPos = vec3(uvt.x, 1.0 - uvt.y, uvt.z) * frequency;
    vec3 gridPoint = floor(gridPos);
    vec3 blend = smoothstep(0.0, 1.0, fract(gridPos));
    vec4 a = vec4(
        uniform3D(gridPoint + vec3(0.0, 0.0, 0.0)),
        uniform3D(gridPoint + vec3(1.0, 0.0, 0.0)),
        uniform3D(gridPoint + vec3(0.0, 1.0, 0.0)),
        uniform3D(gridPoint + vec3(1.0, 1.0, 0.0))
    );
    vec4 b = vec4(
        uniform3D(gridPoint + vec3(0.0, 0.0, 1.0)),
        uniform3D(gridPoint + vec3(1.0, 0.0, 1.0)),
        uniform3D(gridPoint + vec3(0.0, 1.0, 1.0)),
        uniform3D(gridPoint + vec3(1.0, 1.0, 1.0))
    );
    vec4 c = a + blend.z * (b - a);
    vec2 d = c.xz + blend.x * (c.yw - c.xz);
    return d.x + blend.y * (d.y - d.x);
}

void beginSimulation(ivec2 point)
{
    vec3 ftn = imageLoad(FireFTN, point).xyz;
    vec2 vel = imageLoad(FireVelocity, point).xy;
    vec3 noisePoint = vec3(globalUV(point), fract(TotalTime));
    float addedNoise = noise(noisePoint, 256.0) + noise(noisePoint, 128.0) + noise(noisePoint, 64.0) + noise(noisePoint, 32.0);
    ftn.z += NoiseBlend * (0.25 * addedNoise - ftn.z);
    for (uint i = 0; i < NumSources; i++)
    {
        FireSource fs = Sources.item[i];
        float splat = gaussianSplat(globalUV(point), fs.position, fs.size);
        ftn.x = max(ftn.x, splat * fs.addedFuel);
        vel += splat * fs.addedVelocity;
    }
    float fuel = ftn.x /*+ 2.0 * (ftn.z - 0.5) * FuelNoise*/;
    //ftn.y = max(0.0, ftn.y - DeltaTime * Cooling * pow(ftn.y / BurnTemp, 4.0));
    ftn.y = max(0.0, ftn.y - DeltaTime * Cooling * pow(ftn.y , 4.0));
    //ftn.y = max(ftn.y, fuel/* * BurnTemp*/);
    ftn.y = max(ftn.y, fuel);
    imageStore(FireVelocity, point, vec4(vel, 0.0, 0.0));
    imageStore(FireFTN, point, vec4(ftn, 0.0));
}

#define ADVECT(src, point, diss) \
    vec2 vel = imageLoad(FireVelocity, point).xy; \
    vec2 prev = vec2(point) - vel * DeltaTime; \
    ivec2 prevPoint = ivec2(floor(prev)); \
    vec2 weights = fract(prev); \
    vec4 a = imageLoad(src, prevPoint + ivec2(0, 0)); \
    vec4 b = imageLoad(src, prevPoint + ivec2(1, 0)); \
    vec4 c = imageLoad(src, prevPoint + ivec2(0, 1)); \
    vec4 d = imageLoad(src, prevPoint + ivec2(1, 1)); \
    vec4 lerp = mix(mix(a, b, weights.x), mix(c, d, weights.x), weights.y); \
    imageStore(src, point, diss * lerp);

layout (local_size_x = 8, local_size_y = 8) in;

void main()
{
    ivec2 point = ivec2(gl_GlobalInvocationID.xy);

    #if BEGIN_SIMULATION

    beginSimulation(point);

    #elif VELOCITY_ADVECTION

    ADVECT(FireVelocity, point, vec4(0.98, 0.98, 1, 1));

    #elif END_SIMULATION

    ADVECT(FireFTN, point, vec4(0.98, 1, 1, 1));
    
    #endif

}

#endif

#if FRAGMENT_SHADER

layout(location=0) out vec4 OutColor;

vec3 blackbody(float t)
{
    t *= 3500.0;

    // Planckian locus or black body locus approximated in CIE color space.
    float cx = (0.860117757 + 1.54118254e-4*t + 1.28641212e-7*t*t)/(1.0 + 8.42420235e-4*t + 7.08145163e-7*t*t);
    float cy = (0.317398726 + 4.22806245e-5*t + 4.20481691e-8*t*t)/(1.0 - 2.89741816e-5*t + 1.61456053e-7*t*t);

    // Converting the chromacity coordinates to XYZ tristimulus color space.
    float d = (2.*cx - 8.*cy + 4.);
    vec3 XYZ = vec3(3.*cx/d, 2.*cy/d, 1. - (3.*cx + 2.*cy)/d);

    // Converting XYZ color space to RGB: https://www.cs.rit.edu/~ncs/color/t_spectr.html
    vec3 RGB = mat3(3.240479, -0.969256, 0.055648, -1.537150, 1.875992, -0.204043, -0.498535, 0.041556, 1.057311) * vec3(1./XYZ.y*XYZ.x, 1., 1./XYZ.y*XYZ.z);

    // Apply Stefan–Boltzmann's law to the RGB color
    return max(RGB, 0.)*pow(t*0.0004, 4.);
}

void main()
{
    vec3 ftn = imageLoad(FireFTN, ivec2(gl_FragCoord.xy)).xyz;
    float temp = ftn.x + 2.0 * (ftn.z - 0.5) * FuelNoise;
    OutColor = vec4(blackbody(temp), 1.0);
}

#endif

#if VERTEX_SHADER

void main()
{
    vec2 vertices[3] = vec2[3]( vec2(-1, -1), vec2( 3, -1), vec2(-1,  3) );
    gl_Position = vec4(vertices[gl_VertexIndex], 0.0, 1.0);    
    //OutTexCoord = 0.5 * gl_Position.xy + vec2(0.5);
}

#endif
