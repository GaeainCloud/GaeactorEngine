1. 概述
由于依赖的Qt框架是VS2019，所以需要使用VS2019工程

![VS2019工程项目总览](images/100.png)

2. 代码编译
1.拉取代码
```bash
#/bn/bash
platform=linux_amd64
CRTDIR=$(pwd)
check_code_dir() {
    local dir_path="$1"
    local git_branch="$2"
    local git_branch_src="$3"
    local git_url="$4"
    echo "check code directory: $dir_path"
    if [ ! -d "$dir_path" ]; then
        # 如果不存在，创建目录
        echo "git clone :  $git_branch $git_url"
        #git clone -b $git_branch $git_url --depth=1
        git clone -b $git_branch $git_url
    else
        # 如果存在，清理目录内容
        cd $dir_path
        git fetch --all
        echo "git pull :  $git_branch_src $git_url"
        git reset --hard origin/$git_branch_src
        git pull origin $git_branch_src
    fi
    cd $CRTDIR
}

```
2.使用vs2019及qt5.15.2环境

3.下载3rd环境
在聊天消息文件中搜索文件：win64_20250714.zip，大约2.5GB文件大小，下载下来，里面就是3rd文件夹内容。修改CMakeLists.txt中的配置3RD_PATH 和 QT_PATH。注意在上述pullcode.sh脚本所在的目录下的根目录下，新建一个CMakeLists.txt文件，相对关系如下图所示：
![](images/110.png)
新建的CMakeLists.txt文件内容如下：
IF (MSVC)
    set(PLATFORM_DIR "win64" CACHE PATH "You must provide a valid PLATFORM_DIR to continue")
    # 强制用户输入部署路径
    set(3RD_PATH "C:/Workplace/GaeaWorkshop/Gaea/3rd/win64" CACHE PATH "You must provide a valid 3RD_PATH to continue")
    if(NOT 3RD_PATH)
        message(FATAL_ERROR "3RD_PATH must be set to a valid path!")
    endif()

    set(QT_PATH "C:/Qt/5.15.2/msvc2019_64/lib/cmake" CACHE PATH "You must provide a valid QT_PATH to continue")
    if(NOT QT_PATH)
        message(FATAL_ERROR "QT_PATH must be set to a valid path!")
    endif()
ELSE()
    set(PLATFORM_DIR "lnx" CACHE PATH "You must provide a valid PLATFORM_DIR to continue")
    # 强制用户输入部署路径
    set(3RD_PATH "/home/lj/work/projects/3rd/lnx" CACHE PATH "You must provide a valid 3RD_PATH to continue")
    if(NOT 3RD_PATH)
        message(FATAL_ERROR "3RD_PATH must be set to a valid path!")
    endif()

    set(QT_PATH "/opt/Qt/5.15.2/gcc_64/lib/cmake" CACHE PATH "You must provide a valid QT_PATH to continue")
    if(NOT QT_PATH)
        message(FATAL_ERROR "QT_PATH must be set to a valid path!")
    endif()
ENDIF(MSVC)


![](images/120.png)

CMAKE 配置
DISABLE_EVALUATE_SCRIPT_BY_EXPRTK 启动或禁用 exprtk脚本模块
DEPLOY_PATH 配置部署路径
![](images/130.png)
CMAKE_BUILD_TYPE   配置编译类型           Debug/RelWithDebInfo/Release
CMAKE_BUILD_TYPE_DIR   配置编译类型的3rd对应路径  Debug---->Debug         RelWithDebInfo/Release---->Release
然后使用cmake生成工程 vs2019 x64
![](images/140.png)

![](images/150.jpg)
Cmake里面CMAKE_BUILD_TYPE 指定的Debug/Release/RelWithDebInfo,就在这个批生成里生成啥类型的

3. 编译
![](images/160.png)
![](images/170.png)
![](images/180.png)
![](images/190.png)

4. 部署
将environment中的对应支持文件部署到对应位置

![](images/200.png)

同时注意：
![](images/210.jpg)
是由于缺少afsim相关库，所在目录是：
![](images/220.png)


5. 运行
![](images/230.png)

