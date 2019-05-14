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

set(STB_INCLUDE_PATH ${CMAKE_CURRENT_SOURCE_DIR}/include/stb)
set(TINYOBJ_INCLUDE_PATH ${CMAKE_CURRENT_SOURCE_DIR}/include/tinyobjloader)

# директории, где компилятор ищет хедеры
include_directories(
  ${CMAKE_CURRENT_SOURCE_DIR}/bin/
  ${CMAKE_CURRENT_SOURCE_DIR}/render/
  ${CMAKE_CURRENT_SOURCE_DIR}/physics/
  ${CMAKE_CURRENT_SOURCE_DIR}/utils/
  ${CMAKE_CURRENT_SOURCE_DIR}/include/
  ${CMAKE_CURRENT_SOURCE_DIR}/include/fmt/include/
  ${CMAKE_CURRENT_SOURCE_DIR}/include/json/include/
  ${CMAKE_CURRENT_SOURCE_DIR}/include/VulkanMemoryAllocator/src/
  ${CMAKE_CURRENT_SOURCE_DIR}/include/glm/
  ${CMAKE_CURRENT_SOURCE_DIR}/include/cppfs/source/cppfs/include
  ${STB_INCLUDE_PATH}
  ${TINYOBJ_INCLUDE_PATH}
  #                     /home/mikuhatsune/gwc/testingApp/include/libleaktracer/include
  #                     ${CMAKE_CURRENT_SOURCE_DIR}/include/imgui/
  ${CMAKE_CURRENT_SOURCE_DIR}/include/TinyBehavior/
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
  set(CPPFS_LIB_PATH ${CMAKE_CURRENT_SOURCE_DIR}/libs)

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
#     "${CMAKE_CURRENT_SOURCE_DIR}/rawShaders/*.vert"
#     "${CMAKE_CURRENT_SOURCE_DIR}/rawShaders/*.frag"
#     "${CMAKE_CURRENT_SOURCE_DIR}/rawShaders/*.comp"
    ${CMAKE_CURRENT_SOURCE_DIR}/rawShaders/simple.vert
    ${CMAKE_CURRENT_SOURCE_DIR}/rawShaders/simple.frag
    ${CMAKE_CURRENT_SOURCE_DIR}/rawShaders/simpleUV.vert
    ${CMAKE_CURRENT_SOURCE_DIR}/rawShaders/simpleUV.frag
    ${CMAKE_CURRENT_SOURCE_DIR}/rawShaders/simpleAABB.vert
    ${CMAKE_CURRENT_SOURCE_DIR}/rawShaders/simpleAABB.frag
    ${CMAKE_CURRENT_SOURCE_DIR}/rawShaders/simplePoly.vert
    ${CMAKE_CURRENT_SOURCE_DIR}/rawShaders/simplePoly.frag
    ${CMAKE_CURRENT_SOURCE_DIR}/rawShaders/gui.vert
    ${CMAKE_CURRENT_SOURCE_DIR}/rawShaders/gui.frag
    ${CMAKE_CURRENT_SOURCE_DIR}/rawShaders/occlusion.vert
    ${CMAKE_CURRENT_SOURCE_DIR}/rawShaders/occlusion.frag
    ${CMAKE_CURRENT_SOURCE_DIR}/rawShaders/occludee.vert
    ${CMAKE_CURRENT_SOURCE_DIR}/rawShaders/occlusion2.frag
    ${CMAKE_CURRENT_SOURCE_DIR}/rawShaders/deferred.vert
    ${CMAKE_CURRENT_SOURCE_DIR}/rawShaders/deferred.frag
    ${CMAKE_CURRENT_SOURCE_DIR}/rawShaders/deferredObj.vert
    ${CMAKE_CURRENT_SOURCE_DIR}/rawShaders/deferredObj.frag
    ${CMAKE_CURRENT_SOURCE_DIR}/rawShaders/tiling.comp
    ${CMAKE_CURRENT_SOURCE_DIR}/rawShaders/toneMapping.comp
    ${CMAKE_CURRENT_SOURCE_DIR}/rawShaders/calcRotationMatrix.comp
    ${CMAKE_CURRENT_SOURCE_DIR}/rawShaders/velocity.comp
    ${CMAKE_CURRENT_SOURCE_DIR}/rawShaders/recalculateAABB.comp
    ${CMAKE_CURRENT_SOURCE_DIR}/rawShaders/updateOctree.comp
    ${CMAKE_CURRENT_SOURCE_DIR}/rawShaders/preUpdateNodeIdx.comp
    ${CMAKE_CURRENT_SOURCE_DIR}/rawShaders/updateNodeIdx.comp
    ${CMAKE_CURRENT_SOURCE_DIR}/rawShaders/updateNodeIdx2.comp
    ${CMAKE_CURRENT_SOURCE_DIR}/rawShaders/octree.comp
    ${CMAKE_CURRENT_SOURCE_DIR}/rawShaders/octree3.comp
    ${CMAKE_CURRENT_SOURCE_DIR}/rawShaders/pairsToPowerOfTwo.comp
    ${CMAKE_CURRENT_SOURCE_DIR}/rawShaders/octreeRay.comp
    ${CMAKE_CURRENT_SOURCE_DIR}/rawShaders/octreeFrustum.comp
    ${CMAKE_CURRENT_SOURCE_DIR}/rawShaders/octreeFrustum2.comp
    ${CMAKE_CURRENT_SOURCE_DIR}/rawShaders/iterativeFrustum.comp
    ${CMAKE_CURRENT_SOURCE_DIR}/rawShaders/frustumPowerOfTwo.comp
    ${CMAKE_CURRENT_SOURCE_DIR}/rawShaders/islands.comp
    ${CMAKE_CURRENT_SOURCE_DIR}/rawShaders/batching.comp
    ${CMAKE_CURRENT_SOURCE_DIR}/rawShaders/checkSamePairs.comp
    ${CMAKE_CURRENT_SOURCE_DIR}/rawShaders/islands2.comp
    ${CMAKE_CURRENT_SOURCE_DIR}/rawShaders/batching2.comp
    ${CMAKE_CURRENT_SOURCE_DIR}/rawShaders/checkSamePairs2.comp
    ${CMAKE_CURRENT_SOURCE_DIR}/rawShaders/computeIslandsSize.comp
    ${CMAKE_CURRENT_SOURCE_DIR}/rawShaders/sorting.comp
    ${CMAKE_CURRENT_SOURCE_DIR}/rawShaders/sortingOverlapping1.comp
    ${CMAKE_CURRENT_SOURCE_DIR}/rawShaders/sortingOverlapping2.comp
    ${CMAKE_CURRENT_SOURCE_DIR}/rawShaders/posRecalc.comp
    ${CMAKE_CURRENT_SOURCE_DIR}/rawShaders/calcOverlappingDataToSolver.comp
    ${CMAKE_CURRENT_SOURCE_DIR}/rawShaders/newSolver.comp
    ${CMAKE_CURRENT_SOURCE_DIR}/rawShaders/solver2.comp
    ${CMAKE_CURRENT_SOURCE_DIR}/rawShaders/searchAndAddPair.comp
    ${CMAKE_CURRENT_SOURCE_DIR}/rawShaders/calcOverlappingData.comp
    ${CMAKE_CURRENT_SOURCE_DIR}/rawShaders/calcRayIntersect.comp
    ${CMAKE_CURRENT_SOURCE_DIR}/rawShaders/particles.comp
    ${CMAKE_CURRENT_SOURCE_DIR}/rawShaders/particlesSorting.comp
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

set(RESOURCE_DIR ${CMAKE_SOURCE_DIR}/resources)
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
    resources
    DEPENDS ${COPYING_RESOURCES}
    )

