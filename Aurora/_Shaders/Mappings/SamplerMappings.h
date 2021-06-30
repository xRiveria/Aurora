// Slot Matchings

// Persistent Samplers - These are bound once and stay alive forever.
#define SSLOT_LINEAR_CLAMP              4
#define SSLOT_LINEAR_WRAP               5
#define SSLOT_LINEAR_MIRROR             6
#define SSLOT_POINT_CLAMP               7
#define SSLOT_POINT_WRAP                8
#define SSLOT_POINT_MIRROR              9
#define SSLOT_ANISOTROPY_CLAMP          10
#define SSLOT_ANISOTROPY_WRAP           11
#define SSLOT_ANISOTROPY_MIRROR         12
/// CMP Depth
/// Object Shader
#define SSLOT_RESERVED                  15
#define SSLOT_COUNT_PERSISTENT          (SSLOT_RESERVED + 1 - SSLOT_COUNT_ONDEMAND)


// On Demand Samplers - These are bound on demand and are alive until another is bound to the same slot.
#define SSLOT_ONDEMAND_0                0
#define SSLOT_ONDEMAND_1                1
#define SSLOT_ONDEMAND_2                2
#define SSLOT_ONDEMAND_3                3
#define SSLOT_COUNT_ONDEMAND            (SSLOT_ONDEMAND_3 + 1)

#define SSLOT_COUNT                     (SSLOT_COUNT_PERSISTENT + SSLOT_COUNT_ONDEMAND)