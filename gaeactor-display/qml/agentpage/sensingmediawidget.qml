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

    function setValIndexContext(_ctrl,contextdata,_key)
    {
        if( _key in contextdata)
        {
            _ctrl.setSelectVal(contextdata[_key])
        }
    }

    property double m_dpch:0.0
    property double m_dazm:0.0
    function setContextData(contextdata)
    {

        setValIndexContext(sensing_workmode_ctrl, contextdata, "mode")
        setValContext(sensing_key_ctrl, contextdata, "smdkey")
        setValContext(sensing_roll_ctrl, contextdata, "roll")
        setValContext(sensing_pitch_ctrl, contextdata, "pitch")
        setValContext(sensing_dpch_ctrl, contextdata, "dpch")
        setValContext(sensing_azimuth_ctrl, contextdata, "azimuth")
        setValContext(sensing_dazm_ctrl, contextdata, "dazm")
        setValContext(sensing_radius_ctrl, contextdata, "radius")
        setValContext(sensing_epsln_ctrl, contextdata, "epsln")

        setValIndexContext(sensing_elec_ctrl, contextdata, "emgwpty")
        setValIndexContext(sensing_acous_ctrl, contextdata, "sndwpty")

        setValContext(sensing_freqmean_ctrl, contextdata, "frqmean")
        setValContext(sensing_freqdevi_ctrl, contextdata, "frqvarn")
        setValIndexContext(sensing_freqdistri_ctrl, contextdata, "freqdis")

        setValIndexContext(sensing_flulevel_ctrl, contextdata, "wavescale")
        setValIndexContext(sensing_fluusage_ctrl, contextdata, "frqusage")

        setValIndexContext(sensing_flucharact_ctrl, contextdata, "wavesndrcv")

        setValContext(sensing_silentinterval_ctrl, contextdata, "silencegap")
        setValContext(sensing_sensingid_ctrl, contextdata, "fldid")
        setValContext(sensing_debugid_ctrl, contextdata, "modsig")
        setValContext(sensing_stoptime_ctrl, contextdata, "stopSensingIn")
        setValContext(sensing_copynum_ctrl, contextdata, "multiplier")

//        datacontext.insert("mode", 0);
//        datacontext.insert("key", QString::number(FunctionAssistant::generate_random_positive_uint64()));
//        datacontext.insert("pitch", 0.0);
//        datacontext.insert("azimuth", 0.0);
//        datacontext.insert("dpch", 0.0);
//        datacontext.insert("dazm", 0.0);
//        datacontext.insert("epsln", 1e-7);
//        datacontext.insert("emgwpty", 0.0);
//        datacontext.insert("sndwpty", 0.0);
//        datacontext.insert("frqmean", 0);
//        datacontext.insert("frqvarn", 0);
//        datacontext.insert("freqdis", 0);
//        datacontext.insert("wavescale", 0);
//        datacontext.insert("frqusage", 0);
//        datacontext.insert("wavesndrcv", 0);
//        datacontext.insert("silencegap", 0);
//        datacontext.insert("fldid", QString::number(id));
//        datacontext.insert("modsig", QString::number(FunctionAssistant::generate_random_positive_uint64()));
//        datacontext.insert("stopSensingIn", 0);
//        datacontext.insert("multiplier", 0);
//        datacontext.insert("radius", 25);

    }

    function saveContext()
    {
        var exportcontextdata={}

        exportcontextdata["mode"] = sensing_workmode_ctrl.getSelectVal()
        exportcontextdata["smdkey"] = sensing_key_ctrl.getContext()
        exportcontextdata["roll"] = sensing_roll_ctrl.getDoubleContext()
        exportcontextdata["pitch"] = sensing_pitch_ctrl.getDoubleContext()
        exportcontextdata["azimuth"] = sensing_azimuth_ctrl.getDoubleContext()
        exportcontextdata["dpch"] = sensing_dpch_ctrl.getDoubleContext()
        exportcontextdata["dazm"] = sensing_dazm_ctrl.getDoubleContext()
        exportcontextdata["radius"] = sensing_radius_ctrl.getDoubleContext()
        exportcontextdata["epsln"] = sensing_epsln_ctrl.getDoubleContext()
        exportcontextdata["emgwpty"] = sensing_elec_ctrl.getSelectVal()
        exportcontextdata["sndwpty"] = sensing_acous_ctrl.getSelectVal()
        exportcontextdata["frqmean"] = sensing_freqmean_ctrl.getIntgerContext()
        exportcontextdata["frqvarn"] = sensing_freqdevi_ctrl.getIntgerContext()
        exportcontextdata["freqdis"] = sensing_freqdistri_ctrl.getSelectVal()
        exportcontextdata["wavescale"] = sensing_flulevel_ctrl.getSelectVal()
        exportcontextdata["frqusage"] = sensing_fluusage_ctrl.getSelectVal()
        exportcontextdata["wavesndrcv"] = sensing_flucharact_ctrl.getSelectVal()
        exportcontextdata["silencegap"] = sensing_silentinterval_ctrl.getIntgerContext()
        exportcontextdata["fldid"] = sensing_sensingid_ctrl.getContext()
        exportcontextdata["modsig"] = sensing_debugid_ctrl.getContext()
        exportcontextdata["stopSensingIn"] = sensing_stoptime_ctrl.getIntgerContext()
        exportcontextdata["multiplier"] = sensing_copynum_ctrl.getIntgerContext()


        modelWidget.updateContext(dataType_Id, edit_id, exportcontextdata)
    }

    Text {
        id:agent_edit_config
        width:parent.width
        height: ctrl_height
        text: qsTr("Sensing Media Settings")
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

    Wgt.Comboxcontrol
    {
        id:sensing_workmode_ctrl
        anchors.top:agent_edit_config.bottom
        anchors.topMargin:ctrl_height/4
        width: parent.width
        height: ctrl_height
    }

    Wgt.TextFiledcontrol
    {
        id:sensing_key_ctrl
        anchors.top:sensing_workmode_ctrl.bottom
        anchors.topMargin:ctrl_height/4
        width: parent.width
        height: ctrl_height
    }

    Wgt.TextFiledcontrol
    {
        id:sensing_roll_ctrl
        anchors.top:sensing_key_ctrl.bottom
        anchors.topMargin:ctrl_height/4
        width: parent.width
        height: ctrl_height
    }


    Wgt.TextFiledcontrol
    {
        id:sensing_pitch_ctrl
        anchors.top:sensing_roll_ctrl.bottom
        anchors.topMargin:ctrl_height/4
        width: parent.width
        height: ctrl_height
    }

    Wgt.TextFiledcontrol
    {
        id:sensing_dpch_ctrl
        anchors.top:sensing_pitch_ctrl.bottom
        anchors.topMargin:ctrl_height/4
        width: parent.width
        height: ctrl_height
    }

    Wgt.TextFiledcontrol
    {
        id:sensing_azimuth_ctrl
        anchors.top:sensing_dpch_ctrl.bottom
        anchors.topMargin:ctrl_height/4
        width: parent.width
        height: ctrl_height
    }

    Wgt.TextFiledcontrol
    {
        id:sensing_dazm_ctrl
        anchors.top:sensing_azimuth_ctrl.bottom
        anchors.topMargin:ctrl_height/4
        width: parent.width
        height: ctrl_height
    }

    Wgt.TextFiledcontrol
    {
        id:sensing_radius_ctrl
        anchors.top:sensing_dazm_ctrl.bottom
        anchors.topMargin:ctrl_height/4
        width: parent.width
        height: ctrl_height
    }


    Wgt.TextFiledcontrol
    {
        id:sensing_epsln_ctrl
        anchors.top:sensing_radius_ctrl.bottom
        anchors.topMargin:ctrl_height/4
        width: parent.width
        height: ctrl_height
    }



    Wgt.Comboxcontrol
    {
        id:sensing_elec_ctrl
        anchors.top:sensing_epsln_ctrl.bottom
        anchors.topMargin:ctrl_height/4
        width: parent.width
        height: ctrl_height
    }


    Wgt.Comboxcontrol
    {
        id:sensing_acous_ctrl
        anchors.top:sensing_elec_ctrl.bottom
        anchors.topMargin:ctrl_height/4
        width: parent.width
        height: ctrl_height
    }


    Wgt.TextFiledcontrol
    {
        id:sensing_freqmean_ctrl
        anchors.top:sensing_acous_ctrl.bottom
        anchors.topMargin:ctrl_height/4
        width: parent.width
        height: ctrl_height
    }


    Wgt.TextFiledcontrol
    {
        id:sensing_freqdevi_ctrl
        anchors.top:sensing_freqmean_ctrl.bottom
        anchors.topMargin:ctrl_height/4
        width: parent.width
        height: ctrl_height
    }


    Wgt.Comboxcontrol
    {
        id:sensing_freqdistri_ctrl
        anchors.top:sensing_freqdevi_ctrl.bottom
        anchors.topMargin:ctrl_height/4
        width: parent.width
        height: ctrl_height
    }


    Wgt.Comboxcontrol
    {
        id:sensing_flulevel_ctrl
        anchors.top:sensing_freqdistri_ctrl.bottom
        anchors.topMargin:ctrl_height/4
        width: parent.width
        height: ctrl_height
    }

    Wgt.Comboxcontrol
    {
        id:sensing_fluusage_ctrl
        anchors.top:sensing_flulevel_ctrl.bottom
        anchors.topMargin:ctrl_height/4
        width: parent.width
        height: ctrl_height
    }

    Wgt.Comboxcontrol
    {
        id:sensing_flucharact_ctrl
        anchors.top:sensing_fluusage_ctrl.bottom
        anchors.topMargin:ctrl_height/4
        width: parent.width
        height: ctrl_height
    }

    Wgt.TextFiledcontrol
    {
        id:sensing_silentinterval_ctrl
        anchors.top:sensing_flucharact_ctrl.bottom
        anchors.topMargin:ctrl_height/4
        width: parent.width
        height: ctrl_height
    }

    Wgt.TextFiledcontrol
    {
        id:sensing_sensingid_ctrl
        anchors.top:sensing_silentinterval_ctrl.bottom
        anchors.topMargin:ctrl_height/4
        width: parent.width
        height: ctrl_height
    }

    Wgt.TextFiledcontrol
    {
        id:sensing_debugid_ctrl
        anchors.top:sensing_sensingid_ctrl.bottom
        anchors.topMargin:ctrl_height/4
        width: parent.width
        height: ctrl_height
    }

    Wgt.TextFiledcontrol
    {
        id:sensing_stoptime_ctrl
        anchors.top:sensing_debugid_ctrl.bottom
        anchors.topMargin:ctrl_height/4
        width: parent.width
        height: ctrl_height
    }

    Wgt.TextFiledcontrol
    {
        id:sensing_copynum_ctrl
        anchors.top:sensing_stoptime_ctrl.bottom
        anchors.topMargin:ctrl_height/4
        width: parent.width
        height: ctrl_height
    }

    Wgt.Buttoncontrol
    {
        id:sensing_cancel_ctrl
        anchors.top:sensing_copynum_ctrl.bottom
        anchors.topMargin:ctrl_height/2
        anchors.left: parent.left
        anchors.leftMargin: (parent.width*3/8)-(parent.width/8/8)
        width: parent.width/8
        height: ctrl_height
    }

    Wgt.Buttoncontrol
    {
        id:sensing_save_ctrl
        anchors.top:sensing_copynum_ctrl.bottom
        anchors.topMargin:ctrl_height/2
        anchors.right: parent.right
        anchors.rightMargin: (parent.width*3/8)-(parent.width/8/8)
        width: parent.width/8
        height: ctrl_height
    }

    Component.onCompleted: {

        sensing_workmode_ctrl.setTitle(qsTr("WorkMode:"))
        sensing_key_ctrl.setTitle(qsTr("Key:"))
        sensing_roll_ctrl.setTitle(qsTr("Roll:"))
        sensing_pitch_ctrl.setTitle(qsTr("Pitch:"))
        sensing_dpch_ctrl.setTitle(qsTr("Diff. Pitch:"))
        sensing_azimuth_ctrl.setTitle(qsTr("Azimuth:"))
        sensing_dazm_ctrl.setTitle(qsTr("Diff. Azimuth:"))
        sensing_radius_ctrl.setTitle(qsTr("Radius(m):"))
        sensing_epsln_ctrl.setTitle(qsTr("Epsln:"))
        sensing_elec_ctrl.setTitle(qsTr("ElecProperty:"))
        sensing_acous_ctrl.setTitle(qsTr("AcousticProperty:"))
        sensing_freqmean_ctrl.setTitle(qsTr("FreqMean:"))
        sensing_freqdevi_ctrl.setTitle(qsTr("FreqDevi:"))
        sensing_freqdistri_ctrl.setTitle(qsTr("FreqDistribution:"))
        sensing_flulevel_ctrl.setTitle(qsTr("FluctuateLevel:"))
        sensing_fluusage_ctrl.setTitle(qsTr("FluctuateUsage:"))
        sensing_flucharact_ctrl.setTitle(qsTr("FluctuateRecvDisCharact:"))
        sensing_silentinterval_ctrl.setTitle(qsTr("SilentInterval:"))
        sensing_sensingid_ctrl.setTitle(qsTr("SensingMediaId:"))
        sensing_debugid_ctrl.setTitle(qsTr("DebugId:"))
        sensing_stoptime_ctrl.setTitle(qsTr("StopTime:"))
        sensing_copynum_ctrl.setTitle(qsTr("CopyNum:"))
        sensing_cancel_ctrl.setTitle(qsTr("Cancel"))
        sensing_save_ctrl.setTitle(qsTr("Save"))


        sensing_workmode_ctrl.title_font_size=context_font_size
        sensing_key_ctrl.title_font_size=context_font_size
        sensing_roll_ctrl.title_font_size=context_font_size
        sensing_pitch_ctrl.title_font_size=context_font_size
        sensing_dpch_ctrl.title_font_size=context_font_size
        sensing_azimuth_ctrl.title_font_size=context_font_size
        sensing_dazm_ctrl.title_font_size=context_font_size
        sensing_radius_ctrl.title_font_size=context_font_size
        sensing_epsln_ctrl.title_font_size=context_font_size
        sensing_elec_ctrl.title_font_size=context_font_size
        sensing_acous_ctrl.title_font_size=context_font_size
        sensing_freqmean_ctrl.title_font_size=context_font_size
        sensing_freqdevi_ctrl.title_font_size=context_font_size
        sensing_freqdistri_ctrl.title_font_size=context_font_size
        sensing_flulevel_ctrl.title_font_size=context_font_size
        sensing_fluusage_ctrl.title_font_size=context_font_size
        sensing_flucharact_ctrl.title_font_size=context_font_size
        sensing_silentinterval_ctrl.title_font_size=context_font_size
        sensing_sensingid_ctrl.title_font_size=context_font_size
        sensing_debugid_ctrl.title_font_size=context_font_size
        sensing_stoptime_ctrl.title_font_size=context_font_size
        sensing_copynum_ctrl.title_font_size=context_font_size

        sensing_cancel_ctrl.title_font_size=context_font_size
        sensing_save_ctrl.title_font_size=context_font_size

        var treeModel = [{"index":0,"enumid":-999,"modelData":qsTr("Sleep")},
                         {"index":1,"enumid":0,"modelData":qsTr("Search")},
                         {"index":2,"enumid":10,"modelData":qsTr("Coarse Track")},
                         {"index":3,"enumid":11,"modelData":qsTr("Fine Track")},
                         {"index":4,"enumid":20,"modelData":qsTr("Lose Track")},
                         {"index":5,"enumid":999,"modelData":qsTr("Reflection")}]
        sensing_workmode_ctrl.updateData(treeModel)


        var treeModel1 = [{"index":0,"enumid":0x00,"modelData":qsTr("Non Electromagnetic Wave")},
                          {"index":1,"enumid":0x10,"modelData":qsTr("γ-Ray")},
                          {"index":2,"enumid":0x20,"modelData":qsTr("X-Ray")},
                          {"index":3,"enumid":0x30,"modelData":qsTr("Ultraviolet Rays")},
                          {"index":4,"enumid":0x40,"modelData":qsTr("Visible Light")},
                          {"index":5,"enumid":0x60,"modelData":qsTr("Infrared Ray")},
                          {"index":6,"enumid":0x70,"modelData":qsTr("Microwave")},
                          {"index":7,"enumid":0x80,"modelData":qsTr("Industry Radio Wave")}]
        sensing_elec_ctrl.updateData(treeModel1)


        var treeModel2 = [{"index":0,"enumid":0x00,"modelData":qsTr("Non Mechanical Wave")},
                          {"index":1,"enumid":0x10,"modelData":qsTr("Infrasound Wave")},
                          {"index":2,"enumid":0x20,"modelData":qsTr("Audible Sound Wave")},
                          {"index":3,"enumid":0x30,"modelData":qsTr("Ultrasonic Wave")}]
        sensing_acous_ctrl.updateData(treeModel2)


        var treeModel3 = [{"index":0,"enumid":0x00,"modelData":qsTr("Uniform Distribution")},
                          {"index":1,"enumid":0x10,"modelData":qsTr("Normal Distribution")}]
        sensing_freqdistri_ctrl.updateData(treeModel3)


        var treeModel4 = [{"index":0,"enumid":0x00,"modelData":qsTr("Hz")},
                          {"index":1,"enumid":0x01,"modelData":qsTr("KHz")},
                          {"index":2,"enumid":0x02,"modelData":qsTr("MHz")},
                          {"index":3,"enumid":0x03,"modelData":qsTr("GHz")},
                          {"index":4,"enumid":0x04,"modelData":qsTr("THz")},
                          {"index":5,"enumid":0x05,"modelData":qsTr("PHz")}]
        sensing_flulevel_ctrl.updateData(treeModel4)


        var treeModel5 = [{"index":0, "enumid":0x00, "modelData":qsTr("Undefined")},
                          {"index":1, "enumid":0x01, "modelData":qsTr("Echo Wave")},
                          {"index":2, "enumid":0x02, "modelData":qsTr("Disturb")},
                          {"index":3, "enumid":0x03, "modelData":qsTr("Pressing")},
                          {"index":4, "enumid":0x04, "modelData":qsTr("Laser Ablation")},
                          {"index":5, "enumid":0x05, "modelData":qsTr("Vibrate")},
                          {"index":6, "enumid":0x06, "modelData":qsTr("Microwave Ablation")},
                          {"index":7, "enumid":0x07, "modelData":qsTr("Shock Wave")},
                          {"index":8, "enumid":0x08, "modelData":qsTr("Detection")},
                          {"index":9, "enumid":0x09, "modelData":qsTr("Explosion")},
                          {"index":10, "enumid":0xfd, "modelData":qsTr("SpaceVLP")},
                          {"index":11, "enumid":0xfe, "modelData":qsTr("Comm")},
                          {"index":12, "enumid":0xff, "modelData":qsTr("Infrared")}]
        sensing_fluusage_ctrl.updateData(treeModel5)


        var treeModel6 = [{"index":0, "enumid":0x00,"modelData":qsTr("Undefined")},
                          {"index":1, "enumid":0x01,"modelData":qsTr("Send Recv")},
                          {"index":2, "enumid":0x02,"modelData":qsTr("Send Only")},
                          {"index":3, "enumid":0x03,"modelData":qsTr("Recv Only")},
                          {"index":4, "enumid":0xfe,"modelData":qsTr("Second Echo Wave")},
                          {"index":5, "enumid":0xff,"modelData":qsTr("First Echo Wave")}]
        sensing_flucharact_ctrl.updateData(treeModel6)

        sensing_key_ctrl.setEnable(false)
        sensing_sensingid_ctrl.setEnable(false)
        sensing_debugid_ctrl.setEnable(false)
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
        target: sensing_radius_ctrl.getObj()
        function onEditingFinished(){

            console.log("radius " + sensing_radius_ctrl.getContext())

            modelWidget.updateSensingRadius(dataType_Id, edit_id, sensing_radius_ctrl.getContext())
        }
    }

    Connections {
        target: sensing_pitch_ctrl.getObj()
        function onEditingFinished(){
            modelWidget.updateSensingAngle(dataType_Id, edit_id,
                                           sensing_azimuth_ctrl.getContext(),
                                           sensing_dazm_ctrl.getContext(),
                                           sensing_pitch_ctrl.getContext(),
                                         sensing_dpch_ctrl.getContext())
        }
    }

    Connections {
        target: sensing_dpch_ctrl.getObj()
        function onEditingFinished(){
            modelWidget.updateSensingAngle(dataType_Id, edit_id,
                                           sensing_azimuth_ctrl.getContext(),
                                           sensing_dazm_ctrl.getContext(),
                                           sensing_pitch_ctrl.getContext(),
                                         sensing_dpch_ctrl.getContext())
        }
    }

    Connections {
        target: sensing_azimuth_ctrl.getObj()
        function onEditingFinished(){
            modelWidget.updateSensingAngle(dataType_Id, edit_id,
                                           sensing_azimuth_ctrl.getContext(),
                                           sensing_dazm_ctrl.getContext(),
                                           sensing_pitch_ctrl.getContext(),
                                         sensing_dpch_ctrl.getContext())
        }
    }

    Connections {
        target: sensing_dazm_ctrl.getObj()
        function onEditingFinished(){
            modelWidget.updateSensingAngle(dataType_Id, edit_id,
                                           sensing_azimuth_ctrl.getContext(),
                                           sensing_dazm_ctrl.getContext(),
                                           sensing_pitch_ctrl.getContext(),
                                         sensing_dpch_ctrl.getContext())
        }
    }



}
