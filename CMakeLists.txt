# указываем минимальную версию CMAKE, а также название проекта и тип билда
cmake_minimum_required(VERSION 3.0)
project(testingApp)

set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}/${PROJECT_NAME})
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}/${PROJECT_NAME})

set(TARGET_NAME VulkanTest)

find_package(git QUIET)
if(GIT_FOUND AND EXISTS "${PROJECT_SOURCE_DIR}/.git")
# Update submodules as needed
  option(GIT_SUBMODULE "Check submodules during build" ON)
  if(GIT_SUBMODULE)
    message(STATUS "Submodule update")
    execute_process(COMMAND ${GIT_EXECUTABLE} submodule update --init --recursive
                    WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
                    RESULT_VARIABLE GIT_SUBMOD_RESULT)
    if(NOT GIT_SUBMOD_RESULT EQUAL "0")
      message(FATAL_ERROR "git submodule update --init failed with ${GIT_SUBMOD_RESULT}, please checkout submodules")
    endif()
  endif()
endif()

if(NOT EXISTS "${PROJECT_SOURCE_DIR}/include/fmt/CMakeLists.txt")
  message(FATAL_ERROR "The submodules were not downloaded! GIT_SUBMODULE was turned off or failed. Please update submodules and try again.")
endif()

set(STB_INCLUDE_PATH include/stb)
set(TINYOBJ_INCLUDE_PATH include/tinyobjloader)

# директории, где компилятор ищет хедеры
include_directories(
  bin/
  render/
  physics/
  utils/
  sound/
  ai/
  resources/
  include/
  include/fmt/include/
  include/json/include/
  include/VulkanMemoryAllocator/src/
  include/glm/
  include/cppfs/source/cppfs/include
  include/filesystem/
  ${STB_INCLUDE_PATH}
  ${TINYOBJ_INCLUDE_PATH}
  #                     /home/mikuhatsune/gwc/testingApp/include/libleaktracer/include
  #                     include/imgui/
  include/TinyBehavior/
)

link_directories(${CMAKE_CURRENT_BINARY_DIR} ${CMAKE_LIBRARY_OUTPUT_DIRECTORY}) # директории, где компилятор ищет библиотечки

if (WIN32)
  set(CPPFS_LIB_PATH C:/libs/cppfs)

  set(GLFW3_LIB_DIRS C:/libs/glfw-3.2.1.bin.WIN64/lib-mingw-w64)
  set(GLFW3_INCLUDE_DIRS C:/libs/glfw-3.2.1.bin.WIN64/include)
  set(GLFW3_LIBRARIES glfw3)

  set(VULKAN_SDK_PATH C:/VulkanSDK/1.1.97.0)
  set(VULKAN_LIB_PATH ${VULKAN_SDK_PATH}/Lib)
  set(VULKAN_INCLUDE_PATH ${VULKAN_SDK_PATH}/Include)
  set(GLSL_VALIDATOR "${VULKAN_SDK_PATH}/Bin/glslangValidator")
  set(VULKAN_LIB_NAME vulkan-1)

  include_directories(
    ${GLFW3_INCLUDE_DIRS}
	  ${VULKAN_INCLUDE_PATH}
	  ${GLM_LIB_PATH}
  )

  link_directories(
	  ${GLFW3_LIB_DIRS}
	  ${VULKAN_LIB_PATH}
    ${CPPFS_LIB_PATH}
  )
else()
  set(CPPFS_LIB_PATH libs)

  # находим указанные зависимости (нужно чекнуть все ли это зависимости?)
  # вместо glm у меня установлен gli
  #find_package(glm)
  find_package(glfw3)
  find_package(PkgConfig REQUIRED)
  #pkg_search_module(GLFW REQUIRED glfw3)
  pkg_check_modules(GLFW3 REQUIRED glfw3)
  # не может найти почему то vulkan-headers
  #find_package(vulkan-headers)

  set(VULKAN_LIB_NAME vulkan)
  set(GLSL_VALIDATOR "glslangValidator")

  include_directories(
    ${GLFW3_INCLUDE_DIRS}
  )
  
  link_directories(
    ${CPPFS_LIB_PATH}
  )
endif()

set(COMPILED_SHADERS_PATH ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/shaders)

file(MAKE_DIRECTORY ${COMPILED_SHADERS_PATH})