```bash
set local_ip=127.0.0.1
set group_name=default
set gaeactor_hub_http_port=18087
set gaeactor_hub_websocket_port=31769
set gaeactor_hub_heartbeat_port=31770
set gaeactor_process_monitor_http_port=18093

set gaeactor_hub_manager_http_port=18088
set gaeactor_hub_manager_websocket_port=18092

set redis_ip=127.0.0.1
set redis_port=16379
set redis_pwd=gaeactor_MaaS++
//redis 服务
start ../../../../../environment/Debug/redis-7.0.15/redis-server.exe --bind %redis_ip% --port %redis_port% --requirepass %redis_pwd%
timeout /t 2
//18088相关http服务
::start ./gaeactor-hub-manager.exe %group_name% %local_ip% %gaeactor_hub_http_port% %gaeactor_hub_websocket_port% %gaeactor_hub_heartbeat_port% %gaeactor_hub_manager_websocket_port% %redis_ip% %redis_port% %redis_pwd%
start ./gaeactor-hub-manager.exe %group_name% %local_ip% %gaeactor_hub_manager_http_port% %gaeactor_hub_manager_websocket_port% %gaeactor_hub_http_port% %gaeactor_hub_websocket_port% %gaeactor_hub_heartbeat_port% %redis_ip% %redis_port% %redis_pwd%

timeout /t 2
//18087相关http服务，仿真实例化服务 
start ./gaeactor-hub.exe %group_name% %local_ip% %gaeactor_hub_http_port% %gaeactor_hub_websocket_port% %gaeactor_hub_heartbeat_port% %gaeactor_hub_manager_websocket_port% %redis_ip% %redis_port% %redis_pwd%
//调试桌面端前台
start ./LaViCDesktop.exe
//感知域敌我识别服务
start ./SensorClusterService.exe %group_name% %redis_ip% %redis_port% %redis_pwd%
//感知域物理属性增益服务
start ./CommPhysicsLayerService.exe %group_name% %redis_ip% %redis_port% %redis_pwd%
//记录debug信息到文件
start ./gaeactor-process-monitor.exe %group_name% %local_ip% %gaeactor_process_monitor_http_port% %redis_ip% %redis_port% %redis_pwd%
::timeout /t 1
::start ./MPAR-Service.exe %group_name%
::timeout /t 1
::start ./sar_runtime.exe %group_name%
```


6. 调试
TODO：
补充一下如何基于LaViC进行一些基本业务的调试，例如从前台数据库拿到一个想定文件后，如何进行调试？
1.从前台获取想定的doeInstData.json 文件，并替换到如下目录
![](images/240.png)
文件格式内容导致如下
```json
{
  "agents": [
  ],
  "agentRunningPattern": {
    "patternSig": "pttn_228888681705979912",
    "patternName": null,
    "patternNameI18n": null,
    "patternNote": null,
    "patternAgents": [],
    "timelines": null,
    "audit": null
  },
  "doctrinesDto": {
    "doctrines": []
  },
  "formations": {
    "formations": []
  },
  "doeConfig": {
    "doeConfigSig": "doecfg_228841409438535680",
    "attachedScenarioSig": "sostep_228841409424773120",
    "topic": "防空反导想定_副本（1）",
    "background": null,
    "purpose": null,
    "samplingMethod": "田口方法",
    "factorDesign": "均匀设计",
    "runningInstanceNumMax": null,
    "varSelections": [],
    "scenarioLifecycleCtrls": null,
    "doeResult": {
      "doeResultSignature": "doeinst_228841409438535682",
      "attachedDoEConfigSig": "doecfg_228841409438535680",
      "varSamples": [],
      "refValue": 0
    }
  }
}
```

2.安装python环境
安装对应的库
```bash
pip install requests
```

3.启动想定仿真环境，按第五步骤运行，双击执行run.bat
4.在vs code 终端中执行
```bash
python.exe ./exec.py 
```

可根据部署情况，适当修改exec.py，例如
- 如需给远程发送想定内容，则对应修改ip
- 如需加载其他路径下的想定内容，则对应修改加载文件的路径

```python
import subprocess
import urllib.parse
import json
import time

import http_send

ip='127.0.0.1'
#ip='192.168.2.107'
agentdata_url='http://'+ip+':18087/AgentData'
patterndata_url='http://'+ip+':18087/RuntimeData'
doe_url='http://'+ip+':18087/ExperimentConfig'
doc_url='http://'+ip+':18087/DoctrinesConfig'
cmd_url='http://'+ip+':18087/PoiCmd'

formations_url='http://'+ip+':18087/FormationsData'

def exe_cmd_post(url, filepath):
    file_contents = http_send.loaddata_json(filepath)    
    #print(file_contents)
    return http_send.exe_request_post(url,file_contents)

def exe_cmd_post_contents(url, file_contents):
    #print(file_contents)
    return http_send.exe_request_post(url,file_contents)

def read_json_file(file_path):
    with open(file_path, 'r', encoding='utf-8') as file:
        data = json.load(file)
        
    # 提取指定的键值
    
    agents_data = {"agents": data.get("agents", [])}
    result = {
        'agents': agents_data,
        'agentRunningPattern': data.get('agentRunningPattern', {}),
        'doctrinesDto': data.get('doctrinesDto', {}),
        'formations': data.get('formations', {}),
        'doeConfig': data.get('doeConfig', {})
    }
    
    return result

extracted_data = read_json_file('./contents/doeInstData.json')

exe_cmd_post_contents(agentdata_url,extracted_data['agents'])
exe_cmd_post_contents(patterndata_url,extracted_data['agentRunningPattern'])
exe_cmd_post_contents(doc_url,extracted_data['doctrinesDto'])
exe_cmd_post_contents(doe_url,extracted_data['doeConfig'])
```

5.gaeactor-process-viewer.exe可查看各类数据
![](images/250.jpg)
![](images/260.jpg)
6.也可使用vs附加到进程进行代码调试
![](images/270.png)

7. 配置文件说明
![](images/280.png)

