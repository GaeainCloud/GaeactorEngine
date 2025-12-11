# gaeactor 配置
project(gaeactor VERSION 1.0 LANGUAGES CXX)

include(${CMAKE_DIR}/lib3rd.cmake)
include(${CMAKE_DIR}/sdk_gaeactor_utils.cmake)
include(${CMAKE_DIR}/sdk_gaeactor_agent_cores.cmake)
include(${CMAKE_DIR}/sdk_gaeactor_agent_sensors.cmake)
include(${CMAKE_DIR}/sdk_gaeactor_interactions.cmake)
include(${CMAKE_DIR}/sdk_gaeactor_auditions.cmake)
include(${CMAKE_DIR}/sdk_gaeactor_event_engine.cmake)
include(${CMAKE_DIR}/sdk_gaeactor_transmit.cmake)
include(${CMAKE_DIR}/sdk_gaeactor_comm.cmake)
include(${CMAKE_DIR}/sdk_gaeactor_environment.cmake)
include(${CMAKE_DIR}/sdk_loggingservice.cmake)



# IF (MSVC)
#     set(ws232 ws2_32)
# ENDIF (MSVC)

set(SRCS_H 
    ${CMAKE_CURRENT_SOURCE_DIR}/gaeactortransmitmanager.h 
    ${CMAKE_CURRENT_SOURCE_DIR}/gaeactor_global.h)

set(SRCS_CPP 
    ${CMAKE_CURRENT_SOURCE_DIR}/gaeactortransmitmanager.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/main.cpp)

    # 在原有代码基础上优化
option(USING_GAEACTOR_EXPORT_LIB "Build as shared library" ON) # 添加可配置选项

if(USING_GAEACTOR_EXPORT_LIB)
    add_compile_definitions (USING_GAEACTOR_EXPORT_LIB)
    add_library(${PROJECT_NAME} SHARED ${SRCS_H} ${SRCS_CPP})
else()
    add_executable(${PROJECT_NAME} ${SRCS_H} ${SRCS_CPP} ${SRCS_UI_FORMS})
endif()

target_link_libraries(${PROJECT_NAME} 
                        ${QT_LIBRARIES} 
                        ${LIB3RD_LIBRARIES}
                        ${sdk_gaeactor_utils} 
                        ${sdk_gaeactor_agent_cores} 
                        ${sdk_gaeactor_agent_sensors} 
                        ${sdk_gaeactor_interactions} 
                        ${sdk_gaeactor_auditions} 
                        ${sdk_gaeactor_event_engine} 
                        ${sdk_gaeactor_transmit} 
                        ${sdk_gaeactor_comm} 
                        ${sdk_gaeactor_environment} 
                        ${sdk_loggingservice})
                        
#target_compile_definitions(${PROJECT_NAME} PRIVATE $<$<BOOL:${USING_GAEACTOR_EXPORT_LIB}>:GAEACTOR_LIBRARY> UNICODE _UNICODE BUILD_WITH_EASY_PROFILER)
                        
if(USING_GAEACTOR_EXPORT_LIB)
    target_compile_definitions(${PROJECT_NAME} PRIVATE GAEACTOR_LIBRARY UNICODE _UNICODE BUILD_WITH_EASY_PROFILER)

    # install(TARGETS ${PROJECT_NAME} LIBRARY DESTINATION lib RUNTIME DESTINATION bin)
    install(TARGETS ${PROJECT_NAME} ARCHIVE DESTINATION lib RUNTIME DESTINATION bin) 
    install(DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/
            DESTINATION ${DEPLOY_PATH}/${PLATFORM_DIR}/include/${PROJECT_NAME}
            FILES_MATCHING PATTERN "*.h" PATTERN "*.hpp")
else()
    target_compile_definitions(${PROJECT_NAME} PRIVATE  UNICODE _UNICODE BUILD_WITH_EASY_PROFILER)
    
    install(TARGETS ${PROJECT_NAME} ARCHIVE DESTINATION lib RUNTIME DESTINATION bin) 
endif()

