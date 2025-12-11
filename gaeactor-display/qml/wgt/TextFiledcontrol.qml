import QtQuick 2.15
import QtQuick.Controls 2.15



Item   {
    id:textfield_ctrl_rect
    visible: true
    opacity: 1.0
    property string selectcolor:"#157e5b"

    property int title_font_size:14


    property var context:""

    function getObj()
    {
        return _context
    }

    function setContext(context)
    {
        _context.text = context
    }

    function setTitle(title)
    {
        _title.text = title
    }

    function setEnable(bEnable)
    {
        _context.enabled = bEnable
    }

    function getContext()
    {
        return _context.text;
    }

    function getDoubleContext()
    {
        return parseFloat(_context.text);
    }

    function getIntgerContext()
    {
        return parseInt(_context.text);
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
        anchors.left: parent.left
        anchors.leftMargin: 10
        anchors.verticalCenter: parent.verticalCenter;
        horizontalAlignment: Text.AlignRight;
    }


    TextField{
        id:_context
        anchors.top: parent.top
        anchors.topMargin: 0
        anchors.left: _title.right
        anchors.leftMargin: 10
        width:(textfield_ctrl_rect.width-_title.width - 32)
        height: parent.height
        text:context
//        topPadding: 2
        verticalAlignment: Text.AlignVCenter
        placeholderText: qsTr("please input text")
        color:_context.enabled ? "#ffffff":"#656667"
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
}
