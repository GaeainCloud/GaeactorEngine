import QtQuick 2.0
import QtQuick.Controls 2.15
Item {
    id: root

    property string text:    'text'
    property bool   checked: false
    property int title_font_size:14
    signal clicked(bool checked);   //onClicked:{root.checked = checked;  print('onClicked', checked)}

    property real padding: 0.1    // around rectangle: percent of root.height
    property bool radio:   false  // false: check box, true: radio button

    opacity: enabled  &&  !mouseArea.pressed? 1: 0.3 // disabled/pressed state

    Rectangle { // check box (or circle for radio button)
        id: rectangle

        height: root.height * (1 - 2 * padding);  width: height // square
        x: padding * root.height
        anchors.verticalCenter: parent.verticalCenter
        border.width: 0.05 * root.height
        radius: (radio? 0.5: 0.2) * height

        Text { // check
            visible: checked  &&  !radio
            anchors.centerIn: parent
            text: '\u2713' // CHECK MARK
            font.pixelSize: title_font_size
        }

        Rectangle { // radio dot
            visible: checked  &&  radio
            color: 'black'
            width: 0.5 * parent.width;  height: width // square
            anchors.centerIn: parent
            radius: 0.5 * width // circle
        }
    }

    Label {
        id: text

        text: root.text
        anchors {left: rectangle.right;  verticalCenter: rectangle.verticalCenter;  margins: padding * root.height}
        font.pixelSize: title_font_size
        font.family: "Microsoft YaHei"
        color:"#ffffff"
    }

    MouseArea {
        id: mouseArea

        enabled: !(radio  &&  checked) // selected RadioButton isn't selectable
        anchors.fill: parent

        onClicked: root.clicked(!checked) // emit
    }
}
