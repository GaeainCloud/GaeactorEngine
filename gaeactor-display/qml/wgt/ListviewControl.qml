import QtQuick 2.15
import QtQuick.Controls 2.15



Item   {
    id:listview_ctrl_rect
    visible: true
    opacity: 1.0
    property string selectcolor:"#157e5b"

    property int title_font_size:14

    property bool usingicon:false
    function getBtnObj()
    {
        return children[0]
    }

    function setContext(model,delegate,headerview)
    {
        _control.model = model
        _control.delegate = delegate
        _control.header=headerview
    }

    function setSelectIndex(index)
    {
        _control.currentIndex = index
    }

    Rectangle{
        id:listview_rect
        anchors.top: parent.top
        anchors.left: parent.left
        anchors.leftMargin: 0
        width:parent.width
        height:parent.height
        color: 'transparent'

        ListView {
            id: _control
            anchors.top: parent.top
            anchors.left: parent.left
            anchors.leftMargin: 0
            width:parent.width
            height:parent.height
//            highlight: _control_highlight // 高亮设置
//            highlightFollowsCurrentItem: false
            interactive: true // 设置为可交互
            focus: true // 获取焦点
            clip: true; //超出边界的数据进行裁剪
            headerPositioning: ListView.OverlayHeader;//枚举类型

            ScrollBar.vertical: ScrollBar {       //滚动条
                visible: _control.contentHeight > _control.height // 根据内容高度和列表高度判断滚动条是否可见
                policy:ScrollBar.AsNeeded
                anchors.right: _control.right
                anchors.top: _control.top
                anchors.topMargin: 40
                width: 10
                active: true
                background: Item {            //滚动条的背景样式
                    Rectangle {
                        anchors.centerIn: parent
                        height: parent.height
                        width: parent.width*0.3
                        color: '#3e2f30'
                        radius: width/2
                    }
                }

                contentItem: Rectangle {
                    radius: width/3        //bar的圆角
                    color: 'grey'
                }
            }
        }
    }

//    Component{   //高亮条
//        id: _control_highlight
//        Rectangle {
//            width: 180; height: 40
//            color: "lightsteelblue"; radius: 0
//            y: _control.currentItem.y
//            Behavior on y {
//                // 弹簧动画
//                SpringAnimation {
//                    spring: 10
//                    damping: 0.35
//                }
//            }
//        }
//    }
}
