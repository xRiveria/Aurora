// Slot Matchings

// Common Buffers - These are usable by all shaders.
#define CBSLOT_RENDERER_FRAME                      0
#define CBSLOT_RENDERER_CAMERA                     1

// On Demand Buffers - These are bound on demand and are alive until another is bound at the same slot.
#define CBSLOT_IMAGE                               3
#define CBSLOT_FONT                                4

#define CBSLOT_RENDERER_MISC                       5
#define CBSLOT_RENDERER_MATERIAL                   6