IF( NOT CMAKE_BUILD_TYPE )
   SET( CMAKE_BUILD_TYPE Debug ... FORCE )
ENDIF()

if (CMAKE_BUILD_TYPE MATCHES "Debug")
  add_definitions(-D_DEBUG) # пользовательские флаги
  #-fsanitize=address -fsanitize=pointer-compare -fsanitize=pointer-subtract -fsanitize=undefined
  # -march=native флажок для автоматического определения процессорных инструкций
  set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -msse4.2 -std=c++11 -Wall -Wextra -g")
else()
  set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -msse4.2 -std=c++11 -Wall -Wextra -O3 -s")
endif (CMAKE_BUILD_TYPE MATCHES "Debug")

# указываем куда мы будем компилировать файлы, также некоторые настройки
SET(BUILD_SHARED_LIBS ON) # все библиотеки по дефолту компилятся как динамические

# начинаем компилировать (создаем цели компиляции)

# UTILS
set(SRC_LIST ${CMAKE_CURRENT_SOURCE_DIR}/utils/CmdMgr.cpp
             ${CMAKE_CURRENT_SOURCE_DIR}/utils/Variable.cpp
             ${CMAKE_CURRENT_SOURCE_DIR}/utils/Console.cpp
             ${CMAKE_CURRENT_SOURCE_DIR}/utils/TimeMeter.cpp
             ${CMAKE_CURRENT_SOURCE_DIR}/utils/Globals.cpp
             ${CMAKE_CURRENT_SOURCE_DIR}/utils/Utility.cpp
             ${CMAKE_CURRENT_SOURCE_DIR}/utils/Event.cpp
             ${CMAKE_CURRENT_SOURCE_DIR}/utils/Type.cpp
             ${CMAKE_CURRENT_SOURCE_DIR}/utils/ResourceID.cpp
             ${CMAKE_CURRENT_SOURCE_DIR}/utils/EntityComponentSystem.cpp
#              ${CMAKE_CURRENT_SOURCE_DIR}/utils/JobSystem.cpp
             ${CMAKE_CURRENT_SOURCE_DIR}/utils/Random.cpp
#              ${CMAKE_CURRENT_SOURCE_DIR}/utils/State.cpp
             ${CMAKE_CURRENT_SOURCE_DIR}/utils/ThreadPool.cpp
             ${CMAKE_CURRENT_SOURCE_DIR}/bin/Settings.cpp
             ${CMAKE_CURRENT_SOURCE_DIR}/include/fmt/src/format.cc
             ${CMAKE_CURRENT_SOURCE_DIR}/include/fmt/src/posix.cc
#              utils/ResourceManager.cpp
             ${CMAKE_CURRENT_SOURCE_DIR}/utils/FileTools.cpp)
