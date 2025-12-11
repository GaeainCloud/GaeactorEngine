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
        setValContext(sensing_name_ctrl, contextdata, "fldmdName")
        setValContext(sensing_name_i18n_ctrl, contextdata, "fldmdNameI18n")
        setValContext(sensing_key_ctrl, contextdata, "fldmdkey")
        setValContext(sensing_offset_x_ctrl, contextdata, "offsetx")
        setValContext(sensing_offset_y_ctrl, contextdata, "offsety")
        setValContext(sensing_offset_z_ctrl, contextdata, "offsetz")
        setValIndex(sensing_type_ctrl, contextdata, "orient");
        setValSwitch(sensing_passive_ctrl, contextdata, "passive");


    }

    function saveContext()
    {
        var exportcontextdata={}

        exportcontextdata["fldmdName"] = sensing_name_ctrl.getContext()
        exportcontextdata["fldmdNameI18n"] = sensing_name_i18n_ctrl.getContext()
        exportcontextdata["fldmdkey"] = sensing_key_ctrl.getContext()
        exportcontextdata["offsetx"] = sensing_offset_x_ctrl.getContext()
        exportcontextdata["offsety"] = sensing_offset_y_ctrl.getContext()
        exportcontextdata["offsetz"] = sensing_offset_z_ctrl.getContext()
        exportcontextdata["orient"] = sensing_type_ctrl.getSelectIndex()
        exportcontextdata["passive"] = sensing_passive_ctrl.getSwitch()

        modelWidget.updateContext(dataType_Id, edit_id, exportcontextdata)
    }

    Text {
        id:agent_edit_config
        width:parent.width
        height: ctrl_height
        text: qsTr("Field Media Settings")
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
        id:sensing_name_ctrl
        anchors.top:agent_edit_config.bottom
        anchors.topMargin:ctrl_height/4
        width: parent.width
        height: ctrl_height
    }

    Wgt.TextFiledcontrol
    {
        id:sensing_name_i18n_ctrl
        anchors.top:sensing_name_ctrl.bottom
        anchors.topMargin:ctrl_height/4
        width: parent.width
        height: ctrl_height
    }

    Wgt.TextFiledcontrol
    {
        id:sensing_key_ctrl
        anchors.top:sensing_name_i18n_ctrl.bottom
        anchors.topMargin:ctrl_height/4
        width: parent.width
        height: ctrl_height
    }

    Wgt.TextFiledcontrol
    {
        id:sensing_offset_x_ctrl
        anchors.top:sensing_key_ctrl.bottom
        anchors.topMargin:ctrl_height/4
        width: parent.width
        height: ctrl_height
    }

    Wgt.TextFiledcontrol
    {
        id:sensing_offset_y_ctrl
        anchors.top:sensing_offset_x_ctrl.bottom
        anchors.topMargin:ctrl_height/4
        width: parent.width
        height: ctrl_height
    }

    Wgt.TextFiledcontrol
    {
        id:sensing_offset_z_ctrl
        anchors.top:sensing_offset_y_ctrl.bottom
        anchors.topMargin:ctrl_height/4
        width: parent.width
        height: ctrl_height
    }


    Wgt.Comboxcontrol
    {
        id:sensing_type_ctrl
        anchors.top:sensing_offset_z_ctrl.bottom
        anchors.topMargin:ctrl_height/4
        width: parent.width
        height: ctrl_height
    }

    Wgt.Switchcontrol
    {
        id:sensing_passive_ctrl
        anchors.top:sensing_type_ctrl.bottom
        anchors.topMargin:ctrl_height/4
        width: parent.width
        height: ctrl_height
    }


    Wgt.Buttoncontrol
    {
        id:sensing_cancel_ctrl
        anchors.top:sensing_passive_ctrl.bottom
        anchors.topMargin:ctrl_height/2
        anchors.left: parent.left
        anchors.leftMargin: (parent.width*3/8)-(parent.width/8/8)
        width: parent.width/8
        height: ctrl_height
    }

    Wgt.Buttoncontrol
    {
        id:sensing_save_ctrl
        anchors.top:sensing_passive_ctrl.bottom
        anchors.topMargin:ctrl_height/2
        anchors.right: parent.right
        anchors.rightMargin: (parent.width*3/8)-(parent.width/8/8)
        width: parent.width/8
        height: ctrl_height
    }

    Component.onCompleted: {

        sensing_name_ctrl.setTitle(qsTr("Name:"))
        sensing_name_i18n_ctrl.setTitle(qsTr("NameI18n:"))
        sensing_key_ctrl.setTitle(qsTr("Key:"))
        sensing_offset_x_ctrl.setTitle(qsTr("OffsetX:"))
        sensing_offset_y_ctrl.setTitle(qsTr("OffsetY:"))
        sensing_offset_z_ctrl.setTitle(qsTr("OffsetZ:"))
        sensing_type_ctrl.setTitle(qsTr("Type:"))
        sensing_passive_ctrl.setTitle(qsTr("Passive:"))
        sensing_cancel_ctrl.setTitle(qsTr("Cancel"))
        sensing_save_ctrl.setTitle(qsTr("Save"))


        sensing_name_ctrl.title_font_size=context_font_size
        sensing_name_i18n_ctrl.title_font_size=context_font_size
        sensing_key_ctrl.title_font_size=context_font_size
        sensing_offset_x_ctrl.title_font_size=context_font_size
        sensing_offset_y_ctrl.title_font_size=context_font_size
        sensing_offset_z_ctrl.title_font_size=context_font_size
        sensing_type_ctrl.title_font_size=context_font_size
        sensing_passive_ctrl.title_font_size=context_font_size
        sensing_cancel_ctrl.title_font_size=context_font_size
        sensing_save_ctrl.title_font_size=context_font_size

        sensing_key_ctrl.setEnable(false)

        var treeModel = [{"index":0,"modelData":qsTr("Up")},{"index":1,"modelData":qsTr("Horizontal")},{"index":2,"modelData":qsTr("Down")}]
        sensing_type_ctrl.updateData(treeModel)
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

    Connections {
        target: sensing_name_ctrl.getObj()
        function onTextChanged(){
            modelWidget.updateFieldName(dataType_Id, edit_id, sensing_name_ctrl.getContext())
        }
    }


    Connections {
        target: sensing_offset_x_ctrl.getObj()
        function onEditingFinished(){
            modelWidget.updateFieldPos(dataType_Id, edit_id, sensing_offset_x_ctrl.getContext(), sensing_offset_y_ctrl.getContext(), sensing_offset_z_ctrl.getContext())
        }
    }


    Connections {
        target: sensing_offset_y_ctrl.getObj()
        function onEditingFinished(){
            modelWidget.updateFieldPos(dataType_Id, edit_id, sensing_offset_x_ctrl.getContext(), sensing_offset_y_ctrl.getContext(), sensing_offset_z_ctrl.getContext())
        }
    }


    Connections {
        target: sensing_offset_z_ctrl.getObj()
        function onEditingFinished(){
            modelWidget.updateFieldPos(dataType_Id, edit_id, sensing_offset_x_ctrl.getContext(), sensing_offset_y_ctrl.getContext(), sensing_offset_z_ctrl.getContext())
        }
    }
}
