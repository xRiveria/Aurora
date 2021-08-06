#include "Globals.hlsli"

// Prefilters environment cube map using GGX NDF importance samping.
// Part of specular IBL split-sum approximation.

static const float PI = 3.141592;
static const float TwoPI = 2 * PI;
static const float Epsilon = 0.00001;

static const uint NumberOfSamples = 1024;
static const float InverseNumberOfSamples = 1.0 / float(NumberOfSamples);

TextureCube inputTexture : register(t5);
RWTexture2DArray<float4> outputTexture : register(u0);
SamplerState defaultSampler : register(s3);

cbuffer SpecularMapFilterSettings : register(b7)
{
	// Roughness value to pre-filter for.
	float roughness;
};


// Compute Van Der Corput Radical Inverse
// - Uses importance sampling given a random low-discrepency sequence based on the Quasi-Monte Carlo method. See: http://holger.dammertz.org/stuff/notes_HammersleyOnHemisphere.html

float RadicalInverse_VDC(uint bits)
{
	bits = (bits << 16u) | (bits >> 16u);
	bits = ((bits & 0x55555555u) << 1u) | ((bits & 0xAAAAAAAAu) >> 1u);
	bits = ((bits & 0x33333333u) << 2u) | ((bits & 0xCCCCCCCCu) >> 2u);
	bits = ((bits & 0x0F0F0F0Fu) << 4u) | ((bits & 0xF0F0F0F0u) >> 4u);
	bits = ((bits & 0x00FF00FFu) << 8u) | ((bits & 0xFF00FF00u) >> 8u);
	return float(bits) * 2.3283064365386963e-10; // / 0x100000000
}

// Gives us the low-discrepancy sample i of the total sample set of size dictacted by NumberOfSamples.
// The vector generated is in tangent space. 
float2 SampleHammersley(uint i)
{
	return float2(i * InverseNumberOfSamples, RadicalInverse_VDC(i));
}

// Importance Sampling GGX Normal Distributive Function for a fixed roughness value. 
// This returns a normalized half-vector between Li and Lo.
// For derivation, see: http://blog.tobias-franke.eu/2014/03/30/notes_on_importance_sampling.html
float3 ImportanceSamplingGGX(float u1, float u2, float roughness) // U1 - HammersleySample.x, U2 - HammersleySample.y.
{
	float alpha = roughness * roughness;
	float cosTheta = sqrt((1.0 - u2) / (1.0 + (alpha * alpha - 1.0) * u2));
	float sinTheta = sqrt(1.0 - cosTheta * cosTheta); // Trig Identity.
	float phi = TwoPI * u1;

	// Convert to Cartesian upon return.
	return float3(sinTheta * cos(phi), sinTheta * sin(phi), cosTheta);
}

// GGX/Towbridge-Reitz NDF. Use's Disney's reparameterization of alpha = roughness^2.
float NDFGGX(float cosLh, float roughness)
{
	float alpha = roughness * roughness;
	float alphaSquared = alpha * alpha;

	float denominator = (cosLh * cosLh) * (alphaSquared - 1.0) + 1.0;
	return alphaSquared / (PI * denominator * denominator);
}

// Compute Orthonormal Basis for converting from tangent/shading space to World Space.
// N - Sampling direction vector.
void ComputeBasisVectors(const float3 N, out float3 S, out float3 T)
{
	// Branchless select non-degenerate T.
	T = cross(N, float3(0.0, 1.0, 0.0));
	T = lerp(cross(N, float3(1.0, 0.0, 0.0)), T, step(Epsilon, dot(T, T)));

	T = normalize(T);
	S = normalize(cross(N, T));
}

// Convert point from tangent/shading space to world space.
float3 TangentToWorld(const float3 v, const float3 N, const float3 S, const float3 T)
{
	return S * v.x + T * v.y + N * v.z;
}


