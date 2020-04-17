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
  include/dr_libs/
  include/whereami/src/
  include/cxxopts/include/
  include/lz4/lib/
  include/libccd/src/
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
    rawShaders/particles2.comp
    rawShaders/patricles2.vert
    rawShaders/particles2.geom
    rawShaders/particles2.frag
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

IF(NOT CMAKE_BUILD_TYPE)
   SET(CMAKE_BUILD_TYPE Debug ... FORCE)
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
  utils/console_variable.cpp
  utils/Console.cpp
  utils/TimeMeter.cpp
  utils/Globals.cpp
  utils/Utility.cpp
  utils/Type.cpp
  utils/Frustum.cpp
  utils/EntityComponentSystem.cpp
  utils/random.cpp
  utils/ThreadPool.cpp
  utils/TransformComponent.cpp
  utils/settings.cpp
  include/fmt/src/format.cc
  include/fmt/src/posix.cc
  utils/FileTools.cpp
  utils/InputComponent.cpp
  utils/basic_tri.cpp
  utils/stack_allocator.cpp
  utils/attributes_component.cpp
  utils/inventory_component.cpp
  utils/abilities_component.cpp
  utils/interaction.cpp
  utils/astar_search.cpp
  utils/id.cpp
  utils/delayed_work_system.cpp
  utils/states_component.cpp
  utils/effect.cpp
  utils/effects_component.cpp
  utils/core_funcs.cpp
  utils/input.cpp
#   utils/transform_component.cpp
  include/whereami/src/whereami.c
)
set(HDR_LIST
  utils/CmdMgr.h
  utils/console_variable.h
  utils/Console.h
  utils/TimeMeter.h
  utils/Globals.h
  utils/Utility.h
  utils/Type.h
  utils/Frustum.h
  utils/EntityComponentSystem.h
  utils/random.h
  utils/ThreadPool.h
  utils/TransformComponent.h
  utils/settings.h
  include/fmt/include/fmt/format.h
  include/fmt/include/fmt/posix.h
  utils/FileTools.h
  utils/yacs.h
  utils/yacs_pool.h
  utils/yacs_component.h
  utils/InputComponent.h
  utils/TypelessContainer.h
  utils/basic_tri.h
  utils/shared_loaders_header.h
  utils/stack_allocator.h
  utils/typeless_container.h
  utils/shared_application_constant.h
  utils/shared_collision_constants.h
  utils/shared_mathematical_constant.h
  utils/shared_memory_constants.h
  utils/shared_time_constant.h
  utils/global_components_indicies.h
  utils/attributes_component.h
  utils/inventory_component.h
  utils/abilities_component.h
  utils/interaction.h
  utils/astar_search.h
  utils/id.h
  utils/delayed_work_system.h
  utils/states_component.h
  utils/effect.h
  utils/effects_component.h
  utils/core_funcs.h
  utils/input.h
  utils/attribute.h
  utils/weapon.h
  utils/state.h
  utils/resource_container.h
  utils/double_ended_stack.h
  utils/double_stack_allocator.h
  utils/ring_buffer.h
  utils/Logging.h
  include/whereami/src/whereami.h
  utils/map_data.h
#   utils/transform_component.h
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
  
#   physics/physics_component.cpp
#   physics/physics_context.cpp
# #   physics/physics.cpp
#   physics/core_parts.cpp
#   physics/broadphase_context.cpp
#   physics/broadphase_parts.cpp
#   physics/narrowphase_context.cpp
#   physics/narrowphase_parts.cpp
#   physics/solver_context.cpp
#   physics/solver_parts.cpp
#   physics/solver_helper.cpp
#   physics/rigid_body.cpp
#   physics/transform.cpp
#   physics/joint_interface.cpp
#   physics/hinge_joint.cpp
#   physics/manifold.cpp
#   physics/collision_shape.cpp
#   physics/collision_utils.cpp
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

#   physics/physics_component.h
#   physics/physics_core.h
#   physics/core_context.h
#   physics/physics_context.h
#   physics/physics.h
#   physics/phase.h
#   physics/core_parts.h
#   physics/broadphase_context.h
#   physics/broadphase_parts.h
#   physics/narrowphase_context.h
#   physics/narrowphase_parts.h
#   physics/solver_context.h
#   physics/solver_parts.h
#   physics/solver_helper.h
#   physics/rigid_body.h
#   physics/transform.h
#   physics/solver_constraint.h
#   physics/joint_interface.h
#   physics/hinge_joint.h
#   physics/manifold.h
#   physics/collision_shape.h
#   physics/collision_utils.h
)
add_library(physics ${SRC_LIST} ${HDR_LIST})
target_link_libraries(physics render utils)