advancedsettings.json，设置内部各项线程池数量
```json
{
    "cognitivecluster_threads":10,
    "commcluster_frame_threads":20,
    "commcluster_ecal_sync_frame_threads":20,
    "commphysicallayer_threads":10,
    "doctrinecluster_threads":10,
    "sensorcluster_route_theads":10,

    "gaeactor_hub_data_frame_threads":10,
    "gaeactor_hub_event_frame_threads":10,

    "gaeactor_hub_action_event_threads":20,
    "gaeactor_hub_http_event_threads":10,
    "gaeactor_hub_heartbeat_event_threads":10,
    "gaeactor_hub_data_commstacksendframe_threads":20,
    "gaeactor_hub_data_ecal_sync_frame_threads":10,
    "gaeactor_hub_data_custom_msg_frame_threads":10,
    "gaeactor_hub_patterncreateinstance_threads":10,
    "gaeactor_hub_detect_event_threads":40,
    "gaeactor_hub_agent_step_threads":60
}
```

rediscfg.json，设置redis相关的参数设置
```json
{
    "redis_ip":"127.0.0.1",
    "redis_port":16379,
    "redis_pwd":"gaeactor_MaaS++",
    "redis_user":"gaeactor"
}
```

runningmode.json，运行模式设置
```json
{
    "USING_TRANS_MODE_POSDATA":true,//是否对外发送实体位置迭代数据
    "USING_TRANS_MODE_POSDATA_POI":true,//是否对外发送兴趣点位置数据
    "USING_TRANS_MODE_SENSORDATA":true,//是否对外发送感知域迭代数据
    "USING_TRANS_MODE_EVENTDATA":true,//是否对外发送检测事件数据
    "USING_TRANS_MODE_SMDINFODATA":false, //是否对外发送感知域信息数据
    "USING_RECORD_RECEIVE_DATA":false,//是否记录接收的数据【agentdata/partterndata/编队/实验】
    "USING_RECORD_REVIEW_DATA":true,//是否回放数据
    "USING_RECORD_HDF5_DATA":true,//是否hdf5形式的运行数据
    "USING_RECORD_HDF5_DATA_USERDEF":true,//是否hdf5形式的自定义参数数据
    "USING_RECORD_HDF5_DATA_INERVALVAR":true,//是否hdf5形式的内部数据参数数据【检测事件】
    "USING_RECORD_HDF5_DATA_AGENTPOS":true,//是否hdf5形式的实体位置/感知域参数数据
    "USING_RECORD_SQLITE_DATA":true,//是否sqlite形式的数据
    "USING_CREATE_BY_RUNTIMEDATA":false,//是否启用接收到运行样式数据后就创建agent实例，进行仿真
    "USING_EXPERIMENT_AUTO_START":true,//是否启动接收到实验数据之后自动启动仿真
    "USING_DEAL_UPDATE_EVENT":true,//是否处理Update类型的检测事件
    "USING_DEAL_SENSINGMEDIA_FILTER":true,//是否处理过滤感知域
    "USING_DEAL_ATTACHED_FILTER":true,//是否过滤不处理检测事件中发现的处理挂载状态的实例
    "USING_DEAL_ATTACHED_GENERATE":false,//是否处理挂载状态的agent实例
    "USING_DEAL_EVENT_BY_TIMESTAMP_ORDER":true,//是否按照检测事件的发生时间戳顺序处理事件
    "USING_DEAL_GENERATE_SENSE_BY_SWEEP":true,//是否采用扫略模式生成感知的多边形范围
    "USING_DEAL_HIGH_TRIPLE_SPEED":true,//是否采用高倍速采样模式更新位置
    "MODE_USING_CLEAR_OLD_SENSOR_SENSINGMEDIA_FORCE_COVER_MODE":true,//是否启用感知域更新后，强制清理之前迭代产生的感知域数据
    "MODE_USING_FILTER_TARGET":false,//是否只处理经纬度在合理范围内的目标【使用于通信栈传输的数据处理流程】
    "MODE_USING_ECHO_GENERATE_DIRECT":true,//是否启用处理检测事件函数过程中直接生成回波
    "MODE_USING_DEBUG_INFO":true,//是否生成debug信息
    "MODE_USING_939_MODE":true,//是否939模式【单线程，强时序】
    "MODE_USING_RUNNING_PARALLEL_MODE":false//是否常规模式【多线程，并行模式，弱时序】

}
```

