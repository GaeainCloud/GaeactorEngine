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
        setValSwitch(active_ctrl, contextdata, "oodaActivated")
        setValContext(script_id_ctrl ,contextdata, "scriptId")
        if("oodaKeyword" in contextdata)
        {
            action_key_ctrl.setContext(contextdata["oodaKeyword"])
        }
        setValContext(action_usage_filter_ctrl ,contextdata, "oodaUsageFilter")
        setValIndex(action_script_cmbx_ctrl, contextdata, "scriptLang")
        setValContext(action_verison_ctrl ,contextdata, "oodaVersion")
        if("oodaScript" in contextdata)
        {
            var scriptcode = contextdata["oodaScript"];
            console.log("script "+scriptcode)
            var codestr="";
            for(var i = 0; i < scriptcode.length;i++)
            {
                codestr += scriptcode[i]+"\n"
            }
            action_script_ctrl.setContext(codestr)
        }
    }

    function saveContext()
    {
        var exportcontextdata={}

        exportcontextdata["oodaActivated"] = active_ctrl.getSwitch()
        exportcontextdata["scriptId"] = script_id_ctrl.getContext()
        exportcontextdata["oodaKeyword"] = action_key_ctrl.getContext()
        exportcontextdata["oodaUsageFilter"] = action_usage_filter_ctrl.getContext()
        exportcontextdata["scriptLang"] = action_script_cmbx_ctrl.getSelectIndex()
        exportcontextdata["oodaVersion"] = action_verison_ctrl.getContext()

        var codestr = action_script_ctrl.getContext();
        var runtimgdata=[]
        var stringArray = codestr.split("\n")
        for (var i = 0; i < stringArray.length; i++)
        {
            runtimgdata.push(stringArray[i]);
        }
        exportcontextdata["oodaScript"] = runtimgdata
        modelWidget.updateContext(dataType_Id, edit_id, exportcontextdata)
    }
    Text {
        id:agent_edit_config
        width:parent.width
        height: ctrl_height
        text: qsTr("OODAs Settings")
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

    Wgt.Switchcontrol
    {
        id:active_ctrl
        anchors.top:agent_edit_config.bottom
        anchors.topMargin:ctrl_height/4
        width: parent.width
        height: ctrl_height
    }

    Wgt.TextFiledcontrol
    {
        id:script_id_ctrl
        anchors.top:active_ctrl.bottom
        anchors.topMargin:ctrl_height/4
        width: parent.width
        height: ctrl_height
    }

    Wgt.TextFiledcontrol
    {
        id:action_key_ctrl
        anchors.top:script_id_ctrl.bottom
        anchors.topMargin:ctrl_height/4
        width: parent.width
        height: ctrl_height
    }

    Wgt.TextFiledcontrol
    {
        id:action_usage_filter_ctrl
        anchors.top:action_key_ctrl.bottom
        anchors.topMargin:ctrl_height/4
        width: parent.width
        height: ctrl_height
    }

    Wgt.Comboxcontrol
    {
        id:action_script_cmbx_ctrl
        anchors.top:action_usage_filter_ctrl.bottom
        anchors.topMargin:ctrl_height/4
        width: parent.width
        height: ctrl_height
    }

    Wgt.TextFiledcontrol
    {
        id:action_verison_ctrl
        anchors.top:action_script_cmbx_ctrl.bottom
        anchors.topMargin:ctrl_height/4
        width: parent.width
        height: ctrl_height
    }

    Wgt.TextAreacontrol
    {
        id:action_script_ctrl
        anchors.top:action_verison_ctrl.bottom
        anchors.topMargin:ctrl_height/4
        width: parent.width
        height: parent.height-ctrl_height*12
    }

    Wgt.Buttoncontrol
    {
        id:action_cancel_ctrl
        anchors.top:action_script_ctrl.bottom
        anchors.topMargin:ctrl_height/2
        anchors.left: parent.left
        anchors.leftMargin: (parent.width*3/8)-(parent.width/8/8)
        width: parent.width/8
        height: ctrl_height
    }

    Wgt.Buttoncontrol
    {
        id:action_save_ctrl
        anchors.top:action_script_ctrl.bottom
        anchors.topMargin:ctrl_height/2
        anchors.right: parent.right
        anchors.rightMargin: (parent.width*3/8)-(parent.width/8/8)
        width: parent.width/8
        height: ctrl_height
    }

    Component.onCompleted: {

        active_ctrl.setTitle(qsTr("Active:"))
        script_id_ctrl.setTitle(qsTr("Id:"))
        action_key_ctrl.setTitle(qsTr("Key:"))

        action_usage_filter_ctrl.setTitle(qsTr("UsageFilter:"))
        action_script_cmbx_ctrl.setTitle(qsTr("Type:"))
        action_verison_ctrl.setTitle(qsTr("Ver:"))
        action_script_ctrl.setTitle(qsTr("Script:"))
        action_cancel_ctrl.setTitle(qsTr("Cancel"))
        action_save_ctrl.setTitle(qsTr("Save"))


        active_ctrl.title_font_size=context_font_size
        script_id_ctrl.title_font_size=context_font_size
        action_key_ctrl.title_font_size=context_font_size
        action_usage_filter_ctrl.title_font_size=context_font_size
        action_script_cmbx_ctrl.title_font_size=context_font_size
        action_verison_ctrl.title_font_size=context_font_size
        action_script_ctrl.title_font_size=context_font_size
        action_cancel_ctrl.title_font_size=context_font_size
        action_save_ctrl.title_font_size=context_font_size

        var treeModel = [{"index":0,"modelData":qsTr("GaeaScript")},{"index":1,"modelData":qsTr("Lua")},{"index":2,"modelData":qsTr("Python")}]
        action_script_cmbx_ctrl.updateData(treeModel)

        script_id_ctrl.setEnable(false)
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
        target: action_key_ctrl.getObj()
        function onTextChanged(){
            modelWidget.updateOODAName(dataType_Id, edit_id, action_key_ctrl.getContext())
        }
    }
}
