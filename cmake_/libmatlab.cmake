# 设置第三方库的路径
set(3RD_MATLAB_DIR "${3RD_PATH}/matlab2022b_sdk" CACHE PATH "matlab2022b_sdk install path")
# 包含头文件目录
include_directories(${3RD_MATLAB_DIR}/rtw/c/src)
include_directories(${3RD_MATLAB_DIR}/simulink/include)
