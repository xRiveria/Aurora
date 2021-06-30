#include "Mappings/ConstantBufferMappings.h"
#include "Mappings/ResourceMappings.h"
#include "Mappings/SamplerMappings.h"

#ifdef __cplusplus

// Application Side Types
typedef XMMATRIX   matrix;
typedef XMFLOAT4X4 float4x4;
typedef XMFLOAT2   float2;
typedef XMFLOAT3   float3;
typedef XMFLOAT4   float4;
typedef uint32_t   uint;
typedef XMUINT2    uint2;
typedef XMUINT3    uint3;
typedef XMUINT4    uint4;
typedef XMINT2     int2;
typedef XMINT3     int3;
typedef XMINT4     int4;

#define CB_GETBINDSLOT(name) __CBUFFERBINDSLOT__##name##__
#define CBUFFER(name, slot) static const int CB_GETBINDSLOT(name) = slot; struct alignas(16) name
#define CONSTANTBUFFER(name, type, slot) CBUFFER(name, slot)
#define ROOTCONSTANTS(name, type, slot) CBUFFER(name, slot)

#else

#define CBUFFER(name, slot) cbuffer name : register(b ## slot)
#define CONSTANTBUFFER(name, type, slot) ConstantBuffer< type > name : register(b ## slot)

#endif