file(GLOB_RECURSE GLSL_SOURCE_FILES
#     "rawShaders/*.vert"
#     "rawShaders/*.frag"
#     "rawShaders/*.comp"
    rawShaders/simple.vert
    rawShaders/simple.frag
    rawShaders/simpleUV.vert
    rawShaders/simpleUV.frag
    rawShaders/simpleAABB.vert
    rawShaders/simpleAABB.frag
    rawShaders/simplePoly.vert
    rawShaders/simplePoly.frag
    rawShaders/gui.vert
    rawShaders/gui.frag
    rawShaders/occlusion.vert
    rawShaders/occlusion.frag
    rawShaders/occludee.vert
    rawShaders/occlusion2.frag
    rawShaders/deferred.vert
    rawShaders/deferred.frag
    rawShaders/deferredObj.vert
    rawShaders/deferredObj.frag
    rawShaders/tiling.comp
    rawShaders/toneMapping.comp
    rawShaders/calcRotationMatrix.comp
    rawShaders/velocity.comp
    rawShaders/recalculateAABB.comp
    rawShaders/updateOctree.comp
    rawShaders/preUpdateNodeIdx.comp
    rawShaders/updateNodeIdx.comp
    rawShaders/updateNodeIdx2.comp
    rawShaders/octree.comp
    rawShaders/octree3.comp
    rawShaders/pairsToPowerOfTwo.comp
    rawShaders/octreeRay.comp
    rawShaders/octreeFrustum.comp
    rawShaders/octreeFrustum2.comp
    rawShaders/iterativeFrustum.comp
    rawShaders/frustumPowerOfTwo.comp
    rawShaders/islands.comp
    rawShaders/batching.comp
    rawShaders/checkSamePairs.comp
    rawShaders/islands2.comp
    rawShaders/batching2.comp
    rawShaders/checkSamePairs2.comp
    rawShaders/computeIslandsSize.comp
    rawShaders/sorting.comp
    rawShaders/sortingOverlapping1.comp
    rawShaders/sortingOverlapping2.comp
    rawShaders/posRecalc.comp
    rawShaders/calcOverlappingDataToSolver.comp
    rawShaders/newSolver.comp
    rawShaders/solver2.comp
    rawShaders/searchAndAddPair.comp
    rawShaders/calcOverlappingData.comp
    rawShaders/calcRayIntersect.comp
    rawShaders/particles.comp
    rawShaders/particlesSorting.comp
    rawShaders/particles.vert
    rawShaders/particles.geom
    rawShaders/particles.frag
    rawShaders/monsterOptimizer.comp
    rawShaders/geometryOptimizer.comp
    )

foreach(GLSL ${GLSL_SOURCE_FILES})
  get_filename_component(FILE_NAME ${GLSL} NAME_WE)
  get_filename_component(FILE_EXT ${GLSL} EXT)
  set(SPIRV "${COMPILED_SHADERS_PATH}/${FILE_NAME}${FILE_EXT}.spv")
  add_custom_command(
    OUTPUT ${SPIRV}
#     COMMAND ${CMAKE_COMMAND} -E make_directory "${COMPILED_SHADERS_PATH}"
    COMMAND ${GLSL_VALIDATOR} -V ${GLSL} -o ${SPIRV}
    DEPENDS ${GLSL})
  list(APPEND SPIRV_BINARY_FILES ${SPIRV})
endforeach(GLSL)

add_custom_target(
    shaders
    DEPENDS ${SPIRV_BINARY_FILES}
    )

set(RESOURCE_DIR ${CMAKE_SOURCE_DIR}/project_resources)
set(RESOURCE_DEST ${CMAKE_RUNTIME_OUTPUT_DIRECTORY})

file(GLOB PROJECT_RESOURCE_FILES
     "${RESOURCE_DIR}/models"
     "${RESOURCE_DIR}/tmrdata"
     "${RESOURCE_DIR}/settings.json"
     )
foreach(RESOURCE ${PROJECT_RESOURCE_FILES})
  get_filename_component(filename ${RESOURCE} NAME)
  get_filename_component(dir ${RESOURCE} DIRECTORY)
  get_filename_component(dirname ${dir} NAME)
  get_filename_component(sourcedirname ${RESOURCE_DIR} NAME)

  set(output ${RESOURCE_DEST}/${filename})

  if(IS_DIRECTORY ${RESOURCE})
    add_custom_command(
      COMMENT "Moving updated resource-file '${filename}'"
      OUTPUT ${output}
      DEPENDS ${RESOURCE}
      COMMAND ${CMAKE_COMMAND} -E copy_directory
      ${RESOURCE}
      ${output}
    )
  else()
    add_custom_command(
      COMMENT "Moving updated resource-file '${filename}'"
      OUTPUT ${output}
      DEPENDS ${RESOURCE}
      COMMAND ${CMAKE_COMMAND} -E copy
      ${RESOURCE}
      ${output}
    )
  endif()
