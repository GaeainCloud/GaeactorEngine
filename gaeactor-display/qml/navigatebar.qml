import QtQuick 2.15
import QtQuick.Window 2.15
import QtQuick.Controls 2.15

import "./wgt" as Wgt

Rectangle {
    width: 120
    height: 800
    visible: true
    color: "#2e2f30"

    property string selectcolor:"#157e5b"
    property int iconw:32

    Rectangle
    {
        id:runtime_list_parent_rect
        anchors.fill: parent
        color: "#3e3f40"
        opacity: 1.0

        Rectangle{
            id:runtime_list_rect
            anchors.top: runtime_list_parent_rect.top
            anchors.topMargin: 0
            anchors.left: runtime_list_parent_rect.left
            anchors.leftMargin: 0
            width: parent.width
            height:parent.height-parent.width
            color: 'transparent'

            ListView {
                id: navigate_bar_listView
                anchors.fill: parent
                model: navibar_listModel
                delegate: navigate_bar_delegate_list
                highlight: runtime_list_highlight // 高亮设置
                highlightFollowsCurrentItem: false
                interactive: true // 设置为可交互
                focus: true // 获取焦点
                clip: true; //超出边界的数据进行裁剪
                //header:headerView;//只构建表头上滑动时表头也会跟随上滑动消失
                //headerPositioning: ListView.OverlayHeader;//枚举类型

                ScrollBar.vertical: ScrollBar {       //滚动条
                    anchors.right: navigate_bar_listView.right
                    anchors.top: navigate_bar_listView.top
                    anchors.topMargin: 0
                    width: 5
                    active: true
                    visible: navigate_bar_listView.contentHeight > navigate_bar_listView.height // 根据内容高度和列表高度判断滚动条是否可见
                    policy:ScrollBar.AsNeeded
                    background: Item {            //滚动条的背景样式
                        Rectangle {
                            anchors.centerIn: parent
                            height: parent.height
                            width: parent.width*0.5
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

        Rectangle{
            id:settings_rect
            anchors.top: runtime_list_rect.bottom
            anchors.topMargin: 0
            anchors.left: runtime_list_parent_rect.left
            anchors.leftMargin: 0
            width: parent.width
            height:parent.width
            color: 'transparent'

            Wgt.Buttoncontrol
            {
                id:settings_btn
                width: parent.width
                height: parent.width
                anchors.left: settings_rect.left
                anchors.leftMargin: 0
            }

            Connections {
                target: settings_btn.getBtnObj()
                function onClicked() {
                    parentWidget.updateWidget(8)
                }
            }
        }

        Component.onCompleted: {
            settings_btn.setBtnIcon("qrc:/res/qml/svg/settings.svg", settings_btn.width, settings_btn.width,qsTr("Settings"))
        }

        ListModel {
            id: navibar_listModel
            ListElement{modeid: 0
                btnwidgetid:0
                name: qsTr("Entity Edit")
                image_src: "qrc:/res/qml/icon/agentedit.png" }
            ListElement{modeid: 1
                btnwidgetid:1
                name: qsTr("Instagent Edit")
                image_src: "qrc:/res/qml/icon/instagentedit.png" }
            ListElement{modeid: 2
                btnwidgetid:2
                name: qsTr("Runtime Edit")
                image_src: "qrc:/res/qml/icon/runtimestyle.png" }
            ListElement{modeid: 3
                btnwidgetid:3
                name: qsTr("SoStep Edit")
                image_src: "qrc:/res/qml/icon/sostepedit.png" }
            ListElement{modeid: 4
                btnwidgetid:4
                name: qsTr("Deduction 2D")
                image_src: "qrc:/res/qml/icon/play.png" }
            ListElement{modeid: 5
                btnwidgetid:5
                name: qsTr("Deduction 3D")
                image_src: "qrc:/res/qml/icon/ptah.png" }
            ListElement{modeid: 6
                btnwidgetid:6
                name: qsTr("Review")
                image_src: "qrc:/res/qml/svg/review.svg" }
            ListElement{modeid: 7
                btnwidgetid:7
                name: qsTr("Tracking")
                image_src: "qrc:/res/qml/svg/tracking.svg" }
        }

        Component {
            id: navigate_bar_delegate_list
            Rectangle {

                id: delegate_list_rct1
                height: ListView.view.width
                width: ListView.view.width

                color:ListView.isCurrentItem?selectcolor:(index % 2 == 0?"#5f5f5f":"#3f3f3f")
                Item {
                    anchors.fill: parent
                    Wgt.Buttoncontrol
                    {
                        id:_mode_btn
                        width: parent.width
                        height: parent.width
                    }
                    Component.onCompleted: {
                        _mode_btn.setBtnIcon(image_src,parent.width/2,parent.width/2,name)
                    }
                    Connections {
                        target: _mode_btn.getBtnObj()
                        function onClicked() {
                            parentWidget.updateWidget(btnwidgetid)
                            navigate_bar_listView.currentIndex = index
                            //navigate_bar_listView.currentIndex = modeid
                            // 这里可以处理按钮点击后的逻辑
                        }
                    }

                    //                    MouseArea {
                    //                        anchors.fill: parent
                    //                        onClicked:  {
                    //                            parentWidget.updateWidget(index)
                    //                            navigate_bar_listView.currentIndex = index
                    //                        }
                    //                    }
                }
            }
        }


        Component{   //高亮条
            id: runtime_list_highlight
            Rectangle {
                width: 180; height: 40
                color: "lightsteelblue"; radius: 0
                Behavior on y {
                    // 弹簧动画
                    SpringAnimation {
                        spring: 10
                        damping: 0.35
                    }
                }
            }
        }
    }

}