settingsconfig.json
```json
{
    "localip":"127.0.0.1", //本地ip
    "heartbeatwebsocketport":31770,//心跳webcoket端口    
    "heartbeatwebinterval":1000,//心跳对外频率
    "commsnr_update_interval":-1,//snr对外更新频率
    "simparams_update_interval":3000,    //仿真参数对外更新频率
    "refersh_edge_interval":5000,//感知边刷新频率
    "sensorcluster_deal_interval":50,//敌我识别处理频率
    "commphylar_deal_interval":5000,//感知域物理特性【增益/干扰】处理频率

    "gaeactor_hub_http_port":18087,//agentdata/运行样式等想定数据接收对外http服务端口
    "gaeactor_hub_websocket_port":31769,//仿真运行数据对外webcoket端口

    "gaeactor_hub_manager_http_port":18088,//心跳webcoket端口  
    "gaeactor_hub_manager_websocket_port":18092,

    "enable_gaector_hub_return":true,

    "LavicDesktop_log_port":18090,
    "base_step_timeinterval":100,//基础step周期
    //gaeactor相关http服务的url
    "execute_agentpattern_url":"RuntimeData",
    "execute_agentdata_url":"AgentData",
    "execute_runningspeed_url":"RunningSpeed",
    "execute_poicmd_url":"PoiCmd",
    "get_runningspeed_url":"getRunningSpeed",

    "max_cells":70,//感知域网格化最大网格限制数两
    "using_assign_res":false,//是否强制使用using_res分辨率对感知域网格化
    "using_res":14,//强制使用using_res分辨率对感知域网格化的分辨率等级
    "iff_begin_timeinterval":100,//敌我识别周期
    "ooda_update_times":1,//OODA更新频次
    "ooda_invalid_timeout":9000,//OODA无效超时时间
    "ooda_update_period":3000,//OODA更新周期
    "agent_dt_min":0.0001,//实体dt的最小值
    "agent_dt_max":1000,//实体dt的最大值
    "agent_ncycles_min":1,//实体dt的间隔周期的最小值
    "agent_ncycles_max":1000,//实体dt的间隔周期的最大值
    "evaluate_process_timeinterval":1000,    //评估触发生成周期
    "record_local_memory":true//日志记录采用本地内存通信还是采用websocket通信获取数据
}
```