#copy_if_different

  list(APPEND COPYING_RESOURCES ${output})
endforeach()

add_custom_target(
  project_resources
  DEPENDS ${COPYING_RESOURCES}
)

IF( NOT CMAKE_BUILD_TYPE )
   SET( CMAKE_BUILD_TYPE Debug ... FORCE )
ENDIF()

if (CMAKE_BUILD_TYPE MATCHES "Debug")
  add_definitions(-D_DEBUG) # пользовательские флаги
  #-fsanitize=address -fsanitize=pointer-compare -fsanitize=pointer-subtract -fsanitize=undefined
  set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -msse4.2 -std=c++11 -Wall -Wextra -g")
else()
  set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -msse4.2 -std=c++11 -Wall -Wextra -O3 -s")
endif (CMAKE_BUILD_TYPE MATCHES "Debug")

# указываем куда мы будем компилировать файлы, также некоторые настройки
SET(BUILD_SHARED_LIBS ON) # все библиотеки по дефолту компилятся как динамические

# начинаем компилировать (создаем цели компиляции)

# UTILS
set(SRC_LIST
  utils/CmdMgr.cpp
  utils/Variable.cpp
  utils/Console.cpp
  utils/TimeMeter.cpp
  utils/AbilityType.cpp
  utils/ItemType.cpp
  utils/Globals.cpp
  utils/Utility.cpp
  utils/Event.cpp
  utils/EventComponent.cpp
  utils/Effect.cpp
  utils/UniqueID.cpp
  utils/Type.cpp
  utils/Frustum.cpp
  utils/ResourceID.cpp
  utils/EntityComponentSystem.cpp
  #              utils/JobSystem.cpp
  utils/Random.cpp
  #              utils/State.cpp
  utils/ThreadPool.cpp
  utils/TransformComponent.cpp
  resources/Settings.cpp
  include/fmt/src/format.cc
  include/fmt/src/posix.cc
  #              utils/ResourceManager.cpp
  utils/FileTools.cpp
  utils/InputComponent.cpp
  utils/Attributes.cpp
  bin/StateController.cpp
  utils/Interaction.cpp
  utils/StateControllerType.cpp
  utils/basic_tri.cpp
  utils/DelayedWorkSystem.cpp
  utils/InventoryComponent.cpp
  utils/stack_allocator.cpp
)
set(HDR_LIST
  utils/CmdMgr.h
  utils/Variable.h
  utils/Console.h
  utils/TimeMeter.h
  utils/AbilityType.h
  utils/ItemType.h
  utils/Globals.h
  utils/Utility.h
  utils/Event.h
  utils/EventComponent.h
  utils/Effect.h
  utils/UniqueID.h
  utils/Type.h
  utils/Frustum.h
  utils/ResourceID.h
  utils/EntityComponentSystem.h
  #              utils/JobSystem.h
  utils/Random.h
  #              utils/State.h
  utils/ThreadPool.h
  utils/TransformComponent.h
  resources/Settings.h
  include/fmt/include/fmt/format.h
  include/fmt/include/fmt/posix.h
  #              utils/ResourceManager.h
  utils/FileTools.h
  utils/yacs.h
  utils/yacs_pool.h
  utils/yacs_component.h
  utils/InputComponent.h
  utils/TypelessContainer.h
  utils/Transition.h
  utils/Attributes.h
  utils/EventFunctor.h
  utils/InventoryComponent.h
  utils/Interaction.h
  utils/StateControllerType.h
  utils/basic_tri.h
  utils/shared_loaders_header.h
  utils/DelayedWorkSystem.h
  utils/stack_allocator.h
)
add_library(utils ${SRC_LIST} ${HDR_LIST}) # именно add_library объявляет цель в cmake (затем это имя мы используем как переменную в других функциях)
target_link_libraries(utils) # например здесь fmt

