#version 460

#ifdef VERTEX

layout(location=0) in vec3 aPosition;

layout(location=0) out vec3 vNormal;

layout(binding=uniform_buffer_0) uniform Camera
{
	mat4 uViewProj;
};

void main()
{
	vNormal = aPosition;
	gl_Position = uViewProj * vec4(aPosition, 1.);
	gl_Position.y *= -1.;
}

#endif

#ifdef FRAGMENT

#define PI 3.141592

#define PRIMARY_RAY_STEPS 8
#define SECONDARY_RAY_STEPS 4

layout(location=0) in vec3 vNormal;

layout(location=0) out vec4 oColor;

layout(binding=uniform_buffer_1) uniform Skybox
{
	vec3 uSunDirection;
	float uSunIntencity;
	vec3 uRayleighScatter;
	float uRayleighScaleH;
	float uViewElevation;
	float uAatmosphereMaxH;
	float uPlanetRadius;
	float uMieScatter;
	float uMieScaleH;
	float uMiePrefD;
};

float raySphereAtOrigin(in vec3 from, in vec3 to, in float radius) 
{
	const float b = 2.0 * dot(to, from);
	const float c = dot(from, from) - (radius * radius);
	const float d = (b * b) - 4. * c;
	return ((-b + sqrt(d)) * 0.5);
}

vec3 atmosphere(in vec3 direction)
{
	const vec3 primaryRayDir = direction;
	const vec3 primaryRayOrg = vec3(0., uPlanetRadius + uViewElevation, 0.);

	const float primaryStepSize = raySphereAtOrigin(primaryRayOrg, primaryRayDir, uPlanetRadius + uAatmosphereMaxH) / float(PRIMARY_RAY_STEPS);

	const float gg = uMiePrefD * uMiePrefD;
	const float mu = dot(primaryRayDir, uSunDirection);
	const float mumu = mu * mu;
	const float pRlh = 3.0 / (16.0 * PI) * (1.0 + mumu);
	float pMie = 3.0 / (8.0 * PI) * ((1.0 - gg) * (mumu + 1.0)) / (pow(1.0 + gg - 2.0 * mu * uMiePrefD, 1.5) * (2.0 + gg));

	float iTime = 0.0;
	float iOdRlh = 0.0;
	float iOdMie = 0.0;
	vec3 totalRlh = vec3(0,0,0);
	vec3 totalMie = vec3(0,0,0);

	for (int i = 0; i < PRIMARY_RAY_STEPS; i++) 
	{
		const vec3 iPos = primaryRayOrg + primaryRayDir * (iTime + primaryStepSize * 0.5);
		const float iHeight = length(iPos) - uPlanetRadius;
		const float optDepthStepRlh = exp(-iHeight / uRayleighScaleH) * primaryStepSize;
		const float optDepthStepMie = exp(-iHeight / uMieScaleH) * primaryStepSize;
		iOdRlh += optDepthStepRlh;
		iOdMie += optDepthStepMie;

		//////////////////////////////////////////////////////////////////////

		const float jStepSize = raySphereAtOrigin(iPos, uSunDirection, uPlanetRadius + uAatmosphereMaxH) / float(SECONDARY_RAY_STEPS);

		// Initialize the secondary ray time.
		float jTime = 0.0;

		// Initialize optical depth accumulators for the secondary ray.
		float jOdRlh = 0.0;
		float jOdMie = 0.0;

		// Sample the secondary ray.
		for (int j = 0; j < SECONDARY_RAY_STEPS; j++) 
		{
			// Calculate the secondary ray sample position.
			vec3 jPos = iPos + uSunDirection * (jTime + jStepSize * 0.5);
			// Calculate the height of the sample.
			float jHeight = length(jPos) - uPlanetRadius;
			// Accumulate the optical depth.
			jOdRlh += exp(-jHeight / uRayleighScaleH) * jStepSize;
			jOdMie += exp(-jHeight / uMieScaleH) * jStepSize;
			// Increment the secondary ray time.
			jTime += jStepSize;
		}

		//////////////////////////////////////////////////////////////////////

		vec3 attn = exp(-(uMieScatter * (iOdMie + jOdMie) + uRayleighScatter * (iOdRlh + jOdRlh)));
		totalRlh += optDepthStepRlh * attn;
		totalMie += optDepthStepMie * attn;
		iTime += primaryStepSize;
	}
	
	return uSunIntencity * (pRlh * uRayleighScatter * totalRlh + pMie * uMieScatter * totalMie);
}

void main()
{
	vec3 color = atmosphere(vNormal);
	color = 1.0 - exp(-1.0 * color);
	oColor = vec4(color, 1);
}

#endif