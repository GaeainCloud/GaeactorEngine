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

    function appendPoint(pointid, lngval, latval , altval)
    {
        var curindex = fence_points_list_listModel.count
        fence_points_list_listModel.insert(curindex,{"pointid":pointid, "lng":lngval, "lat":latval, "alt":altval})
        fence_points_list_ctrl.setSelectIndex(curindex)
    }

    function updatePoint(updatepointid, lngval, latval , altval)
    {
        var pointidtmp=""
        for(var i = 0; i < fence_points_list_listModel.count; i++)
        {
            if(fence_points_list_listModel.get(i).pointid === updatepointid)
            {
//                fence_points_list_listModel.get(i).lng = lngval
//                fence_points_list_listModel.get(i).lat = latval
//                fence_points_list_listModel.get(i).alt = altval
                fence_points_list_listModel.remove(i)
                fence_points_list_listModel.insert(i,{"pointid":updatepointid, "lng":lngval, "lat":latval, "alt":altval})
                fence_points_list_ctrl.setSelectIndex(i)
                pointidtmp = updatepointid
                break;
            }
        }
    }

    function selectPoint(pointid)
    {
        var pointidtmp=""
        for(var i = 0; i < fence_points_list_listModel.count; i++)
        {
            if(fence_points_list_listModel.get(i).pointid === pointid)
            {
                fence_points_list_ctrl.setSelectIndex(i)
                pointidtmp = pointid
                break;
            }
        }
    }

    function setContextData(contextdata)
    {        
        fence_points_list_listModel.clear();
        setValContext(fenceKey_ctrl, contextdata, "fenceKey")
        setValContext(fenceKeyword_ctrl, contextdata, "fenceKeyword")
        setValContext(fenceName_ctrl, contextdata, "fenceName")
        setValContext(fenceNameI18n_ctrl, contextdata, "fenceNameI18n")

        setValIndex(fenceFrame_ctrl, contextdata, "fenceFrame")

        var agenttype = modelWidget.getAgentType();
        if(agenttype === "Instagent")
        {
            fenceFrame_ctrl.setSelectIndex(1)
        }
        else if(agenttype === "Scene")
        {
            fenceFrame_ctrl.setSelectIndex(0)
        }

        if("fencePoints" in contextdata)
        {
            var wps = contextdata["fencePoints"];

            fence_points_list_listModel.clear();
            for(var i = 0; i < wps.length;i++)
            {
                var wpsitem = wps[i]
                if(wpsitem.length === 4)
                {
                    var curindex = fence_points_list_listModel.count
                    fence_points_list_listModel.insert(curindex,{"pointid":wpsitem[0], "lng":wpsitem[1], "lat":wpsitem[2], "alt":wpsitem[3]})
                    fence_points_list_ctrl.setSelectIndex(curindex)
                }
            }
        }
    }

    function saveContext()
    {
        var exportcontextdata={}

        exportcontextdata["fenceKey"] = fenceKey_ctrl.getContext()
        exportcontextdata["fenceKeyword"] = fenceKeyword_ctrl.getContext()
        exportcontextdata["fenceName"] = fenceName_ctrl.getContext()
        exportcontextdata["fenceNameI18n"] = fenceNameI18n_ctrl.getContext()
        exportcontextdata["fenceFrame"] = fenceFrame_ctrl.getSelectIndex()


        var waypts=[]
        for(var i = 0; i < fence_points_list_listModel.count; i++)
        {
            var waypt=[]
            waypt.push(fence_points_list_listModel.get(i).lng)
            waypt.push(fence_points_list_listModel.get(i).lat)
            waypt.push(fence_points_list_listModel.get(i).alt)
            waypts.push(waypt)
        }
        exportcontextdata["fencePoints"] = waypts;

        modelWidget.updateContext(dataType_Id, edit_id, exportcontextdata)
    }
    Text {
        id:agent_edit_config
        width:parent.width
        height: ctrl_height
        text: qsTr("Fences Settings")
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
        id:fenceKey_ctrl
        anchors.top:agent_edit_config.bottom
        anchors.topMargin:ctrl_height/4
        width: parent.width
        height: ctrl_height
    }

    Wgt.TextFiledcontrol
    {
        id:fenceKeyword_ctrl
        anchors.top:fenceKey_ctrl.bottom
        anchors.topMargin:ctrl_height/4
        width: parent.width
        height: ctrl_height
    }

    Wgt.TextFiledcontrol
    {
        id:fenceName_ctrl
        anchors.top:fenceKeyword_ctrl.bottom
        anchors.topMargin:ctrl_height/4
        width: parent.width
        height: ctrl_height
    }

    Wgt.TextFiledcontrol
    {
        id:fenceNameI18n_ctrl
        anchors.top:fenceName_ctrl.bottom
        anchors.topMargin:ctrl_height/4
        width: parent.width
        height: ctrl_height
    }

    Wgt.Comboxcontrol
    {
        id:fenceFrame_ctrl
        anchors.top:fenceNameI18n_ctrl.bottom
        anchors.topMargin:ctrl_height/4
        width: parent.width
        height: ctrl_height
    }

    Label {
        id:fence_points_list_ctrl_title
        text: qsTr("fence Points:");
        font.pixelSize: context_font_size;
        font.family: "Microsoft YaHei"
        width:120
        height: ctrl_height
        color:"#ffffff"
        anchors.top:fenceFrame_ctrl.bottom
        anchors.topMargin:ctrl_height/4+5
        anchors.left: parent.left
        anchors.leftMargin: 10
        anchors.verticalCenter: parent.verticalCenter;
        horizontalAlignment: Text.AlignRight;
    }

    Wgt.ListviewControl
    {
        id:fence_points_list_ctrl
        anchors.top: fence_points_list_ctrl_title.top
        anchors.left: fence_points_list_ctrl_title.right
        anchors.leftMargin: 10
        width:(parent.width-fence_points_list_ctrl_title.width - 32)
        height: parent.height-ctrl_height*8
    }



    Wgt.Buttoncontrol
    {
        id:action_cancel_ctrl
        anchors.top:fence_points_list_ctrl.bottom
        anchors.topMargin:ctrl_height/2
        anchors.left: parent.left
        anchors.leftMargin: (parent.width*3/8)-(parent.width/8/8)
        width: parent.width/8
        height: ctrl_height
    }

    Wgt.Buttoncontrol
    {
        id:action_save_ctrl
        anchors.top:fence_points_list_ctrl.bottom
        anchors.topMargin:ctrl_height/2
        anchors.right: parent.right
        anchors.rightMargin: (parent.width*3/8)-(parent.width/8/8)
        width: parent.width/8
        height: ctrl_height
    }


    ListModel {
        id: fence_points_list_listModel
    }


    Component {
        id: fence_points_list_delegate_list
        Rectangle {
            id: fence_points_list_rct
            height: ctrl_height*5
            width: ListView.view.width

            color:ListView.isCurrentItem?selectcolor:(index % 2 == 0?"#5f5f5f":"#3f3f3f") //选中颜色设置

            Rectangle {
                anchors.fill: parent
                color: "transparent"
                Wgt.TextFiledcontrol
                {
                    id:fence_points_lng_ctrl
                    anchors.top:parent.top
                    anchors.topMargin:0
                    width: parent.width
                    height: ctrl_height
                }


                Wgt.TextFiledcontrol
                {
                    id:fence_points_lat_ctrl
                    anchors.top:fence_points_lng_ctrl.bottom
                    anchors.topMargin:ctrl_height/4
                    width: parent.width
                    height: ctrl_height
                }

                Wgt.TextFiledcontrol
                {
                    id:fence_points_alt_ctrl
                    anchors.top:fence_points_lat_ctrl.bottom
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
                        fence_points_list_ctrl.setSelectIndex(index)
                        modelWidget.selectModel(datatype,instanceid, nodeid)
                    }
                }
            }
            Component.onCompleted:
            {
                var agenttype = modelWidget.getAgentType();
                if(agenttype === "Instagent")
                {
                    fence_points_lng_ctrl.setTitle(qsTr("X:"))
                    fence_points_lat_ctrl.setTitle(qsTr("Y:"))
                    fence_points_alt_ctrl.setTitle(qsTr("Z:"))

                }
                else if(agenttype === "Scene")
                {
                    fence_points_lng_ctrl.setTitle(qsTr("Lng:"))
                    fence_points_lat_ctrl.setTitle(qsTr("Lat:"))
                    fence_points_alt_ctrl.setTitle(qsTr("Alt:"))
                }

                fence_points_lng_ctrl.title_font_size=context_font_size
                fence_points_lat_ctrl.title_font_size=context_font_size
                fence_points_alt_ctrl.title_font_size=context_font_size

                fence_points_lng_ctrl.setContext(lng)
                fence_points_lat_ctrl.setContext(lat)
                fence_points_alt_ctrl.setContext(alt)

            }
        }
    }

    Component.onCompleted: {

        fenceKey_ctrl.setTitle(qsTr("fence Key:"))
        fenceKeyword_ctrl.setTitle(qsTr("fence Keyword:"))
        fenceFrame_ctrl.setTitle(qsTr("fence Frame:"))

        fenceName_ctrl.setTitle(qsTr("fence Name:"))
        fenceNameI18n_ctrl.setTitle(qsTr("fence NameI18n:"))

        action_cancel_ctrl.setTitle(qsTr("Cancel"))
        action_save_ctrl.setTitle(qsTr("Save"))


        fenceKey_ctrl.title_font_size=context_font_size
        fenceKeyword_ctrl.title_font_size=context_font_size
        fenceFrame_ctrl.title_font_size=context_font_size

        fenceName_ctrl.title_font_size=context_font_size
        fenceNameI18n_ctrl.title_font_size=context_font_size
        action_cancel_ctrl.title_font_size=context_font_size
        action_save_ctrl.title_font_size=context_font_size


        var treeModel6 = [{"index":0,"modelData":qsTr("absolute")},
                          {"index":1,"modelData":qsTr("relative")}]
        fenceFrame_ctrl.updateData(treeModel6)

        fence_points_list_ctrl.setContext(fence_points_list_listModel,fence_points_list_delegate_list, null)
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
            saveContext()
        }
    }
}