# PHYSICS
set(SRC_LIST
  physics/PhysicsUtils.cpp
  physics/Physics.cpp
  physics/BroadphaseInterface.cpp
  #              physics/GPUPhysics.cpp
  physics/CPUPhysicsParallel.cpp
  #              physics/GPUOctreeBroadphase.cpp
  #              physics/CPUOctreeBroadphase.cpp
  physics/CPUOctreeBroadphaseParallel.cpp
  #              physics/GPUNarrowphase.cpp
  #              physics/CPUNarrowphase.cpp
  physics/CPUNarrowphaseParallel.cpp
  #              physics/GPUSolver.cpp
  #              physics/CPUSolver.cpp
  physics/CPUSolverParallel.cpp
  #              physics/GPUPhysicsSorter.cpp
  physics/CPUPhysicsSorter.cpp
  physics/HelperFunctions.cpp
#              physics/Graph.cpp
  physics/PhysicsComponent.cpp
)
set(HDR_LIST
  physics/PhysicsUtils.h
  physics/Physics.h
  physics/BroadphaseInterface.h
  #              physics/GPUPhysics.h
  physics/CPUPhysicsParallel.h
  #              physics/GPUOctreeBroadphase.h
  #              physics/CPUOctreeBroadphase.h
  physics/CPUOctreeBroadphaseParallel.h
  physics/ArrayInterface.h
  physics/GPUArray.h
  physics/NarrowphaseInterface.h
  #              physics/GPUNarrowphase.h
  #              physics/CPUNarrowphase.h
  physics/CPUNarrowphaseParallel.h
  physics/Solver.h
  #              physics/GPUSolver.h
  #              physics/CPUSolver.h
  physics/CPUSolverParallel.h
  physics/PhysicsSorter.h
  #              physics/GPUPhysicsSorter.h
  physics/CPUPhysicsSorter.h
  physics/HelperFunctions.h
  physics/PhysicsComponent.h
#              physics/Graph.h
)
add_library(physics ${SRC_LIST} ${HDR_LIST})
target_link_libraries(physics render utils)

# RENDER
set(SRC_LIST
  render/VulkanRender.cpp
  render/Optimizers.cpp
  render/GPUOptimizers.cpp
  render/RenderStages.cpp
#   render/RenderStructures.cpp
  render/GraphicComponets.cpp
  # render/GraphicComponets.cpp
  render/Window.cpp
  render/Deferred.cpp
  render/GraphicsContainer.cpp
  #              render/Occlusion.cpp
  render/Core.cpp
  render/Tasks.cpp
  render/Internal.cpp
  render/Makers.cpp
  render/Types.cpp
  render/RAII.cpp
#  render/DecalSystem.cpp
#  render/DecalOptimizer.cpp
#  render/DecalComponent.cpp
#  render/ParticleSystem.cpp
#  render/ParticleEmitterComponent.cpp
#              render/TextureManager.cpp
#              render/stbi_image_impl.cpp
)
set(HDR_LIST
  render/Render.h
  render/VulkanRender.h
  render/Optimizer.h
  render/GPUOptimizers.h
  render/Optimizers.h
  render/RenderStages.h
  render/RenderStage.h
  render/RenderStructures.h
  render/GraphicComponets.h
  # render/GraphicComponets.h
  render/Window.h
  render/Deferred.h
  render/GraphicsContainer.h
  render/nuklear_header.h
  #              render/Occlusion.h
  render/yavf.h
  render/Core.h
  render/Tasks.h
  render/Internal.h
  render/Makers.h
  render/Types.h
  render/RAII.h
#  render/DecalSystem.h
#  render/DecalOptimizer.h
#  render/DecalComponent.h
#  render/ParticleSystem.h
#  render/ParticleEmitterComponent.h
#  render/Emitter.h
  #              render/vk_mem_alloc.h
  include/VulkanMemoryAllocator/src/vk_mem_alloc.h
#              render/TextureManager.h
#              render/stbi_image_header.h
  render/vulkan_lite.h)
add_library(render ${SRC_LIST} ${HDR_LIST})
target_link_libraries(render utils ${VULKAN_LIB_NAME} ${GLFW3_LIBRARIES})