lavic_desktopcfg.json
```json
{
    "recording": true,
    "localip":"127.0.0.1",

    "display_remote":true,
    "display_remote_url":"ws://127.0.0.1:31769",
    
    "LavicDesktop_http_port":18089,
    "LavicDesktop_log_port":18090,
    "LavicDesktop_review_port":18091,
    //航线冲突服务及数据端口
    "Deconflictor_http_port":19091,
    "Deconflictor_gui_http_port":19092,
    
    //lavic相关http服务的url
    "tokenstr":"Authorization: Bearer admin-Token=eyJhbGciOiJIUzI1NiIsInR5cCI6IkpXVCJ9.eyJhdWQiOiJHYWVhY3RvcisrQDU2NDE4OTI2NTkwMTI2NTQ2MyIsInBhc3N3b3JkIjoiR2FlYUAyMDIxMDUxNyIsImV4cCI6MTcyMzI0MzY1MywiaWF0IjoxNzIzMjAwNDUzLCJhY2NvdW50IjoiZ2FlYWN0b3IifQ.jM3D8-gm8yUDSzWnvYnt0aNimCDwjKTu0fTYcSzzxsU",
    "delete_action_url":"http://192.168.2.106:7980/api/v1/lavic_agent/deleteAction?agentKey=%1&scriptId=%2",
    "delete_agentdata_url":"http://192.168.2.106:7980/api/v1/lavic_agent/deleteAgent?id=%1",
    "delete_FldmdforAgent_url":"http://192.168.2.106:7980/api/v1/lavic_agent/deleteFldmdforAgent?agentKey=%1&fldmdKey=%2",
    "delete_OODAData_url":"http://192.168.2.106:7980/api/v1/lavic_agent/deleteOODAData?agentKey=%1&scriptId=%2",
    "delete_Variable_url":"http://192.168.2.106:7980/api/v1/lavic_agent/deleteVariable?agentKey=%1&varKeyword=%2&varSig=%3",
    "get_AgentByIdurl":"http://192.168.2.106:7980/api/v1/lavic_agent/getAgentById?id=%1",
    "get_AgentByAgentIdurl":"http://192.168.2.106:7980/api/v1/lavic_agent/getInstanceByAgentId?agentId=%1",
    "get_agentdata_url":"http://192.168.2.106:7980/api/v1/lavic_agent/getAllAgent",
    "save_Action_url":"http://192.168.2.106:7980/api/v1/lavic_agent/saveAction",
    "save_agentdata_url":"http://192.168.2.106:7980/api/v1/lavic_agent/saveAgent",    
    "save_FldmdForAgent_url":"http://192.168.2.106:7980/api/v1/lavic_agent/saveFldmdForAgent",
    "save_OODAData_url":"http://192.168.2.106:7980/api/v1/lavic_agent/saveOODAData",
    "save_SmdforFldmd_url":"http://192.168.2.106:7980/api/v1/lavic_agent/saveSmdforFldmd",
    "save_Variable_url":"http://192.168.2.106:7980/api/v1/lavic_agent/saveVariable",
    "select_FldmdForAgent_url":"http://192.168.2.106:7980/api/v1/lavic_agent/selectFldmdForAgent?agentKey=%1",
    "update_Action_url":"http://192.168.2.106:7980/api/v1/lavic_agent/updateAction",
    "update_Agent_url":"http://192.168.2.106:7980/api/v1/lavic_agent/updateAgent",
    "update_FldmdforAgent_url":"http://192.168.2.106:7980/api/v1/lavic_agent/updateFldmdforAgent",
    "update_OODAData_url":"http://192.168.2.106:7980/api/v1/lavic_agent/updateOODAData",
    "update_Smd_url":"http://192.168.2.106:7980/api/v1/lavic_agent/updateSmd",
    "update_Variable_url":"http://192.168.2.106:7980/api/v1/lavic_agent/updateVariable",

    "get_agentpattern_url":"http://192.168.2.106:7980/api/v1/lavic_agent/getAllAgentPattern",
    "add_agentpattern_url":"http://192.168.2.106:7980/api/v1/lavic_agent/saveAgentPattern",
    "update_agentpattern_url":"http://192.168.2.106:7980/api/v1/lavic_agent/updateAgentPattern",
    "delete_agentpattern_url":"http://192.168.2.106:7980/api/v1/lavic_agent/deleteAgentPatternById",    

    //gaeactor相关http服务的url
    "execute_agentpattern_url":"http://127.0.0.1:18087/RuntimeData",
    "execute_agentdata_url":"http://127.0.0.1:18087/AgentData",
    "execute_runningspeed_url":"http://127.0.0.1:18087/RunningSpeed",
    "get_runningspeed_url":"http://127.0.0.1:18087/getRunningSpeed",
    "execute_poicmd_url":"http://127.0.0.1:18087/PoiCmd",

    //地图相关配置url
    "map_using_local":false,    
    "map_web_url":"http://47.241.108.186/googlemap/vt/lyrs=s&hl={lcl}&x={x}&y={y}&z={z}",
    
    "map_elevation_dir":"E:/hgt/HGT",
    "map_local_dir1":"E:/SaveMapTiles/Bing/Satellite/{z}/{x}-{y}.png",
    "map_local_dir":"E:/map/sc/Bing/Satellite/{z}/{x}-{y}.jpg",
    "map_elevation_cache_dir":"D:/earth/LocalElevation/",
    "map_image_cache_dir":"D:/earth/LocalImage/",
    "showmap":true,
    

    "map_web_url2":"http://192.168.2.61:32696/SaveMapTiles/Bing/Satellite/{z}/{x}-{y}.jpg",    
    "map_web_url__":"https://mts1.google.com/vt/lyrs=s@186112443&hl={lcl}&x={x}&y={y}&z={z}&s=Galile",
    "map_web_url_":"http://47.241.108.186/googlemap/vt/lyrs=s&hl={lcl}&x={x}&y={y}&z={z}",
    "map_web_url_osm1":"http://a.tile.openstreetmap.org/{z}/{x}/{y}.png",
    "map_web_url_osm2":"http://b.tile.openstreetmap.org/{z}/{x}/{y}.png",
    "map_web_url_osm3":"http://c.tile.openstreetmap.org/{z}/{x}/{y}.png",

    "map_web_url_google_satellite1":"https://mts0.google.com/vt/lyrs=s@186112443&hl={lcl}&x={x}&y={y}&z={z}&s=Galile",
    "map_web_url_google_satellite2":"https://mts1.google.com/vt/lyrs=s@186112443&hl={lcl}&x={x}&y={y}&z={z}&s=Galile",
    "map_web_url_google_satellite3":"https://mts2.google.com/vt/lyrs=s@186112443&hl={lcl}&x={x}&y={y}&z={z}&s=Galile",

    "map_web_url_google_schema1":"https://mts0.google.com/vt/lyrs=m@110&hl={lcl}&x={x}&y={y}&z={z}",
    "map_web_url_google_schema2":"https://mts1.google.com/vt/lyrs=m@110&hl={lcl}&x={x}&y={y}&z={z}",
    "map_web_url_google_schema3":"https://mts2.google.com/vt/lyrs=m@110&hl={lcl}&x={x}&y={y}&z={z}",

    "map_web_url_google_hybird1":"https://mts0.google.com/vt/lyrs=s,m@110&hl={lcl}&x={x}&y={y}&z={z}",
    "map_web_url_google_hybird2":"https://mts1.google.com/vt/lyrs=s,m@110&hl={lcl}&x={x}&y={y}&z={z}",
    "map_web_url_google_hybird3":"https://mts2.google.com/vt/lyrs=s,m@110&hl={lcl}&x={x}&y={y}&z={z}",

    "map_web_url_bing_satellite1":"http://t0.tiles.virtualearth.net/tiles/a{qk}.jpeg?g=181&mkt={lcl}",
    "map_web_url_bing_satellite2":"http://t1.tiles.virtualearth.net/tiles/a{qk}.jpeg?g=181&mkt={lcl}",
    "map_web_url_bing_satellite3":"http://t2.tiles.virtualearth.net/tiles/a{qk}.jpeg?g=181&mkt={lcl}",

    "map_web_url_bing_schema1":"http://t0.tiles.virtualearth.net/tiles/r{qk}.jpeg?g=181&mkt={lcl}",
    "map_web_url_bing_schema2":"http://t1.tiles.virtualearth.net/tiles/r{qk}.jpeg?g=181&mkt={lcl}",
    "map_web_url_bing_schema3":"http://t2.tiles.virtualearth.net/tiles/r{qk}.jpeg?g=181&mkt={lcl}",

    "map_web_url_bing_hybird1":"http://t0.tiles.virtualearth.net/tiles/h{qk}.jpeg?g=181&mkt={lcl}",
    "map_web_url_bing_hybird2":"http://t1.tiles.virtualearth.net/tiles/h{qk}.jpeg?g=181&mkt={lcl}",
    "map_web_url_bing_hybird3":"http://t2.tiles.virtualearth.net/tiles/h{qk}.jpeg?g=181&mkt={lcl}"

}
```



