import QtQuick 2.15
import QtQuick.Controls 2.15



Item   {
    id:switch_ctrl_rect
    visible: true
    opacity: 1.0
    property string selectcolor:"#157e5b"

    property int ctrl_height:32
    property int title_font_size:14

    function setSwitch(bSwitch)
    {
        _control.checked = bSwitch
    }

    function setTitle(title)
    {
        _title.text = title
    }

    function setEnable(bEnable)
    {
        _control.enabled = bEnable
    }

    function getSwitch()
    {
        return _control.checked ;
    }
    Label {
        id:_title
        text: qsTr("AltType:");
        font.pixelSize: title_font_size;
        font.family: "Microsoft YaHei"
        width:120
        height: parent.height
        color:"#ffffff"
        anchors.top: switch_ctrl_rect.top
        anchors.left: parent.left
        anchors.leftMargin: 10
        anchors.verticalCenter: parent.verticalCenter;
        horizontalAlignment: Text.AlignRight;
    }

    Switch {
        id: _control
        anchors.top: _title.top
        anchors.left: _title.right
        anchors.leftMargin: 10
        width:(parent.width-_title.width - 32)
        height: parent.height
        checked: false
        //      background: Rectangle{
        //          color: "lightyellow"
        //      }
        indicator: Rectangle {
            id:big_rect
            implicitWidth: ctrl_height*3
            implicitHeight: ctrl_height
            x: _control.leftPadding
            y: parent.height / 2 - height / 2
            opacity: _control.enabled ? 1 : 0.3
            radius: ctrl_height/2
            color: _control.checked ? "#0062BE" : "#ffffff"
            border.color: _control.checked ? "#0062BE" : "#cccccc"
            //小圆点
            Rectangle {
                id : smallRect
                width: ctrl_height-4
                height: ctrl_height-4
                anchors.top: parent.top
                anchors.topMargin:2
                radius: (ctrl_height-4)/2
                opacity: _control.enabled ? 1 : 0.3
                color: _control.down ? "#cccccc" : "#000000"
//                border.color: _control.checked ? (_control.down ? "#17a81a" : "#21be2b") : "#999999"
                border.color: _control.down ? "#cccccc" :"#000000"

                //改变小圆点的位置
                NumberAnimation on x{
                    to: big_rect.width-(ctrl_height-4)
                    running: _control.checked ? true : false
                    duration: 200
                }

                //改变小圆点的位置
                NumberAnimation on x{
                    to: 0
                    running: _control.checked ? false : true
                    duration: 200
                }
            }
        }

        //        //要显示的文本
        //        contentItem: Text {
        //            text: control.checked.toString()
        //            font.pixelSize: 50
        //            //鼠标按下时  control.down
        //            color: control.down ? "green" : "red"
        //            verticalAlignment: Text.AlignVCenter
        //            anchors.left: indicator.right
        //        }
    }
}
