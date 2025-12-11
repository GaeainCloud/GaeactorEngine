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

    property int ctrl_height:32
    property int title_font_size:14

    signal qml_add_entity_signal

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

            var sub_azimuth_context = row_sub_rect.children[4]
            var sub_speed_context = row_sub_rect.children[5]
            var sub_combox_altitudeType_ctrl = row_sub_rect.children[6]
            var sub_combox_agentkey_ctrl = row_sub_rect.children[7]
            var sub_listview = row_sub_rect.children[9].children[0]
            var sub_listview_model = sub_listview.model

            var altitude_type = sub_combox_altitudeType_ctrl.model.get(sub_combox_altitudeType_ctrl.currentIndex).index
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

            subruntimgdata["agentKey"] = sub_combox_agentkey_ctrl.model.get(sub_combox_agentkey_ctrl.currentIndex).agentKey
            subruntimgdata["agentId"] = runtime_style_listView.model.get(i).entityid
            subruntimgdata["azimuth"] = parseFloat(sub_azimuth_context.text)
            subruntimgdata["speed0"] = parseFloat(sub_speed_context.text)
            subruntimgdata["altitudeType"] = sub_combox_altitudeType_ctrl.model.get(sub_combox_altitudeType_ctrl.currentIndex).index
            subruntimgdata["waypoints"] = waypts
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
            runtime_style_listModel.insert(curindex,{"entityid":patternAgentsitem["agentId"],"speed":patternAgentsitem["speed0"],"azimuth":patternAgentsitem["azimuth"], "altitudeType":patternAgentsitem["altitudeType"], "agentKey":patternAgentsitem["agentKey"]})
            runtime_style_listView.currentIndex = curindex


            var item = runtime_style_listView.itemAtIndex(curindex);
            var row = item.children[0]
            var row_rect = row.children[1]

            var row_sub_rect = row_rect.children[0]


            var sub_combox_altitudeType_ctrl = row_sub_rect.children[6]
            for(var m = 0; m < sub_combox_altitudeType_ctrl.model.count;m++)
            {
                if(sub_combox_altitudeType_ctrl.model.get(m).index === patternAgentsitem["altitudeType"])
                {
                    sub_combox_altitudeType_ctrl.currentIndex = m
                }
            }
            var sub_combox_agentkey_ctrl = row_sub_rect.children[7]
            for(var m = 0; m < sub_combox_agentkey_ctrl.model.count;m++)
            {
                if(sub_combox_agentkey_ctrl.model.get(m).agentKey === patternAgentsitem["agentKey"])
                {
                    sub_combox_agentkey_ctrl.currentIndex = m
                }
            }

            var sub_listview = row_sub_rect.children[9].children[0]
            var sub_listview_model = sub_listview.model

            var waypoints = patternAgentsitem["waypoints"]
            for(var j = 0;j < waypoints.length;j++)
            {
                var waycurindex = sub_listview_model.count
                var waypointsitem = waypoints[j]

                sub_listview_model.insert(sub_listview_model.count,{"entityid":patternAgentsitem["agentId"],
                                              "waypointid":patternAgentsitem["agentId"],
                                              "Longitude": waypointsitem[0].toString(),
                                              "Latitude": waypointsitem[1].toString(),
                                              "Altitude": waypointsitem[2].toString(),
                                              "TimeStamp": waypointsitem[4].toString()})
            }
        }
    }


    function appendEntity(entityid, azimuth, speed, altitudeType, AgentKey)
    {
        var curindex = runtime_style_listModel.count
        runtime_style_listModel.insert(curindex,{"entityid":entityid, "speed":speed, "azimuth":azimuth, "altitudeType":altitudeType, "agentKey":AgentKey})
        runtime_style_listView.currentIndex = curindex

        var item = runtime_style_listView.itemAtIndex(curindex);
        var row = item.children[0]
        var row_rect = row.children[1]

        var row_sub_rect = row_rect.children[0]


        var sub_combox_altitudeType_ctrl = row_sub_rect.children[6]
        for(var m = 0; m < sub_combox_altitudeType_ctrl.model.count;m++)
        {
            sub_combox_altitudeType_ctrl.currentIndex = altitudeType
//            if(sub_combox_altitudeType_ctrl.model.get(m).index === altitudeType)
//            {
//                console.log("RECV "+sub_combox_altitudeType_ctrl.model.get(m).index + " "+altitudeType);
//                sub_combox_altitudeType_ctrl.currentIndex = m
//            }
        }
        console.log("++"+AgentKey)
        var sub_combox_agentkey_ctrl = row_sub_rect.children[7]
        for(var m = 0; m < sub_combox_agentkey_ctrl.model.count;m++)
        {
            if(sub_combox_agentkey_ctrl.model.get(m).agentKey === AgentKey)
            {
                sub_combox_agentkey_ctrl.currentIndex = m
            }
        }

        parentWidget.setCurrentEntity(runtime_style_listView.model.get(curindex).entityid);

    }

    function selectEntity(entityid)
    {
        for(var i = 0;i < runtime_style_listView.model.count;i++)
        {
            if(runtime_style_listView.model.get(i).entityid === entityid)
            {
                runtime_style_listView.currentIndex = i
            }
        }
    }

    function updateWaypoint(entityid, waypointid, lng, lat)
    {
        for(var i = 0;i < runtime_style_listView.model.count;i++)
        {
            if(runtime_style_listView.model.get(i).entityid === entityid)
            {
                runtime_style_listView.currentIndex = i

                var item = runtime_style_listView.itemAtIndex(i);
                var row = item.children[0]
                var row_rect = row.children[1]

                var row_sub_rect = row_rect.children[0]

                var sub_azimuth_context = row_sub_rect.children[4]
                var sub_speed_context = row_sub_rect.children[5]
                var sub_combox_altitudeType_ctrl = row_sub_rect.children[6]
                var sub_combox_agentkey_ctrl = row_sub_rect.children[7]
                var sub_listview = row_sub_rect.children[9].children[0]
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

    function appendWaypoint(entityid, waypointid, lng, lat, Altitude, TimeStamp)
    {
        for(var i = 0;i < runtime_style_listView.model.count;i++)
        {
            if(runtime_style_listView.model.get(i).entityid === entityid)
            {
                var item = runtime_style_listView.itemAtIndex(i);
                var row = item.children[0]
                var row_rect = row.children[1]

                var row_sub_rect = row_rect.children[0]

                var sub_azimuth_context = row_sub_rect.children[4]
                var sub_speed_context = row_sub_rect.children[5]
                var sub_altitudeType_context = row_sub_rect.children[6]
                var sub_combox_agentkey_ctrl = row_sub_rect.children[7]
                var sub_listview = row_sub_rect.children[9].children[0]
                var sub_listview_model = sub_listview.model

                sub_listview_model.insert(sub_listview_model.count,{"entityid":entityid,
                                              "waypointid":waypointid,
                                              "Longitude": lng.toString(),
                                              "Latitude": lat.toString(),
                                              "Altitude": Altitude.toString(),
                                              "TimeStamp": TimeStamp.toString()})
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
            //console.log("enter")
        }
        onExited: {
            //console.log("exit")
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

    Text {
        id:runtimestyle_lisview_title
        width:100
        height: 32
        text: qsTr("runtime style")
        anchors.left: runtime_style_list_rect.left
        anchors.top: add_new_entity.top
        anchors.leftMargin: 20
        color: "#ffffff"
        font.family: "Microsoft YaHei"
        font.pixelSize: 20
        verticalAlignment: Text.AlignVCenter
        horizontalAlignment:Text.AlignHCenter

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
            height: 400
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
                        font.pixelSize: 10
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

                        Label {
                            id:agentkey_title
                            text: qsTr("AgentKey:");
                            font.pixelSize: title_font_size;
                            font.family: "Microsoft YaHei"
                            width:parent.width/10
                            height: ctrl_height
                            color:"#ffffff"
                            anchors.top: parent.top
                            anchors.topMargin: 10
                            anchors.left: parent.left
                            anchors.leftMargin: 10
                            //anchors.verticalCenter: parent.verticalCenter;
                            verticalAlignment: Text.AlignVCenter;
                        }

                        Label {
                            id:azimuth_title
                            text: qsTr("azimuth:");
                            font.pixelSize: title_font_size;
                            font.family: "Microsoft YaHei"
                            width:parent.width/10
                            height: ctrl_height
                            color:"#ffffff"
                            anchors.top: agentkey_title.bottom
                            anchors.topMargin: 5
                            anchors.left: parent.left
                            anchors.leftMargin: 10
                            //anchors.verticalCenter: parent.verticalCenter;
                            verticalAlignment: Text.AlignVCenter;
                        }
                        Label {
                            id:speed_title
                            text: qsTr("speed0:");
                            font.pixelSize: title_font_size;
                            font.family: "Microsoft YaHei"
                            width:parent.width/10
                            height: ctrl_height
                            color:"#ffffff"
                            anchors.top: azimuth_title.bottom
                            anchors.topMargin: 5
                            anchors.left: parent.left
                            anchors.leftMargin: 10
                            //anchors.verticalCenter: parent.verticalCenter;
                            verticalAlignment: Text.AlignVCenter;
                        }

                        Label {
                            id:altitudeType_title
                            text: qsTr("AltType:");
                            font.pixelSize: title_font_size;
                            font.family: "Microsoft YaHei"
                            width:parent.width/10
                            height: ctrl_height
                            color:"#ffffff"
                            anchors.top: speed_title.bottom
                            anchors.topMargin: 5
                            anchors.left: parent.left
                            anchors.leftMargin: 10
                            //anchors.verticalCenter: parent.verticalCenter;
                            verticalAlignment: Text.AlignVCenter;
                        }

                        TextField{
                            id:azimuth_context
                            anchors.top: azimuth_title.top
                            anchors.left: azimuth_title.right
                            anchors.leftMargin: 10
                            width:(sub_parent_rect.width-azimuth_title.width - ctrl_height)/2
                            height: ctrl_height
                            text:azimuth
                            color:"#ffffff"
                            font.pixelSize: 10;
                            font.family: "Microsoft YaHei"
                            background:Rectangle {
                                    anchors.centerIn: parent
                                    height: parent.height
                                    width: parent.width
                                    border.color: azimuth_context.pressed ? "#5f5f5f" : "#ffffff"
                                    border.width: azimuth_context.visualFocus ? 2 : 1
                                    color: '#3e2f30'
                                    radius: width/2
                                }
                        }

                        TextField{
                            id:speed_context
                            anchors.top: speed_title.top
                            anchors.left: speed_title.right
                            anchors.leftMargin: 10
                            width:(sub_parent_rect.width-speed_title.width - ctrl_height)/2
                            height: ctrl_height
                            text:speed
                            color:"#ffffff"
                            font.pixelSize: 10;
                            font.family: "Microsoft YaHei"
                            background:Rectangle {
                                    anchors.centerIn: parent
                                    height: parent.height
                                    width: parent.width
                                    border.color: speed_context.pressed ? "#5f5f5f" : "#ffffff"
                                    border.width: speed_context.visualFocus ? 2 : 1
                                    color: '#3e2f30'
                                    radius: width/2
                                }
                        }

                        ComboBox {
                            id: combox_altitudeType_ctrl
                            model: ListModel{}
                            anchors.top: altitudeType_title.top
                            anchors.left: altitudeType_title.right
                            anchors.leftMargin: 10
                            width:(sub_parent_rect.width-altitudeType_title.width - ctrl_height)/2
                            height: ctrl_height
                            delegate:ItemDelegate {
                                width: combox_altitudeType_ctrl.width
                                contentItem: Text {
                                    text: modelData
                                    color: "#ffffff"
                                    font: combox_altitudeType_ctrl.font
                                    elide: Text.ElideRight
                                    verticalAlignment: Text.AlignVCenter
                                }

                                Rectangle {
                                    anchors.fill: parent
                                    height: ctrl_height
                                    color: combox_altitudeType_ctrl.highlightedIndex === index?"#5f5f5f":"#3f3f3f" //选中颜色设置
                                }
                                highlighted: combox_altitudeType_ctrl.highlightedIndex === index

                                required property int index
                                required property var modelData
                            }

                            font.pixelSize: 10;
                            font.family: "Microsoft YaHei"

                            indicator: Canvas {
                                id: combox_altitudeType_canvas
                                x: combox_altitudeType_ctrl.width - width - combox_altitudeType_ctrl.rightPadding
                                y: combox_altitudeType_ctrl.topPadding + (combox_altitudeType_ctrl.availableHeight - height) / 2
                                width: 12
                                height: 8
                                contextType: "2d"

                                Connections {
                                    target: combox_altitudeType_ctrl
                                    function onPressedChanged() { combox_altitudeType_canvas.requestPaint(); }
                                }

                                onPaint: {
                                    context.reset();
                                    context.moveTo(0, 0);
                                    context.lineTo(width, 0);
                                    context.lineTo(width / 2, height);
                                    context.closePath();
                                    context.fillStyle = combox_altitudeType_ctrl.pressed ? "#5f5f5f" : "#ffffff";
                                    context.fill();
                                }
                            }

                            contentItem: Text {
                                leftPadding: 0
                                rightPadding: combox_altitudeType_ctrl.indicator.width + combox_altitudeType_ctrl.spacing

                                text: combox_altitudeType_ctrl.displayText
                                font: combox_altitudeType_ctrl.font
                                color: combox_altitudeType_ctrl.pressed ? "#5f5f5f" : "#ffffff"
                                verticalAlignment: Text.AlignVCenter
                                elide: Text.ElideRight
                            }

                            background: Rectangle {
                                implicitWidth: 120
                                implicitHeight: 20
                                border.color: combox_altitudeType_ctrl.pressed ? "#5f5f5f" : "#ffffff"
                                border.width: combox_altitudeType_ctrl.visualFocus ? 2 : 1
                                radius: 120/2

                                color:"#2e2f30"
                            }

                            popup: Popup {
                                y: combox_altitudeType_ctrl.height+2
                                width: combox_altitudeType_ctrl.width
                                implicitHeight: contentItem.implicitHeight
                                padding: 5

                                contentItem: ListView {
                                    id:cbx_altitudeType_listview
                                    clip: true
                                    implicitHeight: contentHeight
                                    model: combox_altitudeType_ctrl.popup.visible ? combox_altitudeType_ctrl.delegateModel : null
                                    currentIndex: combox_altitudeType_ctrl.highlightedIndex

                                    delegate:Rectangle {
                                        color:ListView.isCurrentItem?selectcolor:(index % 2 == 0?"#5f5f5f":"#3f3f3f") //选中颜色设置
                                    }

                                    ScrollIndicator.vertical: ScrollIndicator { }
                                }

                                background: Rectangle {
                                    border.color: "#3f3f3f"
                                    color:"#3f3f3f"
                                    radius: 2
                                }
                            }

                            Component.onCompleted: {
                                combox_altitudeType_ctrl.model.insert(combox_altitudeType_ctrl.model.count,{"index":0,"modelData":qsTr("absolute")})
                                combox_altitudeType_ctrl.model.insert(combox_altitudeType_ctrl.model.count,{"index":1,"modelData":qsTr("relative")})
                                combox_altitudeType_ctrl.currentIndex = 0

//                                for(var m = 0; m < combox_altitudeType_ctrl.model.count;m++)
//                                {
//                                    if(combox_altitudeType_ctrl.model.get(m).index === altitudeType)
//                                    {
//                                        combox_altitudeType_ctrl.currentIndex = m
//                                    }
//                                }
                            }
                        }
                        ComboBox {
                            id: combox_agentkey_ctrl
                            model: ListModel{}
                            anchors.top: agentkey_title.top
                            anchors.left: agentkey_title.right
                            anchors.leftMargin: 10
                            width:(sub_parent_rect.width-agentkey_title.width - ctrl_height)/2
                            height: ctrl_height
                            displayText:currentText
                            delegate:ItemDelegate {
                                width: combox_agentkey_ctrl.width
                                contentItem: Text {
                                    text: agentKey+" ("+agentName+")"
                                    color: "#ffffff"
                                    font: combox_agentkey_ctrl.font
                                    elide: Text.ElideRight
                                    verticalAlignment: Text.AlignVCenter
                                }

                                Rectangle {
                                    anchors.fill: parent
                                    height: ctrl_height
                                    color: combox_agentkey_ctrl.highlightedIndex === index?"#5f5f5f":"#3f3f3f" //选中颜色设置
                                }
                                highlighted: combox_agentkey_ctrl.highlightedIndex === index

                                required property int index
                                required property var agentName
                                required property var agentKey
                                required property var modelData
                            }

                            font.pixelSize: 10;
                            font.family: "Microsoft YaHei"

                            indicator: Canvas {
                                id: canvas
                                x: combox_agentkey_ctrl.width - width - combox_agentkey_ctrl.rightPadding
                                y: combox_agentkey_ctrl.topPadding + (combox_agentkey_ctrl.availableHeight - height) / 2
                                width: 12
                                height: 8
                                contextType: "2d"

                                Connections {
                                    target: combox_agentkey_ctrl
                                    function onPressedChanged() { canvas.requestPaint(); }
                                }

                                onPaint: {
                                    context.reset();
                                    context.moveTo(0, 0);
                                    context.lineTo(width, 0);
                                    context.lineTo(width / 2, height);
                                    context.closePath();
                                    context.fillStyle = combox_agentkey_ctrl.pressed ? "#5f5f5f" : "#ffffff";
                                    context.fill();
                                }
                            }

                            contentItem: Text {
                                leftPadding: 0
                                rightPadding: combox_agentkey_ctrl.indicator.width + combox_agentkey_ctrl.spacing

                                text: combox_agentkey_ctrl.displayText
                                font: combox_agentkey_ctrl.font
                                color: combox_agentkey_ctrl.pressed ? "#5f5f5f" : "#ffffff"
                                verticalAlignment: Text.AlignVCenter
                                elide: Text.ElideRight
                            }

                            background: Rectangle {
                                implicitWidth: 120
                                implicitHeight: 20
                                border.color: combox_agentkey_ctrl.pressed ? "#5f5f5f" : "#ffffff"
                                border.width: combox_agentkey_ctrl.visualFocus ? 2 : 1
                                radius: 120/2

                                color:"#2e2f30"
                            }

                            popup: Popup {
                                y: combox_agentkey_ctrl.height+2
                                width: combox_agentkey_ctrl.width
                                implicitHeight: contentItem.implicitHeight
                                padding: 5

                                contentItem: ListView {
                                    id:cbx_listview
                                    clip: true
                                    implicitHeight: contentHeight
                                    model: combox_agentkey_ctrl.popup.visible ? combox_agentkey_ctrl.delegateModel : null
                                    currentIndex: combox_agentkey_ctrl.highlightedIndex

                                    delegate:Rectangle {
                                            color:ListView.isCurrentItem?selectcolor:(index % 2 == 0?"#5f5f5f":"#3f3f3f") //选中颜色设置
                                    }

                                    ScrollIndicator.vertical: ScrollIndicator { }
                                }

                                background: Rectangle {
                                    border.color: "#3f3f3f"
                                    color:"#3f3f3f"
                                    radius: 2
                                }
                            }

                            Component.onCompleted: {
                                var agentkeysArray = parentWidget.getAgentKeysArray();
                                for (var i = 0; i < agentkeysArray.length; i++) {
                                    combox_agentkey_ctrl.model.insert(combox_agentkey_ctrl.model.count,{"index":i,
                                                                          "modelData":agentkeysArray[i]["agentKey"]+" ("+agentkeysArray[i]["agentKeyDesc"]+")",
                                                                          "agentKey":agentkeysArray[i]["agentKey"],
                                                                          "agentName":agentkeysArray[i]["agentKeyDesc"]})
                                }
                                combox_agentkey_ctrl.currentIndex = 0
                            }
                        }


                        Label {
                            id:waupoints_title
                            text: qsTr("waypoints:");
                            font.pixelSize: title_font_size;
                            font.family: "Microsoft YaHei"
                            width:parent.width/10
                            height: ctrl_height
                            color:"#ffffff"
                            anchors.top: altitudeType_title.bottom
                            anchors.topMargin: 5
                            anchors.left: parent.left
                            anchors.leftMargin: 10
                            //anchors.verticalCenter: parent.verticalCenter;
                            verticalAlignment: Text.AlignVCenter;
                        }




                        Rectangle{
                            id:waypoints_list_rect
                            anchors.top: waupoints_title.top
                            anchors.left: waupoints_title.right
                            anchors.leftMargin: 10
                            width:sub_parent_rect.width-azimuth_title.width - ctrl_height
                            height:parent.height - speed_title.height-azimuth_title.height-agentkey_title.height-altitudeType_title.height-5*4-10
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
                                            font.pixelSize: 10
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
                                            font.pixelSize: 10
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
                                            font.pixelSize: 10
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
//                                            font.pixelSize: 10
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
                                            font.pixelSize: 10
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
//                                            font.pixelSize: 10
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
                                            font.pixelSize: 10
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
                                                parentWidget.deleteWaypoint(waypoints_listModel.get(index).entityid,waypoints_listModel.get(index).waypointid)
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
                                            parentWidget.locateWaypoint(waypoints_listModel.get(index).entityid,waypoints_listModel.get(index).waypointid)
                                            selectEntity(waypoints_listModel.get(index).entityid)
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
                                y: waypoints_listView.currentItem.y
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
                                            font.pixelSize: 10
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
                                            font.pixelSize: 10
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
                                            font.pixelSize: 10
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
                                            font.pixelSize: 10
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
                                            font.pixelSize: 10
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
                                            font.pixelSize: 10
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
                            parentWidget.deleteEntity(entityid)
                            runtime_style_listModel.remove(index)
                            if(runtime_style_listModel.count > 0)
                            {
                                parentWidget.setCurrentEntity(runtime_style_listView.model.get(0).entityid)
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
                        parentWidget.setCurrentEntity(runtime_style_listView.model.get(index).entityid);
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
                        font.pixelSize: 10
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
                        font.pixelSize: 10
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
                        font.pixelSize: 10
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
