import QtQuick 2.15
import QtQuick.Controls 2.15
import QtQuick.Controls.Styles 1.2
import "../wgt" as Wgt
Item   {
    id:combox_ctrl_rect
    visible: true
    opacity: 1.0
    property string selectcolor:"#157e5b"

    property int title_font_size:14

    function updateData(context1,context2)
    {
        radio1_ctrl.text = context1
        radio2_ctrl.text = context2
    }

    function setTitle(title)
    {
        _title.text = title
    }

    function setEnable(bEnable)
    {
        radio1_ctrl.enabled = bEnable
        radio2_ctrl.enabled = bEnable
    }

    function setSelectIndex(selectindex)
    {
        if(selectindex === 1)
        {
            radio2_ctrl.checked = false
            radio1_ctrl.checked = true
        }
        else if(selectindex === 2)
        {
            radio1_ctrl.checked = false
            radio2_ctrl.checked = true
        }
    }

    function setFontSize(fontsize)
    {
        title_font_size = fontsize
        radio1_ctrl.title_font_size = title_font_size
        radio2_ctrl.title_font_size = title_font_size
    }
    function getSelectIndex()
    {
        if(radio1_ctrl.checked)
        {
            return 1
        }
        else if(radio2_ctrl.checked)
        {
            return 2
        }
    }
    Label {
        id:_title
        text: qsTr(":");
        font.pixelSize: title_font_size;
        font.family: "Microsoft YaHei"
        width:120
        height: parent.height
        color:"#ffffff"
        anchors.top: combox_ctrl_rect.top
        anchors.left: parent.left
        anchors.leftMargin: 10
        anchors.verticalCenter: parent.verticalCenter;
        horizontalAlignment: Text.AlignRight;
    }

    Wgt.Radiocontrol
    {
        id:radio1_ctrl
        anchors.top: parent.top
        anchors.topMargin: 0
        anchors.left: _title.right
        anchors.leftMargin: 10
        width:(combox_ctrl_rect.width-_title.width - 32)/2
        height: parent.height
    }

    Wgt.Radiocontrol
    {
        id:radio2_ctrl
        anchors.top: parent.top
        anchors.topMargin: 0
        anchors.left: radio1_ctrl.right
        anchors.leftMargin: 10
        width:(combox_ctrl_rect.width-_title.width - 32)/2
        height: parent.height
    }
    Component.onCompleted:
    {
        radio1_ctrl.title_font_size = title_font_size
        radio2_ctrl.title_font_size = title_font_size
        radio1_ctrl.checked = true
    }

    Connections {
        target: radio1_ctrl
        function onClicked(checked) {
            radio1_ctrl.checked = true
            radio2_ctrl.checked = false
        }
    }

    Connections {
        target: radio2_ctrl
        function onClicked(checked) {
            radio2_ctrl.checked = true
            radio1_ctrl.checked = false
        }
    }


}
