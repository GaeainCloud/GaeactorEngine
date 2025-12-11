import QtQuick 2.15
import QtQuick.Window 2.15
import QtQuick.Controls 2.15

import QtQuick.Dialogs 1.3

import "../wgt" as Wgt

Rectangle {
    id:agent_edit_attribute
    visible: true
    color: "#5f5f5f"
    opacity: 1.0
    property string selectcolor:"#157e5b"

    property int dataType_Id:0

    property string mpointid:""

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
        mpointid = pointid
        poi_points_list_listModel.clear();
        var curindex = poi_points_list_listModel.count
        poi_points_list_listModel.insert(curindex,{"pointid":pointid, "lng":lngval, "lat":latval, "alt":altval})
        poi_points_list_ctrl.setSelectIndex(curindex)
    }

    function updatePoint(updatepointid, lngval, latval , altval)
    {
        for(var i = 0; i < poi_points_list_listModel.count; i++)
        {
            if(poi_points_list_listModel.get(i).pointid === updatepointid)
            {
                //poi_points_list_listModel.set(i,{"pointid":updatepointid, "lng":lngval, "lat":latval, "alt":altval})
                poi_points_list_listModel.remove(i)
                poi_points_list_listModel.insert(i,{"pointid":updatepointid, "lng":lngval, "lat":latval, "alt":altval})
//                poi_points_list_ctrl.update()
                poi_points_list_ctrl.setSelectIndex(i)
                break;
            }
        }
//        poi_points_list_listModel.clear();
//        var curindex = poi_points_list_listModel.count
//        poi_points_list_listModel.insert(curindex,{"pointid":updatepointid, "lng":lngval, "lat":latval, "alt":altval})
//        poi_points_list_ctrl.setSelectIndex(curindex)
    }

    function selectPoint(pointid)
    {
        for(var i = 0; i < poi_points_list_listModel.count; i++)
        {
            if(poi_points_list_listModel.get(i) === pointid)
            {
                poi_points_list_ctrl.setSelectIndex(i)
                break;
            }
        }
    }

    function setContextData(contextdata)
    {        
        mpointid=""
        setValContext(poiKey_ctrl, contextdata, "poiKey")
        setValContext(poiKeyword_ctrl, contextdata, "poiKeyword")
        setValContext(poiName_ctrl, contextdata, "poiName")
        setValContext(poiNameI18n_ctrl, contextdata, "poiNameI18n")

        setValIndex(poiFrame_ctrl, contextdata, "poiFrame")

        var agenttype = modelWidget.getAgentType();
        if(agenttype === "Instagent")
        {
            poiFrame_ctrl.setSelectIndex(1)
        }
        else if(agenttype === "Scene")
        {
            poiFrame_ctrl.setSelectIndex(0)
        }
        setValContext(poiDirection_ctrl, contextdata, "poiDirection")


        if("poiPoint" in contextdata)
        {
            var wps = contextdata["poiPoint"];
            poi_points_list_listModel.clear();
            if(wps.length === 4)
            {
                console.log("xxxx"+wps)
                var curindex = poi_points_list_listModel.count
                poi_points_list_listModel.insert(curindex,{"pointid":wps[0], "lng":wps[1], "lat":wps[2], "alt":wps[3]})
                poi_points_list_ctrl.setSelectIndex(curindex)
            }
        }
    }

    function saveContext()
    {
        var exportcontextdata={}

        exportcontextdata["poiKey"] = poiKey_ctrl.getContext()
        exportcontextdata["poiKeyword"] = poiKeyword_ctrl.getContext()
        exportcontextdata["poiName"] = poiName_ctrl.getContext()
        exportcontextdata["poiNameI18n"] = poiNameI18n_ctrl.getContext()
        exportcontextdata["poiFrame"] = poiFrame_ctrl.getSelectIndex()
        exportcontextdata["poiDirection"] = poiDirection_ctrl.getContext()

        var waypts=[]
        for(var i = 0; i < poi_points_list_listModel.count; i++)
        {
            waypts.push(poi_points_list_listModel.get(i).lng)
            waypts.push(poi_points_list_listModel.get(i).lat)
            waypts.push(poi_points_list_listModel.get(i).alt)
        }
        exportcontextdata["poiPoint"] = waypts;

        modelWidget.updateContext(dataType_Id, edit_id, exportcontextdata)
    }
    Text {
        id:agent_edit_config
        width:parent.width
        height: ctrl_height
        text: qsTr("POIs Settings")
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
        id:poiKey_ctrl
        anchors.top:agent_edit_config.bottom
        anchors.topMargin:ctrl_height/4
        width: parent.width
        height: ctrl_height
    }

    Wgt.TextFiledcontrol
    {
        id:poiKeyword_ctrl
        anchors.top:poiKey_ctrl.bottom
        anchors.topMargin:ctrl_height/4
        width: parent.width
        height: ctrl_height
    }

    Wgt.TextFiledcontrol
    {
        id:poiName_ctrl
        anchors.top:poiKeyword_ctrl.bottom
        anchors.topMargin:ctrl_height/4
        width: parent.width
        height: ctrl_height
    }

    Wgt.TextFiledcontrol
    {
        id:poiNameI18n_ctrl
        anchors.top:poiName_ctrl.bottom
        anchors.topMargin:ctrl_height/4
        width: parent.width
        height: ctrl_height
    }

    Wgt.Comboxcontrol
    {
        id:poiFrame_ctrl
        anchors.top:poiNameI18n_ctrl.bottom
        anchors.topMargin:ctrl_height/4
        width: parent.width
        height: ctrl_height
    }

    Wgt.TextFiledcontrol
    {
        id:poiDirection_ctrl
        anchors.top:poiFrame_ctrl.bottom
        anchors.topMargin:ctrl_height/4
        width: parent.width
        height: ctrl_height
    }

    Label {
        id:poi_points_list_ctrl_title
        text: qsTr("POI Points:");
        font.pixelSize: context_font_size;
        font.family: "Microsoft YaHei"
        width:120
        height: ctrl_height
        color:"#ffffff"
        anchors.top:poiDirection_ctrl.bottom
        anchors.topMargin:ctrl_height/4+5
        anchors.left: parent.left
        anchors.leftMargin: 10
        anchors.verticalCenter: parent.verticalCenter;
        horizontalAlignment: Text.AlignRight;
    }

    Wgt.ListviewControl
    {
        id:poi_points_list_ctrl
        anchors.top: poi_points_list_ctrl_title.top
        anchors.left: poi_points_list_ctrl_title.right
        anchors.leftMargin: 10
        width:(parent.width-poi_points_list_ctrl_title.width - 32)
        height: ctrl_height*5
    }

    Wgt.TextFiledcontrol
    {
        id:agent_path_ctrl
        anchors.top:poi_points_list_ctrl.bottom
        anchors.topMargin:ctrl_height/4
        width: parent.width - parent.width/8 - ctrl_height
        height: ctrl_height
    }

    Wgt.Buttoncontrol
    {
        id:agent_file_btn_ctrl
        anchors.top:poi_points_list_ctrl.bottom
        anchors.topMargin:ctrl_height/2
        anchors.left: agent_path_ctrl.right
        anchors.leftMargin: 10
        width: parent.width/8
        height: ctrl_height
    }
    Connections {
        target: agent_file_btn_ctrl.getBtnObj()
        function onClicked() {
            fileDialog.open();
        }
    }

    Wgt.Buttoncontrol
    {
        id:action_cancel_ctrl
        anchors.top:agent_path_ctrl.bottom
        anchors.topMargin:ctrl_height/2
        anchors.left: parent.left
        anchors.leftMargin: (parent.width*3/8)-(parent.width/8/8)
        width: parent.width/8
        height: ctrl_height
    }

    Wgt.Buttoncontrol
    {
        id:action_save_ctrl
        anchors.top:agent_path_ctrl.bottom
        anchors.topMargin:ctrl_height/2
        anchors.right: parent.right
        anchors.rightMargin: (parent.width*3/8)-(parent.width/8/8)
        width: parent.width/8
        height: ctrl_height
    }



    FileDialog {
        id: fileDialog
        title: "Choose a File"
        folder: shortcuts.home // 设置初始路径为用户的主目录
        nameFilters: ["Json Files (*.json)","GeoJson Files (*.geojson)","All Files (*)"] // 文件类型过滤器
        modality:Qt.ApplicationModal  // 设置对话框为应用程序级模态
        onAccepted: {

            for (var i = 0; i < fileDialog.fileUrls.length;i++)
            {
                var modelslistitem = fileDialog.fileUrls[i]
                var filepathlist = modelslistitem.split("///")
                if (filepathlist.length === 2)
                {
                    var modelfilepath = filepathlist[1];
                    agent_path_ctrl.setContext(modelfilepath)


                    modelWidget.addPoiGeoJson(dataType_Id, edit_id, modelfilepath)
                }
            }
        }
        onRejected: {
        }
    }


    ListModel {
        id: poi_points_list_listModel
    }


    Component {
        id: poi_points_list_delegate_list
        Rectangle {
            id: poi_points_list_rct
            height: ctrl_height*5
            width: ListView.view.width

            color:ListView.isCurrentItem?selectcolor:(index % 2 == 0?"#5f5f5f":"#3f3f3f") //选中颜色设置

            Rectangle {
                anchors.fill: parent
                color: "transparent"
                Wgt.TextFiledcontrol
                {
                    id:poi_points_lng_ctrl
                    anchors.top:parent.top
                    anchors.topMargin:0
                    width: parent.width
                    height: ctrl_height
                }


                Wgt.TextFiledcontrol
                {
                    id:poi_points_lat_ctrl
                    anchors.top:poi_points_lng_ctrl.bottom
                    anchors.topMargin:ctrl_height/4
                    width: parent.width
                    height: ctrl_height
                }

                Wgt.TextFiledcontrol
                {
                    id:poi_points_alt_ctrl
                    anchors.top:poi_points_lat_ctrl.bottom
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
                        poi_points_list_ctrl.setSelectIndex(index)
                        modelWidget.selectPoiPoint(dataType_Id,edit_id, pointid)
                    }
                }


                Connections {
                    target: poi_points_lng_ctrl.getObj()
                    function onEditingFinished(){
                        modelWidget.updatePoiPos(dataType_Id, edit_id, poi_points_lng_ctrl.getContext(), poi_points_lat_ctrl.getContext(), poi_points_alt_ctrl.getContext())
                    }
                }


                Connections {
                    target: poi_points_lat_ctrl.getObj()
                    function onEditingFinished(){
                        modelWidget.updatePoiPos(dataType_Id, edit_id, poi_points_lng_ctrl.getContext(), poi_points_lat_ctrl.getContext(), poi_points_alt_ctrl.getContext())
                    }
                }


                Connections {
                    target: poi_points_alt_ctrl.getObj()
                    function onEditingFinished(){
                        modelWidget.updatePoiPos(dataType_Id, edit_id, poi_points_lng_ctrl.getContext(), poi_points_lat_ctrl.getContext(), poi_points_alt_ctrl.getContext())
                    }
                }
            }
            Component.onCompleted:
            {

                var agenttype = modelWidget.getAgentType();
                if(agenttype === "Instagent")
                {
                    poi_points_lng_ctrl.setTitle(qsTr("X:"))
                    poi_points_lat_ctrl.setTitle(qsTr("Y:"))
                    poi_points_alt_ctrl.setTitle(qsTr("Z:"))
                }
                else if(agenttype === "Scene")
                {
                    poi_points_lng_ctrl.setTitle(qsTr("Lng:"))
                    poi_points_lat_ctrl.setTitle(qsTr("Lat:"))
                    poi_points_alt_ctrl.setTitle(qsTr("Alt:"))
                }


                poi_points_lng_ctrl.title_font_size=context_font_size
                poi_points_lat_ctrl.title_font_size=context_font_size
                poi_points_alt_ctrl.title_font_size=context_font_size

                poi_points_lng_ctrl.setContext(lng)
                poi_points_lat_ctrl.setContext(lat)
                poi_points_alt_ctrl.setContext(alt)
            }
        }
    }

    Component.onCompleted: {

        poiKey_ctrl.setTitle(qsTr("POI Key:"))
        poiKeyword_ctrl.setTitle(qsTr("POI Keyword:"))
        poiFrame_ctrl.setTitle(qsTr("POI Frame:"))

        poiDirection_ctrl.setTitle(qsTr("POI Direction:"))

        poiName_ctrl.setTitle(qsTr("POI Name:"))
        poiNameI18n_ctrl.setTitle(qsTr("POI NameI18n:"))


        agent_path_ctrl.setTitle(qsTr("POI GEOJSON"))
        agent_file_btn_ctrl.setTitle(qsTr("Select"))

        action_cancel_ctrl.setTitle(qsTr("Cancel"))
        action_save_ctrl.setTitle(qsTr("Save"))


        poiKey_ctrl.title_font_size=context_font_size
        poiKeyword_ctrl.title_font_size=context_font_size
        poiFrame_ctrl.title_font_size=context_font_size
        poiDirection_ctrl.title_font_size=context_font_size

        poiName_ctrl.title_font_size=context_font_size
        poiNameI18n_ctrl.title_font_size=context_font_size

        agent_path_ctrl.title_font_size=context_font_size
        agent_file_btn_ctrl.title_font_size=context_font_size

        action_cancel_ctrl.title_font_size=context_font_size
        action_save_ctrl.title_font_size=context_font_size


        var treeModel6 = [{"index":0,"modelData":qsTr("absolute")},
                          {"index":1,"modelData":qsTr("relative")}]
        poiFrame_ctrl.updateData(treeModel6)


        poi_points_list_ctrl.setContext(poi_points_list_listModel,poi_points_list_delegate_list, null)
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

    Connections {
        target: poiKeyword_ctrl.getObj()
        function onTextChanged(){
            modelWidget.updatePoiName(dataType_Id, edit_id, poiKeyword_ctrl.getContext())
        }
    }
}
