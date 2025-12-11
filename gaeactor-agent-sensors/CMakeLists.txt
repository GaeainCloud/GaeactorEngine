# gaeactor-agent-sensors 配置
project(gaeactor-agent-sensors VERSION 1.0 LANGUAGES CXX)


include(${CMAKE_DIR}/lib3rd.cmake)
include(${CMAKE_DIR}/sdk_gaeactor_utils.cmake)
include(${CMAKE_DIR}/sdk_gaeactor_environment.cmake)
include(${CMAKE_DIR}/sdk_gaeactor_event_engine.cmake)
include(${CMAKE_DIR}/sdk_loggingservice.cmake)

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
                        ${sdk_loggingservice}
                        ${sdk_gaeactor_environment}
                        ${sdk_gaeactor_event_engine}
                        ${sdk_gaeactor_utils})

target_compile_definitions(${PROJECT_NAME} PRIVATE GAEACTOR_AGENT_SENSORS_LIBRARY UNICODE _UNICODE BUILD_WITH_EASY_PROFILER)

# install(TARGETS ${PROJECT_NAME} LIBRARY DESTINATION lib RUNTIME DESTINATION bin)
install(TARGETS ${PROJECT_NAME} ARCHIVE DESTINATION lib RUNTIME DESTINATION bin) 
install(DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/
        DESTINATION ${DEPLOY_PATH}/${PLATFORM_DIR}/include/${PROJECT_NAME}
        FILES_MATCHING PATTERN "*.h" PATTERN "*.hpp")