8. 协议扩展
通信snr协议
参见doc-protos/AgentCommSnrInfo.proto文件
位置感知域协议
参见doc-protos/AgentPositionInfo.proto文件
实体关系协议
参见doc-protos/AgentRelationInfo.proto文件
通信协议栈协议
参见doc-protos/CommStackFrameElement.proto文件
参见doc-protos/CommStackFrameResultElement.proto文件
自定义报文协议
参见doc-protos/CustomTransInfo.proto文件
调试信息协议
参见doc-protos/DebugInfo.proto文件
mpar相关协议
参见doc-protos/MparInfo.proto文件
参见doc-protos/MPARMotionInfo.proto文件
参见doc-protos/ProcessTransInfo.proto文件
sar相关协议
参见doc-protos/SARMotionInfo.proto文件
仿真参数相关协议
参见doc-protos/SimParamsInfo.proto文件


9. 协议修改及扩展

暂时无法在飞书文档外展示此内容
      
```PowerShell
::..\..\..\3rd\win64\ecal-5.12.4\bin\release\protoc -I=. --cpp_out=.\protoc CommStackFrameElement.proto
::..\..\..\3rd\win64\ecal-5.12.4\bin\release\protoc -I=. --cpp_out=.\protoc AgentRelationInfo.proto
..\..\..\3rd\win64\ecal-5.12.4\bin\release\protoc -I=. --cpp_out=.\protoc AgentPositionInfo.proto
::..\..\..\3rd\win64\ecal-5.12.4\bin\release\protoc -I=. --cpp_out=.\protoc AgentCommSnrInfo.proto
::..\..\..\3rd\win64\ecal-5.12.4\bin\release\protoc -I=. --cpp_out=.\protoc CommStackFrameResultElement.proto
::..\..\..\3rd\win64\ecal-5.12.4\bin\release\protoc -I=. --cpp_out=.\protoc SimParamsInfo.proto
::..\..\..\3rd\win64\ecal-5.12.4\bin\release\protoc -I=. --cpp_out=.\protoc MparFrame.proto
::..\..\..\3rd\win64\ecal-5.12.4\bin\release\protoc -I=. --cpp_out=.\protoc ProcessTransInfo.proto
::..\..\..\3rd\win64\ecal-5.12.4\bin\release\protoc -I=. --cpp_out=.\protoc DebugInfo.proto
::..\..\..\3rd\win64\ecal-5.12.4\bin\release\protoc -I=. --cpp_out=.\protoc CustomTransInfo.proto
::..\..\..\3rd\win64\ecal-5.12.4\bin\release\protoc -I=. --cpp_out=.\protoc SARMotionInfo.proto
::..\..\..\3rd\win64\ecal-5.12.4\bin\release\protoc -I=. --cpp_out=.\protoc MPARMotionInfo.proto

::..\..\..\3rd\win64\ecal-5.12.4\bin\release\protoc --js_out=import_style=commonjs,binary:.\protoc_javascript CommStackFrameElement.proto
::..\..\..\3rd\win64\ecal-5.12.4\bin\release\protoc --js_out=import_style=commonjs,binary:.\protoc_javascript AgentRelationInfo.proto
::..\..\..\3rd\win64\ecal-5.12.4\bin\release\protoc  --js_out=import_style=commonjs,binary:.\protoc_javascript AgentPositionInfo.proto
::..\..\..\3rd\win64\ecal-5.12.4\bin\release\protoc  --js_out=import_style=commonjs,binary:.\protoc_javascript AgentCommSnrInfo.proto
::..\..\..\3rd\win64\ecal-5.12.4\bin\release\protoc --js_out=import_style=commonjs,binary:.\protoc_javascript CommStackFrameResultElement.proto
```
    

协议扩展
以修改 AgentPositionInfo.proto 为例

