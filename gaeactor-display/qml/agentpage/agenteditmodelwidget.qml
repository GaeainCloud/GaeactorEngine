import QtQuick 2.15
import QtQuick.Window 2.15
import QtQuick.Controls 2.15


import "../wgt" as Wgt

Rectangle {
    id:agent_edit_attribute
    visible: true
    color: "#5f5f5f"
    opacity: 1.0
    property string selectcolor:"#157e5b"

    property int datatype:1
    property int dataType_Id:0

    function setDataTypeId(datatypeid)
    {
        dataType_Id = datatypeid
    }

    property string headerviewtitle:qsTr("Node")
    function updateData(contextdata)
    {
        var nodeid = contextdata["nodeid"]
        for(var i = 0;i < listcontrol_listModel.count;i++)
        {
            if(listcontrol_listModel.get(i).nodeid === nodeid)
            {
                var nodename = listcontrol_listModel.get(i).nodename
                listcontrol_listModel.set(i,{"nodeid":contextdata["nodeid"],
                                              "instanceid":contextdata["instanceid"],
                                              "nodename":nodename,
                                              "translat_x":contextdata["translat_x"],
                                              "translat_y":contextdata["translat_y"],
                                              "translat_z":contextdata["translat_z"],
                                              "roll":contextdata["roll"],
                                              "pitch":contextdata["pitch"],
                                              "yaw":contextdata["yaw"],
                                              "scale":contextdata["scale"]})
                break;
            }
        }

    }


    function selsectIndex(selinstanceid,selid)
    {
        for(var i = 0;i < listcontrol_listModel.count;i++)
        {
            if(listcontrol_listModel.get(i).nodeid === selid /*&& listcontrol_listModel.get(i).instanceid === selinstanceid*/)
            {
                list_ctrl.setSelectIndex(i)
                break;
            }
        }
    }

    function setContextData(contextdata)
    {
        listcontrol_listModel.clear()
        datatype = contextdata["datatype"]
        switch(datatype)
        {
        case 0:
        {
            agent_edit_config.text = qsTr("Model Config");
            headerviewtitle = qsTr("Model");
        }
        break;
        case 1:
        {
            agent_edit_config.text = qsTr("Node Config");
            headerviewtitle = qsTr("Node");
        }break;
        }
        var datacontext = contextdata["datacontext"]
        for(var i = 0;i < datacontext.length;i++)
        {
            var patternAgentsitem = datacontext[i]
            var curindex = listcontrol_listModel.count

            listcontrol_listModel.insert(i,{"nodeid":patternAgentsitem["nodeid"],
                                             "instanceid":patternAgentsitem["instanceid"],
                                           "nodename":patternAgentsitem["nodename"],
                                           "translat_x":patternAgentsitem["translat_x"],
                                           "translat_y":patternAgentsitem["translat_y"],
                                           "translat_z":patternAgentsitem["translat_z"],
                                           "roll":patternAgentsitem["roll"],
                                           "pitch":patternAgentsitem["pitch"],
                                           "yaw":patternAgentsitem["yaw"],
                                           "scale":patternAgentsitem["scale"]});
            list_ctrl.setSelectIndex(curindex)
        }
        //modelWidget.selectModel(datatype,listcontrol_listModel.get(curindex).instanceid, listcontrol_listModel.get(curindex).nodeid)
    }

    Text {
        id:agent_edit_config
        width:parent.width
        height: ctrl_height
        text: qsTr("Model Settings")
        anchors.left: agent_edit_attribute.left
        anchors.top: agent_edit_attribute.top
        anchors.leftMargin: 20
        anchors.topMargin: 20
        color: "#ffffff"
        font.family: "Microsoft YaHei"
        font.pixelSize: title_font_size
        verticalAlignment: Text.AlignVCenter
        horizontalAlignment:Text.AlignLeft
    }

    Wgt.ListviewControl
    {
        id:list_ctrl
        anchors.top:agent_edit_config.bottom
        anchors.topMargin:ctrl_height/4
        width: parent.width
        height: parent.height-ctrl_height*2
    }

    Wgt.Buttoncontrol
    {
        id:action_cancel_ctrl
        anchors.top:list_ctrl.bottom
        anchors.topMargin:ctrl_height/2
        anchors.left: parent.left
        anchors.leftMargin: (parent.width*3/9)-(parent.width/9/9)
        width: parent.width/9
        height: ctrl_height
    }

    Wgt.Buttoncontrol
    {
        id:action_save_ctrl
        anchors.top:list_ctrl.bottom
        anchors.topMargin:ctrl_height/2
        anchors.right: parent.right
        anchors.rightMargin: (parent.width*3/9)-(parent.width/9/9)
        width: parent.width/9
        height: ctrl_height
    }


    ListModel {
        id: listcontrol_listModel
    }

    Component {
        id: listcontrol_delegate_list
        Rectangle {
            id: listcontrol_list_rct
            height: ctrl_height*3
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
                        text: nodename
                        color: "#ffffff" //选中颜色设置
                        font.family: "Microsoft YaHei"
                        font.pixelSize: context_font_size
                        verticalAlignment: Text.AlignVCenter
                        horizontalAlignment:Text.AlignHCenter
                    }
                }


                Rectangle {
                    width: parent.width*4/6
                    height: parent.height
                    anchors.left: parent.left
                    anchors.leftMargin: parent.width*1/6
                    color:"transparent"

                    Rectangle {
                        width: parent.width/3
                        height: parent.height/3
                        anchors.left: parent.left
                        anchors.leftMargin: 0
                        anchors.top: parent.top
                        anchors.topMargin: 0
                        color:"transparent"
                        Text {
                            width:parent.width
                            height:parent.height
                            anchors.verticalCenter: parent.verticalCenter
                            anchors.horizontalCenter: parent.horizontalCenter
                            text: qsTr("X:")+translat_x
                            color: "#ffffff" //选中颜色设置
                            font.family: "Microsoft YaHei"
                            font.pixelSize: context_font_size
                            verticalAlignment: Text.AlignVCenter
                            horizontalAlignment:Text.AlignLeft
                        }
                    }

                    Rectangle {
                        width: parent.width/3
                        height: parent.height/3
                        anchors.left: parent.left
                        anchors.leftMargin: 0
                        anchors.top: parent.top
                        anchors.topMargin: parent.height/3
                        color:"transparent"
                        Text {
                            width:parent.width
                            height:parent.height
                            anchors.verticalCenter: parent.verticalCenter
                            anchors.horizontalCenter: parent.horizontalCenter
                            text: qsTr("Y:")+translat_y
                            color: "#ffffff" //选中颜色设置
                            font.family: "Microsoft YaHei"
                            font.pixelSize: context_font_size
                            verticalAlignment: Text.AlignVCenter
                            horizontalAlignment:Text.AlignLeft
                        }
                    }

                    Rectangle {
                        width: parent.width/3
                        height: parent.height/3
                        anchors.left: parent.left
                        anchors.leftMargin: 0
                        anchors.top: parent.top
                        anchors.topMargin: parent.height*2/3
                        color:"transparent"
                        Text {
                            width:parent.width
                            height:parent.height
                            anchors.verticalCenter: parent.verticalCenter
                            anchors.horizontalCenter: parent.horizontalCenter
                            text: qsTr("Z:")+translat_z
                            color: "#ffffff" //选中颜色设置
                            font.family: "Microsoft YaHei"
                            font.pixelSize: context_font_size
                            verticalAlignment: Text.AlignVCenter
                            horizontalAlignment:Text.AlignLeft
                        }
                    }

                    Rectangle {
                        width: parent.width/3
                        height: parent.height/3

                        anchors.left: parent.left
                        anchors.leftMargin: parent.width/3
                        anchors.top: parent.top
                        anchors.topMargin: 0
                        color:"transparent"
                        Text {
                            width:parent.width
                            height:parent.height
                            anchors.verticalCenter: parent.verticalCenter
                            anchors.horizontalCenter: parent.horizontalCenter
                            text: qsTr("Roll:")+roll
                            color: "#ffffff" //选中颜色设置
                            font.family: "Microsoft YaHei"
                            font.pixelSize: context_font_size
                            verticalAlignment: Text.AlignVCenter
                            horizontalAlignment:Text.AlignLeft
                        }
                    }

                    Rectangle {
                        width: parent.width/3
                        height: parent.height/3

                        anchors.left: parent.left
                        anchors.leftMargin: parent.width/3
                        anchors.top: parent.top
                        anchors.topMargin: parent.height*1/3
                        color:"transparent"
                        Text {
                            width:parent.width
                            height:parent.height
                            anchors.verticalCenter: parent.verticalCenter
                            anchors.horizontalCenter: parent.horizontalCenter
                            text: qsTr("Pitch:")+pitch
                            color: "#ffffff" //选中颜色设置
                            font.family: "Microsoft YaHei"
                            font.pixelSize: context_font_size
                            verticalAlignment: Text.AlignVCenter
                            horizontalAlignment:Text.AlignLeft
                        }
                    }

                    Rectangle {
                        width: parent.width/3
                        height: parent.height/3

                        anchors.left: parent.left
                        anchors.leftMargin: parent.width/3
                        anchors.top: parent.top
                        anchors.topMargin: parent.height*2/3
                        color:"transparent"
                        Text {
                            width:parent.width
                            height:parent.height
                            anchors.verticalCenter: parent.verticalCenter
                            anchors.horizontalCenter: parent.horizontalCenter
                            text: qsTr("Yaw:")+yaw
                            color: "#ffffff" //选中颜色设置
                            font.family: "Microsoft YaHei"
                            font.pixelSize: context_font_size
                            verticalAlignment: Text.AlignVCenter
                            horizontalAlignment:Text.AlignLeft
                        }
                    }

                    Rectangle {
                        width: parent.width/3
                        height: parent.height

                        anchors.left: parent.left
                        anchors.leftMargin: parent.width*2/3
                        anchors.top: parent.top
                        anchors.topMargin: 0
                        color:"transparent"
                        Text {
                            width:parent.width
                            height:parent.height
                            anchors.verticalCenter: parent.verticalCenter
                            anchors.horizontalCenter: parent.horizontalCenter
                            text: scale
                            color: "#ffffff" //选中颜色设置
                            font.family: "Microsoft YaHei"
                            font.pixelSize: context_font_size
                            verticalAlignment: Text.AlignVCenter
                            horizontalAlignment:Text.AlignHCenter
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
                        id:hide_btn
                        width: parent.width
                        height: ctrl_height
                        anchors.top:parent.top
                        anchors.topMargin: 0
                        anchors.left:parent.left
                        anchors.leftMargin: 0
                        anchors.verticalCenter: parent.verticalCenter
                        anchors.horizontalCenter: parent.horizontalCenter
                    }
                    Component.onCompleted: {
                        hide_btn.setTitle(qsTr("hide node"))
                        hide_btn.setToolTips(qsTr("Hide Node"))
                    }
                    Connections {
                        target: hide_btn.getBtnObj()
                        function onClicked() {
                            modelWidget.switchHideModel(datatype,instanceid, nodeid)
//                            listcontrol_listModel.remove(index)
                        }
                    }
                }
                MouseArea {
                    //                        anchors.fill: parent
                    width: parent.width*5/6
                    height: parent.height
                    anchors.right: parent.right
                    anchors.rightMargin: parent.width/6
                    onClicked:  {
                        list_ctrl.setSelectIndex(index)
                        modelWidget.selectModel(datatype,instanceid, nodeid)
                    }
                }
            }
        }
    }


    Component {
        id:listcontrol_headerView;
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
                        text: headerviewtitle
                        font.pixelSize: context_font_size
                        font.family: "Microsoft YaHei"
                        color:"#222222"
                        verticalAlignment: Text.AlignVCenter
                        horizontalAlignment:Text.AlignHCenter
                    }
                }

                Rectangle {
                    width: (parent.width*4/6)/3
                    height: parent.height
                    anchors.left: parent.left
                    anchors.leftMargin: parent.width*1/6
                    color:"transparent"
                    Text {
                        width:parent.width
                        height:parent.height
                        anchors.verticalCenter: parent.verticalCenter
                        anchors.horizontalCenter: parent.horizontalCenter
                        text: qsTr("Translate")
                        font.pixelSize: context_font_size
                        font.family: "Microsoft YaHei"
                        color:"#222222"
                        verticalAlignment: Text.AlignVCenter
                        horizontalAlignment:Text.AlignHCenter
                    }
                }

                Rectangle {
                    width: (parent.width*4/6)/3
                    height: parent.height
                    anchors.left: parent.left
                    anchors.leftMargin: parent.width*1/6+(parent.width*4/6)*1/3
                    color:"transparent"
                    Text {
                        width:parent.width
                        height:parent.height
                        anchors.verticalCenter: parent.verticalCenter
                        anchors.horizontalCenter: parent.horizontalCenter
                        text: qsTr("Rotate")
                        font.pixelSize: context_font_size
                        font.family: "Microsoft YaHei"
                        color:"#222222"
                        verticalAlignment: Text.AlignVCenter
                        horizontalAlignment:Text.AlignHCenter
                    }
                }

                Rectangle {
                    width: (parent.width*4/6)/3
                    height: parent.height
                    anchors.left: parent.left
                    anchors.leftMargin: parent.width*1/6+(parent.width*4/6)*2/3
                    color:"transparent"
                    Text {
                        width:parent.width
                        height:parent.height
                        anchors.verticalCenter: parent.verticalCenter
                        anchors.horizontalCenter: parent.horizontalCenter
                        text: qsTr("Scale")
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

    Component.onCompleted: {

        action_cancel_ctrl.setTitle(qsTr("Cancel"))
        action_save_ctrl.setTitle(qsTr("Save"))

        action_cancel_ctrl.title_font_size=context_font_size
        action_save_ctrl.title_font_size=context_font_size
        list_ctrl.title_font_size=context_font_size

        list_ctrl.setContext(listcontrol_listModel,listcontrol_delegate_list, listcontrol_headerView)
    }

    Connections {
        target: action_cancel_ctrl.getBtnObj()
        function onClicked() {
            console.log("Cancel button clicked")
            // 这里可以处理按钮点击后的逻辑
        }
    }

    Connections {
        target: action_save_ctrl.getBtnObj()
        function onClicked() {
            console.log("Cancel button clicked")
            // 这里可以处理按钮点击后的逻辑
        }
    }
}
