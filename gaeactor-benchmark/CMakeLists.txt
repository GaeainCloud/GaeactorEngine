# gaeactor-benchmark 配置
project(gaeactor-benchmark VERSION 1.0 LANGUAGES CXX)


include(${CMAKE_DIR}/lib3rd.cmake)
include(${CMAKE_DIR}/sdk_gaeactor_engine.cmake)

find_package(Qt5 COMPONENTS Widgets REQUIRED)

set(QT_INCLUDES
    ${Qt5Widgets_INCLUDE_DIRS})

include_directories(${QT_INCLUDES})
set(QT_LIBRARIES
    Qt5::Widgets)

file(GLOB_RECURSE  SRCS_CPP ./*.cpp ./*.c ./*.cxx)
file(GLOB_RECURSE  SRCS_H ./*.h ./*.hpp)

add_executable(${PROJECT_NAME} ${SRCS_H} ${SRCS_CPP} )



# if(MSVC)
#     target_compile_options(${PROJECT_NAME} PRIVATE "/std:c11")
# elseif(CMAKE_C_COMPILER_ID MATCHES "GNU|Clang")
#     target_compile_options(${PROJECT_NAME} PRIVATE "-std=c11")
# endif()

target_link_libraries(${PROJECT_NAME} 
                        ${QT_LIBRARIES} 
                        ${LIB3RD_LIBRARIES}
                        ${sdk_gaeactor_engine})

target_compile_definitions(${PROJECT_NAME} PRIVATE UNICODE _UNICODE)

install(TARGETS ${PROJECT_NAME} ARCHIVE DESTINATION lib RUNTIME DESTINATION bin) 