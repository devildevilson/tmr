glslangValidator -V rawShaders/simple.vert -o build/shaders/vert.spv
glslangValidator -V rawShaders/simple.frag -o build/shaders/frag.spv

#glslangValidator -V rawShaders/simpleImage.vert -o build/shaders/vertimg.spv

#glslangValidator -V rawShaders/billboard.vert -o build/shaders/billboardVert.spv

glslangValidator -V rawShaders/simpleUV.vert -o build/shaders/vertUV.spv
glslangValidator -V rawShaders/simpleUV.frag -o build/shaders/fragUV.spv

#glslangValidator -V rawShaders/text.vert -o build/shaders/textVert.spv
#glslangValidator -V rawShaders/text.frag -o build/shaders/textFrag.spv

glslangValidator -V rawShaders/simpleAABB.vert -o build/shaders/vertAABB.spv
glslangValidator -V rawShaders/simpleAABB.frag -o build/shaders/fragAABB.spv

glslangValidator -V rawShaders/gui.vert -o build/shaders/vertGui.spv
glslangValidator -V rawShaders/gui.frag -o build/shaders/fragGui.spv

glslangValidator -V rawShaders/occlusion.vert -o build/shaders/occluder.spv
glslangValidator -V rawShaders/occludee.vert -o build/shaders/occludee.spv
glslangValidator -V rawShaders/occlusion.frag -o build/shaders/occlusionTest.spv
glslangValidator -V rawShaders/occlusion2.frag -o build/shaders/occlusionTest2.spv

#glslangValidator -V rawShaders/map.vert -o build/shaders/mapVert.spv
#glslangValidator -V rawShaders/map.frag -o build/shaders/mapFrag.spv

#glslangValidator -V rawShaders/obj.vert -o build/shaders/objVert.spv
#glslangValidator -V rawShaders/obj.frag -o build/shaders/objFrag.spv

glslangValidator -V rawShaders/deferred.vert -o build/shaders/deferredVert.spv
glslangValidator -V rawShaders/deferred.frag -o build/shaders/deferredFrag.spv

glslangValidator -V rawShaders/deferredObj.vert -o build/shaders/deferredObjVert.spv
glslangValidator -V rawShaders/deferredObj.frag -o build/shaders/deferredObjFrag.spv

glslangValidator -V rawShaders/tiling.comp -o build/shaders/tilingComp.spv
glslangValidator -V rawShaders/toneMapping.comp -o build/shaders/toneMappingComp.spv

glslangValidator -V rawShaders/calcRotationMatrix.comp -o build/shaders/calcRotationMatrix.spv
glslangValidator -V rawShaders/velocity.comp -o build/shaders/velocity.spv

glslangValidator -V rawShaders/recalculateAABB.comp -o build/shaders/recalculateAABB.spv
glslangValidator -V rawShaders/updateOctree.comp -o build/shaders/updateOctree.spv
glslangValidator -V rawShaders/preUpdateNodeIdx.comp -o build/shaders/preUpdateNodeIdx.spv
glslangValidator -V rawShaders/updateNodeIdx.comp -o build/shaders/updateNodeIdx.spv
glslangValidator -V rawShaders/updateNodeIdx2.comp -o build/shaders/updateNodeIdx2.spv

#glslangValidator -V rawShaders/octree.comp -o build/shaders/octree.spv

#glslc -Os --target-env=vulkan rawShaders/velocity.comp -o build/shaders/velocity.spv 

#glslc -Os --target-env=vulkan rawShaders/recalculateAABB.comp -o build/shaders/recalculateAABB.spv
#glslc --target-env=vulkan rawShaders/updateOctree.comp -o build/shaders/updateOctree.spv
#glslc -Os --target-env=vulkan rawShaders/updateNodeIdx.comp -o build/shaders/updateNodeIdx.spv

#glslc -S -Os --target-env=vulkan rawShaders/updateOctree.comp -o build/shaders/updateOctree.txt

#glslc --target-env=vulkan rawShaders/octree.comp -o build/shaders/octree.spv

#glslc --target-env=vulkan rawShaders/islands.comp -o build/shaders/islands.spv
#glslc --target-env=vulkan rawShaders/sorting.comp -o build/shaders/sorting.spv
#glslc --target-env=vulkan rawShaders/computeIslandsSize.comp -o build/shaders/computeIslandsSize.spv

glslangValidator -V rawShaders/octree.comp -o build/shaders/octree.spv
glslangValidator -V rawShaders/octree3.comp -o build/shaders/octree3.spv
glslangValidator -V rawShaders/pairsToPowerOfTwo.comp -o build/shaders/pairsToPowerOfTwo.spv
glslangValidator -V rawShaders/octreeRay.comp -o build/shaders/octreeRay.spv
glslangValidator -V rawShaders/octreeFrustum.comp -o build/shaders/octreeFrustum.spv
glslangValidator -V rawShaders/octreeFrustum2.comp -o build/shaders/octreeFrustum2.spv
glslangValidator -V rawShaders/iterativeFrustum.comp -o build/shaders/iterativeFrustum.spv
glslangValidator -V rawShaders/frustumPowerOfTwo.comp -o build/shaders/frustumPowerOfTwo.spv

glslangValidator -V rawShaders/islands.comp -o build/shaders/islands.spv
glslangValidator -V rawShaders/batching.comp -o build/shaders/batching.spv
glslangValidator -V rawShaders/checkSamePairs.comp -o build/shaders/checkSamePairs.spv
glslangValidator -V rawShaders/islands2.comp -o build/shaders/islands2.spv
glslangValidator -V rawShaders/batching2.comp -o build/shaders/batching2.spv
glslangValidator -V rawShaders/checkSamePairs2.comp -o build/shaders/checkSamePairs2.spv
glslangValidator -V rawShaders/computeIslandsSize.comp -o build/shaders/computeIslandsSize.spv

glslangValidator -V rawShaders/sorting.comp -o build/shaders/sorting.spv
glslangValidator -V rawShaders/sortingOverlapping1.comp -o build/shaders/sortingOverlapping1.spv
glslangValidator -V rawShaders/sortingOverlapping2.comp -o build/shaders/sortingOverlapping2.spv

#glslangValidator -V rawShaders/solver.comp -o build/shaders/solver.spv
glslangValidator -V rawShaders/posRecalc.comp -o build/shaders/posRecalc.spv
glslangValidator -V rawShaders/calcOverlappingDataToSolver.comp -o build/shaders/calcOverlappingDataToSolver.spv
glslangValidator -V rawShaders/newSolver.comp -o build/shaders/newSolver.spv
glslangValidator -V rawShaders/solver2.comp -o build/shaders/solver2.spv
glslangValidator -V rawShaders/searchAndAddPair.comp -o build/shaders/searchAndAddPair.spv
glslangValidator -V rawShaders/calcOverlappingData.comp -o build/shaders/calcOverlappingData.spv
glslangValidator -V rawShaders/calcRayIntersect.comp -o build/shaders/calcRayIntersect.spv

#spirv-cross --output test.cpp build/shaders/updateOctree.spv --cpp
#glslangValidator -V rawShaders/velocity.comp -o build/shaders/velocity.spv 

#glslangValidator -V rawShaders/fullscreenquad.vert -o build/shaders/fullscreenquadVert.spv
#glslangValidator -V rawShaders/fullscreenquad.frag -o build/shaders/fullscreenquadFrag.spv

#glslangValidator -V rawShaders/octree.comp -o build/shaders/octreeComp.spv
