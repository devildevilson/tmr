#set(CMAKE_CXX_COMPILER /usr/bin/g++) # компилятор g++ (пока не понятно работает ли)

# указываем минимальную версию CMAKE, а также название проекта и тип билда
cmake_minimum_required(VERSION 3.0)
project(testingApp)

#set(CMAKE_BUILD_TYPE "DEBUG")

#set(CURRENT_DIRECTORY .)

set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}/${PROJECT_NAME})
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}/${PROJECT_NAME})

set(TARGET_NAME VulkanTest)

# message("Binary dir: ${CMAKE_CURRENT_BINARY_DIR}")
# message("Source dir: ${CMAKE_CURRENT_SOURCE_DIR}")
# message("List   dir: ${WORKING_DIRECTORY}")
# message("Output dir: ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}")
# message("Binary dir: ${CMAKE_BINARY_DIR}")
# message("Source dir: ${CMAKE_SOURCE_DIR}")
# message("Output dir: ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}")
#MESSAGE(FATAL_ERROR "End")

set(COMPILED_SHADERS_PATH ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/shaders)
set(GLSL_VALIDATOR "glslangValidator")

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

# add_custom_command(
#         TARGET ${TARGET_NAME} POST_BUILD
#         COMMAND ${CMAKE_COMMAND} -E copy
#                 ${CMAKE_SOURCE_DIR}/test/input.txt
#                 ${CMAKE_CURRENT_BINARY_DIR}/input.txt)

# file(GLOB_RECURSE PROJECT_RESOURCE_FILES
#     ${RESOURCE_DIR}/*
#     )

# file(COPY ${PROJECT_RESOURCE_FILES} DESTINATION ${RESOURCE_DEST})
# file(COPY . DESTINATION ${RESOURCE_DEST} PATTERN build EXCLUDE)

# Scan through resource folder for updated files and copy if none existing or changed
# file (GLOB_RECURSE PROJECT_RESOURCE_FILES "${RESOURCE_DIR}/*")
# foreach(RESOURCE ${PROJECT_RESOURCE_FILES})
#  get_filename_component(filename ${RESOURCE} NAME)
#  get_filename_component(dir ${RESOURCE} DIRECTORY)
#  get_filename_component(dirname ${dir} NAME)
#  get_filename_component(sourcedirname ${RESOURCE_DIR} NAME)
#  
#  set (output "")
#  
#  while(NOT ${dirname} STREQUAL ${sourcedirname})
#   get_filename_component(path_component ${dir} NAME)
#   set (output "${path_component}/${output}")
#   get_filename_component(dir ${dir} DIRECTORY)
#   get_filename_component(dirname ${dir} NAME)
#   # мне тут нужно еще создать директорию для копирования
#  endwhile()
#  
#  set(output "${RESOURCE_DEST}/${output}/${filename}")
#  
#  add_custom_command(
#   COMMENT "Moving updated resource-file '${filename}'"
#   OUTPUT ${output}
#   DEPENDS ${resource}
#   COMMAND ${CMAKE_COMMAND} -E copy_if_different ${resource} ${output}
#  )
#  #add_custom_target(${filename} ALL DEPENDS ${resource} ${output})
#  list(APPEND COPYING_RESOURCES ${output})
#  
# endforeach()
# 
# add_custom_target(
#     resources 
#     DEPENDS ${COPYING_RESOURCES}
#     )
    
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
      
if (WIN32) 
  
else()
  # находим указанные зависимости (нужно чекнуть все ли это зависимости?)
  # вместо glm у меня установлен gli
  #find_package(glm)
  find_package(glfw3)
  find_package(PkgConfig REQUIRED)
  #pkg_search_module(GLFW REQUIRED glfw3)
  pkg_check_modules(GLFW3 REQUIRED glfw3)
  # не может найти почему то vulkan-headers
  #find_package(vulkan-headers)
endif()

IF( NOT CMAKE_BUILD_TYPE )
   SET( CMAKE_BUILD_TYPE Debug ... FORCE )
ENDIF()

