import QtQuick 2.15
import QtQuick.Window 2.15
import QtQuick.Controls 2.15

import "./wgt" as Wgt

Rectangle  {
    id:runtime_style_window
    width: 800
    height: 600
    visible: true
    color: "#2e2f30"
    opacity: 1.0
    property string selectcolor:"#157e5b"

    signal qml_add_entity_signal
    signal qml_quit_agent_edit_panel_sig
    signal cSignal_qwidget

    signal cSignal

    onCSignal: {
        //console.log("RECV SIG");
    }

    onCSignal_qwidget: {
        //console.log("RECV SIG");
    }

    function updateAgentKey(arr)
    {
        for (var i = 0; i < arr.length; i++) {
        }
    }

    function resetData()
    {
        runtime_style_listView.model.clear()
    }

    function exportData()
    {
        var runtimgdata=[]
        for(var i = 0;i < runtime_style_listView.model.count;i++)
        {
            var subruntimgdata={}
            var item = runtime_style_listView.itemAtIndex(i);
            var row = item.children[0]
            var row_rect = row.children[1]

            var row_sub_rect = row_rect.children[0]

            var sub_combox_agentTile_ctrl = row_sub_rect.children[0]
            var sub_azimuth_context = row_sub_rect.children[1]
            var sub_speed_context = row_sub_rect.children[2]
            var sub_combox_altitudeType_ctrl = row_sub_rect.children[3]
            var sub_listview = row_sub_rect.children[12].children[0]
            var sub_listview_model = sub_listview.model

            var altitude_type = sub_combox_altitudeType_ctrl.getSelectModel().index
            var waypts=[]
            for(var j = 0;j < sub_listview_model.count;j++)
            {
                var waypt=[]
                waypt.push(parseFloat(sub_listview_model.get(j).Longitude))
                waypt.push(parseFloat(sub_listview_model.get(j).Latitude))

                var subitem = sub_listview.itemAtIndex(j);
                var subrow = subitem.children[0]
                var altitude = subrow.children[3].children[0].text
                var timeStamp = subrow.children[4].children[0].text
                waypt.push(parseFloat(altitude))
                waypt.push(parseFloat(altitude_type))
                waypt.push(parseFloat(timeStamp))
                waypts.push(waypt)
            }

            subruntimgdata["agentKey"] = sub_combox_agentTile_ctrl.getSelectModel().agentKey
            //            subruntimgdata["agentId"] = runtime_style_listView.model.get(i).agentId
            subruntimgdata["agentId"] =  sub_combox_agentTile_ctrl.getSelectModel().agentId
            subruntimgdata["azimuth"] = parseFloat(sub_azimuth_context.getContext())
            subruntimgdata["speed0"] = parseFloat(sub_speed_context.getContext())
            subruntimgdata["altitudeType"] = sub_combox_altitudeType_ctrl.getSelectModel().index
            subruntimgdata["waypoints"] = waypts

            subruntimgdata["agentInstId"] = sub_combox_agentTile_ctrl.getSelectModel().agentNameI18n
            subruntimgdata["agentEntityId"] = ""
            subruntimgdata["agentLabel"] = sub_combox_agentTile_ctrl.getSelectModel().agentName
            subruntimgdata["agentNote"] = ""
            subruntimgdata["agentIcon"] = ""

            subruntimgdata["iffcode"] = ""
            subruntimgdata["clusterCodes"] = []

            runtimgdata.push(subruntimgdata)
        }
        var runtimestyle={}
        runtimestyle["patternAgents"]=runtimgdata
        parentWidget.saveRuntimeData(runtimestyle)
    }

    function importData(runtimestyle_data)
    {
        var runtimgdata = runtimestyle_data["patternAgents"]
        for(var i = 0;i < runtimgdata.length;i++)
        {
            var patternAgentsitem = runtimgdata[i]
            var curindex = runtime_style_listModel.count
            runtime_style_listModel.insert(curindex,{"agentkey" : patternAgentsitem["agentkey"],
                                               "agentId":patternAgentsitem["agentId"],
                                               "agentInstId":patternAgentsitem["agentInstId"],
                                               "agentEntityId":patternAgentsitem["agentEntityId"],
                                               "agentLabel":patternAgentsitem["agentLabel"],
                                               "agentNote":patternAgentsitem["agentNote"],
                                               "agentIcon":patternAgentsitem["agentIcon"],
                                               "speed":patternAgentsitem["speed0"],
                                               "azimuth":patternAgentsitem["azimuth"],
                                               "altitudeType":patternAgentsitem["altitudeType"]})
            runtime_style_listView.currentIndex = curindex


            var item = runtime_style_listView.itemAtIndex(curindex);
            if(item === null)
            {
            }
            else
            {
                var row = item.children[0]
                var row_rect = row.children[1]

                var row_sub_rect = row_rect.children[0]

                var sub_combox_altitudeType_ctrl = row_sub_rect.children[3]
                sub_combox_altitudeType_ctrl.setSelectIndex(patternAgentsitem["altitudeType"])
                var sub_combox_agentTile_ctrl = row_sub_rect.children[0]
                sub_combox_agentTile_ctrl.setSelectVal(patternAgentsitem["agentId"])


                var sub_agentKey_context_ctrl = row_sub_rect.children[4]
                var sub_agentId_context_ctrl = row_sub_rect.children[5]
                var sub_agentInstId_context_ctrl = row_sub_rect.children[6]
                var sub_agentEntityId_context_ctrl = row_sub_rect.children[7]
                var sub_agentLabel_context_ctrl = row_sub_rect.children[8]
                var sub_agentNote_context_ctrl = row_sub_rect.children[9]
                var sub_agentIcon_context_ctrl = row_sub_rect.children[10]



                sub_agentKey_context_ctrl.setContext(patternAgentsitem["agentkey"])
                sub_agentId_context_ctrl.setContext(patternAgentsitem["agentId"])
                sub_agentInstId_context_ctrl.setContext(patternAgentsitem["agentInstId"])
                sub_agentEntityId_context_ctrl.setContext(patternAgentsitem["agentEntityId"])
                sub_agentLabel_context_ctrl.setContext(patternAgentsitem["agentLabel"])

                //                                sub_agentNote_context_ctrl.setContext(patternAgentsitem["agentNote"])
                //                                sub_agentIcon_context_ctrl.setContext(patternAgentsitem["agentIcon"])



                var sub_listview = row_sub_rect.children[12].children[0]
                var sub_listview_model = sub_listview.model

                var waypoints = patternAgentsitem["waypoints"]
                for(var j = 0;j < waypoints.length;j++)
                {
                    var waycurindex = sub_listview_model.count
                    var waypointsitem = waypoints[j]

                    sub_listview_model.insert(sub_listview_model.count,{"agentId":patternAgentsitem["agentId"],
                                                  "waypointid":patternAgentsitem["agentId"],
                                                  "Longitude": waypointsitem[0].toString(),
                                                  "Latitude": waypointsitem[1].toString(),
                                                  "Altitude": waypointsitem[2].toString(),
                                                  "TimeStamp": waypointsitem[4].toString()})
                }
            }
        }
    }


    function appendEntity(agentId, azimuth, speed, altitudeType, AgentKey,agentInstId,agentEntityId,agentLabel,agentNote,agentIcon)
    {
        var curindex = runtime_style_listModel.count
        runtime_style_listModel.insert(curindex,{"agentKey":AgentKey,
                                           "agentId":agentId,
                                           "agentInstId":agentInstId,
                                           "agentEntityId":agentEntityId,
                                           "agentLabel":agentLabel,
                                           "agentNote":agentNote,
                                           "agentIcon":agentIcon,


                                           "speed":speed,
                                           "azimuth":azimuth,
                                           "altitudeType":altitudeType})
        runtime_style_listView.currentIndex = curindex

        var item = runtime_style_listView.itemAtIndex(curindex);
        if(item === null)
        {

        }
        else
        {
            var row = item.children[0]
            var row_rect = row.children[1]

            var row_sub_rect = row_rect.children[0]


            var sub_combox_altitudeType_ctrl = row_sub_rect.children[3]
            sub_combox_altitudeType_ctrl.setSelectIndex(altitudeType)
            var sub_combox_agentTile_ctrl = row_sub_rect.children[0]
            sub_combox_agentTile_ctrl.setSelectVal(agentId)

            var sub_agentKey_context_ctrl = row_sub_rect.children[4]
            var sub_agentId_context_ctrl = row_sub_rect.children[5]
            var sub_agentInstId_context_ctrl = row_sub_rect.children[6]
            var sub_agentEntityId_context_ctrl = row_sub_rect.children[7]
            var sub_agentLabel_context_ctrl = row_sub_rect.children[8]
            var sub_agentNote_context_ctrl = row_sub_rect.children[9]
            var sub_agentIcon_context_ctrl = row_sub_rect.children[10]



            sub_agentKey_context_ctrl.setContext(AgentKey)
            sub_agentId_context_ctrl.setContext(agentId)
            sub_agentInstId_context_ctrl.setContext(agentInstId)
            sub_agentEntityId_context_ctrl.setContext(agentEntityId)
            sub_agentLabel_context_ctrl.setContext(agentLabel)

            //                                sub_agentNote_context_ctrl.setContext(agentNote)
            //                                sub_agentIcon_context_ctrl.setContext(agentIcon)


//            console.log("agentId"+agentId + " agentInstId "+ agentInstId)
        }
        parentWidget.setCurrentEntity(runtime_style_listView.model.get(curindex).agentId);
    }

    function selectEntity(agentId)
    {
        for(var i = 0;i < runtime_style_listView.model.count;i++)
        {
            if(runtime_style_listView.model.get(i).agentId === agentId)
            {
                runtime_style_listView.currentIndex = i
            }
        }
    }

    function updateWaypoint(agentId, waypointid, lng, lat)
    {
        for(var i = 0;i < runtime_style_listView.model.count;i++)
        {
            if(runtime_style_listView.model.get(i).agentId === agentId)
            {
                runtime_style_listView.currentIndex = i

                var item = runtime_style_listView.itemAtIndex(i);
                if(item === null)
                {
                }
                else
                {
                    var row = item.children[0]
                    var row_rect = row.children[1]

                    var row_sub_rect = row_rect.children[0]


                    var sub_listview = row_sub_rect.children[12].children[0]
                    var sub_listview_model = sub_listview.model

                    for(var j = 0;j < sub_listview_model.count;j++)
                    {
                        if(sub_listview_model.get(j).waypointid === waypointid)
                        {
                            sub_listview_model.get(j).Longitude  = lng.toString()
                            sub_listview_model.get(j).Latitude = lat.toString()
                        }
                    }
                }
            }
        }
    }

    function appendWaypoint(agentId, waypointid, lng, lat, Altitude, TimeStamp)
    {
        for(var i = 0;i < runtime_style_listView.model.count;i++)
        {
            if(runtime_style_listView.model.get(i).agentId === agentId)
            {
                var item = runtime_style_listView.itemAtIndex(i);
                if(item === null)
                {
                }
                else
                {
                    var row = item.children[0]
                    var row_rect = row.children[1]

                    var row_sub_rect = row_rect.children[0]

                    var sub_listview = row_sub_rect.children[12].children[0]
                    var sub_listview_model = sub_listview.model

                    sub_listview_model.insert(sub_listview_model.count,{"agentId":agentId,
                                                  "waypointid":waypointid,
                                                  "Longitude": lng.toString(),
                                                  "Latitude": lat.toString(),
                                                  "Altitude": Altitude.toString(),
                                                  "TimeStamp": TimeStamp.toString()})
                }
            }
        }
    }

    MouseArea {
        id:add_new_entity
        anchors.top: parent.top
        anchors.topMargin: 2
        anchors.right: parent.right
        anchors.rightMargin: 2
        width: 100
        height: ctrl_height
        Wgt.Buttoncontrol
        {
            id:add_new_entity_btn
            anchors.fill: parent
        }
        Component.onCompleted: {
            add_new_entity_btn.setTitle(qsTr("Add Entity"))
            add_new_entity_btn.setToolTips(qsTr("Add New Entity."))
        }
        Connections {
            target: add_new_entity_btn.getBtnObj()
            function onClicked() {
                qml_add_entity_signal()
            }
        }
        hoverEnabled: true
        onEntered:{
        }
        onExited: {
        }
    }

    MouseArea {
        id:save_entity
        anchors.top: parent.top
        anchors.topMargin: 2
        anchors.right: add_new_entity.left
        anchors.rightMargin: 2
        width: 100
        height: ctrl_height
        Wgt.Buttoncontrol
        {
            id:save_entity_btn
            anchors.fill: parent
        }
        Component.onCompleted: {
            save_entity_btn.setTitle(qsTr("Save Runtime Style"))
            save_entity_btn.setToolTips(qsTr("Save Agent Runtime Style."))
        }
        Connections {
            target: save_entity_btn.getBtnObj()
            function onClicked() {
                exportData()
            }
        }
        hoverEnabled: true
        onEntered:{
            //console.log("enter")
        }
        onExited: {
            //console.log("exit")
        }
    }


    Label {
        id:runtimestyle_lisview_title
        width:320
        height: 32
        text: qsTr("runtime style")
        anchors.left: runtime_style_list_rect.left
        anchors.top: add_new_entity.top
        anchors.leftMargin: 20
        color: "#ffffff"
        font.family: "Microsoft YaHei"
        font.pixelSize: title_font_size
        verticalAlignment: Text.AlignVCenter
        horizontalAlignment:Text.AlignLeft

    }

    Rectangle{
        id:runtime_style_list_rect
        anchors.top: add_new_entity.bottom
        anchors.topMargin: 5
        width: parent.width
        height: parent.height - ctrl_height - 5
        color: 'transparent'

        ListView {
            id: runtime_style_listView
            anchors.fill: parent
            model:     ListModel {
                id: runtime_style_listModel
            }
            delegate: runtime_style_delegate_list
            highlight: highlight2 // 高亮设置
            highlightFollowsCurrentItem: false
            interactive: true // 设置为可交互
            focus: true // 获取焦点
            clip: true; //超出边界的数据进行裁剪
            header:runtimestyle_headerView;//只构建表头上滑动时表头也会跟随上滑动消失
            headerPositioning: ListView.OverlayHeader;//枚举类型

            ScrollBar.vertical: ScrollBar {       //滚动条
                visible: runtime_style_listView.contentHeight > runtime_style_listView.height // 根据内容高度和列表高度判断滚动条是否可见
                policy:ScrollBar.AsNeeded
                anchors.right: runtime_style_listView.right
                anchors.top: runtime_style_listView.top
                anchors.topMargin: 40
                width: 10
                active: true
                background: Item {            //滚动条的背景样式
                    Rectangle {
                        anchors.centerIn: parent
                        height: parent.height
                        width: parent.width*0.3
                        color: '#3e2f30'
                        radius: width/2
                    }
                }

                contentItem: Rectangle {
                    radius: width/3        //bar的圆角
                    color: 'grey'
                }
            }
        }

        Component.onCompleted: {
        }

    }

    Component{   //高亮条
        id: highlight2
        Rectangle {
            width: 180; height: ctrl_height
            color: "lightsteelblue"; radius: 0
            //y: runtime_style_listView.currentItem.y
            Behavior on y {
                // 弹簧动画
                SpringAnimation {
                    spring: 10
                    damping: 0.35
                }
            }
        }
    }



    Component {
        id: runtime_style_delegate_list
        Rectangle {

            id: runtime_style_list_rct1
            height: 800
            width: ListView.view.width

            color:ListView.isCurrentItem?selectcolor:(index % 2 == 0?"#5f5f5f":"#3f3f3f")

            Row {
                anchors.fill: parent
                spacing: 0
                Rectangle {
                    width: parent.width/10
                    height: parent.height
                    anchors.left: parent.left
                    anchors.leftMargin: 0
                    color:"transparent"
                    Text {
                        width:parent.width
                        height:parent.height
                        anchors.verticalCenter: parent.verticalCenter
                        anchors.horizontalCenter: parent.horizontalCenter
                        text: (index+1).toString()
                        color: "#ffffff" //选中颜色设置
                        font.family: "Microsoft YaHei"
                        font.pixelSize: context_font_size
                        verticalAlignment: Text.AlignVCenter
                        horizontalAlignment:Text.AlignHCenter
                    }
                }

                Rectangle {
                    width: parent.width*8/10
                    height: parent.height
                    anchors.left: parent.left
                    anchors.leftMargin: parent.width*1/10
                    color:"transparent"


                    Rectangle
                    {
                        id:sub_parent_rect
                        anchors.fill: parent
                        //                        color: "#2e2f30"
                        color:"transparent"
                        opacity: 1.0
                        Wgt.Comboxcontrol
                        {
                            id:combox_agentTile_ctrl
                            anchors.top: parent.top
                            anchors.topMargin: 10
                            anchors.left: parent.left
                            anchors.leftMargin: 10
                            width:(sub_parent_rect.width - ctrl_height)
                            height: ctrl_height
                        }



                        Wgt.TextFiledcontrol
                        {
                            id:azimuth_context
                            anchors.top: combox_agentTile_ctrl.bottom
                            anchors.topMargin: 10
                            anchors.left: parent.left
                            anchors.leftMargin: 10
                            width:(sub_parent_rect.width - ctrl_height)
                            height: ctrl_height
                        }

                        Wgt.TextFiledcontrol
                        {
                            id:speed_context
                            anchors.top: azimuth_context.bottom
                            anchors.topMargin: 10
                            anchors.left: parent.left
                            anchors.leftMargin: 10
                            width:(sub_parent_rect.width - ctrl_height)
                            height: ctrl_height
                        }

                        Wgt.Comboxcontrol
                        {
                            id:combox_altitudeType_ctrl
                            anchors.top: speed_context.bottom
                            anchors.topMargin: 10
                            anchors.left: parent.left
                            anchors.leftMargin: 10
                            width:(sub_parent_rect.width - ctrl_height)/2
                            height: ctrl_height
                        }
                        Wgt.TextFiledcontrol
                        {
                            id:agentKey_context
                            anchors.top: combox_altitudeType_ctrl.bottom
                            anchors.topMargin: 10
                            anchors.left: parent.left
                            anchors.leftMargin: 10
                            width:(sub_parent_rect.width - ctrl_height)
                            height: ctrl_height
                        }

                        Wgt.TextFiledcontrol
                        {
                            id:agentId_context
                            anchors.top: agentKey_context.bottom
                            anchors.topMargin: 10
                            anchors.left: parent.left
                            anchors.leftMargin: 10
                            width:(sub_parent_rect.width - ctrl_height)
                            height: ctrl_height
                        }

                        Wgt.TextFiledcontrol
                        {
                            id:agentInstId_context
                            anchors.top: agentId_context.bottom
                            anchors.topMargin: 10
                            anchors.left: parent.left
                            anchors.leftMargin: 10
                            width:(sub_parent_rect.width - ctrl_height)
                            height: ctrl_height
                        }



                        Wgt.TextFiledcontrol
                        {
                            id:agentEntityId_context
                            anchors.top: agentInstId_context.bottom
                            anchors.topMargin: 10
                            anchors.left: parent.left
                            anchors.leftMargin: 10
                            width:(sub_parent_rect.width - ctrl_height)
                            height: ctrl_height
                        }

                        Wgt.TextFiledcontrol
                        {
                            id:agentLabel_context
                            anchors.top: agentEntityId_context.bottom
                            anchors.topMargin: 10
                            anchors.left: parent.left
                            anchors.leftMargin: 10
                            width:(sub_parent_rect.width - ctrl_height)
                            height: ctrl_height
                        }

                        Wgt.TextFiledcontrol
                        {
                            id:agentNote_context
                            anchors.top: agentLabel_context.bottom
                            anchors.topMargin: 10
                            anchors.left: parent.left
                            anchors.leftMargin: 10
                            width:(sub_parent_rect.width - ctrl_height)
                            height: ctrl_height
                        }

                        Wgt.TextFiledcontrol
                        {
                            id:agentIcon_context
                            anchors.top: agentNote_context.bottom
                            anchors.topMargin: 10
                            anchors.left: parent.left
                            anchors.leftMargin: 10
                            width:(sub_parent_rect.width - ctrl_height)
                            height: ctrl_height
                        }

                        Connections {
                            target: combox_agentTile_ctrl.getBtnObj()
                            function onCurrentIndexChanged() {
                                var selmodel = combox_agentTile_ctrl.getSelectModel()

                                agentKey_context.setContext(selmodel.agentKey)
                                agentId_context.setContext(selmodel.agentId)
                                agentInstId_context.setContext(selmodel.agentInstId)
                                agentEntityId_context.setContext(selmodel.agentNameI18n)
                                agentLabel_context.setContext(selmodel.agentName)
//                                agentNote_context.setContext(selmodel.agentNote)
//                                agentIcon_context.setContext(selmodel.agentIcon)

                            }
                        }
                        Component.onCompleted: {
                            combox_agentTile_ctrl.setTitle(qsTr("AgentTitle:"))
                            azimuth_context.setTitle(qsTr("azimuth:"))
                            speed_context.setTitle(qsTr("speed0:"))
                            combox_altitudeType_ctrl.setTitle( qsTr("AltType:"))


                            agentKey_context.setTitle(qsTr("AgentKey:"))
                            agentId_context.setTitle(qsTr("AgentId:"))
                            agentInstId_context.setTitle(qsTr("AgentInstId:"))
                            agentEntityId_context.setTitle(qsTr("AgentEntityId:"))
                            agentLabel_context.setTitle(qsTr("AgentLabel:"))
                            agentNote_context.setTitle(qsTr("AgentNote:"))
                            agentIcon_context.setTitle(qsTr("AgentIcon:"))

                            combox_agentTile_ctrl.title_font_size=context_font_size
                            azimuth_context.title_font_size=context_font_size
                            speed_context.title_font_size=context_font_size
                            combox_altitudeType_ctrl.title_font_size=context_font_size
                            agentKey_context.title_font_size=context_font_size
                            agentId_context.title_font_size=context_font_size
                            agentInstId_context.title_font_size=context_font_size
                            agentEntityId_context.title_font_size=context_font_size
                            agentLabel_context.title_font_size=context_font_size
                            agentNote_context.title_font_size=context_font_size
                            agentIcon_context.title_font_size=context_font_size


                            var treeModel = []
                            var agentkeysArray = parentWidget.getAgentKeysArray();
                            for (var i = 0; i < agentkeysArray.length; i++) {
                                treeModel.push({"index":i,"enumid":agentkeysArray[i]["agentId"],"modelData":agentkeysArray[i]["agentName"],
                                                   "modelData":agentkeysArray[i]["agentName"],
                                                   "agentKey":agentkeysArray[i]["agentKey"],
                                                   "agentInstId":agentkeysArray[i]["agentInstId"],
                                                   "agentOffsetKey":agentkeysArray[i]["agentOffsetKey"],
                                                   "asmKey":agentkeysArray[i]["asmKey"],
                                                   "agentId":agentkeysArray[i]["agentId"],
                                                   "agentKeyword":agentkeysArray[i]["agentKeyword"],
                                                   "agentName":agentkeysArray[i]["agentName"],
                                                   "agentNameI18n":agentkeysArray[i]["agentNameI18n"],
                                                   "agentType":agentkeysArray[i]["agentType"],
                                                   "modelUrlSlim":agentkeysArray[i]["modelUrlSlim"],
                                                   "modelUrlFat":agentkeysArray[i]["modelUrlFat"],
                                                   "modelUrlMedium":agentkeysArray[i]["modelUrlMedium"],
                                                   "modelUrlSymbols":agentkeysArray[i]["modelUrlSymbols"]})
                            }
                            combox_agentTile_ctrl.updateData(treeModel)

                            var treeModel6 = [{"index":0,"enumid":0,"modelData":qsTr("absolute")},
                                              {"index":1,"enumid":1,"modelData":qsTr("relative")}]
                            combox_altitudeType_ctrl.updateData(treeModel6)

                            azimuth_context.setContext(azimuth)
                            speed_context.setContext(speed)

//                            agentKey_context.setContext(agentKey)
//                            agentId_context.setContext(agentId)
//                            agentInstId_context.setContext(agentInstId)
//                            agentEntityId_context.setContext(agentEntityId)
//                            agentLabel_context.setContext(agentLabel)
//                            agentNote_context.setContext(agentNote)
//                            agentIcon_context.setContext(agentIcon)
                        }

                        Label {
                            id:waupoints_title
                            text: qsTr("waypoints:");
                            font.pixelSize: context_font_size;
                            font.family: "Microsoft YaHei"
                            width:120
                            height: ctrl_height
                            color:"#ffffff"
                            anchors.top: agentIcon_context.bottom
                            anchors.topMargin: 5
                            anchors.left: parent.left
                            anchors.leftMargin: 10
                            anchors.verticalCenter: parent.verticalCenter;
                            horizontalAlignment: Text.AlignRight;
                        }
                        Rectangle{
                            id:waypoints_list_rect
                            anchors.top: waupoints_title.top
                            anchors.left: waupoints_title.right
                            anchors.leftMargin: 10
                            width:sub_parent_rect.width - waupoints_title.width - ctrl_height
                            height:parent.height - combox_agentTile_ctrl.height-azimuth_context.height-speed_context.height-combox_altitudeType_ctrl.height-5*4-10
                            color: 'transparent'

                            ListView {
                                id: waypoints_listView
                                anchors.fill: parent
                                model:ListModel {
                                    id: waypoints_listModel
                                }
                                delegate: waypoints_delegate_list
                                highlight: highlight4 // 高亮设置
                                highlightFollowsCurrentItem: false
                                interactive: true // 设置为可交互
                                focus: true // 获取焦点
                                clip: true; //超出边界的数据进行裁剪
                                header:waypoints_headerView;//只构建表头上滑动时表头也会跟随上滑动消失
                                headerPositioning: ListView.OverlayHeader;//枚举类型

                                ScrollBar.vertical: ScrollBar {       //滚动条
                                    visible: waypoints_listView.contentHeight > waypoints_listView.height // 根据内容高度和列表高度判断滚动条是否可见
                                    policy:ScrollBar.AsNeeded
                                    anchors.right: waypoints_listView.right
                                    anchors.top: waypoints_listView.top
                                    anchors.topMargin: 40
                                    width: 10
                                    active: true
                                    background: Item {            //滚动条的背景样式
                                        Rectangle {
                                            anchors.centerIn: parent
                                            height: parent.height
                                            width: parent.width*0.3
                                            color: '#3e2f30'
                                            radius: width/2
                                        }
                                    }

                                    contentItem: Rectangle {
                                        radius: width/3        //bar的圆角
                                        color: 'grey'
                                    }
                                }
                            }
                        }



                        Component {
                            id: waypoints_delegate_list
                            Rectangle {
                                id: waypoints_list_rct1
                                height: 35
                                width: ListView.view.width

                                color:ListView.isCurrentItem?selectcolor:(index % 2 == 0?"#5f5f5f":"#3f3f3f") //选中颜色设置

                                Row {
                                    anchors.fill: parent
                                    spacing: 0
                                    Rectangle {
                                        width: parent.width/6
                                        height: parent.height
                                        anchors.left: parent.left
                                        anchors.leftMargin: 0
                                        color:"transparent"
                                        Text {
                                            width:parent.width
                                            height:parent.height
                                            anchors.verticalCenter: parent.verticalCenter
                                            anchors.horizontalCenter: parent.horizontalCenter
                                            text: index+1
                                            color: "#ffffff" //选中颜色设置
                                            font.family: "Microsoft YaHei"
                                            font.pixelSize: context_font_size
                                            verticalAlignment: Text.AlignVCenter
                                            horizontalAlignment:Text.AlignHCenter
                                        }
                                    }


                                    Rectangle {
                                        width: parent.width/6
                                        height: parent.height
                                        anchors.left: parent.left
                                        anchors.leftMargin: parent.width*1/6
                                        color:"transparent"
                                        Text {
                                            width:parent.width
                                            height:parent.height
                                            anchors.verticalCenter: parent.verticalCenter
                                            anchors.horizontalCenter: parent.horizontalCenter
                                            text: Longitude
                                            color: "#ffffff" //选中颜色设置
                                            font.family: "Microsoft YaHei"
                                            font.pixelSize: context_font_size
                                            verticalAlignment: Text.AlignVCenter
                                            horizontalAlignment:Text.AlignHCenter
                                        }
                                    }

                                    Rectangle {
                                        width: parent.width/6
                                        height: parent.height
                                        anchors.left: parent.left
                                        anchors.leftMargin: parent.width*2/6
                                        color:"transparent"
                                        Text {
                                            width:parent.width
                                            height:parent.height
                                            anchors.verticalCenter: parent.verticalCenter
                                            anchors.horizontalCenter: parent.horizontalCenter
                                            text: Latitude
                                            color: "#ffffff" //选中颜色设置
                                            font.family: "Microsoft YaHei"
                                            font.pixelSize: context_font_size
                                            verticalAlignment: Text.AlignVCenter
                                            horizontalAlignment:Text.AlignHCenter
                                        }
                                    }

                                    Rectangle {
                                        width: parent.width/6
                                        height: parent.height
                                        anchors.left: parent.left
                                        anchors.leftMargin: parent.width*3/6
                                        color:"transparent"
                                        //                                        Text {
                                        //                                            width:parent.width
                                        //                                            height:parent.height
                                        //                                            anchors.verticalCenter: parent.verticalCenter
                                        //                                            anchors.horizontalCenter: parent.horizontalCenter
                                        //                                            text:  Altitude
                                        //                                            color: "#ffffff" //选中颜色设置
                                        //                                            font.family: "Microsoft YaHei"
                                        //                                            font.pixelSize: context_font_size
                                        //                                            verticalAlignment: Text.AlignVCenter
                                        //                                            horizontalAlignment:Text.AlignHCenter
                                        //                                        }
                                        TextField{
                                            id:altitude_context
                                            width:parent.width*2/3
                                            height:parent.height*6/7
                                            anchors.verticalCenter: parent.verticalCenter
                                            anchors.horizontalCenter: parent.horizontalCenter
                                            text:  Altitude
                                            color: "#ffffff" //选中颜色设置
                                            font.family: "Microsoft YaHei"
                                            font.pixelSize: context_font_size
                                            verticalAlignment: Text.AlignVCenter
                                            horizontalAlignment:Text.AlignHCenter
                                            background:Rectangle {
                                                anchors.centerIn: parent
                                                height: parent.height
                                                width: parent.width
                                                border.color: altitude_context.pressed ? "#5f5f5f" : "#ffffff"
                                                border.width: altitude_context.visualFocus ? 2 : 1
                                                color: '#3e2f30'
                                                radius: width/2
                                            }
                                        }
                                    }

                                    Rectangle {
                                        width: parent.width/6
                                        height: parent.height
                                        anchors.left: parent.left
                                        anchors.leftMargin: parent.width*4/6
                                        color:"transparent"
                                        //                                        Text {
                                        //                                            width:parent.width
                                        //                                            height:parent.height
                                        //                                            anchors.verticalCenter: parent.verticalCenter
                                        //                                            anchors.horizontalCenter: parent.horizontalCenter
                                        //                                            text: TimeStamp
                                        //                                            color: "#ffffff" //选中颜色设置
                                        //                                            font.family: "Microsoft YaHei"
                                        //                                            font.pixelSize: context_font_size
                                        //                                            verticalAlignment: Text.AlignVCenter
                                        //                                            horizontalAlignment:Text.AlignHCenter
                                        //                                        }

                                        TextField{
                                            id:timestamp_context
                                            width:parent.width*2/3
                                            height:parent.height*6/7
                                            anchors.verticalCenter: parent.verticalCenter
                                            anchors.horizontalCenter: parent.horizontalCenter
                                            text: TimeStamp
                                            color: "#ffffff" //选中颜色设置
                                            font.family: "Microsoft YaHei"
                                            font.pixelSize: context_font_size
                                            verticalAlignment: Text.AlignVCenter
                                            horizontalAlignment:Text.AlignHCenter
                                            background:Rectangle {
                                                anchors.centerIn: parent
                                                height: parent.height
                                                width: parent.width
                                                border.color: timestamp_context.pressed ? "#5f5f5f" : "#ffffff"
                                                border.width: timestamp_context.visualFocus ? 2 : 1
                                                color: '#3e2f30'
                                                radius: width/2
                                            }
                                        }
                                    }

                                    Rectangle {
                                        width: parent.width/6
                                        height: parent.height
                                        anchors.left: parent.left
                                        anchors.leftMargin: parent.width*5/6
                                        color:"transparent"

                                        Wgt.Buttoncontrol
                                        {
                                            id:delete_point_btn
                                            width: parent.width-4
                                            height: parent.height-4
                                            anchors.top:parent.top
                                            anchors.topMargin: 2
                                            anchors.left:parent.left
                                            anchors.leftMargin: 2
                                            anchors.verticalCenter: parent.verticalCenter
                                        }
                                        Component.onCompleted: {
                                            delete_point_btn.setTitle(qsTr("delete point"))
                                            delete_point_btn.setToolTips(qsTr("Delete the point"))
                                        }
                                        Connections {
                                            target: delete_point_btn.getBtnObj()
                                            function onClicked() {
                                                parentWidget.deleteWaypoint(waypoints_listModel.get(index).agentId,waypoints_listModel.get(index).waypointid)
                                                waypoints_listModel.remove(index)
                                            }
                                        }
                                    }
                                    MouseArea {
                                        //                        anchors.fill: parent
                                        width: parent.width*4/7
                                        height: parent.height
                                        anchors.right: parent.right
                                        anchors.rightMargin: parent.width*3/7
                                        onClicked:  {
                                            waypoints_listView.currentIndex = index
                                            parentWidget.locateWaypoint(waypoints_listModel.get(index).agentId,waypoints_listModel.get(index).waypointid)
                                            selectEntity(waypoints_listModel.get(index).agentId)
                                        }

                                    }
                                }
                            }
                        }

                        Component{   //高亮条
                            id: highlight4
                            Rectangle {
                                width: 180; height: 40
                                color: "lightsteelblue"; radius: 0
                                //y: waypoints_listView.currentItem.y
                                Behavior on y {
                                    // 弹簧动画
                                    SpringAnimation {
                                        spring: 10
                                        damping: 0.35
                                    }
                                }
                            }
                        }

                        Component {
                            id:waypoints_headerView;
                            Rectangle {
                                width:parent.width;
                                height: 40;
                                color: "lightgrey";
                                z:2;//将表头的z坐标设置在上层，表头在设置属性为overlayHeader时就不会随滑动而消失，始终显示在最上面

                                Row {
                                    anchors.fill: parent
                                    Rectangle {
                                        width: parent.width/6
                                        height: parent.height
                                        anchors.left: parent.left
                                        anchors.leftMargin: 0
                                        color:"transparent"
                                        Text {
                                            width:parent.width
                                            height:parent.height
                                            anchors.verticalCenter: parent.verticalCenter
                                            anchors.horizontalCenter: parent.horizontalCenter
                                            text: qsTr("Index")
                                            font.pixelSize: context_font_size
                                            font.family: "Microsoft YaHei"
                                            color:"#222222"
                                            verticalAlignment: Text.AlignVCenter
                                            horizontalAlignment:Text.AlignHCenter
                                        }
                                    }

                                    Rectangle {
                                        width: parent.width/6
                                        height: parent.height
                                        anchors.left: parent.left
                                        anchors.leftMargin: parent.width*1/7
                                        color:"transparent"
                                        Text {
                                            width:parent.width
                                            height:parent.height
                                            anchors.verticalCenter: parent.verticalCenter
                                            anchors.horizontalCenter: parent.horizontalCenter
                                            text: qsTr("Longitude")
                                            font.pixelSize: context_font_size
                                            font.family: "Microsoft YaHei"
                                            color:"#222222"
                                            verticalAlignment: Text.AlignVCenter
                                            horizontalAlignment:Text.AlignHCenter
                                        }
                                    }

                                    Rectangle {
                                        width: parent.width/6
                                        height: parent.height
                                        anchors.left: parent.left
                                        anchors.leftMargin: parent.width*2/6
                                        color:"transparent"
                                        Text {
                                            width:parent.width
                                            height:parent.height
                                            anchors.verticalCenter: parent.verticalCenter
                                            anchors.horizontalCenter: parent.horizontalCenter
                                            text: qsTr("Latitude")
                                            font.pixelSize: context_font_size
                                            font.family: "Microsoft YaHei"
                                            color:"#222222"
                                            verticalAlignment: Text.AlignVCenter
                                            horizontalAlignment:Text.AlignHCenter
                                        }
                                    }

                                    Rectangle {
                                        width: parent.width/6
                                        height: parent.height
                                        anchors.left: parent.left
                                        anchors.leftMargin: parent.width*3/6
                                        color:"transparent"
                                        Text {
                                            width:parent.width
                                            height:parent.height
                                            anchors.verticalCenter: parent.verticalCenter
                                            anchors.horizontalCenter: parent.horizontalCenter
                                            text: qsTr("Altitude")
                                            font.pixelSize: context_font_size
                                            font.family: "Microsoft YaHei"
                                            color:"#222222"
                                            verticalAlignment: Text.AlignVCenter
                                            horizontalAlignment:Text.AlignHCenter
                                        }
                                    }

                                    Rectangle {
                                        width: parent.width/6
                                        height: parent.height
                                        anchors.left: parent.left
                                        anchors.leftMargin: parent.width*4/6
                                        color:"transparent"
                                        Text {
                                            width:parent.width
                                            height:parent.height
                                            anchors.verticalCenter: parent.verticalCenter
                                            anchors.horizontalCenter: parent.horizontalCenter
                                            text: qsTr("TimeStamp")
                                            font.pixelSize: context_font_size
                                            font.family: "Microsoft YaHei"
                                            color:"#222222"
                                            verticalAlignment: Text.AlignVCenter
                                            horizontalAlignment:Text.AlignHCenter
                                        }
                                    }

                                    Rectangle {
                                        width: parent.width/6
                                        height: parent.height
                                        anchors.left: parent.left
                                        anchors.leftMargin: parent.width*5/6
                                        color:"transparent"
                                        Text {
                                            width:parent.width
                                            height:parent.height
                                            anchors.verticalCenter: parent.verticalCenter
                                            anchors.horizontalCenter: parent.horizontalCenter
                                            text: qsTr("Operate")
                                            font.pixelSize: context_font_size
                                            font.family: "Microsoft YaHei"
                                            color:"#222222"
                                            verticalAlignment: Text.AlignVCenter
                                            horizontalAlignment:Text.AlignHCenter
                                        }
                                    }
                                }

                                MouseArea {
                                    anchors.fill: parent;
                                    onPressed: {
                                        mouse.accepted = true;
                                    }
                                }
                            }
                        }
                    }
                }

                Rectangle {
                    width: parent.width/10
                    height: parent.height
                    anchors.left: parent.left
                    anchors.leftMargin: parent.width*9/10
                    color:"transparent"
                    Wgt.Buttoncontrol
                    {
                        id:delete_entity_btn
                        width: parent.width - parent.width/5
                        height: parent.height/10
                        anchors.left: parent.left
                        anchors.leftMargin: parent.width/10

                        anchors.verticalCenter: parent.verticalCenter
                    }
                    Component.onCompleted: {
                        delete_entity_btn.setTitle(qsTr("delete entity"))
                        delete_entity_btn.setToolTips(qsTr("Delete The Entity"))
                    }
                    Connections {
                        target: delete_entity_btn.getBtnObj()
                        function onClicked() {
                            parentWidget.deleteEntity(agentId)
                            runtime_style_listModel.remove(index)
                            if(runtime_style_listModel.count > 0)
                            {
                                parentWidget.setCurrentEntity(runtime_style_listView.model.get(0).agentId)
                            }
                            runtime_style_listView.currentIndex = 0
                        }
                    }
                }

                MouseArea {
                    //                        anchors.fill: parent
                    width: parent.width*1/10 + (parent.width*8/10/10)
                    height: parent.height
                    anchors.left: parent.left
                    onClicked:  {
                        runtime_style_listView.currentIndex = index
                        parentWidget.setCurrentEntity(runtime_style_listView.model.get(index).agentId);
                    }
                }
            }
        }
    }

    Component {
        id:runtimestyle_headerView;
        Rectangle {
            width:parent.width;
            height: 40;
            color: "lightgrey";
            z:2;//将表头的z坐标设置在上层，表头在设置属性为overlayHeader时就不会随滑动而消失，始终显示在最上面

            Item {
                anchors.fill: parent
                Rectangle {
                    width: parent.width/10
                    height: parent.height
                    anchors.left: parent.left
                    anchors.leftMargin: 0
                    color:"transparent"
                    Text {
                        width:parent.width
                        height:parent.height
                        anchors.verticalCenter: parent.verticalCenter
                        anchors.horizontalCenter: parent.horizontalCenter
                        text: qsTr("Index")
                        font.pixelSize: context_font_size
                        font.family: "Microsoft YaHei"
                        color:"#222222"
                        verticalAlignment: Text.AlignVCenter
                        horizontalAlignment:Text.AlignHCenter
                    }
                }

                Rectangle {
                    width: parent.width*8/10
                    height: parent.height
                    anchors.left: parent.left
                    anchors.leftMargin: parent.width*1/10
                    color:"transparent"
                    Text {
                        width:parent.width
                        height:parent.height
                        anchors.verticalCenter: parent.verticalCenter
                        anchors.horizontalCenter: parent.horizontalCenter
                        text: qsTr("Context")
                        font.pixelSize: context_font_size
                        font.family: "Microsoft YaHei"
                        color:"#222222"
                        verticalAlignment: Text.AlignVCenter
                        horizontalAlignment:Text.AlignHCenter
                    }
                }

                Rectangle {
                    width: parent.width/10
                    height: parent.height
                    anchors.left: parent.left
                    anchors.leftMargin: parent.width*9/10
                    color:"transparent"
                    Text {
                        width:parent.width
                        height:parent.height
                        anchors.verticalCenter: parent.verticalCenter
                        anchors.horizontalCenter: parent.horizontalCenter
                        text: qsTr("Operate")
                        font.pixelSize: context_font_size
                        font.family: "Microsoft YaHei"
                        color:"#222222"
                        verticalAlignment: Text.AlignVCenter
                        horizontalAlignment:Text.AlignHCenter
                    }
                }
            }

            MouseArea {
                anchors.fill: parent;
                onPressed: {
                    mouse.accepted = true;
                }
            }
        }
    }


    Wgt.Buttoncontrol
    {
        id:back_btn
        width: ctrl_height
        height: ctrl_height
        anchors.top: parent.top
        anchors.topMargin: 2
        anchors.right: save_entity.left
        anchors.rightMargin: 10
    }

    Connections {
        target: back_btn.getBtnObj()
        function onClicked() {
            qml_quit_agent_edit_panel_sig()
        }
    }
    Component.onCompleted: {
        back_btn.setBtnIcon("qrc:/res/qml/icon/back.png", ctrl_height, ctrl_height,qsTr("Back"))
    }
}
