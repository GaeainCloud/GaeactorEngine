import QtQuick 2.15
import QtQuick.Controls 2.15



Item   {
    id:textfield_ctrl_rect
    visible: true
    opacity: 1.0
    property string selectcolor:"#157e5b"

    property int title_font_size:14

    property var context:""

    function setContext(context)
    {
        _context.text = context
    }

    function setTitle(title)
    {
        _title.text = title
    }

    function setContextWrapMode(wrap)
    {
        if(warp)
        {
            _context.wrapMode=TextArea.Wrap
        }
        else
        {
            _context.wrapMode=TextArea.NoWrap
        }
    }

    function setEnable(bEnable)
    {
        _context.enabled = bEnable
    }

    function getContext()
    {
        return _context.text;
    }
    Label {
        id:_title
        text: qsTr("AltType:");
        font.pixelSize: title_font_size;
        font.family: "Microsoft YaHei"
        width:120
        height: parent.height
        color:"#ffffff"
        anchors.top: textfield_ctrl_rect.top
        anchors.topMargin: 5
        anchors.left: parent.left
        anchors.leftMargin: 10
        anchors.verticalCenter: parent.verticalCenter;
        horizontalAlignment: Text.AlignRight;
        //wrapMode: TextArea.Wrap//换行
    }
//            ScrollView
//            {
//                id: view
//                anchors.fill: parent
//                TextArea
//                {
//                    id: _context
//                    visible: true;
//                    focusReason: Qt.MouseFocusReason
//                    wrapMode: TextArea.Wrap//换行
//                    font.pixelSize: 20;
//                    font.weight: Font.Light
//                    focus: true;
//                    textFormat: TextArea.AutoText
//                    selectByMouse:true;
//                    selectByKeyboard: true
//                    color: "red"
//                    text: "侠客行\n赵客缦胡缨，吴钩霜雪明。\n银鞍照白马，飒沓如流星。\n十步杀一人，千里不留行。\n事了拂衣去，深藏身与名。\n闲过信陵饮⑹，脱剑膝前横。"
//                }
//            }

    ScrollView {
        id:view_ctrl
        anchors.top: _title.top
        anchors.left: _title.right
        anchors.leftMargin: 10
        width:(textfield_ctrl_rect.width-_title.width - 32)
        height: parent.height
        contentWidth: _context.implicitWidth
        contentHeight: _context.implicitHeight
        TextArea{
            id:_context
            anchors.fill: parent
            text:context
            placeholderText: qsTr("please input text")
            color:_context.enabled ? "#ffffff":"#353637"
            //wrapMode: TextArea.Wrap//换行
            font.pixelSize: title_font_size;
            font.family: "Microsoft YaHei"
            background:Rectangle {
                anchors.centerIn: parent
                height: parent.height
                width: parent.width
                border.color: _context.pressed ? "#5f5f5f" : "#ffffff"
                border.width: _context.visualFocus ? 2 : 1
                color: _context.enabled ? "#181a1b":"#1c1d1f"
                //radius: width/2
            }
        }

//        ScrollBar.vertical: ScrollBar {       //滚动条
//            id: verticalbar
//            anchors.right: _context.right
//            anchors.top: _context.top
//            anchors.topMargin: 40
//            width: 10
//            hoverEnabled: true
//            active: hovered ||pressed
//            //            visible: _context.contentHeight > _context.height // 根据内容高度和列表高度判断滚动条是否可见
//            //            policy:ScrollBar.AsNeeded

//            background: Item {            //滚动条的背景样式
//                Rectangle {
//                    anchors.centerIn: parent
//                    height: parent.height
//                    width: parent.width*0.3
//                    color: '#3e2f30'
//                    radius: width/2
//                }
//            }

//            contentItem: Rectangle {
//                radius: width/3        //bar的圆角
//                color: 'grey'
//            }
//        }

//        ScrollBar.horizontal: ScrollBar {       //滚动条
//            id: horizonBar

//            anchors.bottom: view_ctrl.bottom
//            anchors.left: view_ctrl.left
//            anchors.leftMargin: 0
//            width: 10
//            height: 5
//            hoverEnabled: true
//            active: hovered ||pressed
//            visible: view_ctrl.contentWidth > view_ctrl.width // 根据内容高度和列表高度判断滚动条是否可见
//            policy:ScrollBar.AlwaysOn
//            background: Item {            //滚动条的背景样式
//                Rectangle {
//                    anchors.centerIn: parent
//                    height: parent.height*0.5
//                    width: parent.width
//                    color: '#3e2f30'
//                    radius: height/2
//                }
//            }

//            contentItem: Rectangle {
//                radius: height/3        //bar的圆角
//                color: 'grey'
//            }
//        }
    }
}


