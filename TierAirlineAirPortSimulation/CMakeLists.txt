# TierAirlineAirPortSimulation 配置
project(TierAirlineAirPortSimulation VERSION 1.0 LANGUAGES CXX)


include(${CMAKE_DIR}/lib3rd.cmake)

include(${CMAKE_DIR}/sdk_gaeactor_utils.cmake)
include(${CMAKE_DIR}/sdk_gaeactor_utils_location.cmake)
include(${CMAKE_DIR}/sdk_gaeactor_transmit.cmake)
include(${CMAKE_DIR}/sdk_gaeactor_comm.cmake)
include(${CMAKE_DIR}/sdk_loggingservice.cmake)
include(${CMAKE_DIR}/sdk_crashservice.cmake)



find_package(Qt5 COMPONENTS Network REQUIRED)
find_package(Qt5 COMPONENTS Gui REQUIRED)

set(QT_INCLUDES
    ${Qt5Gui_INCLUDE_DIRS}
    ${Qt5Network_INCLUDE_DIRS})

include_directories(${QT_INCLUDES})
set(QT_LIBRARIES
    Qt5::Gui
    Qt5::Network)

# file(GLOB_RECURSE  SRCS_CPP ./*.cpp ./*.c ./*.cxx)
# file(GLOB_RECURSE  SRCS_H ./*.h ./*.hpp)

set(SRCS_H 
    ${CMAKE_CURRENT_SOURCE_DIR}/src/components/configmanager.h
    ${CMAKE_CURRENT_SOURCE_DIR}/src/components/eventdriver/eventdriver.h
    ${CMAKE_CURRENT_SOURCE_DIR}/src/datamanager/datamanager.hpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/components/function.h
    ${CMAKE_CURRENT_SOURCE_DIR}/src/components/gaeactormanager.h
    ${CMAKE_CURRENT_SOURCE_DIR}/src/components/httpclient/httpclient.hpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/uiwidget/widgetmanager.h
    ${CMAKE_CURRENT_SOURCE_DIR}/src/httpserver/httpserver/HttpServerComponent.hpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/httpserver/httpserver/ErrorHandler.hpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/httpserver/httpserver/SwaggerComponent.hpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/httpserver/httpserver/controller/StaticController.hpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/httpserver/httpserver/dto/StatusDto.hpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/httpserver/httpserver/handler.h
    ${CMAKE_CURRENT_SOURCE_DIR}/src/httpserver/httpserver.h
    ${CMAKE_CURRENT_SOURCE_DIR}/src/httpserver/httpserver/controller/AgentController.hpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/httpserver/httpserver/dto/AgentDto.hpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/httpserver/httpserver/dto/PageDto.hpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/httpserver/httpserver/service/AgentService.hpp)

set(SRCS_CPP 
    ${CMAKE_CURRENT_SOURCE_DIR}/src/components/configmanager.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/components/eventdriver/eventdriver.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/datamanager/datamanager.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/components/function.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/components/gaeactormanager.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/components/httpclient/httpclient.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/main.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/uiwidget/widgetmanager.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/httpserver/httpserver/ErrorHandler.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/httpserver/httpserver/service/AgentService.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/httpserver/httpserver.cpp)
    

add_executable(${PROJECT_NAME} ${SRCS_H} ${SRCS_CPP} )


target_link_libraries(${PROJECT_NAME} 
                        ${QT_LIBRARIES} 
                        ${LIB3RD_LIBRARIES}
                        ${sdk_gaeactor_utils}
                        ${sdk_gaeactor_utils_location}                        
                        ${sdk_gaeactor_transmit}
                        ${sdk_gaeactor_comm}
                        ${sdk_loggingservice}
                        ${sdk_crashservice})

target_compile_definitions(${PROJECT_NAME} PRIVATE UNICODE _UNICODE)

install(TARGETS ${PROJECT_NAME} ARCHIVE DESTINATION lib RUNTIME DESTINATION bin) 