```proto
syntax = "proto3";

package msg.AgentPositionInfo;

enum E_DISPLAY_MODE {
    E_DISPLAY_MODE_ENTITY=0;
    E_DISPLAY_MODE_WAVE=1;
    E_DISPLAY_MODE_INTERSECTION=2;
    E_DISPLAY_MODE_ECHO=3;
};

enum E_EVENT_MODE
{
    E_EVENT_MODE_NULL=0;
    E_EVENT_MODE_ADD=1;
    E_EVENT_MODE_REMOVE=2;
    E_EVENT_MODE_UPDATE=3;
};

message msg_transdata_entityposinfo
{
    string PARAM_pos_pack_index=1;
    string PARAM_timestamp=2;
    string PARAM_sim_timestamp=3;
    uint32 PARAM_sensor_property=4;
    string   PARAM_pos_hexidx=5;
    double   PARAM_longitude=6;
    double   PARAM_latitude=7;
    double   PARAM_amsl=8;
    double   PARAM_terrain_elevation=9;
    int32   PARAM_ref=10;
    float PARAM_roll=11;
    float PARAM_pitch=12;
    float PARAM_yaw=13;
    int32   PARAM_speed=14;
    int32 param_reserved1=15;
    int32 param_reserved2=16; 
    int32 param_reserved3=17; 
    int32 param_reserved4=18; 
    int32 param_reserved5=19; 
    int32 param_reserved6=20; 
    int32   PARAM_attached=21;
    string PARAM_attached_agentid=22;
};

message msg_transentityhexidxpostdata 
{
    string uildsrc=1;
    string uilddst=2;    
    E_DISPLAY_MODE eDdisplayMode=3;
    msg_transdata_entityposinfo entityinfo=4;    
};


message msg_transentityhexidxpostdata_array 
{
    repeated msg_transentityhexidxpostdata transentityhexidxpostdata = 1; 
};

message msg_transdata_sensorposinfo
{
    string PARAM_sensor_pack_index=1;
    string PARAM_timestamp=2;
    string PARAM_sim_timestamp=3;
    string PARAM_source_sensingmediaid=4;
    uint32 PARAM_elecmag_wave_properties=5;
    uint32 PARAM_sound_wave_properties=6;
    int32  PARAM_wave_freq_mean_sgnfcnt=7;
    int32  PARAM_wave_freq_std_dvtn_sgnfcnt=8;
    uint32 PARAM_wave_freq_distbtn=9;
    uint32 PARAM_wave_frequency_scale=10;
    uint32 PARAM_wave_usage=11;
    uint32 PARAM_wave_snd_rcv=12;
    uint32 PARAM_wave_silent_time_gap=13;
    uint32 PARAM_wave_field_media_id=14;
    string PARAM_wave_modsig_id=15;

    double PARAM_wave_power_in_watts=16;
    uint32 PARAM_wave_propagation_model=17;

    double PARAM_wave_direction_pitch=18;
    double PARAM_wave_direction_azimuth=19;
    uint32 PARAM_wave_gaingraph=20;
    double PARAM_wave_direction_radius=21;
};


message msg_TYPE_POLYGON_PT
{
    double   PARAM_longitude=1;
    double   PARAM_latitude=2;
    double   PARAM_amsl=3;
    double   PARAM_terrain_elevation=4;
    int32   PARAM_ref=5;
};

message msg_transdata_param_seq_hexidx_hgt
{
    double   PARAM_seq_hexidx_hgt = 1;
    double   PARAM_seq_hexidx_hgt0 = 2;
    double   PARAM_seq_hexidx_hgtn = 3;
    int32   PARAM_wave_info_entropy = 4;
};

message msg_transdata_param_seq_hexidx
{
    string  PARAM_seq_hexidx_element = 1;
    msg_transdata_param_seq_hexidx_hgt  PARAM_seq_hexidx_hgt = 2;
};


message msg_transentityhexidxdata
{
    string uildsrc=1;
    string uilddst=2;
    E_DISPLAY_MODE eDdisplayMode=3;
    msg_transdata_sensorposinfo sensorinfo=4;
    repeated msg_transdata_param_seq_hexidx hexidxslist = 5;
    repeated msg_TYPE_POLYGON_PT polygonlist=6;
};

message msg_transdata_eventInfo
{
    string m_sensorid=1;
    string m_entityid=2;
    string m_sensingmediaid=3;
    msg_transdata_entityposinfo m_sensorposinfo=4;
    msg_transdata_entityposinfo m_entityposinfo=5;
    msg_transdata_sensorposinfo m_sensorproprety=6;
    bool m_entityisSensorProprety=7;
    msg_transdata_sensorposinfo m_entityproprety=8;
    double m_distance=9;
    string m_timestamp=10;
    
    msg_transdata_param_seq_hexidx_hgt  m_sensor_hgt_range = 11;
    double m_entity_hgt=12;
};

message msg_transeventdata
{
    E_EVENT_MODE eventType=1;
    msg_transdata_eventInfo eventifo=2;
};

message msg_transeventlistdata
{
    E_EVENT_MODE eventType=1;
    repeated msg_transeventdata eventlist=2;
};

message msg_transeventlistdatasimple
{
    E_EVENT_MODE eventType=1;
    msg_transeventdata event=2;
};

message msg_transeventlistdatasimple_array
{
    repeated msg_transeventlistdatasimple transeventlistdatasimple=1;
};

message msg_transdata_smd
{
    string m_sensorid=1;
    string m_fieldid=2;
    string m_sensingmediaid=3;
    double radius = 4;
    double lat = 5;
    double lon = 6;
    double direction_pitch = 7;
    double direction_azimuth = 8;
    int32 usage_type = 9;
    double dpch = 10;
    double dazm = 11;
    double pch  = 12;
    double azm  = 13;
    bool bvalid = 14;
};

message msg_transprejusdgmentline
{
    string targetindex=1;
    string handlekey=2;
    string modelname=3;
    int32 total_index=4;
    int32 series_index=5;    
    int32 cl_r=6;
    int32 cl_g=7;
    int32 cl_b=8;
    msg_TYPE_POLYGON_PT srclocation=9;    
    repeated msg_TYPE_POLYGON_PT prejudmentline=10;
};
```


1.在对应的xxxx.proto文件中，对应结构位置添加相应变量，后续变量序号递增即可
2.修改protocol.bat 指定文件，执行重新生成指定得xxxx.proto 文件的扩展文件 xxxx.pb.h/xxxx.pb.cc
//修改内容为以下，
//1.需要修改定位到本地环境中的 ..\..\..\3rd\win64\ecal-5.12.4\bin\release\protoc所在位置
//2.修改打开修改的proto文件

