import QtQuick 2.15
import QtQuick.Controls 2.15



Item   {
    id:textfield_ctrl_rect
    visible: true
    opacity: 1.0
    property string selectcolor:"#157e5b"

    property int title_font_size:14

    property bool usingicon:false
    function getBtnObj()
    {
        return children[0]
    }

    function setTitle(title)
    {
        _control.text = title
        _control_tooltip.text = title        
        _control.contentItem=btn_text
    }


    function setToolTips(title)
    {
        _control_tooltip.text = title
    }


    function setBtnIcon(imgsrc,w,h,title)
    {
        _control.icon.source=imgsrc // 设置按钮图像
        _control.icon.width= w // 图像宽度
        _control.icon.height= h // 图像高度
        _control.icon.color="transparent" // 图像颜色
        _control_tooltip.text = title
        usingicon=true
    }


    Button {
        id: _control
        text: qsTr("")
        font.pixelSize: title_font_size;
        font.family: "Microsoft YaHei"
        width: parent.width
        height: parent.height
        anchors.top:parent.top

        background: Rectangle {
            id:_control_bk
            implicitWidth: parent.width
            implicitHeight: parent.height
            opacity: enabled ? 1 : 0.3
            color: _control.down ? "#0062BE" : usingicon?"transparent":"#00ff00"
            radius: width/8
        }
        ToolTip{
            id:_control_tooltip
            delay: 500              //tooltip 500ms后出现
            timeout: 5000           //tooltip 5s后自动消失
            visible: _control.hovered    //鼠标进入button中
            text: ''
            font.pixelSize: title_font_size;
            font.family: "Microsoft YaHei"
            background: Rectangle {
                color: "lightgray"
                radius: 4
            }
        }
        onClicked: {
        }
    }

    Text {
        id:btn_text
        text: _control.text
        font: _control.font
        color: btn_text.enabled ? '#000000':"#353637"  // 在这里修改文字颜色
        horizontalAlignment: Text.AlignHCenter
        verticalAlignment: Text.AlignVCenter
    }
}
