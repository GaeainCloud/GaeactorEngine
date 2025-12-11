if (MSVC)

else()
    # 设置第三方库的路径
    set(3RD_NCURSES_DIR "${3RD_PATH}/ncurses-6.3" CACHE PATH "ncurses install path")
    # 包含头文件目录
    include_directories(${3RD_NCURSES_DIR}/$<CONFIG>/include)
    # 链接库目录
    link_directories("${3RD_NCURSES_DIR}/$<CONFIG>/lib")
    # 设置库名称
    ######################################################
    set(libncurses form form_g menu menu_g ncurses++ ncurses ncurses++_g ncurses_g panel panel_g)
    ######################################################
endif ()