if (CMAKE_BUILD_TYPE MATCHES "Debug")
  #message("Debug build.")
  #MESSAGE(FATAL_ERROR "End")
  add_definitions(-D_DEBUG) # пользовательские флаги
  #-fsanitize=address -fsanitize=pointer-compare -fsanitize=pointer-subtract -fsanitize=undefined
  # -march=native флажок для автоматического определения процессорных инструкций
  set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -msse4.2 -std=c++11 -Wall -Wextra -g")
else()
  #message("Release build.")
  #MESSAGE(FATAL_ERROR "End")
  set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -msse4.2 -std=c++11 -Wall -Wextra -O3 -s")
endif (CMAKE_BUILD_TYPE MATCHES "Debug")

# указываем куда мы будем компилировать файлы, также некоторые настройки
#SET(EXECUTABLE_OUTPUT_PATH build/) # исполняемый файл
#SET(LIBRARY_OUTPUT_PATH build/) # библиотечки
SET(BUILD_SHARED_LIBS ON) # все библиотеки по дефолту компилятся как динамические
#set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)

#set(VULKAN_SDK_PATH /home/mikuhatsune/VulkanSDK/1.0.24.0/x86_64) # set задает переменную в cmake
set(STB_INCLUDE_PATH ${CMAKE_CURRENT_SOURCE_DIR}/include/stb)
set(TINYOBJ_INCLUDE_PATH ${CMAKE_CURRENT_SOURCE_DIR}/include/tinyobjloader)

#pkg_check_modules(GLFW3 REQUIRED glfw3) # используем модуль pkg-config для glfw3 (замена `pkg-config --static --libs glfw3`)
#set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11 -Wall -Wextra -g") # сишные флаги

#${CMAKE_CURRENT_LIST_DIR}/build/
# директории, где компилятор ищет хедеры
include_directories(${CMAKE_CURRENT_SOURCE_DIR}/bin/ 
                    ${CMAKE_CURRENT_SOURCE_DIR}/render/ 
                    ${CMAKE_CURRENT_SOURCE_DIR}/physics/ 
                    ${CMAKE_CURRENT_SOURCE_DIR}/utils/ 
                    ${CMAKE_CURRENT_SOURCE_DIR}/include/ 
                    ${GLFW3_INCLUDE_DIRS} 
                    ${STB_INCLUDE_PATH} 
                    ${TINYOBJ_INCLUDE_PATH} 
#                     /home/mikuhatsune/gwc/testingApp/include/libleaktracer/include
#                     ${CMAKE_CURRENT_SOURCE_DIR}/include/imgui/ 
                    ${CMAKE_CURRENT_SOURCE_DIR}/include/TinyBehavior/) 

link_directories(${CMAKE_CURRENT_BINARY_DIR} ${CMAKE_LIBRARY_OUTPUT_DIRECTORY}) # директории, где компилятор ищет библиотечки

ADD_LIBRARY(fmt STATIC IMPORTED)
SET_TARGET_PROPERTIES(fmt PROPERTIES
    IMPORTED_LOCATION ${CMAKE_CURRENT_SOURCE_DIR}/include/fmt/libfmt.a)
    
# ADD_LIBRARY(imgui STATIC IMPORTED)
# SET_TARGET_PROPERTIES(imgui PROPERTIES
#     IMPORTED_LOCATION ${CMAKE_CURRENT_SOURCE_DIR}/include/imgui/build/imgui.a)
    
# ADD_LIBRARY(soil STATIC IMPORTED)
# SET_TARGET_PROPERTIES(soil PROPERTIES
#     IMPORTED_LOCATION ${CMAKE_CURRENT_SOURCE_DIR}/include/SOIL/libSOIL.a)
    
#add_library(leaktracer DYNAMIC IMPORTED)
#set_target_properties(leaktracer PROPERTIES
#    IMPORTED_LOCATION ${CMAKE_LIBRARY_OUTPUT_DIRECTORY}/libleaktracer.so)
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
             ${CMAKE_CURRENT_SOURCE_DIR}/utils/EntityComponentSystem.cpp 
#              ${CMAKE_CURRENT_SOURCE_DIR}/utils/JobSystem.cpp
             ${CMAKE_CURRENT_SOURCE_DIR}/utils/Random.cpp
