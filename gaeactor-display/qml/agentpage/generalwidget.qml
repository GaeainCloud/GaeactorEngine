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
            if(contextdata[_key] === null)
            {
                _ctrl.setContext("")
            }
            else
            {
                _ctrl.setContext(contextdata[_key].toString())
            }
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
        setValContext(agent_key_ctrl,contextdata,"agentKey")
        setValContext(agent_keyword_ctrl,contextdata,"agentKeyword")
        setValContext(agent_name_ctrl,contextdata,"agentName")
        setValContext(agent_namei18n_ctrl,contextdata,"agentNameI18n")
        agenttype_ctrl.setSelectIndex(contextdata["agentType"] === qsTr("Instagent") ? 1:2 )
        setValContext(agent_path_ctrl,contextdata,"agentPath")
        setValContext(modelUrlSlim_ctrl,contextdata,"modelUrlSlim")
        setValContext(modelUrlFat_ctrl,contextdata,"modelUrlFat")
        setValContext(modelUrlMedium_ctrl,contextdata,"modelUrlMedium")
        //setValContext(modelAbstracted_ctrl,contextdata,"modelAbstracted")
        setValSwitch(freelanceable_ctrl,contextdata,"freelanceable")
        setValJsonContext(freelanceableDynamics_ctrl,contextdata,"freelanceableDynamics")
        setValSwitch(locatable_ctrl,contextdata,"locatable")
        setValJsonContext(locatableDynamics_ctrl,contextdata,"locatableDynamics")
        setValSwitch(navigatable_ctrl,contextdata,"navigatable")
        setValJsonContext(navigatableDynamics_ctrl,contextdata,"navigatableDynamics")
        setValSwitch(missionable_ctrl,contextdata,"missionable")
        setValJsonContext(missionableDynamics_ctrl,contextdata,"missionableDynamics")
        setValSwitch(operatable_ctrl,contextdata,"operatable")
        setValSwitch(useIb_ctrl,contextdata,"useIb")
        setValContext(stopCond_ctrl,contextdata,"stopCond")
        setValJsonContext(axnViewDetails_ctrl,contextdata,"axnViewDetails")
        setValSwitch(isDelete_ctrl,contextdata,"isDelete")
        setValContext(agent_desc_ctrl,contextdata,"agentDesc")
    }

    function saveContext()
    {
        var exportcontextdata={}

        exportcontextdata["agentKey"] = agent_key_ctrl.getContext()
        exportcontextdata["agentKeyword"] = agent_keyword_ctrl.getContext()
        exportcontextdata["agentName"] = agent_name_ctrl.getContext()
        exportcontextdata["agentNameI18n"] = agent_namei18n_ctrl.getContext()
        exportcontextdata["agentType"] = agenttype_ctrl.getSelectIndex() === 1 ? qsTr("Instagent"):qsTr("Scene")
        exportcontextdata["agentPath"] = agent_path_ctrl.getContext()
        exportcontextdata["modelUrlSlim"] = modelUrlSlim_ctrl.getContext()
        exportcontextdata["modelUrlFat"] = modelUrlFat_ctrl.getContext()
        exportcontextdata["modelUrlMedium"] = modelUrlMedium_ctrl.getContext()
        exportcontextdata["modelAbstracted"] = modelAbstracted_ctrl.getContext()
        exportcontextdata["freelanceable"] = freelanceable_ctrl.getSwitch()
        exportcontextdata["freelanceableDynamics"] = freelanceableDynamics_ctrl.getContext()
        exportcontextdata["locatable"] = locatable_ctrl.getSwitch()
        exportcontextdata["locatableDynamics"] = locatableDynamics_ctrl.getContext()
        exportcontextdata["navigatable"] = navigatable_ctrl.getSwitch()
        exportcontextdata["navigatableDynamics"] = navigatableDynamics_ctrl.getContext()
        exportcontextdata["missionable"] = missionable_ctrl.getSwitch()
        exportcontextdata["missionableDynamics"] = missionableDynamics_ctrl.getContext()
        exportcontextdata["operatable"] = operatable_ctrl.getSwitch()
        exportcontextdata["useIb"] = useIb_ctrl.getSwitch()
        exportcontextdata["stopCond"] = stopCond_ctrl.getContext()
        exportcontextdata["axnViewDetails"] = axnViewDetails_ctrl.getContext()
        exportcontextdata["isDelete"] = isDelete_ctrl.getSwitch()
        exportcontextdata["agentDesc"] = agent_desc_ctrl.getContext()

        modelWidget.updateContext(dataType_Id, edit_id, exportcontextdata)
    }

    Text {
        id:agent_edit_config
        width:parent.width
        height: ctrl_height
        text: qsTr("General Settings")
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
        id:agent_key_ctrl
        anchors.top:agent_edit_config.bottom
        anchors.topMargin:ctrl_height/4
        width: parent.width
        height: ctrl_height
    }

    Wgt.TextFiledcontrol
    {
        id:agent_keyword_ctrl
        anchors.top:agent_key_ctrl.bottom
        anchors.topMargin:ctrl_height/4
        width: parent.width
        height: ctrl_height
    }


    Wgt.TextFiledcontrol
    {
        id:agent_name_ctrl
        anchors.top:agent_keyword_ctrl.bottom
        anchors.topMargin:ctrl_height/4
        width: parent.width
        height: ctrl_height
    }


    Wgt.TextFiledcontrol
    {
        id:agent_namei18n_ctrl
        anchors.top:agent_name_ctrl.bottom
        anchors.topMargin:ctrl_height/4
        width: parent.width
        height: ctrl_height
    }

    Wgt.RadioGroupcontrol
    {
        id:agenttype_ctrl
        anchors.top:agent_namei18n_ctrl.bottom
        anchors.topMargin:ctrl_height/4
        width: parent.width
        height: ctrl_height
    }

    Wgt.TextFiledcontrol
    {
        id:agent_path_ctrl
        anchors.top:agenttype_ctrl.bottom
        anchors.topMargin:ctrl_height/4
        width: parent.width - parent.width/8 - ctrl_height
        height: ctrl_height
    }

    Wgt.Buttoncontrol
    {
        id:agent_file_btn_ctrl
        anchors.top:agenttype_ctrl.bottom
        anchors.topMargin:ctrl_height/2
        anchors.left: agent_path_ctrl.right
        anchors.leftMargin: 10
        width: parent.width/8
        height: ctrl_height
    }



    Wgt.TextFiledcontrol
    {
        id:modelUrlSlim_ctrl
        anchors.top:agent_file_btn_ctrl.bottom
        anchors.topMargin:ctrl_height/4
        width: parent.width
        height: ctrl_height
    }

    Wgt.TextFiledcontrol
    {
        id:modelUrlFat_ctrl
        anchors.top:modelUrlSlim_ctrl.bottom
        anchors.topMargin:ctrl_height/4
        width: parent.width
        height: ctrl_height
    }

    Wgt.TextFiledcontrol
    {
        id:modelUrlMedium_ctrl
        anchors.top:modelUrlFat_ctrl.bottom
        anchors.topMargin:ctrl_height/4
        width: parent.width
        height: ctrl_height
    }

    Wgt.TextFiledcontrol
    {
        id:modelAbstracted_ctrl
        anchors.top:modelUrlMedium_ctrl.bottom
        anchors.topMargin:ctrl_height/4
        width: parent.width
        height: ctrl_height
    }

    Wgt.Switchcontrol
    {
        id:freelanceable_ctrl
        anchors.top:modelAbstracted_ctrl.bottom
        anchors.topMargin:ctrl_height/4
        width: parent.width
        height: ctrl_height
    }

    Wgt.TextFiledcontrol
    {
        id:freelanceableDynamics_ctrl
        anchors.top:freelanceable_ctrl.bottom
        anchors.topMargin:ctrl_height/4
        width: parent.width
        height: ctrl_height
    }


    Wgt.Switchcontrol
    {
        id:locatable_ctrl
        anchors.top:freelanceableDynamics_ctrl.bottom
        anchors.topMargin:ctrl_height/4
        width: parent.width
        height: ctrl_height
    }

    Wgt.TextFiledcontrol
    {
        id:locatableDynamics_ctrl
        anchors.top:locatable_ctrl.bottom
        anchors.topMargin:ctrl_height/4
        width: parent.width
        height: ctrl_height
    }

    Wgt.Switchcontrol
    {
        id:navigatable_ctrl
        anchors.top:locatableDynamics_ctrl.bottom
        anchors.topMargin:ctrl_height/4
        width: parent.width
        height: ctrl_height
    }

    Wgt.TextFiledcontrol
    {
        id:navigatableDynamics_ctrl
        anchors.top:navigatable_ctrl.bottom
        anchors.topMargin:ctrl_height/4
        width: parent.width
        height: ctrl_height
    }

    Wgt.Switchcontrol
    {
        id:missionable_ctrl
        anchors.top:navigatableDynamics_ctrl.bottom
        anchors.topMargin:ctrl_height/4
        width: parent.width
        height: ctrl_height
    }

    Wgt.TextFiledcontrol
    {
        id:missionableDynamics_ctrl
        anchors.top:missionable_ctrl.bottom
        anchors.topMargin:ctrl_height/4
        width: parent.width
        height: ctrl_height
    }

    Wgt.Switchcontrol
    {
        id:operatable_ctrl
        anchors.top:missionableDynamics_ctrl.bottom
        anchors.topMargin:ctrl_height/4
        width: parent.width
        height: ctrl_height
    }

    Wgt.Switchcontrol
    {
        id:useIb_ctrl
        anchors.top:operatable_ctrl.bottom
        anchors.topMargin:ctrl_height/4
        width: parent.width
        height: ctrl_height
    }

    Wgt.TextAreacontrol
    {
        id:stopCond_ctrl
        anchors.top:useIb_ctrl.bottom
        anchors.topMargin:ctrl_height/4
        width: parent.width
        height: ctrl_height*3
    }

    Wgt.TextFiledcontrol
    {
        id:axnViewDetails_ctrl
        anchors.top:stopCond_ctrl.bottom
        anchors.topMargin:ctrl_height/4
        width: parent.width
        height: ctrl_height
    }

    Wgt.Switchcontrol
    {
        id:isDelete_ctrl
        anchors.top:axnViewDetails_ctrl.bottom
        anchors.topMargin:ctrl_height/4
        width: parent.width
        height: ctrl_height
    }

    Wgt.TextAreacontrol
    {
        id:agent_desc_ctrl
        anchors.top:isDelete_ctrl.bottom
        anchors.topMargin:ctrl_height/4
        width: parent.width
        height: ctrl_height*3
    }

    Wgt.Buttoncontrol
    {
        id:action_cancel_ctrl
        anchors.top:agent_desc_ctrl.bottom
        anchors.topMargin:ctrl_height/2
        anchors.left: parent.left
        anchors.leftMargin: (parent.width*3/8)-(parent.width/8/8)
        width: parent.width/8
        height: ctrl_height
    }

    Wgt.Buttoncontrol
    {
        id:action_save_ctrl
        anchors.top:agent_desc_ctrl.bottom
        anchors.topMargin:ctrl_height/2
        anchors.right: parent.right
        anchors.rightMargin: (parent.width*3/8)-(parent.width/8/8)
        width: parent.width/8
        height: ctrl_height
    }

    Component.onCompleted: {

        agent_key_ctrl.setTitle(qsTr("Agent Key:"))
        agent_keyword_ctrl.setTitle(qsTr("Agent KeyWord:"))
        agent_name_ctrl.setTitle(qsTr("Agent Name:"))
        agent_namei18n_ctrl.setTitle(qsTr("Agent Namei18n:"))
        agenttype_ctrl.setTitle(qsTr("Agent Type:"))
        agent_path_ctrl.setTitle(qsTr("Agent Path:"))
        agent_file_btn_ctrl.setTitle(qsTr("Select"))

        modelUrlSlim_ctrl.setTitle(qsTr("modelUrlSlim:"))
        modelUrlFat_ctrl.setTitle(qsTr("modelUrlFat:"))
        modelUrlMedium_ctrl.setTitle(qsTr("modelUrlMedium:"))
        modelAbstracted_ctrl.setTitle(qsTr("modelAbstracted:"))
        freelanceable_ctrl.setTitle(qsTr("freelanceable:"))
        freelanceableDynamics_ctrl.setTitle(qsTr("freelanceableDynamics:"))
        locatable_ctrl.setTitle(qsTr("locatable:"))
        locatableDynamics_ctrl.setTitle(qsTr("locatableDynamics:"))
        navigatable_ctrl.setTitle(qsTr("navigatable:"))
        navigatableDynamics_ctrl.setTitle(qsTr("navigatableDynamics:"))
        missionable_ctrl.setTitle(qsTr("missionable:"))
        missionableDynamics_ctrl.setTitle(qsTr("missionableDynamics:"))

        operatable_ctrl.setTitle(qsTr("operatable:"))
        useIb_ctrl.setTitle(qsTr("useIb:"))
        stopCond_ctrl.setTitle(qsTr("stopCond:"))
        axnViewDetails_ctrl.setTitle(qsTr("axnViewDetails:"))
        isDelete_ctrl.setTitle(qsTr("isDelete:"))


        agent_desc_ctrl.setTitle(qsTr("Agent Desc:"))
        action_cancel_ctrl.setTitle(qsTr("Cancel"))
        action_save_ctrl.setTitle(qsTr("Save"))

        agent_key_ctrl.title_font_size=context_font_size
        agent_keyword_ctrl.title_font_size=context_font_size
        agent_name_ctrl.title_font_size=context_font_size
        agent_namei18n_ctrl.title_font_size=context_font_size
        agenttype_ctrl.setFontSize(context_font_size)
        agent_path_ctrl.title_font_size=context_font_size
        agent_file_btn_ctrl.title_font_size=context_font_size

        modelUrlSlim_ctrl.title_font_size=context_font_size
        modelUrlFat_ctrl.title_font_size=context_font_size
        modelUrlMedium_ctrl.title_font_size=context_font_size
        modelAbstracted_ctrl.title_font_size=context_font_size
        freelanceable_ctrl.title_font_size=context_font_size
        freelanceableDynamics_ctrl.title_font_size=context_font_size
        locatable_ctrl.title_font_size=context_font_size
        locatableDynamics_ctrl.title_font_size=context_font_size
        navigatable_ctrl.title_font_size=context_font_size
        navigatableDynamics_ctrl.title_font_size=context_font_size
        missionable_ctrl.title_font_size=context_font_size
        missionableDynamics_ctrl.title_font_size=context_font_size
        operatable_ctrl.title_font_size=context_font_size
        useIb_ctrl.title_font_size=context_font_size
        stopCond_ctrl.title_font_size=context_font_size
        axnViewDetails_ctrl.title_font_size=context_font_size
        isDelete_ctrl.title_font_size=context_font_size



        agent_desc_ctrl.title_font_size=context_font_size
        action_cancel_ctrl.title_font_size=context_font_size
        action_save_ctrl.title_font_size=context_font_size

        agenttype_ctrl.setEnable(false)
        agenttype_ctrl.updateData(qsTr("Instagent"),qsTr("Scene"))
    }

    Connections {
        target: action_cancel_ctrl.getBtnObj()
        function onClicked() {
            // 这里可以处理按钮点击后的逻辑
        }
    }

    Connections {
        target: action_save_ctrl.getBtnObj()
        function onClicked() {
            saveContext()
            // 这里可以处理按钮点击后的逻辑
        }
    }
}
