# gaeactor-engine 配置
project(gaeactor-engine VERSION 1.0 LANGUAGES CXX)


include(${CMAKE_DIR}/lib3rd.cmake)
include(${CMAKE_DIR}/sdk_gaeactor_utils_heads.cmake)
include(${CMAKE_DIR}/sdk_gaeactor_utils_location.cmake)
include(${CMAKE_DIR}/sdk_gaeactor_comm.cmake)
include(${CMAKE_DIR}/sdk_gaeactor_transmit.cmake)
include(${CMAKE_DIR}/sdk_gaeactor.cmake)

# IF (MSVC)
#     set(ws232 ws2_32)
# ENDIF (MSVC)

file(GLOB_RECURSE  SRCS_CPP ./*.cpp ./*.c ./*.cxx)
file(GLOB_RECURSE  SRCS_H ./*.h ./*.hpp)

add_library(${PROJECT_NAME} SHARED ${SRCS_H} ${SRCS_CPP})


# if(MSVC)
#     target_compile_options(${PROJECT_NAME} PRIVATE "/std:c11")
# elseif(CMAKE_C_COMPILER_ID MATCHES "GNU|Clang")
#     target_compile_options(${PROJECT_NAME} PRIVATE "-std=c11")
# endif()

target_link_libraries(${PROJECT_NAME} 
                        ${QT_LIBRARIES} 
                        ${LIB3RD_LIBRARIES}
                        ${sdk_gaeactor_comm}
                        ${sdk_gaeactor_utils_location}
                        ${sdk_gaeactor_transmit}
                        ${sdk_gaeactor})

target_compile_definitions(${PROJECT_NAME} PRIVATE GAEACTOR_ENGINE_LIBRARY UNICODE _UNICODE )

# install(TARGETS ${PROJECT_NAME} LIBRARY DESTINATION lib RUNTIME DESTINATION bin)
install(TARGETS ${PROJECT_NAME} ARCHIVE DESTINATION lib RUNTIME DESTINATION bin) 
install(DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/
        DESTINATION ${DEPLOY_PATH}/${PLATFORM_DIR}/include/${PROJECT_NAME}
        FILES_MATCHING PATTERN "*.h" PATTERN "*.hpp")