#              ${CMAKE_CURRENT_SOURCE_DIR}/utils/State.cpp
             ${CMAKE_CURRENT_SOURCE_DIR}/utils/ThreadPool.cpp
             ${CMAKE_CURRENT_SOURCE_DIR}/bin/Settings.cpp 
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
             ${CMAKE_CURRENT_SOURCE_DIR}/utils/EntityComponentSystem.h 
#              ${CMAKE_CURRENT_SOURCE_DIR}/utils/JobSystem.h
             ${CMAKE_CURRENT_SOURCE_DIR}/utils/Random.h
#              ${CMAKE_CURRENT_SOURCE_DIR}/utils/State.h
             ${CMAKE_CURRENT_SOURCE_DIR}/utils/ThreadPool.h
             ${CMAKE_CURRENT_SOURCE_DIR}/bin/Settings.h
#              utils/ResourceManager.h
             ${CMAKE_CURRENT_SOURCE_DIR}/utils/FileTools.h)
add_library(utils ${SRC_LIST} ${HDR_LIST}) # именно add_library объявляет цель в cmake (затем это имя мы используем как переменную в других функциях)
target_link_libraries(utils fmt) # например здесь

# PHYSICS             
set(SRC_LIST ${CMAKE_CURRENT_SOURCE_DIR}/physics/PhysicsUtils.cpp
             ${CMAKE_CURRENT_SOURCE_DIR}/physics/Physics.cpp 
             ${CMAKE_CURRENT_SOURCE_DIR}/physics/BroadphaseInterface.cpp
             ${CMAKE_CURRENT_SOURCE_DIR}/physics/GPUPhysics.cpp 
             ${CMAKE_CURRENT_SOURCE_DIR}/physics/CPUPhysicsParallel.cpp
             ${CMAKE_CURRENT_SOURCE_DIR}/physics/GPUOctreeBroadphase.cpp
             ${CMAKE_CURRENT_SOURCE_DIR}/physics/CPUOctreeBroadphase.cpp
             ${CMAKE_CURRENT_SOURCE_DIR}/physics/CPUOctreeBroadphaseParallel.cpp
             ${CMAKE_CURRENT_SOURCE_DIR}/physics/GPUNarrowphase.cpp
             ${CMAKE_CURRENT_SOURCE_DIR}/physics/CPUNarrowphase.cpp
             ${CMAKE_CURRENT_SOURCE_DIR}/physics/CPUNarrowphaseParallel.cpp
             ${CMAKE_CURRENT_SOURCE_DIR}/physics/GPUSolver.cpp 
             ${CMAKE_CURRENT_SOURCE_DIR}/physics/CPUSolver.cpp 
             ${CMAKE_CURRENT_SOURCE_DIR}/physics/CPUSolverParallel.cpp 
             ${CMAKE_CURRENT_SOURCE_DIR}/physics/GPUPhysicsSorter.cpp
             ${CMAKE_CURRENT_SOURCE_DIR}/physics/CPUPhysicsSorter.cpp
             ${CMAKE_CURRENT_SOURCE_DIR}/physics/HelperFunctions.cpp
#              ${CMAKE_CURRENT_SOURCE_DIR}/physics/Graph.cpp 
             )
