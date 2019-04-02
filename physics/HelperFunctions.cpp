#include "HelperFunctions.h"

#define OUTSIDE 0
#define INSIDE 1
#define INTERSECT 2

#include <iostream>

//#include <glm/gtx/norm.hpp>

float getAngle(const simd::vec4 &first, const simd::vec4 &second) {
  const float dotV = simd::dot(first, second);
  const float lenSq1 = simd::length2(first);
  const float lenSq2 = simd::length2(second);

  return glm::acos(dotV / glm::sqrt(lenSq1 * lenSq2));
}

void clipVelocity(const simd::vec4 &clipNormal, const float &bounce, simd::vec4 &vel) {
  const float backoff = simd::dot(vel, clipNormal) * bounce;

  vel = vel - clipNormal * backoff;
}

simd::vec4 transform(const simd::vec4 &p, const simd::vec4 &translation, const simd::mat4 &orientation) {
  return orientation * p + translation;
}

bool isNormalAdequate(const ArrayInterface<simd::vec4>* verts, const uint32_t &offset, const uint32_t &normalIndex, const simd::vec4 &normal) {
  const uint32_t vertexIndex = glm::floatBitsToUint(verts->at(normalIndex).w);
  const simd::vec4 &normalVertex = verts->at(vertexIndex);
  
  for (uint32_t i = offset; i < offset+3; ++i) {
    const simd::vec4 &vert = verts->at(i);
    
    const simd::vec4 &vectorOP = vert - normalVertex;
    
    const float dist = simd::dot(vectorOP, normal);
    
    if (glm::abs(dist) > EPSILON) return false;
  }
  
  return true;
}

simd::vec4 getPolyPolyFace(const ArrayInterface<simd::vec4>* verts,
                          const uint32_t &firstFace,  const uint32_t &firstFaceSize,  const simd::mat4 &firstOrn,
                          const uint32_t &secondFace, const uint32_t &secondFaceSize, const simd::mat4 &secondOrn,
                          const uint32_t &index) {
  const simd::vec4 &polyNormal1 = verts->at(firstFace+index);
  const simd::vec4 &polyNormal2 = verts->at(secondFace+(index%secondFaceSize));
  
  float arr1[4];
  float arr2[4];
  polyNormal1.store(arr1);
  polyNormal2.store(arr2);
  
//   return index < firstFaceSize ?
//            firstOrn  * simd::vec4(polyNormal1.x, polyNormal1.y, polyNormal1.z, 0.0f) : 
//            secondOrn * simd::vec4(polyNormal2.x, polyNormal2.y, polyNormal2.z, 0.0f);
  
  return index < firstFaceSize ?
           firstOrn  * simd::vec4(arr1[0], arr1[1], arr1[2], 0.0f) : 
           secondOrn * simd::vec4(arr2[0], arr2[1], arr2[2], 0.0f);
}

simd::vec4 getPolySphereFace(const ArrayInterface<simd::vec4>* verts, const uint32_t &face, const uint32_t &faceSize, const simd::mat4 &ornFirst, const simd::vec4 &pos1, const simd::vec4 &pos2, const uint32_t &index) {
  const simd::vec4 &polyNormal = verts->at(face+(index%faceSize));
  float arr[4];
  polyNormal.store(arr);
  return index > 0 ? ornFirst * simd::vec4(arr[0], arr[1], arr[2], 0.0f) : simd::normalize(pos2 - pos1);
}

simd::vec4 getBoxPolyFace(const ArrayInterface<simd::vec4>* verts, const uint32_t &face, const uint32_t &faceSize, const simd::mat4 &polyOrn, const simd::mat4 &orn, const uint32_t &index) {
  const simd::vec4 &polyNormal = verts->at(face+(index%faceSize));
  float arr[4];
  polyNormal.store(arr);
  return index > 2 ? polyOrn * simd::vec4(arr[0], arr[1], arr[2], 0.0f) : orn[index];
}

simd::vec4 getBoxSphereFace(const simd::mat4 &orn, const simd::vec4 &pos1, const simd::vec4 &pos2, const uint32_t &index) {
  return index > 0 ? orn[index%3] : simd::normalize(pos2 - pos1);
}

simd::vec4 getBoxBoxFace(const simd::mat4 &orn1, const simd::mat4 &orn2, const uint32_t &index) {
  // СЧЕТ НАЧИНАЕТСЯ С НУЛЯ (0!!!) СЛЕДОВАТЕЛЬНО 3-ИЙ ВЕКТОР ИМЕЕТ ИНДЕКС 2
  // А ЗНАЧИТ ВСЕ ЧТО БОЛЬШЕ 2 (ДВУХ) (А НЕ ТРЕХ) УЖЕ ДРУГАЯ МАТРИЦА
  return index > 2 ? orn1[index%3] : orn2[index];
}

