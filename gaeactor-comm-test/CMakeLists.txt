# gaeactor-comm-test 配置
project(gaeactor-comm-test VERSION 1.0 LANGUAGES CXX)


include(${CMAKE_DIR}/lib3rd.cmake)
include(${CMAKE_DIR}/sdk_gaeactor_comm.cmake)
include(${CMAKE_DIR}/sdk_gaeactor_utils.cmake)


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
                        ${sdk_gaeactor_utils})


target_compile_definitions(${PROJECT_NAME} PRIVATE UNICODE _UNICODE)

install(TARGETS ${PROJECT_NAME} ARCHIVE DESTINATION lib RUNTIME DESTINATION bin) 