set(HDR_LIST ${CMAKE_CURRENT_SOURCE_DIR}/physics/PhysicsUtils.h
             ${CMAKE_CURRENT_SOURCE_DIR}/physics/Physics.h 
             ${CMAKE_CURRENT_SOURCE_DIR}/physics/BroadphaseInterface.h
             ${CMAKE_CURRENT_SOURCE_DIR}/physics/GPUPhysics.h
             ${CMAKE_CURRENT_SOURCE_DIR}/physics/CPUPhysicsParallel.h
             ${CMAKE_CURRENT_SOURCE_DIR}/physics/GPUOctreeBroadphase.h
             ${CMAKE_CURRENT_SOURCE_DIR}/physics/CPUOctreeBroadphase.h
             ${CMAKE_CURRENT_SOURCE_DIR}/physics/CPUOctreeBroadphaseParallel.h
             ${CMAKE_CURRENT_SOURCE_DIR}/physics/ArrayInterface.h
             ${CMAKE_CURRENT_SOURCE_DIR}/physics/GPUArray.h
             ${CMAKE_CURRENT_SOURCE_DIR}/physics/NarrowphaseInterface.h
             ${CMAKE_CURRENT_SOURCE_DIR}/physics/GPUNarrowphase.h
             ${CMAKE_CURRENT_SOURCE_DIR}/physics/CPUNarrowphase.h
             ${CMAKE_CURRENT_SOURCE_DIR}/physics/CPUNarrowphaseParallel.h
             ${CMAKE_CURRENT_SOURCE_DIR}/physics/Solver.h 
             ${CMAKE_CURRENT_SOURCE_DIR}/physics/GPUSolver.h
             ${CMAKE_CURRENT_SOURCE_DIR}/physics/CPUSolver.h
             ${CMAKE_CURRENT_SOURCE_DIR}/physics/CPUSolverParallel.h
             ${CMAKE_CURRENT_SOURCE_DIR}/physics/PhysicsSorter.h
             ${CMAKE_CURRENT_SOURCE_DIR}/physics/GPUPhysicsSorter.h
             ${CMAKE_CURRENT_SOURCE_DIR}/physics/CPUPhysicsSorter.h
             ${CMAKE_CURRENT_SOURCE_DIR}/physics/HelperFunctions.h 
#              ${CMAKE_CURRENT_SOURCE_DIR}/physics/Graph.h
             )
add_library(physics ${SRC_LIST} ${HDR_LIST})
target_link_libraries(physics utils)

# RENDER
set(SRC_LIST ${CMAKE_CURRENT_SOURCE_DIR}/render/VulkanRender.cpp 
             ${CMAKE_CURRENT_SOURCE_DIR}/render/Optimizers.cpp
             ${CMAKE_CURRENT_SOURCE_DIR}/render/RenderStages.cpp
             ${CMAKE_CURRENT_SOURCE_DIR}/render/GraphicComponets.cpp
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
             ${CMAKE_CURRENT_SOURCE_DIR}/render/GraphicComponets.h
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
             ${CMAKE_CURRENT_SOURCE_DIR}/render/vk_mem_alloc.h 
#              render/TextureManager.h
#              render/stbi_image_header.h
             )
add_library(render ${SRC_LIST} ${HDR_LIST})
target_link_libraries(render utils vulkan ${GLFW3_LIBRARIES})

# BINARY
set(SRC_LIST ${CMAKE_CURRENT_SOURCE_DIR}/bin/Components.cpp 
#              bin/CombatComponent.cpp 
#              model/Animation.cpp 
#              bin/AISystem.cpp 
#              bin/Game.cpp 
#              bin/main.cpp
             ${CMAKE_CURRENT_SOURCE_DIR}/bin/EventComponent.cpp
             ${CMAKE_CURRENT_SOURCE_DIR}/bin/AnimationComponent.cpp
             #${CMAKE_CURRENT_SOURCE_DIR}/sound/SoundSystem.cpp
             ${CMAKE_CURRENT_SOURCE_DIR}/bin/HardcodedLoaders.cpp
             ${CMAKE_CURRENT_SOURCE_DIR}/bin/TextureLoader.cpp
             ${CMAKE_CURRENT_SOURCE_DIR}/bin/CPUAnimationSystem.cpp
             ${CMAKE_CURRENT_SOURCE_DIR}/bin/PostPhysics.cpp
             ${CMAKE_CURRENT_SOURCE_DIR}/bin/ResourceManager.cpp
             ${CMAKE_CURRENT_SOURCE_DIR}/bin/Helper.cpp
             ${CMAKE_CURRENT_SOURCE_DIR}/bin/main.cpp)

set(HDR_LIST ${CMAKE_CURRENT_SOURCE_DIR}/bin/Components.h 
#              bin/CombatComponent.h 
#              model/Animation.h 
#              bin/AISystem.h 
#              bin/Game.h
             #${CMAKE_CURRENT_SOURCE_DIR}/sound/SoundSystem.h
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
target_link_libraries(${TARGET_NAME} render physics utils)
add_dependencies(${TARGET_NAME} shaders resources)
