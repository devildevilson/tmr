#define BBOX 0
#define SPHERE 1
#define POLYGON 2

#define DYNAMIC_BIT_PLACEMENT   0
#define OBJECT_TYPE_PLACEMENT   (DYNAMIC_BIT_PLACEMENT+1)
#define BLOCKING_BIT_PLACEMENT  (OBJECT_TYPE_PLACEMENT+3)
#define TRIGGER_BIT_PLACEMENT   (BLOCKING_BIT_PLACEMENT+1)
#define RAY_BLOCK_BIT_PLACEMENT (TRIGGER_BIT_PLACEMENT+1)
#define VISIBLE_BIT_PLACEMENT   (RAY_BLOCK_BIT_PLACEMENT+1)

#define OBJECT_TYPE_BITS 0x7;

bool isDynamic(const uint type) {
  const uint mask = 1 << DYNAMIC_BIT_PLACEMENT;
  return (type & mask) == mask;
}

uint getObjType(const uint type) {
  return (type >> OBJECT_TYPE_PLACEMENT) & OBJECT_TYPE_BITS;
}

bool isBlocking(const uint type) {
  const uint mask = 1 << BLOCKING_BIT_PLACEMENT;
  return (type & mask) == mask;
}

bool isTrigger(const uint type) {
  const uint mask = 1 << TRIGGER_BIT_PLACEMENT;
  return (type & mask) == mask;
}

bool canBlockRays(const uint type) {
  const uint mask = 1 << RAY_BLOCK_BIT_PLACEMENT;
  return (type & mask) == mask;
}

bool isVisible(const uint type) {
  const uint mask = 1 << VISIBLE_BIT_PLACEMENT;
  return (type & mask) == mask;
}