# RENDER
set(SRC_LIST
#   render/VulkanRender.cpp
#   render/Optimizers.cpp
#   render/GPUOptimizers.cpp
#   render/RenderStages.cpp
#   render/Window.cpp
#   render/Deferred.cpp
#   render/GraphicsContainer.cpp
  render/render.cpp
  render/window.cpp
  render/stages.cpp
  render/targets.cpp
  render/container.cpp
  render/particles.cpp
  render/Core.cpp
  render/Tasks.cpp
  render/Internal.cpp
  render/Makers.cpp
  render/Types.cpp
  render/RAII.cpp
  render/graphics_component.cpp
  render/image_container.cpp
  bin/interface_context.cpp
)
set(HDR_LIST
#   render/Render.h
#   render/VulkanRender.h
#   render/Optimizer.h
#   render/GPUOptimizers.h
#   render/Optimizers.h
#   render/RenderStages.h
#   render/RenderStage.h
#   render/RenderStructures.h
#   render/Window.h
#   render/Deferred.h
#   render/GraphicsContainer.h
  render/render.h
  render/window.h
  render/stage.h
  render/stages.h
  render/shared_structures.h
  render/target.h
  render/targets.h
  render/context.h
  render/container.h
  render/particles.h
  render/nuklear_header.h
  render/yavf.h
  render/Core.h
  render/Tasks.h
  render/Internal.h
  render/Makers.h
  render/Types.h
  render/RAII.h
  render/graphics_component.h
  render/image_container.h
  include/VulkanMemoryAllocator/src/vk_mem_alloc.h
  render/vulkan_lite.h
  bin/interface_context.h
)
add_library(render ${SRC_LIST} ${HDR_LIST})
target_link_libraries(render utils ${VULKAN_LIB_NAME} ${GLFW3_LIBRARIES})

# SOUND
set(SRC_LIST 
  sound/alHelpers.cpp
  sound/Buffer.cpp
  sound/Source.cpp
  sound/SoundDecoder.cpp
  sound/sound_data.cpp
  sound/sound_loader.cpp
  sound/sound_system.cpp
)
set(HDR_LIST 
  sound/alHelpers.h
  sound/Buffer.h
  sound/Source.h
  sound/SoundDecoder.h
  sound/sound_data.h
  sound/sound_loader.h
  sound/sound_system.h
  sound/sound_info.h
)
add_library(sound ${SRC_LIST} ${HDR_LIST})
target_link_libraries(sound utils openal)

# AI
set(SRC_LIST 
  ai/ai_component.cpp
  ai/graph.cpp
  ai/vertex_component.cpp
  ai/pathfinder_system.cpp
  ai/movement_component.cpp
)
set(HDR_LIST 
  ai/ai_component.h
  ai/graph.h
  ai/vertex_component.h
  ai/pathfinder_system.h
  ai/movement_component.h
  ai/gheap.hpp
)
add_library(ai ${SRC_LIST} ${HDR_LIST})
target_link_libraries(ai utils physics ccd)

# RESOURCES
set(SRC_LIST
  resources/abilities_loader.cpp
  resources/attributes_loader.cpp
  resources/image_loader.cpp
  resources/state_loader.cpp
  resources/entity_loader.cpp
  resources/entity_creator.cpp
  resources/modification_container.cpp
  resources/effects_loader.cpp
#   resources/HardcodedLoaders.cpp
  resources/map_loader.cpp
)
set(HDR_LIST
  resources/resource.h
  resources/resource_parser.h
  resources/game_resources.h
  resources/abilities_loader.h
  resources/attributes_loader.h
  resources/image_loader.h
  resources/state_loader.h
  resources/entity_loader.h
  resources/entity_creator.h
  resources/entity_creator_resources.h
  resources/modification_container.h
  resources/effects_loader.h
#   resources/HardcodedLoaders.h
  resources/map_loader.h
)
add_library(resources ${SRC_LIST} ${HDR_LIST})
target_link_libraries(resources ai sound render physics utils cppfs)
# нужно ли лоадеры пихать в ресурсы? или оставить в других библиотеках?

# BINARY
set(SRC_LIST
  bin/interfaces.cpp
  bin/overlay.cpp
  bin/Helper.cpp
  bin/main.cpp
  bin/camera.cpp
  bin/game_funcs.cpp
  bin/collision_interaction_system.cpp
  bin/post_physics.cpp
)
set(HDR_LIST
  bin/interface.h
  bin/interfaces.h
  bin/overlay.h
  bin/Helper.h
  bin/camera.h
  bin/game_funcs.h
  bin/collision_interaction_system.h
  bin/post_physics.h
)
add_executable(${TARGET_NAME} ${SRC_LIST} ${HDR_LIST})
target_link_libraries(${TARGET_NAME} ai sound render physics resources utils)
add_dependencies(${TARGET_NAME} shaders project_resources)

