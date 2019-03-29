#include "HelperFunctions.h"

#define OUTSIDE 0
#define INSIDE 1
#define INTERSECT 2

#include <glm/gtx/norm.hpp>

float getAngle(const glm::vec4 &first, const glm::vec4 &second) {
  const float dotV = glm::dot(first, second);
  const float lenSq1 = glm::length2(first);
  const float lenSq2 = glm::length2(second);

  return glm::acos(dotV / glm::sqrt(lenSq1 * lenSq2));
}

void clipVelocity(const glm::vec4 &clipNormal, const float &bounce, glm::vec4 &vel) {
  const float backoff = glm::dot(vel, clipNormal) * bounce;

  vel = vel - clipNormal * backoff;
}

glm::vec4 transform(const glm::vec4 &p, const glm::vec4 &translation, const glm::mat4 &orientation) {
  return orientation * p + translation;
}

bool isNormalAdequate(const ArrayInterface<glm::vec4>* verts, const uint32_t &offset, const uint32_t &normalIndex, const glm::vec4 &normal) {
  const uint32_t vertexIndex = glm::floatBitsToUint(verts->at(normalIndex).w);
  const glm::vec4 &normalVertex = verts->at(vertexIndex);
  
  for (uint32_t i = offset; i < offset+3; ++i) {
    const glm::vec4 &vert = verts->at(i);
    
    const glm::vec4 &vectorOP = vert - normalVertex;
    
    const float dist = glm::dot(vectorOP, normal);
    
    if (glm::abs(dist) > EPSILON) return false;
  }
  
  return true;
}

glm::vec4 getPolyPolyFace(const ArrayInterface<glm::vec4>* verts,
                          const uint32_t &firstFace,  const uint32_t &firstFaceSize,  const glm::mat4 &firstOrn,
                          const uint32_t &secondFace, const uint32_t &secondFaceSize, const glm::mat4 &secondOrn,
                          const uint32_t &index) {
  const glm::vec4 &polyNormal1 = verts->at(firstFace+index);
  const glm::vec4 &polyNormal2 = verts->at(secondFace+(index%secondFaceSize));
  
  return index < firstFaceSize ?
           firstOrn  * glm::vec4(polyNormal1.x, polyNormal1.y, polyNormal1.z, 0.0f) : 
           secondOrn * glm::vec4(polyNormal2.x, polyNormal2.y, polyNormal2.z, 0.0f);
}

glm::vec4 getPolySphereFace(const ArrayInterface<glm::vec4>* verts, const uint32_t &face, const uint32_t &faceSize, const glm::mat4 &ornFirst, const glm::vec4 &pos1, const glm::vec4 &pos2, const uint32_t &index) {
  const glm::vec4 &polyNormal = verts->at(face+(index%faceSize));
  return index > 0 ? ornFirst * glm::vec4(polyNormal.x, polyNormal.y, polyNormal.z, 0.0f) : glm::normalize(pos2 - pos1);
}

glm::vec4 getBoxPolyFace(const ArrayInterface<glm::vec4>* verts, const uint32_t &face, const uint32_t &faceSize, const glm::mat4 &polyOrn, const glm::mat4 &orn, const uint32_t &index) {
  const glm::vec4 &polyNormal = verts->at(face+(index%faceSize));
  return index > 2 ? polyOrn * glm::vec4(polyNormal.x, polyNormal.y, polyNormal.z, 0.0f) : orn[index];
}

glm::vec4 getBoxSphereFace(const glm::mat4 &orn, const glm::vec4 &pos1, const glm::vec4 &pos2, const uint32_t &index) {
  return index > 0 ? orn[index%3] : glm::normalize(pos2 - pos1);
}

glm::vec4 getBoxBoxFace(const glm::mat4 &orn1, const glm::mat4 &orn2, const uint32_t &index) {
  // СЧЕТ НАЧИНАЕТСЯ С НУЛЯ (0!!!) СЛЕДОВАТЕЛЬНО 3-ИЙ ВЕКТОР ИМЕЕТ ИНДЕКС 2
  // А ЗНАЧИТ ВСЕ ЧТО БОЛЬШЕ 2 (ДВУХ) (А НЕ ТРЕХ) УЖЕ ДРУГАЯ МАТРИЦА
  return index > 2 ? orn1[index%3] : orn2[index];
}

bool overlap(const float &treshold, const float &min1, const float &max1, const float &min2, const float &max2, const glm::vec4 &axis, glm::vec4 &mtv, float &dist) {
  const float test1 = min1 - max2;
  const float test2 = min2 - max1;

  if (test1 > 0.0f || test2 > 0.0f) return false;

  const float d = glm::max(glm::min(glm::abs(test1), glm::abs(test2)) - treshold, 0.0f);
  
  if (d < dist) {
    mtv = axis;
    dist = d;
  }

  return true;
}

FastAABB recalcAABB(const glm::vec4 &pos, const glm::vec4 &ext, const glm::mat4 &orn) {
  glm::vec4 mx = getVertex(pos, ext, orn, 0), mn = getVertex(pos, ext, orn, 0);

  for (uint32_t i = 1; i < 8; ++i) {
    mx = glm::max(mx, getVertex(pos, ext, orn, i));
    mn = glm::min(mn, getVertex(pos, ext, orn, i));
  }

  const glm::vec4 boxPos =         (mn + mx) / 2.0f;
  const glm::vec4 boxExt = glm::abs(mn - mx) / 2.0f;

  return {boxPos, boxExt};
}

