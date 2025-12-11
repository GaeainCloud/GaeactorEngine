/****************************************************************************
**
** Copyright (C) 2015-2016 Dinu SV.
** (contact: mail@dinusv.com)
** This file is part of QML Gantt library.
**
** GNU Lesser General Public License Usage
** This file may be used under the terms of the GNU Lesser
** General Public License version 3 as published by the Free Software
** Foundation and appearing in the file LICENSE.LGPLv3 included in the
** packaging of this file. Please review the following information to
** ensure the GNU Lesser General Public License version 3 requirements
** will be met: https://www.gnu.org/licenses/lgpl.html.
**
****************************************************************************/

import QtQuick 2.15
import QtQuick.Controls 1.1
import QtCharts 2.2
import QtQuick.Dialogs 1.3

import "../wgt" as Wgt

Rectangle {
    id:rootrect
    color: "#5f5f5f"
    visible: true
    property double zoomScale: 1
    property int context_font_size: 14

    property color cl :"#5f5f5f"

    signal qml_quit_agent_edit_panel_sig
    function updateHeader()
    {
        ganttView.header = null;
        ganttView.header = gantt_headerView;
    }

    function updateZoom(zoom)
    {
        zoomScale = zoom
        for(var i = 0;i < ganttView.count;i++)
        {
            var item = ganttView.itemAtIndex(i);
            if(item === null)
            {
            }
            else
            {
                var row = item.children[0]
                var row2 = row.children[0]
                row2.updateZoom(zoomScale)
            }
        }
    }

    FileDialog {
        id: fileDialog
        title: "Choose a File"
        folder: shortcuts.home // 设置初始路径为用户的主目录
        nameFilters: ["Excel Files (*.xlsx)","All Files (*)"] // 文件类型过滤器
        modality:Qt.ApplicationModal  // 设置对话框为应用程序级模态
        onAccepted: {

            for (var i = 0; i < fileDialog.fileUrls.length;i++)
            {
                var modelslistitem = fileDialog.fileUrls[i]
                var filepathlist = modelslistitem.split("///")
                if (filepathlist.length === 2)
                {
                    var modelfilepath = filepathlist[1];
                    ganttWidget.openFlightFile(modelfilepath)
                }
            }
        }
        onRejected: {
        }
    }


    Wgt.Buttoncontrol
    {
        id:back_btn
        width: ctrl_height
        height: ctrl_height
        anchors.top: parent.top
        anchors.topMargin: 2
        anchors.left: parent.left
        anchors.leftMargin: 10
    }

    Connections {
        target: back_btn.getBtnObj()
        function onClicked() {
            qml_quit_agent_edit_panel_sig()
        }
    }


    Rectangle{
        anchors.left: parent.left
        anchors.top: back_btn.bottom
        anchors.topMargin: 5
        height: parent.height - 14 - back_btn.height
        clip: true
        width: 150

        ListView{
            id: ganttNameView

            anchors.fill: parent

            interactive: false
            contentY: ganttView.contentY

            header:name_headerView;//只构建表头上滑动时表头也会跟随上滑动消失
            headerPositioning: ListView.OverlayHeader;//枚举类型


            model: ganttModelList
            delegate: Rectangle{
                width: ganttNameView.width
                height: 40
                color: "#fff"
                Rectangle{
                    color: cl
                    width: parent.width
                    height: 38
                    Text{
                        anchors.verticalCenter: parent.verticalCenter
                        anchors.left: parent.left
                        anchors.leftMargin: 10
                        text: name
                        font.family: "Microsoft YaHei"
                        font.pixelSize: context_font_size
                        color: "#fff"
                    }
                    MouseArea{
                        anchors.fill: parent
                        onDoubleClicked: {
                            trackEditBox.visible = true
                            trackEdit.selectAll()
                            trackEdit.forceActiveFocus()
                        }
                    }
                    Rectangle{
                        id: trackEditBox
                        anchors.verticalCenter: parent.verticalCenter
                        anchors.fill: parent
                        anchors.margins: 5
                        visible: false
                        color: "#aaa"
                        border.color: "#6666cc"
                        border.width: 1
                        TextInput{
                            id: trackEdit
                            anchors.verticalCenter: parent.verticalCenter
                            anchors.left: parent.left
                            anchors.leftMargin: 5
                            width: parent.width - 10
                            text: name
                            color: "#000"
                            font.pixelSize: 13
                            font.bold: false
                            font.family: "Microsoft YaHei"
                            selectByMouse: true
                            selectionColor: "#444499"
                            onActiveFocusChanged: {
                                if ( !activeFocus ){
                                    name = text
                                    trackEditBox.visible = false
                                }
                            }
                            Keys.onReturnPressed: {
                                name = text
                                event.accepted = true
                                trackEditBox.visible = false
                            }
                            Keys.onEnterPressed: {
                                name = text
                                event.accepted = true
                                trackEditBox.visible = false
                            }

                            Keys.onEscapePressed: {
                                text = name
                                event.accepted = true
                                trackEditBox.visible = false
                            }
                            MouseArea{
                                anchors.fill: parent
                                acceptedButtons: Qt.NoButton
                                cursorShape: Qt.IBeamCursor
                            }
                        }
                    }
                }
            }

            MouseArea{
                anchors.fill: parent
                onWheel: {
                    wheel.accepted = true
                    var newContentY = ganttView.contentY -= wheel.angleDelta.y / 6
                    if ( newContentY > ganttView.contentHeight - ganttView.height )
                        ganttView.contentY = ganttView.contentHeight - ganttView.height
                    else if ( newContentY < 0 )
                        ganttView.contentY = 0
                    else
                        ganttView.contentY = newContentY
                }
                onClicked: mouse.accepted = false;
                onPressed: mouse.accepted = false;
                onReleased: mouse.accepted = false
                onDoubleClicked: mouse.accepted = false;
                onPositionChanged: mouse.accepted = false;
                onPressAndHold: mouse.accepted = false;
            }
        }
    }

    Rectangle{
        anchors.top: back_btn.bottom
        anchors.topMargin: 5
        height: parent.height - back_btn.height
        width: parent.width - 152
        x: 152

        ScrollView{
            id: mainScroll
            anchors.fill: parent
            //width: ganttModelList.contentWidth*zoomScale
            ListView{
                id: ganttView

                header:gantt_headerView;//只构建表头上滑动时表头也会跟随上滑动消失
                headerPositioning: ListView.OverlayHeader;//枚举类型

                height: parent.height
                contentWidth: ganttModelList.contentWidth
                model: ganttModelList
                delegate: Rectangle{
                    height: 40
                    width: ganttLine.width
                    color: "#fff"

                    Rectangle{
                        height: 38
                        width: ganttLine.width
                        color: cl
                        GanttLine{
                            id: ganttLine
                            color: cl
                            height: 24
                            anchors.centerIn: parent
                            viewportX: ganttView.contentX / ganttLine.zoomScale
                            viewportWidth: ganttView.width / ganttLine.zoomScale
                            model: ganttModel
                            onEditItem: {
                                ganttEditWindow.ganttItem = item
                                ganttEditWindow.visible = true
                            }
                        }
                    }
                }
            }
        }

    }
    Connections {
        target: mainScroll.__verticalScrollBar
        function onValueChanged() {
            for(var i = 0;i < ganttView.count;i++)
            {
                var item = ganttView.itemAtIndex(i);
                if(item === null)
                {
                }
                else
                {
                    var row = item.children[0]
                    var row2 = row.children[0]
                    row2.updateZoom(zoomScale)
                }
            }
        }
    }

    Component {
        id:name_headerView;
        Rectangle {
            width:parent.width;
            height: 40;
            color: "lightgrey";
            z:2;//将表头的z坐标设置在上层，表头在设置属性为overlayHeader时就不会随滑动而消失，始终显示在最上面

            Item {
                anchors.fill: parent
                Rectangle {
                    width: parent.width
                    height: parent.height
                    anchors.left: parent.left
                    anchors.leftMargin: 0
                    color:"transparent"
                    Text {
                        width:parent.width
                        height:parent.height
                        anchors.verticalCenter: parent.verticalCenter
                        anchors.horizontalCenter: parent.horizontalCenter
                        text: qsTr("Index")
                        font.pixelSize: context_font_size
                        font.family: "Microsoft YaHei"
                        color:"#222222"
                        verticalAlignment: Text.AlignVCenter
                        horizontalAlignment:Text.AlignHCenter
                    }
                }
            }

            MouseArea {
                anchors.fill: parent;
                onPressed: {
                    mouse.accepted = true;
                }
            }
        }
    }

    Component {
        id:gantt_headerView;
        Rectangle {
            width:parent.width;
            height: 40;
            //            color: "lightgrey";
            z:2;//将表头的z坐标设置在上层，表头在设置属性为overlayHeader时就不会随滑动而消失，始终显示在最上面

            //            // 定义时间轴背景颜色
            color: "transparent"

            // 定义时间轴线条颜色和宽度
            property color lineColor: "green"
            property int lineWidth: 1
            property int hourWidth: 16
            property int minuteWidth: 8
            property int sencond10Width: 4

            // 定义时间轴刻度颜色和宽度
            property color tickColor: "gray"
            property int tickWidth: 5
            property color secondColor: "gray"

            // 定义时间轴刻度间隔
            property int scale_diff:10

            property int tickInterval: 60/scale_diff
            property int sec10Interval: 10/scale_diff


            // 定义时间轴标签间隔
            property int labelInterval: 120

            // 定义时间轴文本颜色和字体大小
            property color textColor: "black"
            property int fontSize: 12
            property int hourfontSize: 14



            Item {
                anchors.fill: parent
                // 绘制时间轴线条
                Rectangle {

                        anchors.left: parent.left
                        anchors.right: parent.right
                        anchors.rightMargin: parent.width - parent.width*zoomScale
                        anchors.verticalCenter: parent.verticalCenter

                    width: parent.width*zoomScale
                    height: 1
                    color: lineColor
                }


                // 绘制时间轴刻度和标签
                Repeater {
                    model: parent.width / tickInterval

                    Rectangle {
                        x: index*tickInterval*zoomScale
                        y: 0
                        width: tickInterval*zoomScale
                        height: parent.height
                        opacity: 0.1
                        color: index%2===0?"green":"blue"

                    }
                }

                // 绘制时间轴刻度和标签
                Repeater {
                    model: parent.width / (tickInterval*60)

                    Rectangle {
                        x: index*(tickInterval*60)*zoomScale
                        y: 0
                        width: (tickInterval*60)*zoomScale
                        height: parent.height/2
                        opacity: 1.0
                        color: index%2===0 ? "#292929":"#595959"

                    }
                }


                // 绘制时间轴刻度和标签
                Repeater {
                    model: parent.width/sec10Interval

                    Rectangle {
                        x: index*sec10Interval*zoomScale
                        y: parent.height / 2 - lineWidth / 2 - sencond10Width
                        width: lineWidth*zoomScale
                        height: sencond10Width
                        color: secondColor
                    }
                }

                // 绘制时间轴刻度和标签
                Repeater {
                    model: parent.width / tickInterval

                    Rectangle {
                        x: index * tickInterval*zoomScale
                        y: index % 60 === 0 ? (parent.height / 2 - lineWidth / 2 - hourWidth): (((index)%60)%10 === 0 ? (parent.height / 2 - lineWidth / 2 - hourWidth + 5):(parent.height / 2 - lineWidth / 2 - minuteWidth))
                        width: lineWidth*zoomScale
                        height: index % 60 === 0 ? hourWidth : (((index)%60)%10 === 0 ? hourWidth - 5 :minuteWidth)
                        color: tickColor

                        Text {
//                            x:(index-1) % 60 === 0 ? (- labelInterval / 2 - 70) : -11
                            x:(index-1) % 60 === 0 ? (- labelInterval / 2 -10) : -11
                            y:(index-1) % 60 === 0 ? parent.height + 11 : parent.height
                            text:  (index-1) % 60 === 0 ? ganttWidget.getDateTimeStr((index-1)*tickInterval): (((index-1)%60)%10 === 0 ? ((index-1)%60).toString():"")
                            font.family: "Microsoft YaHei"
                            font.pixelSize: index % 60 === 0 ? hourfontSize-4 : fontSize
                            color: textColor
                        }
                    }
                }
            }
        }
    }

    Component.onCompleted: {
        back_btn.setBtnIcon("qrc:/res/qml/icon/back.png", ctrl_height, ctrl_height,qsTr("Back"))
    }
    GanttEditWindow{
        id: ganttEditWindow
    }
}

