import QtQuick 2.15
import QtQuick.Window 2.15
import QtQuick.Controls 1.4    //1.4版本才有TreeView
import QtQuick.Controls.Styles 1.4
import QtQml.Models 2.2
import QtQuick.Controls 2.15
import QtQuick.Dialogs 1.3


import "./wgt" as Wgt

Rectangle {
    id:agent_edit_panel
    visible: true
    color: "transparent"
    opacity: 1.0
    //    color: "#157e5b"
    //    opacity: 0.5
    property string selectcolor:"#157e5b"

    property int toolbar_height:ctrl_height*3/2

    property int m_agentType:0
    property var agentindex

    signal qml_quit_agent_edit_panel_sig

    function updateAgentDesc(agenttype,desc)
    {
        m_agentType = agenttype
        desc_ctrl.text = desc
    }

    function collapse(modelindex)
    {
        tree_ctrl_view.collapse(modelindex)
    }

    function clearmodel()
    {
        var treeModelnull;
        tree_ctrl_view.model = treeModelnull
    }

    function initmodel()
    {
        tree_ctrl_view.model = editing_tree_rect.treeModel
    }

    function selectIndex(modelindex)
    {
        sel.setCurrentIndex(modelindex, 0x0010)
    }

    function expand(modelindex)
    {
        tree_ctrl_view.expand(modelindex)
    }

    Rectangle
    {
        id:view_toggle_toolbar_rect_bk
        anchors.top: parent.top
        anchors.topMargin: toolbar_height/2
        anchors.right: parent.right
        anchors.rightMargin: toolbar_height*2
        width: toolbar_height*5+toolbar_height/2
        height: toolbar_height
        color: "#2e2f30"
        opacity: 0.3
    }
    Rectangle
    {
        id:view_toggle_toolbar_rect
        anchors.top: parent.top
        anchors.topMargin: toolbar_height/2
        anchors.right: parent.right
        anchors.rightMargin: toolbar_height*2
        width: toolbar_height*5+toolbar_height/2
        height: toolbar_height
        color: "transparent"
        opacity: 1.0

        Wgt.Buttoncontrol
        {
            id:lon_lat_2d_btn
            width: toolbar_height
            height: toolbar_height
            anchors.right: parent.right
            anchors.rightMargin: toolbar_height/4
        }

        Wgt.Buttoncontrol
        {
            id:carte_2d_btn
            width: toolbar_height
            height: toolbar_height
            anchors.right: lon_lat_2d_btn.left
            anchors.rightMargin: 0
        }

        Wgt.Buttoncontrol
        {
            id:discrete_3d_btn
            width: toolbar_height
            height: toolbar_height
            anchors.right: carte_2d_btn.left
            anchors.rightMargin: 0
        }

        Wgt.Buttoncontrol
        {
            id:l_res_3d_btn
            width: toolbar_height
            height: toolbar_height
            anchors.right: discrete_3d_btn.left
            anchors.rightMargin: 0
        }

        Wgt.Buttoncontrol
        {
            id:h_res_3d_btn
            width: toolbar_height
            height: toolbar_height
            anchors.right: l_res_3d_btn.left
            anchors.rightMargin: 0
        }
    }

    Rectangle
    {
        id:editing_toolbar_rect_bk
        anchors.top: parent.top
        anchors.topMargin: toolbar_height/2
        anchors.right: view_toggle_toolbar_rect.left
        anchors.rightMargin: toolbar_height/2
        width: toolbar_height*4+toolbar_height/2
        height: toolbar_height

        color: "#2e2f30"
        opacity: 0.3
    }
    Rectangle
    {
        id:editing_toolbar_rect
        anchors.top: parent.top
        anchors.topMargin: toolbar_height/2
        anchors.right: view_toggle_toolbar_rect.left
        anchors.rightMargin: toolbar_height/2
        width: toolbar_height*6+toolbar_height/2
//        width: toolbar_height*16+toolbar_height/2
        height: toolbar_height

        color: "transparent"
        opacity: 1.0

        Wgt.Buttoncontrol
        {
            id:back_btn
            width: toolbar_height
            height: toolbar_height
            anchors.left: editing_toolbar_rect.left
            anchors.leftMargin: toolbar_height/4
        }

        Connections {
            target: back_btn.getBtnObj()
            function onClicked() {
                qml_quit_agent_edit_panel_sig()
            }
        }        

        Wgt.Buttoncontrol
        {
            id:copy_agent_btn
            width: toolbar_height
            height: toolbar_height
            anchors.left: back_btn.right
            anchors.leftMargin: 0
        }

        Wgt.Buttoncontrol
        {
            id:save_agent_btn
            width: toolbar_height
            height: toolbar_height
            anchors.left: copy_agent_btn.right
            anchors.leftMargin: 0
        }
        Connections {
            target: save_agent_btn.getBtnObj()
            function onClicked() {
                parentWidget.saveAgent()
            }
        }


        Wgt.Buttoncontrol
        {
            id:del_agnet_btn
            width: toolbar_height
            height: toolbar_height
            anchors.right: editing_toolbar_rect.right
            anchors.rightMargin: toolbar_height/4
        }

        Connections {
            target: del_agnet_btn.getBtnObj()
            function onClicked() {
                parentWidget.deleteAgent()
            }
        }
    }


    Label {
        id:desc_ctrl
        color:"#ffffff"
        width: parent.width/10
        height: width
        anchors.top: parent.top
        text: "agent desc"
        wrapMode: Text.Wrap // 设置文本的换行模式为自动换行
        elide: Text.ElideRight // 设置文本超出边界时的处理方式为向右省略号
        horizontalAlignment: Text.AlignHCenter // 设置水平对齐方式为居中
        verticalAlignment: Text.AlignTop // 设置垂直对齐方式为居中
        font.family: "Microsoft YaHei"
        font.pixelSize: context_font_size;
        background: Rectangle {
            implicitWidth: parent.width
            implicitHeight: parent.height
            opacity: enabled ? 1 : 0.3
            color: "transparent"
        }
    }

    Rectangle
    {
        id:bk_rect
        anchors.top: parent.top
        anchors.topMargin: ctrl_height*3
        anchors.left: parent.left
        anchors.leftMargin: ctrl_height*2
        width: parent.width/4
        height: parent.height-ctrl_height*4

        color: "#2e2f30"
        opacity: 0.3
    }
    Rectangle
    {
        id:editing_tree_rect
        anchors.top: parent.top
        anchors.topMargin: ctrl_height*3
        anchors.left: parent.left
        anchors.leftMargin: ctrl_height*2
        width: parent.width/4
        height: parent.height-ctrl_height*4

        color: "transparent"
        opacity: 1.0

        property var treeModel: mControl.getTreeModel()


        //用于节点选中
        ItemSelectionModel {
            id: sel
            model: editing_tree_rect.treeModel
        }
//        ScrollView
//        {
//            id: tree_scroll_ctrl_view
//            anchors.fill: parent

            TreeView {
                id: tree_ctrl_view
                anchors.fill: parent
                headerVisible: false
                backgroundVisible : false   //隐藏背景
                style: treeViewStyle
                model: editing_tree_rect.treeModel
                selection: sel

                horizontalScrollBarPolicy: Qt.ScrollBarAlwaysOff//隐藏水平滚动条
                //verticalScrollBarPolicy: Qt.ScrollBarAlwaysOff//隐藏竖直滚动条


                frameVisible: false
                focus: true // 获取焦点
                clip: true; //超出边界的数据进行裁剪

                TableViewColumn {
                    role: "name"
                    title: "Name"
                    width: tree_ctrl_view.width
                }

                property bool isExpand: true    //点击展开树节点
                itemDelegate: Item {
                    Image {
                        id: nodeTextImage
                        anchors.verticalCenter: parent.verticalCenter
                        source: ""
                    }
                    Text {
                        id: nameText
                        anchors.left: nodeTextImage.right
                        anchors.leftMargin: 5
                        anchors.verticalCenter: parent.verticalCenter
                        color: "#ffffff"
                        elide: styleData.elideMode
                        text: styleData.value
                        font.family: "Microsoft YaHei"
                        font.pixelSize: context_font_size;
                    }
                    Drag.active:itemMouse.drag.active
                    Drag.dragType: Drag.Automatic;      //选择自动开始拖动
                    Drag.supportedActions: Qt.CopyAction;   //选择复制数据到DropArea
                    Drag.onDragFinished: {              //拖拽结束

                    }

                    MouseArea {
                        id: itemMouse
                        anchors.fill: parent
                        hoverEnabled: true
                        drag.target: nameText

                        onClicked: {
                            if(mouse.button == Qt.LeftButton)
                            {
                                //                       tree_ctrl_view.setExpanded(styleData.index, !tree_ctrl_view.isExpanded(styleData.index))
                                sel.setCurrentIndex(styleData.index, 0x0010)    //点击Item，设置选中当前节点
                                if (tree_ctrl_view.isExpand)
                                {
                                    emit: tree_ctrl_view.expand(styleData.index)
                                    tree_ctrl_view.isExpand = false;
                                }
                                else {
                                    emit: tree_ctrl_view.collapse(styleData.index)
                                    tree_ctrl_view.isExpand = true;
                                }
                                mControl.updateSelectNode(styleData.index,tree_ctrl_view.isExpand)
                            }
                        }
                        onDoubleClicked: {  //刷新子节点
                            //                        tree_ctrl_view.collapse(styleData.index)  //先收起再展开，不然会有bug
                            //                        mControl.updateNode(styleData.index)
                            //                        tree_ctrl_view.expand(styleData.index)
                        }
                    }
                }

                MouseArea {
                    id: rightMouse
                    anchors.fill: parent
                    acceptedButtons: Qt.RightButton
                    onClicked: {
                        if(mouse.button == Qt.RightButton)
                        {
                            // 在此处显示自定义的右键菜单
                            var menu = menuComponent.createObject(tree_ctrl_view, {
                                                                      "x": mouse.x,
                                                                      "y": mouse.y
                                                                  });
                            menu.open();
                        }
                    }
                }


                Component {
                    id: menuComponent

                    Menu {
                        id: contextMenu
                        MenuItem {
                            text: qsTr("New");
                            font.family: "Microsoft YaHei"
                            font.pixelSize: context_font_size;
                            onTriggered:
                            {
                                var indexx = tree_ctrl_view.indexAt(contextMenu.x, contextMenu.y)

                                var type = mControl.getModelIndexType(indexx)

                                switch(type)
                                {
                                case 1:
                                {
                                    agentindex = indexx
                                    if(m_agentType === 0)
                                    {
                                        fileDialog.nameFilters=["FBX Files (*.fbx)","IVE Files (*.ive)","OSG Files (*.osg)","All Files (*)"]
                                    }
                                    else if(m_agentType === 1)
                                    {
                                        fileDialog.nameFilters=["GeoJson Files (*.geojson)","Json Files (*.json)","All Files (*)"]
                                    }

                                    fileDialog.open();
                                }
                                break;
                                default:
                                {
                                    console.log(type)
                                    sel.setCurrentIndex(indexx, 0x0010)    //点击Item，设置选中当前节点
                                    tree_ctrl_view.collapse(indexx)  //先收起再展开，不然会有bug
                                    parentWidget.addSubNode(indexx)
                                    tree_ctrl_view.expand(indexx)
                                }
                                break;
                                }
                            }
                        }
                    }
                }
            }

//            ScrollBar.vertical: ScrollBar {       //滚动条
//                id: verticalBar

//                anchors.right: tree_ctrl_view.right
//                anchors.top: tree_ctrl_view.top
//                anchors.topMargin: 0
//                width: 5
//                hoverEnabled: true
//                active: hovered ||pressed
//                visible: tree_ctrl_view.contentHeight > tree_ctrl_view.height // 根据内容高度和列表高度判断滚动条是否可见
//                //policy:ScrollBar.AsNeeded
//                policy: ScrollBar.AlwaysOn
//                background: Item {            //滚动条的背景样式
//                    Rectangle {
//                        anchors.centerIn: parent
//                        height: parent.height
//                        width: parent.width*0.5
//                        color: '#3e2f30'
//                        radius: width/2
//                    }
//                }

//                contentItem: Rectangle {
//                    radius: width/3        //bar的圆角
//                    color: 'grey'
//                }
//            }


//            ScrollBar.horizontal: ScrollBar {       //滚动条
//                id: horizonBar

//                anchors.bottom: tree_ctrl_view.bottom
//                anchors.left: tree_ctrl_view.left
//                anchors.leftMargin: 0
//                height: 5
//                hoverEnabled: true
//                active: hovered ||pressed
//                visible: tree_ctrl_view.contentWidth > tree_ctrl_view.width // 根据内容高度和列表高度判断滚动条是否可见
//                policy:ScrollBar.AsNeeded
//                background: Item {            //滚动条的背景样式
//                    Rectangle {
//                        anchors.centerIn: parent
//                        height: parent.height*0.5
//                        width: parent.width
//                        color: '#3e2f30'
//                        radius: height/2
//                    }
//                }

//                contentItem: Rectangle {
//                    radius: height/3        //bar的圆角
//                    color: 'grey'
//                }
//            }
//        }


        Component {
            id: treeViewStyle
            TreeViewStyle {
                padding.left: 2
                padding.right: 2
                indentation: 30  //节点间缩进
                rowDelegate: Rectangle {
                    color: styleData.selected ? "#007DFF" : "transparent"   //选中节点后的颜色和背景色
                    height: 30
                }
                /*
                branchDelegate: Image {  //节点展开收缩图标
                    id: nodeImage
                    source: {
                        return styleData.isExpanded ? "" : ""
                    }
                }
    */
            }
        }
    }

    FileDialog {
        id: fileDialog
        title: "Choose a File"
        folder: shortcuts.home // 设置初始路径为用户的主目录
        nameFilters: (m_agentType === 0)?(["FBX Files (*.fbx)","IVE Files (*.ive)","OSG Files (*.osg)","All Files (*)"]):(["GeoJson Files (*.geojson)","Json Files (*.json)","All Files (*)"])
        modality:Qt.ApplicationModal  // 设置对话框为应用程序级模态
        onAccepted: {
            parentWidget.addModelNode(agentindex, fileDialog.fileUrls)
        }
        onRejected: {
        }
    }


    Component.onCompleted: {
        back_btn.setBtnIcon("qrc:/res/qml/icon/back.png", toolbar_height/2, toolbar_height/2,qsTr("Back"))
        copy_agent_btn.setBtnIcon("qrc:/res/qml/icon/copy.png", toolbar_height/2, toolbar_height/2,qsTr("Copy Agent"))
        save_agent_btn.setBtnIcon("qrc:/res/qml/icon/save.png", toolbar_height/2, toolbar_height/2,qsTr("Save Agent"))
        del_agnet_btn.setBtnIcon("qrc:/res/qml/icon/delete.png", toolbar_height/2, toolbar_height/2,qsTr("Del Agent"))

        //del_agnet_btn.setBtnIcon("qrc:/res/qml/svg/send.svg", toolbar_height/2, toolbar_height/2,qsTr("Del Agent"))

        lon_lat_2d_btn.setBtnIcon("qrc:/res/qml/icon/2D_LonLat.jpg", toolbar_height/2, toolbar_height/2,qsTr("2D Lon/Lat"))
        carte_2d_btn.setBtnIcon("qrc:/res/qml/icon/2D_Cartesian.jpg", toolbar_height/2, toolbar_height/2,qsTr("2D Carte"))
        discrete_3d_btn.setBtnIcon("qrc:/res/qml/icon/dispersed.jpg", toolbar_height/2, toolbar_height/2,qsTr("3D Discrete"))
        l_res_3d_btn.setBtnIcon("qrc:/res/qml/icon/low.jpg", toolbar_height/2, toolbar_height/2,qsTr("3D L-Res"))
        h_res_3d_btn.setBtnIcon("qrc:/res/qml/icon/high.jpg", toolbar_height/2, toolbar_height/2,qsTr("3D H-Res"))
    }
}