FastAABB recalcAABB(const ArrayInterface<glm::vec4>* verticies, const glm::vec4 &pos, const uint32_t &vertexOffset, const uint32_t &vertexSize, const glm::mat4 &orn) {
  const glm::vec4 dir = glm::vec4(pos.x, pos.y, pos.z, 0.0f);
  glm::vec4 mx = transform(verticies->at(vertexOffset), dir, orn);// orn * (dir + verticies->at(vertexOffset)); 
  glm::vec4 mn = mx;

  for (uint32_t i = 1; i < vertexSize; ++i) {
//     mx = glm::max(mx, orn * (dir + verticies->at(vertexOffset + i)));
//     mn = glm::min(mn, orn * (dir + verticies->at(vertexOffset + i)));
    const glm::vec4 transformedPos = transform(verticies->at(vertexOffset+i), dir, orn);
    
    mx = glm::max(mx, transformedPos);
    mn = glm::min(mn, transformedPos);
  }

  const glm::vec4 boxPos =         (mn + mx) / 2.0f;
  const glm::vec4 boxExt = glm::abs(mn - mx) / 2.0f;

  return {boxPos, boxExt};
}

// bool testAABB(const FastAABB &first, const FastAABB &second) {
//   const glm::vec4 center = glm::abs(first.center - second.center);
//   const glm::vec4 extent =          first.extent + second.extent;

//   return extent.x > center.x && extent.y > center.y && extent.z > center.z;
// }

bool intersection(const FastAABB &box, const RayData &ray) {
  const glm::vec4 boxMin = box.center - box.extent;
  const glm::vec4 boxMax = box.center + box.extent;

  float t1 = (boxMin[0] - ray.pos[0]) / ray.dir[0];
  float t2 = (boxMax[0] - ray.pos[0]) / ray.dir[0];

  float tmin = glm::min(t1, t2);
  float tmax = glm::max(t1, t2);

  for (int i = 1; i < 3; ++i) {
    t1 = (boxMin[i] - ray.pos[i]) / ray.dir[i];
    t2 = (boxMax[i] - ray.pos[i]) / ray.dir[i];

    tmin = glm::max(tmin, glm::min(t1, t2));
    tmax = glm::min(tmax, glm::max(t1, t2));
  }

  //return tmax > glm::max(tmin, 0.0f);
  return tmax >= tmin;
}

#define NUMDIM 3
#define RIGHT 0
#define LEFT 1
#define MIDDLE 2

// bool boxVSRay(const FastAABB &box, const RayData &ray) {
//   bool inside = true;
//   char quadrant[NUMDIM];
//   int whichPlane;
//   float maxT[NUMDIM];
//   float candidatePlane[NUMDIM];
//   const glm::vec4 boxMin = box.center - box.extent;
//   const glm::vec4 boxMax = box.center + box.extent;
//   
// 
//   /* Find candidate planes; this loop can be avoided if
//     rays cast all from the eye(assume perpsective view) */
//   for (uint32_t i = 0; i < NUMDIM; ++i) {
//     if (origin[i] < minB[i]) {
//       quadrant[i] = LEFT;
//       candidatePlane[i] = minB[i];
//       inside = FALSE;
//     } else if (origin[i] > maxB[i]) {
//       quadrant[i] = RIGHT;
//       candidatePlane[i] = maxB[i];
//       inside = FALSE;
//     } else {
//       quadrant[i] = MIDDLE;
//     }
//   }
// 
//   /* Ray origin inside bounding box */
//   if (inside) {
//     coord = origin;
//     return true;
//   }
// 
// 
//   /* Calculate T distances to candidate planes */
//   for (uint32_t i = 0; i < NUMDIM; ++i)
//     if (quadrant[i] != MIDDLE && dir[i] !=0.)
//       maxT[i] = (candidatePlane[i]-origin[i]) / dir[i];
//     else
//       maxT[i] = -1.;
// 
//   /* Get largest of the maxT's for final choice of intersection */
//   whichPlane = 0;
//   for (uint32_t i = 1; i < NUMDIM; ++i)
//     if (maxT[whichPlane] < maxT[i])
//       whichPlane = i;
// 
//   /* Check final candidate actually inside box */
//   if (maxT[whichPlane] < 0.) return false;
//   for (uint32_t i = 0; i < NUMDIM; ++i)
//     if (whichPlane != i) {
//       coord[i] = origin[i] + maxT[whichPlane] *dir[i];
//       if (coord[i] < minB[i] || coord[i] > maxB[i])
//         return false;
//     } else {
//       coord[i] = candidatePlane[i];
//     }
//   return true;     /* ray hits box */
// }

uint32_t testFrustumAABB(const FrustumStruct &frustum, const FastAABB &box) {
  uint32_t result = INSIDE; // Assume that the aabb will be inside the frustum
  for(uint32_t i = 0; i < 6; ++i) {
    const glm::vec4 frustumPlane = frustum.planes[i];

    const float d = glm::dot(box.center,          glm::vec4(frustumPlane.x, frustumPlane.y, frustumPlane.z, 0.0f));

    const float r = glm::dot(box.extent, glm::abs(glm::vec4(frustumPlane.x, frustumPlane.y, frustumPlane.z, 0.0f)));

    const float d_p_r = d + r;
    const float d_m_r = d - r;

    if (d_p_r < -frustumPlane.w) {
      result = OUTSIDE;
      break;
    } else if (d_m_r < -frustumPlane.w) result = INTERSECT;
  }

  return result;
}

uint32_t nextPowerOfTwo(const uint32_t& number) {
  uint32_t v = number; // compute the next highest power of 2 of 32-bit v
  v--;
  v |= v >> 1;
  v |= v >> 2;
  v |= v >> 4;
  v |= v >> 8;
  v |= v >> 16;
  v++;
  
  return v;
}