// Calculate normalized sampling direction vector based on current fragment coordinates.
// This is essentially "inverse-sampling": we reconstruct what the sampling vector would be if we wanted it to "hit"
// this particular fragment in a cubemap.
float3 GetSamplingVector(uint3 ThreadID)
{
	float outputWidth, outputHeight, outputDepth;
	outputTexture.GetDimensions(outputWidth, outputHeight, outputDepth);

	float2 st = ThreadID.xy / float2(outputWidth, outputHeight);
	float2 uv = 2.0 * float2(st.x, 1.0 - st.y) - 1.0;

	// Select vector based on cubemap face index.
	float3 ret;
	switch (ThreadID.z)
	{
	case 0: ret = float3(1.0, uv.y, -uv.x); break;
	case 1: ret = float3(-1.0, uv.y, uv.x); break;
	case 2: ret = float3(uv.x, 1.0, -uv.y); break;
	case 3: ret = float3(uv.x, -1.0, uv.y); break;
	case 4: ret = float3(uv.x, uv.y, 1.0); break;
	case 5: ret = float3(-uv.x, uv.y, -1.0); break;
	}
	return normalize(ret);
}

[numthreads(32, 32, 1)]
void main(uint3 ThreadID : SV_DispatchThreadID)
{
	// Make sure we won't write past output when computing higher mipmap levels.
	uint outputWidth, outputHeight, outputDepth;
	outputTexture.GetDimensions(outputWidth, outputHeight, outputDepth);
	if (ThreadID.x >= outputWidth || ThreadID.y >= outputHeight)
	{
		return;
	}

	// Get input cubemap dimensions at zero mipmap levels.
	float inputWidth, inputHeight, inputLevels;
	inputTexture.GetDimensions(0, inputWidth, inputHeight, inputLevels);

	// Solid angle associated with a single cubemap texel at zero mipmap level. This will come in handy for importance sampling.
	float wt = 4.0 * PI / (6 * inputWidth * inputHeight);

	// Approximation: Assume zero viewing angle (isotropic reflections).
	float3 N = GetSamplingVector(ThreadID);
	float3 Lo = N; // Viewing Direction
	float3 S, T;

	// Convert from tangent-space vector to world-space sample vector.
	ComputeBasisVectors(N, S, T);

	float3 color = 0; // Result is stored in here.
	float weight = 0; // The resulting color is divided by this total sample weight, where samples with less influence on the final result (for small NdotL) contributes less to the final weight.

	// Convolute environment using GGX NDF importance sampling. We weight by the consine term as Epic claims it generally improves quality.
	for (uint i = 0; i < NumberOfSamples; ++i)
	{
		float2 u = SampleHammersley(i); // Generate sample vector in tangent space.
		float3 Lh = TangentToWorld(ImportanceSamplingGGX(u.x, u.y, roughness), N, S, T); // Returns halfway vector oriented around fragment. Get roughness from CBuffer. 

		// Compute incident direction (Li) by reflecting viewing direction (Lo) around half-vector LH.
		float3 Li = 2.0 * dot(Lo, Lh) * Lh - Lo;   // Incident direction.

		float cosLi = dot(N, Li); // Dot product between incident direction and view direction.
		if (cosLi > 0.0)
		{
			// Use Mipmap Filtered Importance Sampling to improve convergence and reduce dot artifacts due to rougher mip levels.
			float cosLh = max(dot(N, Lh), 0.0); // Dot product between viewing direction and halfway vector.

			// GGX Normal Distribution Function PDF (Probability Density Function)
			float pdf = NDFGGX(cosLh, roughness) * 0.25;

			// Solid angle associated with this sample.
			float ws = 1.0 / (NumberOfSamples * pdf);

			// Mip level to sample from.
			float mipLevel = max(0.5 * log2(ws / wt) + 1.0, 0.0);

			color += inputTexture.SampleLevel(defaultSampler, Li, mipLevel).rgb * cosLi;
			weight += cosLi;
		}
	}

	color /= weight;

	outputTexture[ThreadID] = float4(color, 1.0);
}