set(HDR_LIST ${CMAKE_CURRENT_SOURCE_DIR}/utils/CmdMgr.h
             ${CMAKE_CURRENT_SOURCE_DIR}/utils/Variable.h
             ${CMAKE_CURRENT_SOURCE_DIR}/utils/Console.h
             ${CMAKE_CURRENT_SOURCE_DIR}/utils/TimeMeter.h
             ${CMAKE_CURRENT_SOURCE_DIR}/utils/Globals.h
             ${CMAKE_CURRENT_SOURCE_DIR}/utils/Utility.h
             ${CMAKE_CURRENT_SOURCE_DIR}/utils/Event.h
             ${CMAKE_CURRENT_SOURCE_DIR}/utils/Type.h
             ${CMAKE_CURRENT_SOURCE_DIR}/utils/ResourceID.h
             ${CMAKE_CURRENT_SOURCE_DIR}/utils/EntityComponentSystem.h
#              ${CMAKE_CURRENT_SOURCE_DIR}/utils/JobSystem.h
             ${CMAKE_CURRENT_SOURCE_DIR}/utils/Random.h
#              ${CMAKE_CURRENT_SOURCE_DIR}/utils/State.h
             ${CMAKE_CURRENT_SOURCE_DIR}/utils/ThreadPool.h
             ${CMAKE_CURRENT_SOURCE_DIR}/bin/Settings.h
             ${CMAKE_CURRENT_SOURCE_DIR}/include/fmt/include/fmt/format.h
             ${CMAKE_CURRENT_SOURCE_DIR}/include/fmt/include/fmt/posix.h
#              utils/ResourceManager.h
             ${CMAKE_CURRENT_SOURCE_DIR}/utils/FileTools.h)
