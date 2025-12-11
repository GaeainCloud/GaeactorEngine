if (MSVC)
    # 设置第三方库的路径
    set(3RD_UWS_DIR "${3RD_PATH}/uWS" CACHE PATH "uws install path")
    # 包含头文件目录
    include_directories(${3RD_UWS_DIR}/include)
    # 链接库目录
    link_directories("${3RD_UWS_DIR}/lib/${CMAKE_BUILD_TYPE_DIR}")
    # 设置库名称
    set(libuws uWS)
else()
    # 设置第三方库的路径
    set(3RD_UWS_DIR "${3RD_PATH}/uWebSockets-0.14/src" CACHE PATH "uws install path")
    # 包含头文件目录
    include_directories(${3RD_UWS_DIR})
    
    set(UWS_SRC_H 
        ${3RD_UWS_DIR}/Asio.h
        ${3RD_UWS_DIR}/Epoll.h
        ${3RD_UWS_DIR}/HTTPSocket.h
        ${3RD_UWS_DIR}/Libuv.h
        ${3RD_UWS_DIR}/Socket.h
        ${3RD_UWS_DIR}/WebSocket.h
        ${3RD_UWS_DIR}/Backend.h
        ${3RD_UWS_DIR}/Group.h
        ${3RD_UWS_DIR}/Node.h
        ${3RD_UWS_DIR}/uWS.h
        ${3RD_UWS_DIR}/WebSocketProtocol.h
        ${3RD_UWS_DIR}/Extensions.h
        ${3RD_UWS_DIR}/Hub.h
        ${3RD_UWS_DIR}/Networking.h
        ${3RD_UWS_DIR}/Room.h)

    set(UWS_SRC_CPP 
        ${3RD_UWS_DIR}/Group.cpp
        ${3RD_UWS_DIR}/Node.cpp
        ${3RD_UWS_DIR}/Extensions.cpp
        ${3RD_UWS_DIR}/Hub.cpp
        ${3RD_UWS_DIR}/Networking.cpp
        ${3RD_UWS_DIR}/Epoll.cpp
        ${3RD_UWS_DIR}/Socket.cpp
        ${3RD_UWS_DIR}/WebSocket.cpp
        ${3RD_UWS_DIR}/HTTPSocket.cpp
        ${3RD_UWS_DIR}/Room.cpp)
endif ()
