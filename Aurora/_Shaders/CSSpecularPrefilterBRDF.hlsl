// Preintegrates Cook-Torrace specular BRDF for varying roughness and viewing directions.
// Results are saved into 2D LUT texture in the form of DFG1 and DFG1 split-sum approximation terms, which as a a scale and bias to F0 (Fresnel reflectance at normal incidence) during rendering.

static const float PI = 3.141592;
static const float TwoPI = 2 * PI;
static const float Epsilon = 0.001; // This program needs larger eps.

static const uint NumberOfSamples = 1024;
static const float InverseNumberOfSamples = 1.0 / float(NumberOfSamples);

RWTexture2D<float2> LUT : register(u0);

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

// Single term for seperable Schlick GGX below.
float SchlickG1(float cosTheta, float k)
{
	return cosTheta / (cosTheta * (1.0 - k) + k);
}

// Schlick - GGX Approximation of geometric attenuation using Smith's method (IBL version)
float SchlickGGX_IBL(float cosLi, float cosLo, float roughness)
{
	float r = roughness;
	float k = (r * r) / 2.0;
	return SchlickG1(cosLi, k) * SchlickG1(cosLo, k);
}

[numthreads(32, 32, 1)]
void main(uint2 ThreadID : SV_DispatchThreadID)
{
	// Get output LUT dimensions.
	float outputWidth, outputHeight;
	LUT.GetDimensions(outputWidth, outputHeight);

	// Get integration parameters.
	float cosLo = ThreadID.x / outputWidth;
	float roughness = ThreadID.y / outputHeight;

	// Make sure viewing angle is non-zero to avoid division by zeros and subsequently NaNs.
	cosLo = max(cosLo, Epsilon);

	// Derive tangent space viewing vector from angle to normal (pointing towards +Z in this reference frame).
	float3 Lo = float3(sqrt(1.0 - cosLo * cosLo), 0.0, cosLo);

	float DFG1 = 0;
	float DFG2 = 0;

	for (uint i = 0; i < NumberOfSamples; ++i)
	{
		float2 u = SampleHammersley(i);

		// Sample directly in tangent/shading space since we don't care about reference frame as long as it is consistent.
		float3 Lh = ImportanceSamplingGGX(u.x, u.y, roughness);

		// Compute incident angle direction (Li) by reflecting view direction (Lo) over half vector (Lh).
		float3 Li = 2.0 * dot(Lo, Lh) * Lh - Lo;

		float cosLi = Li.z; // Incident Angle
		float cosLh = Lh.z; // Halfway Angle
		float cosLoLh = max(dot(Lo, Lh), 0.0);

		if (cosLi > 0.0)
		{
			float G = SchlickGGX_IBL(cosLi, cosLh, roughness);
			float Gv = G * cosLoLh / (cosLh * cosLo);
			float Fc = pow(1.0 - cosLoLh, 5);

			DFG1 += (1 - Fc) * Gv;
			DFG2 += Fc * Gv;
		}
	}

	LUT[ThreadID] = float2(DFG1, DFG2) * InverseNumberOfSamples;
}