add_library(utils ${SRC_LIST} ${HDR_LIST}) # именно add_library объявляет цель в cmake (затем это имя мы используем как переменную в других функциях)
target_link_libraries(utils) # например здесь fmt

# PHYSICS
set(SRC_LIST ${CMAKE_CURRENT_SOURCE_DIR}/physics/PhysicsUtils.cpp
             ${CMAKE_CURRENT_SOURCE_DIR}/physics/Physics.cpp
             ${CMAKE_CURRENT_SOURCE_DIR}/physics/BroadphaseInterface.cpp
#              ${CMAKE_CURRENT_SOURCE_DIR}/physics/GPUPhysics.cpp
             ${CMAKE_CURRENT_SOURCE_DIR}/physics/CPUPhysicsParallel.cpp
#              ${CMAKE_CURRENT_SOURCE_DIR}/physics/GPUOctreeBroadphase.cpp
#              ${CMAKE_CURRENT_SOURCE_DIR}/physics/CPUOctreeBroadphase.cpp
             ${CMAKE_CURRENT_SOURCE_DIR}/physics/CPUOctreeBroadphaseParallel.cpp
#              ${CMAKE_CURRENT_SOURCE_DIR}/physics/GPUNarrowphase.cpp
#              ${CMAKE_CURRENT_SOURCE_DIR}/physics/CPUNarrowphase.cpp
             ${CMAKE_CURRENT_SOURCE_DIR}/physics/CPUNarrowphaseParallel.cpp
#              ${CMAKE_CURRENT_SOURCE_DIR}/physics/GPUSolver.cpp
#              ${CMAKE_CURRENT_SOURCE_DIR}/physics/CPUSolver.cpp
             ${CMAKE_CURRENT_SOURCE_DIR}/physics/CPUSolverParallel.cpp
#              ${CMAKE_CURRENT_SOURCE_DIR}/physics/GPUPhysicsSorter.cpp
             ${CMAKE_CURRENT_SOURCE_DIR}/physics/CPUPhysicsSorter.cpp
             ${CMAKE_CURRENT_SOURCE_DIR}/physics/HelperFunctions.cpp
#              ${CMAKE_CURRENT_SOURCE_DIR}/physics/Graph.cpp
             )
set(HDR_LIST ${CMAKE_CURRENT_SOURCE_DIR}/physics/PhysicsUtils.h
             ${CMAKE_CURRENT_SOURCE_DIR}/physics/Physics.h
             ${CMAKE_CURRENT_SOURCE_DIR}/physics/BroadphaseInterface.h
#              ${CMAKE_CURRENT_SOURCE_DIR}/physics/GPUPhysics.h
             ${CMAKE_CURRENT_SOURCE_DIR}/physics/CPUPhysicsParallel.h
#              ${CMAKE_CURRENT_SOURCE_DIR}/physics/GPUOctreeBroadphase.h
#              ${CMAKE_CURRENT_SOURCE_DIR}/physics/CPUOctreeBroadphase.h
             ${CMAKE_CURRENT_SOURCE_DIR}/physics/CPUOctreeBroadphaseParallel.h
             ${CMAKE_CURRENT_SOURCE_DIR}/physics/ArrayInterface.h
             ${CMAKE_CURRENT_SOURCE_DIR}/physics/GPUArray.h
             ${CMAKE_CURRENT_SOURCE_DIR}/physics/NarrowphaseInterface.h
#              ${CMAKE_CURRENT_SOURCE_DIR}/physics/GPUNarrowphase.h
#              ${CMAKE_CURRENT_SOURCE_DIR}/physics/CPUNarrowphase.h
             ${CMAKE_CURRENT_SOURCE_DIR}/physics/CPUNarrowphaseParallel.h
             ${CMAKE_CURRENT_SOURCE_DIR}/physics/Solver.h
#              ${CMAKE_CURRENT_SOURCE_DIR}/physics/GPUSolver.h
#              ${CMAKE_CURRENT_SOURCE_DIR}/physics/CPUSolver.h
             ${CMAKE_CURRENT_SOURCE_DIR}/physics/CPUSolverParallel.h
             ${CMAKE_CURRENT_SOURCE_DIR}/physics/PhysicsSorter.h
#              ${CMAKE_CURRENT_SOURCE_DIR}/physics/GPUPhysicsSorter.h
             ${CMAKE_CURRENT_SOURCE_DIR}/physics/CPUPhysicsSorter.h
             ${CMAKE_CURRENT_SOURCE_DIR}/physics/HelperFunctions.h
#              ${CMAKE_CURRENT_SOURCE_DIR}/physics/Graph.h
             )