bool overlap(const float &treshold, const float &min1, const float &max1, const float &min2, const float &max2, const simd::vec4 &axis, simd::vec4 &mtv, float &dist) {
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

#define PRINT_VEC(name, vec) std::cout << name << "(" << vec.x << ", " << vec.y << ", " << vec.z << ", " << vec.w << ")\n";

FastAABB recalcAABB(const simd::vec4 &pos, const simd::vec4 &ext, const simd::mat4 &orn) {
  simd::vec4 mx = getVertex(pos, ext, orn, 0), mn = getVertex(pos, ext, orn, 0);

  for (uint32_t i = 1; i < 8; ++i) {
    mx = simd::max(mx, getVertex(pos, ext, orn, i));
    mn = simd::min(mn, getVertex(pos, ext, orn, i));
  }

  const simd::vec4 boxPos =          (mn + mx) / 2.0f;
  const simd::vec4 boxExt = simd::abs(mn - mx) / 2.0f;

  return {boxPos, boxExt};
}

FastAABB recalcAABB(const ArrayInterface<simd::vec4>* verticies, const simd::vec4 &pos, const uint32_t &vertexOffset, const uint32_t &vertexSize, const simd::mat4 &orn) {
  const simd::vec4 dir = pos * simd::vec4(1.0f, 1.0f, 1.0f, 0.0f);
  simd::vec4 mx = transform(verticies->at(vertexOffset), dir, orn);// orn * (dir + verticies->at(vertexOffset)); 
  simd::vec4 mn = mx;

  for (uint32_t i = 1; i < vertexSize; ++i) {
//     mx = glm::max(mx, orn * (dir + verticies->at(vertexOffset + i)));
//     mn = glm::min(mn, orn * (dir + verticies->at(vertexOffset + i)));
    const simd::vec4 transformedPos = transform(verticies->at(vertexOffset+i), dir, orn);
    
    mx = simd::max(mx, transformedPos);
    mn = simd::min(mn, transformedPos);
  }

  const simd::vec4 boxPos =          (mn + mx) / 2.0f;
  const simd::vec4 boxExt = simd::abs(mn - mx) / 2.0f;

  return {boxPos, boxExt};
}

// bool testAABB(const FastAABB &first, const FastAABB &second) {
//   const simd::vec4 center = glm::abs(first.center - second.center);
//   const simd::vec4 extent =          first.extent + second.extent;

//   return extent.x > center.x && extent.y > center.y && extent.z > center.z;
// }

bool intersection(const FastAABB &box, const RayData &ray) {
  const simd::vec4 boxMin = box.center - box.extent;
  const simd::vec4 boxMax = box.center + box.extent;
  
  const simd::vec4 t1 = (boxMin - ray.pos) / ray.dir;
  const simd::vec4 t2 = (boxMax - ray.pos) / ray.dir;
  
  float arr1[4];
  float arr2[4];
  t1.store(arr1);
  t2.store(arr2);
  
  float tmin = glm::min(arr1[0], arr2[0]);
  float tmax = glm::max(arr1[0], arr2[0]);
  
  for (uint32_t i = 1; i < 3; ++i) {
    tmin = glm::max(tmin, glm::min(arr1[i], arr2[i]));
    tmax = glm::min(tmax, glm::max(arr1[i], arr2[i]));
  }

//   float t1 = (boxMin[0] - ray.pos[0]) / ray.dir[0];
//   float t2 = (boxMax[0] - ray.pos[0]) / ray.dir[0];
// 
//   float tmin = glm::min(t1, t2);
//   float tmax = glm::max(t1, t2);
// 
//   for (int i = 1; i < 3; ++i) {
//     t1 = (boxMin[i] - ray.pos[i]) / ray.dir[i];
//     t2 = (boxMax[i] - ray.pos[i]) / ray.dir[i];
// 
//     tmin = glm::max(tmin, glm::min(t1, t2));
//     tmax = glm::min(tmax, glm::max(t1, t2));
//   }

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
//   const simd::vec4 boxMin = box.center - box.extent;
//   const simd::vec4 boxMax = box.center + box.extent;
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
    float arr[4];
    frustum.planes[i].store(arr);
    const simd::vec4 frustumPlane = simd::vec4(arr[0], arr[1], arr[2], 0.0f); //frustum.planes[i] * simd::vec4(1.0f, 1.0f, 1.0f, 0.0f);

    const float d = simd::dot(box.center,           frustumPlane);

    const float r = simd::dot(box.extent, simd::abs(frustumPlane));

    const float d_p_r = d + r;
    const float d_m_r = d - r;
    
    //frustumPlane.w
    if (d_p_r < -arr[3]) {
      result = OUTSIDE;
      break;
    } else if (d_m_r < -arr[3]) result = INTERSECT;
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

