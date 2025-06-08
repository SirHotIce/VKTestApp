cmake_minimum_required(VERSION 3.29)
project(VulkanProject)

set(CMAKE_CXX_STANDARD 20)

add_executable(VulkanProject src/main.cpp
        src/VulkanPipeline/VKFrame.cpp
        src/VulkanPipeline/VKFrame.h
        src/VulkanPipeline/VKRenderer.cpp
        src/VulkanPipeline/VKRenderer.h)

# === Detect Platform ===
if (APPLE)
    set(PLATFORM "mac")
elseif (WIN32)
    set(PLATFORM "win")
endif()
# === Vulkan (macOS + MoltenVK Manual Setup) ===
if(APPLE)
    set(VULKAN_SDK "/Users/hotice/VulkanSDK/1.4.313.1/macOS")
    find_package(Vulkan REQUIRED)
    set(Vulkan_INCLUDE_DIR "${VULKAN_SDK}/include")
    set(Vulkan_LIBRARY "${VULKAN_SDK}/lib/libvulkan.1.dylib")
#    set(Molten_Vk_LIBRARY "${VULKAN_SDK}/lib/libMoltenVK.dylib")
    set(Vulkan_LIBRARY_CPY "${Vulkan_LIBRARY}")
#    set(Molten_Vk_LIBRARY_CPY "${Molten_Vk_LIBRARY}")

    target_include_directories(VulkanProject PRIVATE ${Vulkan_INCLUDE_DIR})
    target_link_libraries(VulkanProject ${Vulkan_LIBRARY})
#    target_link_libraries(VulkanProject ${Molten_Vk_LIBRARY})
    add_custom_command(TARGET VulkanProject POST_BUILD
            COMMAND ${CMAKE_COMMAND} -E copy_if_different
            "${Vulkan_LIBRARY_CPY}"
            $<TARGET_FILE_DIR:VulkanProject>
    )
#    add_custom_command(TARGET VulkanProject POST_BUILD
#            COMMAND ${CMAKE_COMMAND} -E copy_if_different
#            "${Molten_Vk_LIBRARY_CPY}"
#            $<TARGET_FILE_DIR:VulkanProject>
#    )
else()
    find_package(Vulkan REQUIRED)
    target_link_libraries(VulkanProject Vulkan::Vulkan)
endif()

# === GLFW ===
set(GLFW_DIR "${CMAKE_SOURCE_DIR}/external/glfw_${PLATFORM}")
set(GLFW_INCLUDE_DIR "${GLFW_DIR}/include")
if (APPLE)
    set(GLFW_LIBRARY_FILE "${GLFW_DIR}/lib-arm64/libglfw.3.dylib")
    set(GLFW_COPY_FILE "${GLFW_LIBRARY_FILE}")
else()
    set(GLFW_LIBRARY_DIR "${GLFW_DIR}/lib-vc2022")
    set(GLFW_LIBRARY_FILE "${GLFW_LIBRARY_DIR}/glfw3.lib")
    set(GLFW_COPY_FILE "${GLFW_LIBRARY_DIR}/glfw3.dll")
endif()

target_include_directories(VulkanProject PRIVATE ${GLFW_INCLUDE_DIR})
target_link_libraries(VulkanProject ${GLFW_LIBRARY_FILE})
add_custom_command(TARGET VulkanProject POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy_if_different
        "${GLFW_COPY_FILE}"
        $<TARGET_FILE_DIR:VulkanProject>
)

# === ASSIMP ===
set(ASSIMP_DIR "${CMAKE_SOURCE_DIR}/external/assimp_${PLATFORM}")
set(ASSIMP_INCLUDE_DIR "${ASSIMP_DIR}/include")
if (APPLE)
    set(ASSIMP_LIBRARY_FILE "${ASSIMP_DIR}/lib/libassimp.5.4.3.dylib")
    set(ASSIMP_COPY_FILE "${ASSIMP_LIBRARY_FILE}")
else()
    set(ASSIMP_LIBRARY_FILE "${ASSIMP_LIBRARY_DIR}/assimp-vc143-mt.lib")
    set(ASSIMP_COPY_FILE "${ASSIMP_DIR}/bin/x64/assimp-vc143-mt.dll")
endif ()

target_include_directories(VulkanProject PRIVATE ${ASSIMP_INCLUDE_DIR})
target_link_libraries(VulkanProject ${ASSIMP_LIBRARY_FILE})
add_custom_command(TARGET VulkanProject POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy_if_different
        "${ASSIMP_COPY_FILE}"
        $<TARGET_FILE_DIR:VulkanProject>
)

# === GLM (Header Only) ===
set(GLM_INCLUDE_DIR "${CMAKE_SOURCE_DIR}/external/glm")
target_include_directories(VulkanProject PRIVATE ${GLM_INCLUDE_DIR})

# === Shader Copy Targets ===
set(SHADER_DIRECTORY "${CMAKE_SOURCE_DIR}/shaders")
set(SHADER_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/shaders")

add_custom_target(CopyShaders ALL
        COMMAND ${CMAKE_COMMAND} -E remove_directory "${SHADER_OUTPUT_DIRECTORY}"
        COMMAND ${CMAKE_COMMAND} -E copy_directory "${SHADER_DIRECTORY}" "${SHADER_OUTPUT_DIRECTORY}"
        COMMENT "Refreshing and copying shader folder..."
)
add_dependencies(VulkanProject CopyShaders)