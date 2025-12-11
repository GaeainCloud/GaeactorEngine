if (MSVC)
  add_compile_definitions (SQLITE_ENABLE_MATH_FUNCTIONS
    SQLITE_ENABLE_JSON1)  

  # 设置第三方库的路径
  set(3RD_SQLITEORM_DIR "${3RD_PATH}/sqlite_orm-master" CACHE PATH "sqlite_orm install path")
  set(3RD_SQLITEAMALGAMATION_DIR "${3RD_PATH}/sqlite-amalgamation-3450000" CACHE PATH "sqlite-amalgamation-3450000 install path")
  set(3RD_SQLITE3_DIR "${3RD_PATH}/sqlite3" CACHE PATH "sqlite3 install path")
  # 包含头文件目录
  include_directories(${3RD_SQLITEORM_DIR}/include)
  include_directories(${3RD_SQLITEAMALGAMATION_DIR})
  include_directories(${3RD_SQLITE3_DIR}/include)
  # 链接库目录
  link_directories("${3RD_SQLITE3_DIR}/lib/${CMAKE_BUILD_TYPE_DIR}")
  # 设置库名称

  ######################################################
  set(libsqlite sqlite)
  ######################################################

else()
  # 设置第三方库的路径
  set(3RD_SQLITEORM_DIR "${3RD_PATH}/sqlite_orm-1.8.2" CACHE PATH "sqlite_orm install path")
  set(3RD_SQLITE3_DIR "${3RD_PATH}/sqlite-3430100" CACHE PATH "sqlite3 install path")
  # 包含头文件目录
  include_directories(${3RD_SQLITEORM_DIR}/include)
  string(TOLOWER ${CMAKE_BUILD_TYPE} BUILD_TYPE_LOWER)
  include_directories(${3RD_SQLITE3_DIR}/${BUILD_TYPE_LOWER}/include)
  # 链接库目录
  link_directories("${3RD_SQLITE3_DIR}/${BUILD_TYPE_LOWER}/lib")
  # 设置库名称
  ######################################################
  set(libsqlite sqlite)
  ######################################################
endif ()