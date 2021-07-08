// Slot Matchings

// Texture Slots

// These are reserved texture slots for systems.
#define TEXSLOT_DEPTH               0
#define TEXSLOT_LINEAR_DEPTH        1

#define TEXSLOT_GBUFFER_0           2
#define TEXSLOT_GBUFFER_1           3
#define TEXSLOT_GBUFFER_2           4

#define TEXSLOT_GLOBAL_ENVIRONMENTAL_MAP    6
#define TEXSLOT_SKY_VIEW_LUT                10
#define TEXSLOT_TRANSMITTANCE_LUT           11
#define TEXSLOT_MULTISCATTERING_LUT         12
#define TEXSLOT_SKY_LUMINANCE_LUT           13

// On Demand Textures - These are 2D textures and are declared in shader globals. They can be used independantly in any shader.
#define TEXSLOT_ONDEMAND_0          30
#define TEXSLOT_ONDEMAND_1          31
#define TEXSLOT_ONDEMAND_2          32
#define TEXSLOT_ONDEMAND_3          33
#define TEXSLOT_ONDEMAND_4          34
#define TEXSLOT_ONDEMAND_5          35
#define TEXSLOT_ONDEMAND_6          36
#define TEXSLOT_ONDEMAND_7          37
#define TEXSLOT_ONDEMAND_8          38
#define TEXSLOT_ONDEMAND_9          39
#define TEXSLOT_ONDEMAND_10         40
#define TEXSLOT_ONDEMAND_11         41
#define TEXSLOT_ONDEMAND_12         42
#define TEXSLOT_ONDEMAND_13         43
#define TEXSLOT_ONDEMAND_14         44
#define TEXSLOT_ONDEMAND_15         45
#define TEXSLOT_ONDEMAND_16         46
#define TEXSLOT_ONDEMAND_17         47
#define TEXSLOT_ONDEMAND_18         48
#define TEXSLOT_ONDEMAND_19         49
#define TEXSLOT_ONDEMAND_20         50
#define TEXSLOT_ONDEMAND_21         51
#define TEXSLOT_ONDEMAND_22         52
#define TEXSLOT_ONDEMAND_23         53
#define TEXSLOT_ONDEMAND_24         54
#define TEXSLOT_ONDEMAND_25         55
#define TEXSLOT_ONDEMAND_26         56
#define TEXSLOT_ONDEMAND_27         57
#define TEXSLOT_ONDEMAND_28         58
#define TEXSLOT_ONDEMAND_29         59
#define TEXSLOT_ONDEMAND_30         60
#define TEXSLOT_ONDEMAND_31         61
#define TEXSLOT_ONDEMAND_32         62
#define TEXSLOT_ONDEMAND_33         63
#define TEXSLOT_ONDEMAND_COUNT      (TEXSLOT_ONDEMAND_30 - TEXSLOT_ONDEMAND_0 + 1)

#define TEXSLOT_COUNT               64

/// Skinning

// Renderer Object Shader Resources. These are bound into our on demand texture slots above.
#define TEXSLOT_RENDERER_BASECOLOR_MAP              TEXSLOT_ONDEMAND_0
#define TEXSLOT_RENDERER_NORMAL_MAP                 TEXSLOT_ONDEMAND_1
#define TEXSLOT_RENDERER_SURFACE_MAP                TEXSLOT_ONDEMAND_2
#define TEXSLOT_RENDERER_EMMISIVE_MAP               TEXSLOT_ONDEMAND_3
#define TEXSLOT_RENDERER_DISPLACEMENT_MAP           TEXSLOT_ONDEMAND_4
#define TEXSLOT_RENDERER_OCCLUSION_MAP              TEXSLOT_ONDEMAND_5
#define TEXSLOT_RENDERER_TRANSMISSION_MAP           TEXSLOT_ONDEMAND_6
#define TEXSLOT_RENDERER_SHEENCOLOR_MAP             TEXSLOT_ONDEMAND_7
#define TEXSLOT_RENDERER_SHEENROUGHNESS_MAP         TEXSLOT_ONDEMAND_8
#define TEXSLOT_RENDERER_CLEARCOAT_MAP              TEXSLOT_ONDEMAND_9
#define TEXSLOT_RENDERER_CLEARCOATROUGHNESS_MAP     TEXSLOT_ONDEMAND_10
#define TEXSLOT_RENDERER_CLEARCOATNORMAL_MAP        TEXSLOT_ONDEMAND_11
#define TEXSLOT_RENDERER_SPECULAR_MAP               TEXSLOT_ONDEMAND_12

/// Blend 1
/// Blend 2
/// Blend 3

// Render Path Texture Mappings
#define TEXSLOT_RENDERPATH_ENTITY_TITLES            TEXSLOT_ONDEMAND_13
#define TEXSLOT_RENDERPATH_REFLECTION               TEXSLOT_ONDEMAND_14
#define TEXSLOT_RENDERPATH_REFRACTION               TEXSLOT_ONDEMAND_15
#define TEXSLOT_RENDERPATH_WATERRIPPLES             TEXSLOT_ONDEMAND_16
#define TEXSLOT_RENDERPATH_AO                       TEXSLOT_ONDEMAND_17
#define TEXSLOT_RENDERPATH_SSR                      TEXSLOT_ONDEMAND_18
#define TEXSLOT_RENDERPATH_RTSHADOW                 TEXSLOT_ONDEMAND_19

// Images
#define TEXSLOT_IMAGE_BASE                          TEXSLOT_ONDEMAND_0
#define TEXSLOT_IMAGE_MASK                          TEXSLOT_ONDEMAND_1
#define TEXSLOT_IMAGE_BACKGROUND                    TEXSLOT_ONDEMAND_2