```PowerShell
::..\..\..\3rd\win64\ecal-5.12.4\bin\release\protoc -I=. --cpp_out=.\protoc CommStackFrameElement.proto
::..\..\..\3rd\win64\ecal-5.12.4\bin\release\protoc -I=. --cpp_out=.\protoc AgentRelationInfo.proto
..\..\..\3rd\win64\ecal-5.12.4\bin\release\protoc -I=. --cpp_out=.\protoc AgentPositionInfo.proto
::..\..\..\3rd\win64\ecal-5.12.4\bin\release\protoc -I=. --cpp_out=.\protoc AgentCommSnrInfo.proto
::..\..\..\3rd\win64\ecal-5.12.4\bin\release\protoc -I=. --cpp_out=.\protoc CommStackFrameResultElement.proto
::..\..\..\3rd\win64\ecal-5.12.4\bin\release\protoc -I=. --cpp_out=.\protoc SimParamsInfo.proto
::..\..\..\3rd\win64\ecal-5.12.4\bin\release\protoc -I=. --cpp_out=.\protoc MparFrame.proto
::..\..\..\3rd\win64\ecal-5.12.4\bin\release\protoc -I=. --cpp_out=.\protoc ProcessTransInfo.proto
::..\..\..\3rd\win64\ecal-5.12.4\bin\release\protoc -I=. --cpp_out=.\protoc DebugInfo.proto
::..\..\..\3rd\win64\ecal-5.12.4\bin\release\protoc -I=. --cpp_out=.\protoc CustomTransInfo.proto
::..\..\..\3rd\win64\ecal-5.12.4\bin\release\protoc -I=. --cpp_out=.\protoc SARMotionInfo.proto
::..\..\..\3rd\win64\ecal-5.12.4\bin\release\protoc -I=. --cpp_out=.\protoc MPARMotionInfo.proto

::..\..\..\3rd\win64\ecal-5.12.4\bin\release\protoc --js_out=import_style=commonjs,binary:.\protoc_javascript CommStackFrameElement.proto
::..\..\..\3rd\win64\ecal-5.12.4\bin\release\protoc --js_out=import_style=commonjs,binary:.\protoc_javascript AgentRelationInfo.proto
::..\..\..\3rd\win64\ecal-5.12.4\bin\release\protoc  --js_out=import_style=commonjs,binary:.\protoc_javascript AgentPositionInfo.proto
::..\..\..\3rd\win64\ecal-5.12.4\bin\release\protoc  --js_out=import_style=commonjs,binary:.\protoc_javascript AgentCommSnrInfo.proto
::..\..\..\3rd\win64\ecal-5.12.4\bin\release\protoc --js_out=import_style=commonjs,binary:.\protoc_javascript CommStackFrameResultElement.proto
```
    
再命令行执行
./protocol.bat 
3.需要将文件中的类 导出到 dll 中，修改对应的 xxxx.pb.h，使该文件 包含以下头文件：
#include "gaeactor_comm_global.h"
将 xxxx.pb.h 中所有的 class xxxxxxxx final :  修改为 class GAEACTOR_COMM_EXPORT xxxxxxxx final :
class msg_transdata_entityposinfo final :
修改为
```C++
class GAEACTOR_COMM_EXPORT msg_transdata_entityposinfo final :
gaeactor_transmit_interface.h中以下主要数据结构的dto
    static ::msg::AgentPositionInfo::msg_transentityhexidxpostdata SerializePositionData(const transentityhexidxpostdata& _data);
    static void DeserializePositionData(const ::msg::AgentPositionInfo::msg_transentityhexidxpostdata& _data, transentityhexidxpostdata& positiondata);


    static ::msg::AgentPositionInfo::msg_transentityhexidxdata SerializeHexidxData(const transentityhexidxdata& _data, const HEXIDX_HGT_ARRAY &hexidxslist, const POLYGON_LIST &polygonlist);
    static void DeserializeHexidxData(const ::msg::AgentPositionInfo::msg_transentityhexidxdata& _data,transentityhexidxdata &hexidxdata,HEXIDX_HGT_ARRAY &hexidxslist,POLYGON_LIST &polygonlist);


    static ::msg::AgentPositionInfo::msg_transdata_entityposinfo SerializeEntityData(const transdata_entityposinfo& _data);
    static void DeserializeEntityData(const ::msg::AgentPositionInfo::msg_transdata_entityposinfo& _data, transdata_entityposinfo& entityinfo);


    static ::msg::AgentPositionInfo::msg_transdata_sensorposinfo SerializeSensorData(const transdata_sensorposinfo& _data);
    static void DeserializeSensorData(const ::msg::AgentPositionInfo::msg_transdata_sensorposinfo& _data, transdata_sensorposinfo& sensorinfo);


    static ::msg::AgentPositionInfo::msg_transeventdata SerializeEventData(const transeventdata& _data);
    static void DeserializeEventData(const ::msg::AgentPositionInfo::msg_transeventdata& _data, EVENT_INFO& sensorinfo);
```
或者包含对应的头文件  即可获取proto结构

#include "./proto/protoc/AgentPositionInfo.pb.h"