add_library(physics ${SRC_LIST} ${HDR_LIST})
target_link_libraries(physics render utils)

# RENDER
set(SRC_LIST ${CMAKE_CURRENT_SOURCE_DIR}/render/VulkanRender.cpp
             ${CMAKE_CURRENT_SOURCE_DIR}/render/Optimizers.cpp
             ${CMAKE_CURRENT_SOURCE_DIR}/render/RenderStages.cpp
             # ${CMAKE_CURRENT_SOURCE_DIR}/render/GraphicComponets.cpp
             ${CMAKE_CURRENT_SOURCE_DIR}/render/Window.cpp
             ${CMAKE_CURRENT_SOURCE_DIR}/render/Deferred.cpp
             ${CMAKE_CURRENT_SOURCE_DIR}/render/GraphicsContainer.cpp
#              ${CMAKE_CURRENT_SOURCE_DIR}/render/Occlusion.cpp
             ${CMAKE_CURRENT_SOURCE_DIR}/render/Core.cpp
             ${CMAKE_CURRENT_SOURCE_DIR}/render/Tasks.cpp
             ${CMAKE_CURRENT_SOURCE_DIR}/render/Internal.cpp
             ${CMAKE_CURRENT_SOURCE_DIR}/render/Makers.cpp
             ${CMAKE_CURRENT_SOURCE_DIR}/render/Types.cpp
             ${CMAKE_CURRENT_SOURCE_DIR}/render/RAII.cpp
#              render/TextureManager.cpp
#              render/stbi_image_impl.cpp
             )
set(HDR_LIST ${CMAKE_CURRENT_SOURCE_DIR}/render/Render.h
             ${CMAKE_CURRENT_SOURCE_DIR}/render/VulkanRender.h
             ${CMAKE_CURRENT_SOURCE_DIR}/render/Optimizer.h
             ${CMAKE_CURRENT_SOURCE_DIR}/render/Optimizers.h
             ${CMAKE_CURRENT_SOURCE_DIR}/render/RenderStages.h
             ${CMAKE_CURRENT_SOURCE_DIR}/render/RenderStage.h
             # ${CMAKE_CURRENT_SOURCE_DIR}/render/GraphicComponets.h
             ${CMAKE_CURRENT_SOURCE_DIR}/render/Window.h
             ${CMAKE_CURRENT_SOURCE_DIR}/render/Deferred.h
             ${CMAKE_CURRENT_SOURCE_DIR}/render/GraphicsContainer.h
             ${CMAKE_CURRENT_SOURCE_DIR}/render/nuklear_header.h
#              ${CMAKE_CURRENT_SOURCE_DIR}/render/Occlusion.h
             ${CMAKE_CURRENT_SOURCE_DIR}/render/yavf.h
             ${CMAKE_CURRENT_SOURCE_DIR}/render/Core.h
             ${CMAKE_CURRENT_SOURCE_DIR}/render/Tasks.h
             ${CMAKE_CURRENT_SOURCE_DIR}/render/Internal.h
             ${CMAKE_CURRENT_SOURCE_DIR}/render/Makers.h
             ${CMAKE_CURRENT_SOURCE_DIR}/render/Types.h
             ${CMAKE_CURRENT_SOURCE_DIR}/render/RAII.h
#              ${CMAKE_CURRENT_SOURCE_DIR}/render/vk_mem_alloc.h
             ${CMAKE_CURRENT_SOURCE_DIR}/include/VulkanMemoryAllocator/src/vk_mem_alloc.h
#              render/TextureManager.h
#              render/stbi_image_header.h
             )
