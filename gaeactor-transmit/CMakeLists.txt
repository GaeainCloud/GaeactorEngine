# gaeactor-transmit 配置
project(gaeactor-transmit VERSION 1.0 LANGUAGES CXX)


include(${CMAKE_DIR}/lib3rd.cmake)
include(${CMAKE_DIR}/sdk_gaeactor_utils.cmake)
include(${CMAKE_DIR}/sdk_loggingservice.cmake)
include(${CMAKE_DIR}/sdk_gaeactor_comm.cmake)

set(SRCS_H 
    ${CMAKE_CURRENT_SOURCE_DIR}/gaeactor_transmit_define.h 
    ${CMAKE_CURRENT_SOURCE_DIR}/gaeactor_transmit_global.h 
    ${CMAKE_CURRENT_SOURCE_DIR}/gaeactor_transmit_interface.h
    ${CMAKE_CURRENT_SOURCE_DIR}/demservice/DemserviceStandalone.h
    ${CMAKE_CURRENT_SOURCE_DIR}/demservice/srtmHgtReader.h)

set(SRCS_CPP 
    ${CMAKE_CURRENT_SOURCE_DIR}/gaeactor_transmit_interface.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/demservice/DemserviceStandalone.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/demservice/srtmHgtReader.c)

add_library(${PROJECT_NAME} SHARED ${SRCS_H} ${SRCS_CPP})

# if(MSVC)
#     target_compile_options(${PROJECT_NAME} PRIVATE "/std:c11")
# elseif(CMAKE_C_COMPILER_ID MATCHES "GNU|Clang")
#     target_compile_options(${PROJECT_NAME} PRIVATE "-std=c11")
# endif()

# target_include_directories(${PROJECT_NAME} PUBLIC
#     ${CMAKE_CURRENT_SOURCE_DIR}
# )


target_link_libraries(${PROJECT_NAME} 
                        ${QT_LIBRARIES} 
                        ${LIB3RD_LIBRARIES}
                        ${sdk_loggingservice}
                        ${sdk_gaeactor_utils}
                        ${sdk_gaeactor_comm})

target_compile_definitions(${PROJECT_NAME} PRIVATE GAEACTOR_TRANSMIT_LIBRARY UNICODE _UNICODE)

# install(TARGETS ${PROJECT_NAME} LIBRARY DESTINATION lib RUNTIME DESTINATION bin)
install(TARGETS ${PROJECT_NAME} ARCHIVE DESTINATION lib RUNTIME DESTINATION bin) 
install(DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/
        DESTINATION ${DEPLOY_PATH}/${PLATFORM_DIR}/include/${PROJECT_NAME}
        FILES_MATCHING PATTERN "*.h" PATTERN "*.hpp")