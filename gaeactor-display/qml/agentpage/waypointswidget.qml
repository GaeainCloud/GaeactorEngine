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

    property int dataType_Id:0

    function setDataTypeId(datatypeid)
    {
        dataType_Id = datatypeid
    }

    property string edit_id:""

    function setContextDataId(contextid)
    {
        edit_id = contextid
    }
    function setValContext(_ctrl,contextdata,_key)
    {
        if( _key in contextdata)
        {
            _ctrl.setContext(contextdata[_key])
        }
    }

    function setValJsonContext(_ctrl,contextdata,_key)
    {
        if( _key in contextdata)
        {
            _ctrl.setContext(contextdata[_key].toString())
        }
    }

    function setValSwitch(_ctrl,contextdata,_key)
    {
        if( _key in contextdata)
        {
            _ctrl.setSwitch(contextdata[_key])
        }
    }
    function setValIndex(_ctrl,contextdata,_key)
    {
        if( _key in contextdata)
        {
            _ctrl.setSelectIndex(contextdata[_key])
        }
    }
    function setContextData(contextdata)
    {
        setValContext(waypoints_usage_ctrl, contextdata, "wpsUsage")
        setValContext(waypoints_mark_ctrl, contextdata, "wpsKeyword")

        if("wps" in contextdata)
        {
            var wps = contextdata["wps"];
            waypoints_list_listModel.clear();
            for(var i = 0; i < wps.length;i++)
            {
                var wpsitem = wps[i]
                if(wpsitem.length === 5)
                {
                    var curindex = waypoints_list_listModel.count
                    waypoints_list_listModel.insert(curindex,{"lng":wpsitem[0], "lat":wpsitem[1], "alt":wpsitem[2], "altitudeType":wpsitem[3], "index":wpsitem[4]})
                    waypoints_list_ctrl.setSelectIndex(curindex)
                }
            }
        }
    }

    function saveContext()
    {
        var exportcontextdata={}

        exportcontextdata["wpsUsage"] = waypoints_usage_ctrl.getContext()
        exportcontextdata["wpsKeyword"] = waypoints_mark_ctrl.getContext()

        var waypts=[]
        for(var i = 0; i < waypoints_list_listModel.count; i++)
        {
            var waypt=[]
            waypt.push(waypoints_list_listModel.get(i).lng)
            waypt.push(waypoints_list_listModel.get(i).lat)
            waypt.push(waypoints_list_listModel.get(i).alt)
            waypt.push(waypoints_list_listModel.get(i).altitudeType)
            waypt.push(waypoints_list_listModel.get(i).index)
            waypts.push(waypt)
        }
        exportcontextdata["wps"] = waypts;

        modelWidget.updateContext(dataType_Id, edit_id, exportcontextdata)
    }


    Text {
        id:agent_edit_config
        width:parent.width
        height: ctrl_height
        text: qsTr("WayPoints Settings")
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

    Wgt.TextFiledcontrol
    {
        id:waypoints_usage_ctrl
        anchors.top:agent_edit_config.bottom
        anchors.topMargin:ctrl_height/4
        width: parent.width
        height: ctrl_height
    }

    Wgt.TextFiledcontrol
    {
        id:waypoints_mark_ctrl
        anchors.top:waypoints_usage_ctrl.bottom
        anchors.topMargin:ctrl_height/4
        width: parent.width
        height: ctrl_height
    }

    Label {
        id:waypoints_list_ctrl_title
        text: qsTr("wps:");
        font.pixelSize: context_font_size;
        font.family: "Microsoft YaHei"
        width:120
        height: ctrl_height
        color:"#ffffff"
        anchors.top:waypoints_mark_ctrl.bottom
        anchors.topMargin:ctrl_height/4+5
        anchors.left: parent.left
        anchors.leftMargin: 10
        anchors.verticalCenter: parent.verticalCenter;
        horizontalAlignment: Text.AlignRight;
    }

    Wgt.ListviewControl
    {
        id:waypoints_list_ctrl
        anchors.top: waypoints_list_ctrl_title.top
        anchors.left: waypoints_list_ctrl_title.right
        anchors.leftMargin: 10
        width:(parent.width-waypoints_list_ctrl_title.width - 32)
        height: parent.height-ctrl_height*8
    }

    Wgt.Buttoncontrol
    {
        id:sensing_cancel_ctrl
        anchors.top:waypoints_list_ctrl.bottom
        anchors.topMargin:ctrl_height/2
        anchors.left: parent.left
        anchors.leftMargin: (parent.width*3/8)-(parent.width/8/8)
        width: parent.width/8
        height: ctrl_height
    }

    Wgt.Buttoncontrol
    {
        id:sensing_save_ctrl
        anchors.top:waypoints_list_ctrl.bottom
        anchors.topMargin:ctrl_height/2
        anchors.right: parent.right
        anchors.rightMargin: (parent.width*3/8)-(parent.width/8/8)
        width: parent.width/8
        height: ctrl_height
    }

    ListModel {
        id: waypoints_list_listModel
    }


    Component {
        id: waypoints_list_delegate_list
        Rectangle {
            id: waypoints_list_rct
            height: ctrl_height*7
            width: ListView.view.width

            color:ListView.isCurrentItem?selectcolor:(index % 2 == 0?"#5f5f5f":"#3f3f3f") //选中颜色设置

            Rectangle {
                anchors.fill: parent
                color: "transparent"
                Wgt.TextFiledcontrol
                {
                    id:waypoints_lng_ctrl
                    anchors.top:parent.top
                    anchors.topMargin:0
                    width: parent.width
                    height: ctrl_height
                }


                Wgt.TextFiledcontrol
                {
                    id:waypoints_lat_ctrl
                    anchors.top:waypoints_lng_ctrl.bottom
                    anchors.topMargin:ctrl_height/4
                    width: parent.width
                    height: ctrl_height
                }

                Wgt.TextFiledcontrol
                {
                    id:waypoints_alt_ctrl
                    anchors.top:waypoints_lat_ctrl.bottom
                    anchors.topMargin:ctrl_height/4
                    width: parent.width
                    height: ctrl_height
                }

                Wgt.Comboxcontrol
                {
                    id:waypoints_alt_type_ctrl
                    anchors.top:waypoints_alt_ctrl.bottom
                    anchors.topMargin:ctrl_height/4
                    width: parent.width
                    height: ctrl_height
                }


                Wgt.TextFiledcontrol
                {
                    id:waypoints_index_ctrl
                    anchors.top:waypoints_alt_type_ctrl.bottom
                    anchors.topMargin:ctrl_height/4
                    width: parent.width
                    height: ctrl_height
                }

                MouseArea {
                    //                        anchors.fill: parent
                    width: parent.width/10+10
                    height: parent.height
                    anchors.left: parent.left
                    anchors.leftMargin: 0
                    onClicked:  {
                        waypoints_list_ctrl.setSelectIndex(index)
                        modelWidget.selectModel(datatype,instanceid, nodeid)
                    }
                }
            }
            Component.onCompleted:
            {
                var treeModel6 = [{"index":0,"modelData":qsTr("absolute")},
                                  {"index":1,"modelData":qsTr("relative")}]
                waypoints_alt_type_ctrl.updateData(treeModel6)

                var agenttype = modelWidget.getAgentType();
                if(agenttype === "Instagent")
                {
                    waypoints_lng_ctrl.setTitle(qsTr("X:"))
                    waypoints_lat_ctrl.setTitle(qsTr("Y:"))
                    waypoints_alt_ctrl.setTitle(qsTr("Z:"))
                }
                else if(agenttype === "Scene")
                {
                    waypoints_lng_ctrl.setTitle(qsTr("Lng:"))
                    waypoints_lat_ctrl.setTitle(qsTr("Lat:"))
                    waypoints_alt_ctrl.setTitle(qsTr("Alt:"))
                }

                waypoints_alt_type_ctrl.setTitle(qsTr("Alt Type:"))
                waypoints_index_ctrl.setTitle(qsTr("Index:"))

                waypoints_lng_ctrl.title_font_size=context_font_size
                waypoints_lat_ctrl.title_font_size=context_font_size
                waypoints_alt_ctrl.title_font_size=context_font_size
                waypoints_alt_type_ctrl.title_font_size=context_font_size
                waypoints_index_ctrl.title_font_size=context_font_size

                waypoints_lng_ctrl.setContext(lng)
                waypoints_lat_ctrl.setContext(lat)
                waypoints_alt_ctrl.setContext(alt)
                waypoints_alt_type_ctrl.setSelectIndex(altitudeType)
                waypoints_index_ctrl.setContext(index)


            }
        }
    }

    Component.onCompleted: {

        waypoints_usage_ctrl.setTitle(qsTr("Usage:"))
        waypoints_mark_ctrl.setTitle(qsTr("wpsKeyword:"))

        sensing_cancel_ctrl.setTitle(qsTr("Cancel"))
        sensing_save_ctrl.setTitle(qsTr("Save"))


        waypoints_usage_ctrl.title_font_size=context_font_size
        waypoints_mark_ctrl.title_font_size=context_font_size


        sensing_cancel_ctrl.title_font_size=context_font_size
        sensing_save_ctrl.title_font_size=context_font_size


        waypoints_list_ctrl.setContext(waypoints_list_listModel,waypoints_list_delegate_list, null)

        var agenttype = modelWidget.getAgentType();
        if(agenttype === "Instagent")
        {
            waypoints_list_ctrl_title.text = qsTr("wps:")+"lng/lat/alt";
        }
        else if(agenttype === "Scene")
        {
            waypoints_list_ctrl_title.text = qsTr("wps:")+"x/y/z";
        }
    }

    Connections {
        target: sensing_cancel_ctrl.getBtnObj()
        function onClicked() {
            console.log("Cancel button clicked")
            // 这里可以处理按钮点击后的逻辑
        }
    }

    Connections {
        target: sensing_save_ctrl.getBtnObj()
        function onClicked() {
            saveContext()
        }
    }
}
