# gaeactor-record 配置
project(gaeactor-record VERSION 1.0 LANGUAGES CXX)


include(${CMAKE_DIR}/lib3rd.cmake)
include(${CMAKE_DIR}/sdk_gaeactor_utils.cmake)
include(${CMAKE_DIR}/sdk_gaeactor_utils_location.cmake)
include(${CMAKE_DIR}/sdk_gaeactor_transmit.cmake)
include(${CMAKE_DIR}/sdk_gaeactor_comm.cmake)
include(${CMAKE_DIR}/sdk_loggingservice.cmake)

# IF (MSVC)
#     set(ws232 ws2_32)
# ENDIF (MSVC)


find_package(Qt5 COMPONENTS Network REQUIRED)

set(QT_INCLUDES
    ${Qt5Network_INCLUDE_DIRS})

include_directories(${QT_INCLUDES})
set(QT_LIBRARIES
    Qt5::Network)

set(SRCS_H 
    ${CMAKE_CURRENT_SOURCE_DIR}/src/configmanager.h 
    ${CMAKE_CURRENT_SOURCE_DIR}/src/gaeactormanager.h 
    ${CMAKE_CURRENT_SOURCE_DIR}/src/gzip.h
    ${CMAKE_CURRENT_SOURCE_DIR}/src/jsonprocessor.h)

set(SRCS_CPP 
    ${CMAKE_CURRENT_SOURCE_DIR}/src/configmanager.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/gaeactormanager.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/gzip.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/jsonprocessor.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/main.cpp)

add_executable(${PROJECT_NAME} ${SRCS_H} ${SRCS_CPP})


target_link_libraries(${PROJECT_NAME} 
                        ${QT_LIBRARIES} 
                        ${LIB3RD_LIBRARIES}
                        ${sdk_gaeactor_utils}
                        ${sdk_gaeactor_utils_location}                        
                        ${sdk_gaeactor_transmit}
                        ${sdk_gaeactor_comm}
                        ${sdk_loggingservice})

target_compile_definitions(${PROJECT_NAME} PRIVATE UNICODE _UNICODE)

install(TARGETS ${PROJECT_NAME} ARCHIVE DESTINATION lib RUNTIME DESTINATION bin) 