add_library(render ${SRC_LIST} ${HDR_LIST})
target_link_libraries(render utils ${VULKAN_LIB_NAME} ${GLFW3_LIBRARIES})

# BINARY
set(SRC_LIST ${CMAKE_CURRENT_SOURCE_DIR}/bin/Components.cpp
             ${CMAKE_CURRENT_SOURCE_DIR}/render/GraphicComponets.cpp
#              bin/CombatComponent.cpp
#              model/Animation.cpp
#              bin/AISystem.cpp
#              bin/Game.cpp
#              bin/main.cpp
             ${CMAKE_CURRENT_SOURCE_DIR}/bin/EventComponent.cpp
             ${CMAKE_CURRENT_SOURCE_DIR}/bin/AnimationComponent.cpp
             ${CMAKE_CURRENT_SOURCE_DIR}/sound/alHelpers.cpp
             ${CMAKE_CURRENT_SOURCE_DIR}/sound/Buffer.cpp
             ${CMAKE_CURRENT_SOURCE_DIR}/sound/Source.cpp
             ${CMAKE_CURRENT_SOURCE_DIR}/sound/SoundData.cpp
             ${CMAKE_CURRENT_SOURCE_DIR}/sound/SoundSystem.cpp
             ${CMAKE_CURRENT_SOURCE_DIR}/bin/HardcodedLoaders.cpp
             ${CMAKE_CURRENT_SOURCE_DIR}/bin/TextureLoader.cpp
             ${CMAKE_CURRENT_SOURCE_DIR}/bin/CPUAnimationSystem.cpp
             ${CMAKE_CURRENT_SOURCE_DIR}/bin/PostPhysics.cpp
             ${CMAKE_CURRENT_SOURCE_DIR}/bin/ResourceManager.cpp
             ${CMAKE_CURRENT_SOURCE_DIR}/bin/Helper.cpp
             ${CMAKE_CURRENT_SOURCE_DIR}/bin/main.cpp)

set(HDR_LIST ${CMAKE_CURRENT_SOURCE_DIR}/bin/Components.h
             ${CMAKE_CURRENT_SOURCE_DIR}/render/GraphicComponets.h
#              bin/CombatComponent.h
#              model/Animation.h
#              bin/AISystem.h
#              bin/Game.h
             ${CMAKE_CURRENT_SOURCE_DIR}/sound/alHelpers.h
             ${CMAKE_CURRENT_SOURCE_DIR}/sound/Buffer.h
             ${CMAKE_CURRENT_SOURCE_DIR}/sound/Source.h
             ${CMAKE_CURRENT_SOURCE_DIR}/sound/SoundData.h
             ${CMAKE_CURRENT_SOURCE_DIR}/sound/SoundSystem.h
             ${CMAKE_CURRENT_SOURCE_DIR}/bin/EventComponent.h
             ${CMAKE_CURRENT_SOURCE_DIR}/bin/AnimationComponent.h
             ${CMAKE_CURRENT_SOURCE_DIR}/bin/HardcodedLoaders.h
             ${CMAKE_CURRENT_SOURCE_DIR}/bin/TextureLoader.h
             ${CMAKE_CURRENT_SOURCE_DIR}/bin/CPUAnimationSystem.h
             ${CMAKE_CURRENT_SOURCE_DIR}/bin/PostPhysics.h
             ${CMAKE_CURRENT_SOURCE_DIR}/bin/ResourceManager.h
             ${CMAKE_CURRENT_SOURCE_DIR}/bin/Helper.h
             )
add_executable(${TARGET_NAME} ${SRC_LIST} ${HDR_LIST})
target_link_libraries(${TARGET_NAME} render physics utils cppfs openal)
add_dependencies(${TARGET_NAME} shaders resources)
