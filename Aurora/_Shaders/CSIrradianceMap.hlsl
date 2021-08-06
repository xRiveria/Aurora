
// Computes diffuse irradiance cubemap convolution for IBL.
// Uses Quasi Monte arlo sampling with Hammersley Sequence.

static const float PI = 3.141592;
static const float TwoPI = 2 * PI;
static const float Epsilon = 0.00001;

static const uint NumberOfSamples = 64 * 1024;
static const float InverseNumberOfSamples = 1.0 / float(NumberOfSamples);

TextureCube inputTexture : register(t5);
RWTexture2DArray<float4> outputTexture : register(u0);

SamplerState defaultSampler : register(s3);

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

// Uniformly sample point on a hemisphere. Cosine weighted sampling would be better fit for a Lambertian BRDF but since this compute shader runs only once as a pre-processing step, its okay.
float3 SampleHemisphere(float u1, float u2)
{
	const float u1p = sqrt(max(0.0, 1.0 - u1 * u1));
	return float3(cos(TwoPI * u2) * u1p, sin(TwoPI * u2) * u1p, u1);
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

[numthreads(32, 32, 1)]
void main(uint3 ThreadID : SV_DispatchThreadID)
{
	float3 N = GetSamplingVector(ThreadID);

	float3 S, T;
	ComputeBasisVectors(N, S, T);
	float3 irradiance = 0.0f;

	for (uint i = 0; i < NumberOfSamples; ++i)
	{
		float2 u = SampleHammersley(i);
		float3 Li = TangentToWorld(SampleHemisphere(u.x, u.y), N, S, T);
		float cosTheta = max(0.0, dot(Li, N));

		// PIs here cancel out because of division of PDF.
		irradiance += 2.0 * inputTexture.SampleLevel(defaultSampler, Li, 0).rgb * cosTheta;
	}

	irradiance /= float(NumberOfSamples);
	outputTexture[ThreadID] = float4(irradiance, 1.0);

}