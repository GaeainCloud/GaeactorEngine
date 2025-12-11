import QtQuick 2.15
import QtQuick.Window 2.15
import QtQuick.Controls 2.15

import "./wgt" as Wgt

Rectangle {
    id:root
    visible: true
    color: "#2e2f30"
    property string selectcolor:"#157e5b"
    Loader{
        id:loader
    }

    signal qml_add_runtime_signal
    signal qml_send_agent_data_signal

    function appendRuntimeStyle(runtimeid,item_name,item_time,item_founder, item_num, item_status)
    {
        var curindex = runtimestyle_listModel.count
        runtimestyle_listModel.insert(curindex,{"runtimeid":runtimeid,"item_name":item_name,"item_time":item_time,"item_founder":item_founder, "item_num":item_num, "item_status":item_status})
        runtimestyle_listView.currentIndex = curindex
    }

    function updateRuntimeStyle(runtimeid,item_name,item_time,item_founder, item_num, item_status)
    {
        for(var m = 0; m < runtimestyle_listModel.count;m++)
        {
            if(runtimestyle_listModel.get(m).runtimeid === runtimeid)
            {
                runtimestyle_listModel.set(m,{"runtimeid":runtimeid,"item_name":item_name,"item_time":item_time,"item_founder":item_founder, "item_num":item_num, "item_status":item_status})
            }
        }
    }

    Rectangle
    {
        id:parent_rect
        anchors.fill: parent
        color: "#2e2f30"
        opacity: 1.0
        Text {
            id:user_code
            color: "#ffffff"
            width:100
            height: 32
            text: qsTr("user code")
            anchors.right: parent.right
            anchors.margins: 10
            anchors.topMargin: 0
            font.pixelSize: context_font_size
            font.family: "Microsoft YaHei"
            verticalAlignment: Text.AlignVCenter
            horizontalAlignment:Text.AlignHCenter

        }
        //        Image {
        //            id:user_iamge
        //            width: 32
        //            height: 32
        //            source:"H:/work/gaeactor/gaeactor-display/res/icon/radar_5.png"
        //            anchors.right: parent.right
        //            anchors.margins: 10
        //            anchors.topMargin: 0
        //        }

        MouseArea {
            id:add_new
            anchors.top: user_code.bottom
            anchors.right: parent.right
            anchors.rightMargin: 2
            width: 204
            height: 32
            Wgt.Buttoncontrol
            {
                id:add_new_btn
                width: (parent.width - 4)/2
                height: parent.height
                anchors.right: parent.right
                anchors.rightMargin: 0
            }
            Connections {
                target: add_new_btn.getBtnObj()
                function onClicked() {
                    qml_add_runtime_signal()

                }
            }

            Wgt.Buttoncontrol
            {
                id:send_agentdata_btn
                width: (parent.width - 4)/2
                height: parent.height
                anchors.right: add_new_btn.left
                anchors.rightMargin: 4
            }
            Component.onCompleted: {
                add_new_btn.setTitle(qsTr("Add New Runtime Style"))
                send_agentdata_btn.setTitle(qsTr("Send Agent Data"))
            }
            Connections {
                target: send_agentdata_btn.getBtnObj()
                function onClicked() {
                    qml_send_agent_data_signal()

                }
            }

            hoverEnabled: true
            onEntered:{
            }
            onExited: {
            }
        }



        Label {
            id:lisview_title
            width:320
            height: 32
            text: qsTr("runtime title")
            anchors.left: runtime_list_rect.left
            anchors.top: add_new.top
            anchors.leftMargin: 20
            color: "#ffffff"
            font.family: "Microsoft YaHei"
            font.pixelSize: title_font_size
            verticalAlignment: Text.AlignVCenter
            horizontalAlignment:Text.AlignLeft

        }


        Rectangle{
            id:runtime_list_rect
            anchors.top: add_new.bottom
            anchors.topMargin: 2
            anchors.right: parent_rect.right
            anchors.rightMargin: 5
            width: parent.width
            height:parent.height - add_new.height-user_code.height-5
            color: 'transparent'

            ListView {
                id: runtimestyle_listView
                anchors.fill: parent
                model:ListModel {
                    id: runtimestyle_listModel
                }
                delegate: runtimestyle_delegate
                highlight: highlight1 // 高亮设置
                highlightFollowsCurrentItem: false
                interactive: true // 设置为可交互
                focus: true // 获取焦点
                clip: true; //超出边界的数据进行裁剪
                header:headerView;//只构建表头上滑动时表头也会跟随上滑动消失
                headerPositioning: ListView.OverlayHeader;//枚举类型

                ScrollBar.vertical: ScrollBar {       //滚动条
                    visible: runtimestyle_listView.contentHeight > runtimestyle_listView.height // 根据内容高度和列表高度判断滚动条是否可见
                    policy:ScrollBar.AsNeeded
                    anchors.right: runtimestyle_listView.right
                    anchors.top: runtimestyle_listView.top
                    anchors.topMargin: 32
                    width: 5
                    active: true
                    background: Item {            //滚动条的背景样式
                        Rectangle {
                            anchors.centerIn: parent
                            height: parent.height
                            width: parent.width*0.5
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
            id: runtimestyle_delegate
            Rectangle {

                id: delegate_list_rct1
                height: 35
                width: ListView.view.width

                color:ListView.isCurrentItem?selectcolor:(index % 2 == 0?"#5f5f5f":"#3f3f3f")

                Item {
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
                            text: item_name
                            color: ListView.isCurrentItem ? selectcolor : "#ffffff" //选中颜色设置
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
                            text: item_time
                            color: ListView.isCurrentItem ? selectcolor : "#ffffff" //选中颜色设置
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
                            text:  item_founder
                            color: ListView.isCurrentItem ? selectcolor : "#ffffff" //选中颜色设置
                            font.family: "Microsoft YaHei"
                            font.pixelSize: context_font_size
                            verticalAlignment: Text.AlignVCenter
                            horizontalAlignment:Text.AlignHCenter
                        }
                    }

                    Rectangle {
                        id:num_context
                        width: parent.width/12
                        height: parent.height
                        anchors.left: parent.left
                        anchors.leftMargin: parent.width*3/6
                        color:"transparent"
                        Text {
                            width:parent.width
                            height:parent.height
                            anchors.verticalCenter: parent.verticalCenter
                            anchors.horizontalCenter: parent.horizontalCenter
                            text: item_num
                            color: ListView.isCurrentItem ? selectcolor : "#ffffff" //选中颜色设置
                            font.family: "Microsoft YaHei"
                            font.pixelSize: context_font_size
                            verticalAlignment: Text.AlignVCenter
                            horizontalAlignment:Text.AlignHCenter
                        }
                    }


                    Rectangle {
                        id:status_context
                        width: parent.width/12
                        height: parent.height
                        anchors.left: num_context.right
                        anchors.leftMargin: 0
                        color:"transparent"

                        Button {
                            id:status_btn
                            width: 15
                            height: 15
                            background: Rectangle {
                                color: item_status?'green':'red'
                                radius: width/2
                            }
                            anchors.horizontalCenter: parent.horizontalCenter
                            anchors.verticalCenter: parent.verticalCenter
                            ToolTip{
                                delay: 500              //tooltip 500ms后出现
                                timeout: 5000           //tooltip 5s后自动消失
                                visible: status_btn.hovered    //鼠标进入button中
//                                text: item_status?qsTr("running"):qsTr("stopping")
                                text: item_status?qsTr("departure"):qsTr("arrived")
                                background: Rectangle {
                                    color: "lightgray"
                                    radius: 4
                                }
                            }
                        }
                    }

                    Rectangle {
                        width: parent.width*2/6
                        height: parent.height
                        anchors.left: status_context.right
                        anchors.leftMargin: 0
                        color:"transparent"
                        Wgt.Buttoncontrol
                        {
                            id:execute_btn
//                            width: item_status ? parent.width - parent.width/6:(parent.width - parent.width*4/12)/3
                            width: (parent.width - parent.width*4/12)/3
                            height: parent.height/2
                            anchors.left: parent.left
                            anchors.leftMargin: parent.width/12
                            anchors.verticalCenter: parent.verticalCenter
                        }
                        Connections {
                            target: execute_btn.getBtnObj()
                            function onClicked() {
                                parentWidget.runRuntimeStyle(runtimestyle_listModel.get(index).runtimeid)
                            }
                        }

                        Wgt.Buttoncontrol
                        {
                            id:edit_btn
                            width: (parent.width - parent.width*4/12)/3
                            height: parent.height/2
                            anchors.left: execute_btn.right
                            anchors.leftMargin: parent.width/12
                            anchors.verticalCenter: parent.verticalCenter
                        }
                        Connections {
                            target: edit_btn.getBtnObj()
                            function onClicked() {
                                parentWidget.editRuntimeStyle(runtimestyle_listModel.get(index).runtimeid)
                            }
                        }

                        Wgt.Buttoncontrol
                        {
                            id:delete_btn
//                            visible: item_status?false:true
                            visible: true
                            width: (parent.width - parent.width*4/12)/3
                            height: parent.height/2
                            anchors.right: parent.right
                            anchors.rightMargin: parent.width/12
                            anchors.verticalCenter: parent.verticalCenter
                        }
                        Connections {
                            target: delete_btn.getBtnObj()
                            function onClicked() {
                                parentWidget.deleteRuntimeStyle(runtimestyle_listModel.get(index).runtimeid)
                                runtimestyle_listModel.remove(index)
                            }
                        }
                        Component.onCompleted: {
                            execute_btn.setTitle(qsTr("execute"))
                            execute_btn.setToolTips(qsTr("Execute Runtime Style"))
                            edit_btn.setTitle(qsTr("edit"))
                            edit_btn.setToolTips(qsTr("Eidt Runtime Style"))
                            delete_btn.setTitle(qsTr("delete"))
                            delete_btn.setToolTips(qsTr("Delete Runtime Style"))
                        }
                    }
                    MouseArea {
                        //                        anchors.fill: parent
                        width: parent.width*4/6
                        height: parent.height
                        anchors.right: parent.right
                        anchors.rightMargin: parent.width*2/6
                        onClicked:  {
                            runtimestyle_listView.currentIndex = index
                        }
                    }
                }
            }
        }

        Component.onCompleted: {
            // 动态生成 listmodel
            //        for ( var i = 0; i < 100; i++ ) {
            //            runtimestyle_listModel.append({"txtName": "text_" + i});
            //        }
        }

        Component{   //高亮条
            id: highlight1
            Rectangle {
                width: 180;
                height: 32
                color: "lightsteelblue"; radius: 0
                y: runtimestyle_listView.currentItem.y
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
            id:headerView;
            Rectangle {
                width:parent.width;
                height: 32;
                color: "lightgrey";
                z:2;//将表头的z坐标设置在上层，表头在设置属性为overlayHeader时就不会随滑动而消失，始终显示在最上面

                Item {
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
                            text: qsTr("Name")
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
                        anchors.leftMargin: parent.width*1/6
                        color:"transparent"
                        Text {
                            width:parent.width
                            height:parent.height
                            anchors.verticalCenter: parent.verticalCenter
                            anchors.horizontalCenter: parent.horizontalCenter
                            text: qsTr("Time")
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
                            text: qsTr("Founder")
                            font.pixelSize: context_font_size
                            font.family: "Microsoft YaHei"
                            color:"#222222"
                            verticalAlignment: Text.AlignVCenter
                            horizontalAlignment:Text.AlignHCenter
                        }
                    }

                    Rectangle {
                        id:num_title
                        width: parent.width/12
                        height: parent.height
                        anchors.left: parent.left
                        anchors.leftMargin: parent.width*3/6
                        color:"transparent"
                        Text {
                            width:parent.width
                            height:parent.height
                            anchors.verticalCenter: parent.verticalCenter
                            anchors.horizontalCenter: parent.horizontalCenter
                            text: qsTr("Num")
                            font.pixelSize: context_font_size
                            font.family: "Microsoft YaHei"
                            color:"#222222"
                            verticalAlignment: Text.AlignVCenter
                            horizontalAlignment:Text.AlignHCenter
                        }
                    }

                    Rectangle {
                        id:status_title
                        width: parent.width/12
                        height: parent.height
                        anchors.left: num_title.right
                        anchors.leftMargin: 0
                        color:"transparent"
                        Text {
                            width:parent.width
                            height:parent.height
                            anchors.verticalCenter: parent.verticalCenter
                            anchors.horizontalCenter: parent.horizontalCenter
                            text: qsTr("Status")
                            font.pixelSize: context_font_size
                            font.family: "Microsoft YaHei"
                            color:"#222222"
                            verticalAlignment: Text.AlignVCenter
                            horizontalAlignment:Text.AlignHCenter
                        }
                    }

                    Rectangle {
                        width: parent.width*2/6
                        height: parent.height
                        anchors.left: status_title.right
                        anchors.leftMargin: 0
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
