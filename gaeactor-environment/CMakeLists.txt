# gaeactor-environment 配置
project(gaeactor-environment VERSION 1.0 LANGUAGES CXX)


include(${CMAKE_DIR}/lib3rd.cmake)
include(${CMAKE_DIR}/sdk_loggingservice.cmake)
include(${CMAKE_DIR}/sdk_gaeactor_utils.cmake)
include(${CMAKE_DIR}/sdk_gaeactor_utils_location.cmake)

# IF (MSVC)
#     set(ws232 ws2_32)
# ENDIF (MSVC)

#file(GLOB_RECURSE  SRCS_CPP ./*.cpp ./*.c ./*.cxx)
#file(GLOB_RECURSE  SRCS_H ./*.h ./*.hpp)
set(SRCS_H 
    ${CMAKE_CURRENT_SOURCE_DIR}/gaeactor_environment_define.h 
    ${CMAKE_CURRENT_SOURCE_DIR}/gaeactor_environment_global.h 
    ${CMAKE_CURRENT_SOURCE_DIR}/gaeactor_environment_tbb_datahelper.h
    ${CMAKE_CURRENT_SOURCE_DIR}/gaeactor_processor.h
    ${CMAKE_CURRENT_SOURCE_DIR}/gaeactor_processor_interface.h
    ${CMAKE_CURRENT_SOURCE_DIR}/gaeactor_processor_normal.h)

set(SRCS_CPP 
    ${CMAKE_CURRENT_SOURCE_DIR}/gaeactor_processor.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/gaeactor_processor_interface.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/gaeactor_processor_normal.cpp)

add_library(${PROJECT_NAME} SHARED ${SRCS_H} ${SRCS_CPP})


# if(MSVC)
#     target_compile_options(${PROJECT_NAME} PRIVATE "/std:c11")
# elseif(CMAKE_C_COMPILER_ID MATCHES "GNU|Clang")
#     target_compile_options(${PROJECT_NAME} PRIVATE "-std=c11")
# endif()

target_link_libraries(${PROJECT_NAME} 
                        ${QT_LIBRARIES} 
                        ${LIB3RD_LIBRARIES}
                        ${libconcurrentqueue}
                        ${sdk_loggingservice}
                        ${sdk_gaeactor_utils}
                        ${sdk_gaeactor_utils_location})

target_compile_definitions(${PROJECT_NAME} PRIVATE GAEACTOR_ENVIRONMENT_LIBRARY UNICODE _UNICODE BUILD_WITH_EASY_PROFILER)

# install(TARGETS ${PROJECT_NAME} LIBRARY DESTINATION lib RUNTIME DESTINATION bin)
install(TARGETS ${PROJECT_NAME} ARCHIVE DESTINATION lib RUNTIME DESTINATION bin) 
install(DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/
        DESTINATION ${DEPLOY_PATH}/${PLATFORM_DIR}/include/${PROJECT_NAME}
        FILES_MATCHING PATTERN "*.h" PATTERN "*.hpp")