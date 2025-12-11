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
        setValContext(keyword_ctrl, contextdata, "varKeyword")
        setValContext(name_ctrl, contextdata, "varName")
        setValContext(name_i18n_ctrl, contextdata, "varNameI18n")
        setValContext(type_ctrl, contextdata, "varType")
        setValContext(code_ctrl, contextdata, "stdCode")
        setValContext(schema_ctrl, contextdata, "varSchema")
        setValContext(default_val_ctrl, contextdata, "varDefault")
    }

    function saveContext()
    {
        var exportcontextdata={}

        exportcontextdata["varKeyword"] = keyword_ctrl.getContext()
        exportcontextdata["varName"] = name_ctrl.getContext()
        exportcontextdata["varNameI18n"] = name_i18n_ctrl.getContext()
        exportcontextdata["varType"] = type_ctrl.getContext()
        exportcontextdata["stdCode"] = code_ctrl.getContext()
        exportcontextdata["varSchema"] = schema_ctrl.getContext()
        exportcontextdata["varDefault"] = default_val_ctrl.getContext()

        modelWidget.updateContext(dataType_Id, edit_id, exportcontextdata)
    }

    Text {
        id:agent_edit_config
        width:parent.width
        height: ctrl_height
        text: qsTr("Variables Settings")
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
        id:keyword_ctrl
        anchors.top:agent_edit_config.bottom
        anchors.topMargin:ctrl_height/4
        width: parent.width
        height: ctrl_height
    }

    Wgt.TextFiledcontrol
    {
        id:name_ctrl
        anchors.top:keyword_ctrl.bottom
        anchors.topMargin:ctrl_height/4
        width: parent.width
        height: ctrl_height
    }


    Wgt.TextFiledcontrol
    {
        id:name_i18n_ctrl
        anchors.top:name_ctrl.bottom
        anchors.topMargin:ctrl_height/4
        width: parent.width
        height: ctrl_height
    }


    Wgt.TextFiledcontrol
    {
        id:type_ctrl
        anchors.top:name_i18n_ctrl.bottom
        anchors.topMargin:ctrl_height/4
        width: parent.width
        height: ctrl_height
    }

    Wgt.TextFiledcontrol
    {
        id:code_ctrl
        anchors.top:type_ctrl.bottom
        anchors.topMargin:ctrl_height/4
        width: parent.width
        height: ctrl_height
    }

    Wgt.TextAreacontrol
    {
        id:schema_ctrl
        anchors.top:code_ctrl.bottom
        anchors.topMargin:ctrl_height/4
        width: parent.width
        height: ctrl_height*5
    }

    Wgt.TextFiledcontrol
    {
        id:default_val_ctrl
        anchors.top:schema_ctrl.bottom
        anchors.topMargin:ctrl_height/4
        width: parent.width
        height: ctrl_height
    }

    Wgt.Buttoncontrol
    {
        id:action_cancel_ctrl
        anchors.top:default_val_ctrl.bottom
        anchors.topMargin:ctrl_height/2
        anchors.left: parent.left
        anchors.leftMargin: (parent.width*3/8)-(parent.width/8/8)
        width: parent.width/8
        height: ctrl_height
    }

    Wgt.Buttoncontrol
    {
        id:action_save_ctrl
        anchors.top:default_val_ctrl.bottom
        anchors.topMargin:ctrl_height/2
        anchors.right: parent.right
        anchors.rightMargin: (parent.width*3/8)-(parent.width/8/8)
        width: parent.width/8
        height: ctrl_height
    }

    Component.onCompleted: {

        keyword_ctrl.setTitle(qsTr("VarKeyWord:"))
        name_ctrl.setTitle(qsTr("ParameterName:"))
        name_i18n_ctrl.setTitle(qsTr("ParameterNameI18n:"))
        type_ctrl.setTitle(qsTr("ParameterType:"))
        code_ctrl.setTitle(qsTr("ParameterCode:"))
        schema_ctrl.setTitle(qsTr("Schema:"))
        default_val_ctrl.setTitle(qsTr("DefaultParameter:"))
        action_cancel_ctrl.setTitle(qsTr("Cancel"))
        action_save_ctrl.setTitle(qsTr("Save"))


        keyword_ctrl.title_font_size=context_font_size
        name_ctrl.title_font_size=context_font_size
        name_i18n_ctrl.title_font_size=context_font_size
        type_ctrl.title_font_size=context_font_size
        code_ctrl.title_font_size=context_font_size
        schema_ctrl.title_font_size=context_font_size
        default_val_ctrl.title_font_size=context_font_size
        action_cancel_ctrl.title_font_size=context_font_size
        action_save_ctrl.title_font_size=context_font_size

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