# SOUND
set(SRC_LIST 
  sound/alHelpers.cpp
  sound/Buffer.cpp
  sound/Source.cpp
  sound/SoundData.cpp
  sound/SoundSystem.cpp
  sound/SoundComponent.cpp
  sound/SoundDecoder.cpp
  sound/SoundLoader.cpp
)
set(HDR_LIST 
  sound/alHelpers.h
  sound/Buffer.h
  sound/Source.h
  sound/SoundData.h
  sound/SoundSystem.h
  sound/SoundComponent.h
  sound/SoundDecoder.h
  sound/SoundLoader.h
)
add_library(sound ${SRC_LIST} ${HDR_LIST})
target_link_libraries(sound utils openal)

# AI
set(SRC_LIST 
  ai/EntityAI.cpp
  ai/AIComponent.cpp
  ai/Graph.cpp
  ai/CPUPathFindingPhaseParallel.cpp
  ai/AISystem.cpp
  ai/AStarSearch.cpp
  ai/AIInputComponent.cpp
  ai/MovementComponent.cpp
)
set(HDR_LIST 
  ai/EntityAI.h
  ai/AIComponent.h
  ai/Graph.h
  ai/CPUPathFindingPhaseParallel.h
  ai/AISystem.h
  ai/AStarSearch.h
  ai/MoveEventData.h
  ai/PathFindingPhase.h
  ai/AIInputComponent.h
  ai/MovementComponent.h
)
add_library(ai ${SRC_LIST} ${HDR_LIST})
target_link_libraries(ai utils physics)

# RESOURCES
set(SRC_LIST
  resources/Conflict.cpp
  resources/Resource.cpp
  resources/Modification.cpp
  resources/HardcodedLoaders.cpp
#  resources/ResourceManager.cpp
  resources/ModificationContainer.cpp
#   resources/ComponentCreators.cpp
#   resources/ComponentCreator.cpp
  resources/ImageLoader.cpp
  resources/AnimationLoader.cpp
  resources/AttributesLoader.cpp
  resources/EffectsLoader.cpp
  resources/AbilityTypeLoader.cpp
  resources/ItemLoader.cpp
  resources/EntityCreators.cpp
  resources/EntityLoader.cpp
)
set(HDR_LIST
  resources/ResourceParser.h
  resources/Resource.h
  resources/Conflict.h
  resources/Modification.h
  resources/ModificationParser.h
  resources/HardcodedLoaders.h
#  resources/ResourceManager.h
  resources/ModificationContainer.h
#   resources/ComponentCreators.h
#   resources/ComponentCreator.h
  resources/ImageLoader.h
  resources/AnimationLoader.h
  resources/AttributesLoader.h
  resources/EffectsLoader.h
  resources/AbilityTypeLoader.h
  resources/ItemLoader.h
  resources/EntityCreators.h
  resources/EntityLoader.h
)
add_library(resources ${SRC_LIST} ${HDR_LIST})
target_link_libraries(resources ai sound render physics utils cppfs)
# нужно ли лоадеры пихать в ресурсы? или оставить в других библиотеках?

# BINARY
set(SRC_LIST
  bin/AnimationComponent.cpp
  bin/AttributesComponent.cpp
  bin/EffectComponent.cpp
#   bin/InteractionComponent.cpp
  bin/CPUAnimationSystemParallel.cpp
  bin/AnimationSystem.cpp
  bin/PostPhysics.cpp
#   bin/MenuItems.cpp
#   bin/Menu.cpp
  bin/Interfaces.cpp
  bin/overlay.cpp
  bin/Helper.cpp
  bin/main.cpp
  bin/CameraComponent.cpp
  bin/InfoComponent.cpp
  bin/Interactions.cpp 
#   bin/SkillComponent.cpp
#   utils/InventoryComponent.cpp
)
set(HDR_LIST
  bin/AttributesComponent.h
  bin/EffectComponent.h
#   bin/InteractionComponent.h
  bin/AnimationComponent.h
  bin/CPUAnimationSystemParallel.h
  bin/AnimationSystem.h
  bin/PostPhysics.h
#   bin/MenuItems.h
#   bin/Menu.h
  bin/Interface.h
  bin/Interfaces.h
  bin/overlay.h
  bin/Helper.h
  bin/CameraComponent.h
  bin/InfoComponent.h
  bin/UserDataComponent.h
  bin/StateController.h
#   bin/SkillComponent.h 
  bin/Interactions.h
#   utils/InventoryComponent.h
)
add_executable(${TARGET_NAME} ${SRC_LIST} ${HDR_LIST})
target_link_libraries(${TARGET_NAME} ai sound render physics resources utils)
add_dependencies(${TARGET_NAME} shaders project_resources)
