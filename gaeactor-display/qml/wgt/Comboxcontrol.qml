import QtQuick 2.15
import QtQuick.Controls 2.15



Item   {
    id:combox_ctrl_rect
    visible: true
    opacity: 1.0
    property string selectcolor:"#157e5b"

    property int title_font_size:14

    function getBtnObj()
    {
        return _control
    }

    function getSelectModel()
    {
        return _control.model.get(_control.currentIndex)
    }

    function updateData(data)
    {
//        console.log("data"+data.length)
        for(var i = 0;i < data.length;i++)
        {
            var patternAgentsitem = data[i]
//            console.log("data"+patternAgentsitem["index"]+" "+patternAgentsitem["modelData"])
            _control.model.insert(_control.model.count,patternAgentsitem)
        }
        setSelectIndex(0)
    }

    function setTitle(title)
    {
        _title.text = title
    }

    function setEnable(bEnable)
    {
        _control.enabled = bEnable
    }

    function setSelectIndex(selectindex)
    {
        _control.currentIndex = selectindex
    }

    function setSelectVal(val)
    {
        for(var i = 0; i < _control.model.count;i++)
        {
            if(_control.model.get(i).enumid === val)
            {
                _control.currentIndex = i
                break;
            }
        }
    }

    function getSelectVal()
    {
        return _control.model.get(_control.currentIndex).enumid
    }


    function getSelectIndex()
    {
        return _control.currentIndex;
    }
    Label {
        id:_title
        text: qsTr("AltType:");
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

    ComboBox {
        id: _control

        model: ListModel{}
        anchors.top: _title.top
        anchors.left: _title.right
        anchors.leftMargin: 10
        width:(combox_ctrl_rect.width-_title.width - parent.height)*2/3
        height: parent.height
        delegate:ItemDelegate {
            width: _control.width
            contentItem: Text {
                text: modelData
//                anchors.left: _title.right
//                anchors.leftMargin: 10
                color:_control.enabled ? '#ffffff':"#353637"
                font: _control.font
                elide: Text.ElideRight
                verticalAlignment: Text.AlignVCenter
            }

            Rectangle {
                anchors.fill: parent
                height: parent.height
                color: _control.highlightedIndex === index?"#5f5f5f":"#3f3f3f" //选中颜色设置
            }
            highlighted: _control.highlightedIndex === index

            required property int index
            required property var modelData
        }


        font.pixelSize: title_font_size;
        font.family: "Microsoft YaHei"

        indicator: Canvas {
            id: combox_altitudeType_canvas
            x: _control.width - width - _control.rightPadding
            y: _control.topPadding + (_control.availableHeight - height) / 2
            width: 12
            height: 8
            contextType: "2d"

            Connections {
                target: _control
                function onPressedChanged() { combox_altitudeType_canvas.requestPaint(); }
            }

            onPaint: {
                context.reset();
                context.moveTo(0, 0);
                context.lineTo(width, 0);
                context.lineTo(width / 2, height);
                context.closePath();
                context.fillStyle = _control.pressed ? "#5f5f5f" : (_control.enabled ? '#ffffff':"#353637");
                context.fill();
            }
        }

        contentItem: Text {
            anchors.left: _control.left
            anchors.leftMargin: 10
            text: _control.displayText
            font: _control.font
            color: _control.pressed ? "#5f5f5f" : (_control.enabled ? '#ffffff':"#353637")
            verticalAlignment: Text.AlignVCenter
            elide: Text.ElideRight
        }

        background: Rectangle {
            implicitWidth: 120
            implicitHeight: 20
            border.color: _control.pressed ? "#5f5f5f" : "#ffffff"
            border.width: _control.visualFocus ? 2 : 1
            //radius: 120/2
            color: _control.enabled ? "#181a1b":"#1c1d1f"
        }

        popup: Popup {
            y: _control.height+2
            width: _control.width
            implicitHeight: contentItem.implicitHeight
            padding: 5

            contentItem: ListView {
                id:cbx_altitudeType_listview
                clip: true
                implicitHeight: contentHeight
                model: _control.popup.visible ? _control.delegateModel : null
                currentIndex: _control.highlightedIndex

                delegate:Rectangle {
                    color:ListView.isCurrentItem?selectcolor:(index % 2 == 0?"#5f5f5f":"#3f3f3f") //选中颜色设置
                }

                ScrollIndicator.vertical: ScrollIndicator { }
            }

            background: Rectangle {
                border.color: "#3f3f3f"
                color:"#3f3f3f"
                radius: 2
            }
        }

        Component.onCompleted: {
            _control.currentIndex = 0
        }
    }

}
