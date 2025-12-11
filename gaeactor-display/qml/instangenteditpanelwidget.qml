import QtQuick 2.15
import QtQuick.Window 2.15
import QtQuick.Controls 2.15
import QtQml.Models 2.2
import QtQuick.Dialogs 1.3
import "./wgt" as Wgt

Rectangle {
    width: 120
    height: 800
    visible: true
    color: "#2e2f30"

    property string selectcolor:"#157e5b"

    function resetData()
    {
        agents_listModel.clear()
    }

    function addAgentItems(contexts)
    {
        for(var i = 0; i < contexts.length; i++)
        {
            var contextp = contexts[i]
            var curindex = agents_listModel.count
            agents_listModel.insert(curindex,{"modeid": curindex,
                                        "agentKey":contextp["agentKey"],
                                        "agentId":contextp["agentId"],
                                        "agentInstId":contextp["agentInstId"],
                                        "agentOffsetKey":contextp["agentOffsetKey"],
                                        "asmKey":contextp["asmKey"],
                                        "agentKeyword":contextp["agentKeyword"],
                                        "agentName":contextp["agentName"],
                                        "agentNameI18n":contextp["agentNameI18n"],
                                        "agentType":contextp["agentType"],
                                        "modelUrlSlim":contextp["modelUrlSlim"],
                                        "modelUrlMedium":contextp["modelUrlMedium"],
                                        "modelUrlSymbols":contextp["modelUrlSymbols"],
                                        "modelUrlFat":contextp["modelUrlFat"]});
            agents_listView.currentIndex = curindex
        }
    }

    function editTargetAgent(agentIndex)
    {
        var agentdata = agents_listModel.get(agentIndex)
        var subruntimgdata={}
        subruntimgdata["modeid"]= agentdata.modeid
        subruntimgdata["agentKey"]=agentdata.agentKey
        subruntimgdata["agentId"]=agentdata.agentId
        subruntimgdata["agentInstId"]=agentdata.agentInstId
        subruntimgdata["agentOffsetKey"]=agentdata.agentOffsetKey
        subruntimgdata["asmKey"]=agentdata.asmKey
        subruntimgdata["agentKeyword"]=agentdata.agentKeyword
        subruntimgdata["agentName"]=agentdata.agentName
        subruntimgdata["agentNameI18n"]=agentdata.agentNameI18n
        subruntimgdata["agentType"]=agentdata.agentType
        subruntimgdata["modelUrlSlim"]=agentdata.modelUrlSlim
        subruntimgdata["modelUrlMedium"]=agentdata.modelUrlMedium
        subruntimgdata["modelUrlSymbols"]=agentdata.modelUrlSymbols
        subruntimgdata["modelUrlFat"]=agentdata.modelUrlFat
        parentWidget.editAgent(subruntimgdata)
    }



    Rectangle
    {
        id:agents_list_parent_rect
        anchors.fill: parent
        color: "#3e3f40"
        opacity: 1.0


        Rectangle{
            id:agents_search_rect
            anchors.top: agents_list_parent_rect.top
            anchors.topMargin: 0
            anchors.left: agents_list_parent_rect.left
            anchors.leftMargin: 0
            width: parent.width
            height:parent.height/6
            color: 'transparent'
            Rectangle{
                id:_search_rect
                anchors.top: agents_search_rect.top
                anchors.topMargin: (parent.height - ctrl_height)/2
                anchors.left: agents_search_rect.left
                anchors.leftMargin: parent.width/4
                width: parent.width/2
                height:ctrl_height*2
                color: 'transparent'

                TextField{
                    id:search_context
                    anchors.top: parent.top
                    anchors.topMargin: 0
                    anchors.left: parent.left
                    anchors.leftMargin: 10
                    width:_search_rect.width - ctrl_height*2
                    height: ctrl_height*2
                    text:''
                    color:"#ffffff"
                    font.pixelSize: title_font_size;
                    font.family: "Microsoft YaHei"
                    background:Rectangle {
                        anchors.centerIn: parent
                        height: parent.height
                        width: parent.width
                        border.color: search_context.pressed ? "#5f5f5f" : "#ffffff"
                        border.width: search_context.visualFocus ? 2 : 1
                        color: '#3e2f30'
                        //radius: width/2
                    }
                }

                Wgt.Buttoncontrol
                {
                    id:search_btn
                    anchors.top: search_context.top
                    anchors.left: search_context.right
                    width: ctrl_height*2
                    height: ctrl_height*2
                }
                Component.onCompleted: {
                    search_btn.setBtnIcon("qrc:/res/qml/icon/search.png",search_btn.width,search_btn.height,qsTr("Search Agent"))
                }
                Connections {
                    target: search_btn.getBtnObj()
                    function onClicked() {
                        console.log("serach agents")
                        // 这里可以处理按钮点击后的逻辑
                    }
                }
            }
        }

        Rectangle{
            id:agents_list_toolbar_rect
            anchors.top: agents_search_rect.bottom
            anchors.topMargin: 0
            anchors.left: agents_list_parent_rect.left
            anchors.leftMargin: parent.height/25
            anchors.right: agents_list_parent_rect.right
            anchors.rightMargin: parent.height/25
            width: parent.width
            height:ctrl_height*2
            color: 'transparent'

            Wgt.Buttoncontrol
            {
                id:list_btn
                anchors.top: agents_list_toolbar_rect.top
                anchors.left: agents_list_toolbar_rect.left
                width: ctrl_height*2
                height: ctrl_height*2
            }
            Connections {
                target: list_btn.getBtnObj()
                function onClicked() {
                    console.log("list agents")
                    // 这里可以处理按钮点击后的逻辑
                }
            }

            Wgt.Buttoncontrol
            {
                id:card_btn
                anchors.top: agents_list_toolbar_rect.top
                anchors.left: list_btn.right
                anchors.leftMargin: ctrl_height/8
                width: ctrl_height*2
                height: ctrl_height*2
            }
            Connections {
                target: card_btn.getBtnObj()
                function onClicked() {
                    console.log("card agents")
                    // 这里可以处理按钮点击后的逻辑
                }
            }

            Wgt.Buttoncontrol
            {
                id:refersh_btn
                anchors.top: agents_list_toolbar_rect.top
                anchors.left: card_btn.right
                anchors.leftMargin: ctrl_height/8
                width: ctrl_height*2
                height: ctrl_height*2
            }
            Connections {
                target: refersh_btn.getBtnObj()
                function onClicked() {
                    parentWidget.refershAgents();
                    console.log("refersh agents")
                    // 这里可以处理按钮点击后的逻辑
                }
            }


            Wgt.Buttoncontrol
            {
                id:new_btn
                anchors.top: agents_list_toolbar_rect.top
                anchors.right: agents_list_toolbar_rect.right
                width: ctrl_height*2
                height: ctrl_height*2
            }
            Connections {
                target: new_btn.getBtnObj()
                function onClicked() {
                    console.log("new agents")
                    // 这里可以处理按钮点击后的逻辑
                }
            }

            Component.onCompleted: {
                list_btn.setBtnIcon("qrc:/res/qml/icon/list.png",list_btn.width,list_btn.height,qsTr("List Agent"))
                card_btn.setBtnIcon("qrc:/res/qml/icon/card.png",card_btn.width,card_btn.height,qsTr("Card Agent"))
                refersh_btn.setBtnIcon("qrc:/res/qml/icon/refersh.png",refersh_btn.width,refersh_btn.height,qsTr("Refersh Agent"))
                new_btn.setBtnIcon("qrc:/res/qml/icon/new.png",new_btn.width,new_btn.height,qsTr("New Agent"))
            }

        }
        Rectangle{
            id:agents_list_rect
            anchors.top: agents_list_toolbar_rect.bottom
            anchors.topMargin: 0
            anchors.left: agents_list_parent_rect.left
            anchors.leftMargin: 0
            anchors.bottom: agents_list_parent_rect.bottom
            anchors.bottomMargin: 2
            width: parent.width
            color: 'transparent'

            GridView {
                id: agents_listView
                anchors.fill: parent
                anchors.top: parent.top
                anchors.leftMargin: parent.height/25
                anchors.rightMargin: parent.height/25
                anchors.bottomMargin: 0
                model: agents_listModel
                cellWidth: width/8   // 设置单元格宽度
                cellHeight: width/8  // 设置单元格高度
                delegate:agents_list_delegate

                interactive: true // 设置为可交互
                clip: true; //超出边界的数据进行裁剪
                focus: true

                ScrollBar.vertical: ScrollBar {       //滚动条
                    anchors.right: agents_listView.right
                    anchors.top: agents_listView.top
                    anchors.topMargin: 0
                    width: 5
                    active: true
                    visible: agents_listView.contentHeight > agents_listView.height // 根据内容高度和列表高度判断滚动条是否可见
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

        ListModel {
            id: agents_listModel
        }

        Component {
            id: agents_list_delegate
            Rectangle {

                id: agents_list_delegate_rect
                height: agents_listView.cellWidth
                width: agents_listView.cellHeight
                color:GridView.isCurrentItem?selectcolor:"transparent"

                Item {
                    id: agents_delegate_item
                    anchors.fill: parent
                    anchors.topMargin: 0
                    anchors.leftMargin: 10
                    anchors.rightMargin: 10
                    anchors.bottomMargin: 30

                    Wgt.Buttoncontrol
                    {
                        id:preview_btn
                        width: parent.width
                        height: parent.height*4/6
                    }
                    Component.onCompleted: {
                        preview_btn.setBtnIcon(modelUrlFat,128,128,agentKey +"_"+agentType+"_"+agentInstId)
//                        preview_btn.setBtnIcon(modelUrlFat,parent.width,parent.height*4/6,agentKey +"_"+agentType+"_"+agentInstId)
                    }
                    Connections {
                        target: preview_btn.getBtnObj()
                        function onClicked() {
                            editTargetAgent(modeid)
                            agents_listView.currentIndex = modeid
                        }
                    }

                    Label {
                        id:desc
                        color:"#ffffff"
                        width: parent.width
                        height: parent.height*2/6
                        anchors.top: preview_btn.bottom
                        text: agentName;
                        wrapMode: Text.Wrap // 设置文本的换行模式为自动换行
                        elide: Text.ElideRight // 设置文本超出边界时的处理方式为向右省略号
                        horizontalAlignment: Text.AlignHCenter // 设置水平对齐方式为居中
                        verticalAlignment: Text.AlignVCenter // 设置垂直对齐方式为居中

                        font.pixelSize: context_font_size/2;
                        font.family: "Microsoft YaHei"
                        background: Rectangle {
                            implicitWidth: parent.width
                            implicitHeight: parent.height
                            opacity: enabled ? 1 : 0.3
                            color: "#333333"
                        }
                    }
                }

                //                    MouseArea {
                //                        anchors.fill: parent
                //                        onClicked:  {
                //                            //parentWidget.updateWidget(index)
                //                            agents_listView.currentIndex = index
                //                        }
                //                    }
            }
        }

        Component.onCompleted: {
        }
    }



    Dialog {
        id: dialog
        title: qsTr("New Instagent")
        modality: Qt.ApplicationModal
        visible: false
        //width: parent.width/2
        //height: parent.height/2
        standardButtons: Dialog.NoButton

    Rectangle {
        color: "#5f5f5f"
        anchors.fill: parent
    }

//    Popup{

//        id: dialog

//        modal: true
//        visible: false
//        width: parent.width/2
//        height: parent.height/2
//        x:parent.width/4
//        y:parent.height/4
//        anchors.centerIn: parent.Center

//        background: Rectangle {
//            implicitWidth: parent.width
//            implicitHeight: parent.height
//            opacity: enabled ? 1 : 0.3
//            color: "#5f5f5f"
//        }



        Wgt.TextFiledcontrol
        {
            id:instagent_key_ctrl
            anchors.top:parent.top
            anchors.topMargin:ctrl_height/4
            width: parent.width
            height: ctrl_height
        }

        Wgt.Comboxcontrol
        {
            id:agent_key_ctrl
            anchors.top:instagent_key_ctrl.bottom
            anchors.topMargin:ctrl_height/4
            width: parent.width
            height: ctrl_height
        }


        Wgt.TextFiledcontrol
        {
            id:instagent_name_ctrl
            anchors.top:agent_key_ctrl.bottom
            anchors.topMargin:ctrl_height/4
            width: parent.width
            height: ctrl_height
        }


        Wgt.TextFiledcontrol
        {
            id:instagent_namei18n_ctrl
            anchors.top:instagent_name_ctrl.bottom
            anchors.topMargin:ctrl_height/4
            width: parent.width
            height: ctrl_height
        }

        Wgt.RadioGroupcontrol
        {
            id:instagenttype_ctrl
            anchors.top:instagent_namei18n_ctrl.bottom
            anchors.topMargin:ctrl_height/4
            width: parent.width
            height: ctrl_height
        }


        Wgt.TextAreacontrol
        {
            id:instagent_desc_ctrl
            anchors.top:instagenttype_ctrl.bottom
            anchors.topMargin:ctrl_height/4
            width: parent.width
            height: ctrl_height*3
        }

        Wgt.Buttoncontrol
        {
            id:action_cancel_ctrl
            anchors.top:instagent_desc_ctrl.bottom
            anchors.topMargin:ctrl_height/2
            anchors.left: parent.left
            anchors.leftMargin: (parent.width*3/8)-(parent.width/8/8)
            width: parent.width/8
            height: ctrl_height
        }

        Wgt.Buttoncontrol
        {
            id:action_save_ctrl
            anchors.top:instagent_desc_ctrl.bottom
            anchors.topMargin:ctrl_height/2
            anchors.right: parent.right
            anchors.rightMargin: (parent.width*3/8)-(parent.width/8/8)
            width: parent.width/8
            height: ctrl_height
        }



        Component.onCompleted: {
            instagent_key_ctrl.setTitle(qsTr("Instagent Id:"))
            agent_key_ctrl.setTitle(qsTr("Agent Key:"))
            instagent_name_ctrl.setTitle(qsTr("Agent Name:"))
            instagent_namei18n_ctrl.setTitle(qsTr("Agent Namei18n:"))
            instagenttype_ctrl.setTitle(qsTr("Agent Type:"))
            instagent_desc_ctrl.setTitle(qsTr("Agent Desc:"))
            action_cancel_ctrl.setTitle(qsTr("Cancel"))
            action_save_ctrl.setTitle(qsTr("Save"))

            agent_key_ctrl.title_font_size=context_font_size
            instagent_key_ctrl.title_font_size=context_font_size
            instagent_name_ctrl.title_font_size=context_font_size
            instagent_namei18n_ctrl.title_font_size=context_font_size
            instagenttype_ctrl.setFontSize(context_font_size)
            instagent_desc_ctrl.title_font_size=context_font_size
            action_cancel_ctrl.title_font_size=context_font_size
            action_save_ctrl.title_font_size=context_font_size

            instagenttype_ctrl.updateData(qsTr("Instagent"),qsTr("Scene"))
            instagenttype_ctrl.setSelectIndex(1)

            instagenttype_ctrl.setEnable(false)

        }

        Connections {
            target: agent_key_ctrl.getBtnObj()
            function onCurrentIndexChanged() {
                var agentType = agent_key_ctrl.getBtnObj().model.get(agent_key_ctrl.getBtnObj().currentIndex).agentType
                if(agentType === "Instagent")
                {
                    instagenttype_ctrl.setSelectIndex(1)
                }
                else if(agentType === "Scene")
                {
                    instagenttype_ctrl.setSelectIndex(2)
                }
            }
        }


        Connections {
            target: action_cancel_ctrl.getBtnObj()
            function onClicked() {
                dialog.close()
                // 这里可以处理按钮点击后的逻辑
            }
        }

        Connections {
            target: action_save_ctrl.getBtnObj()
            function onClicked() {
                var curindex = agents_listModel.count
                agents_listModel.insert(curindex,{"modeid": curindex,
                                            "agentKey":agent_key_ctrl.getBtnObj().model.get(agent_key_ctrl.getBtnObj().currentIndex).agentKey,
                                            "agentId":instagent_key_ctrl.getContext(),
                                            "agentInstId":instagent_name_ctrl.getContext(),
                                            "agentOffsetKey":"",
                                            "asmKey":"",
                                            "agentKeyword":agent_key_ctrl.getBtnObj().model.get(agent_key_ctrl.getBtnObj().currentIndex).agentKeyword,
                                            "agentName":instagent_name_ctrl.getContext(),
                                            "agentNameI18n":instagent_namei18n_ctrl.getContext(),
                                            "agentType":agent_key_ctrl.getBtnObj().model.get(agent_key_ctrl.getBtnObj().currentIndex).agentType,
                                            "modelUrlSlim":agent_key_ctrl.getBtnObj().model.get(agent_key_ctrl.getBtnObj().currentIndex).modelUrlSlim,
                                            "modelUrlMedium":agent_key_ctrl.getBtnObj().model.get(agent_key_ctrl.getBtnObj().currentIndex).modelUrlMedium,
                                            "modelUrlSymbols":agent_key_ctrl.getBtnObj().model.get(agent_key_ctrl.getBtnObj().currentIndex).modelUrlSymbols,
                                            "modelUrlFat":agent_key_ctrl.getBtnObj().model.get(agent_key_ctrl.getBtnObj().currentIndex).modelUrlFat});
                agents_listView.currentIndex = curindex
                dialog.close()

                editTargetAgent(curindex)
            }
        }
    }

    Connections {
        target: new_btn.getBtnObj()
        function onClicked() {
            console.log("Cancel button clicked")
            dialog.open()

            var treeModel =[]
            var agentkeysArray = parentWidget.getAgentKeysArray();
            for (var i = 0; i < agentkeysArray.length; i++)
            {
                var item ={}
                item["index"] = i
                item["modelData"] = agentkeysArray[i]["agentName"]+" ("+agentkeysArray[i]["agentKey"]+")"
                item["agentKey"] = agentkeysArray[i]["agentKey"]
                item["agentId"] = agentkeysArray[i]["agentId"]
                item["agentKeyword"] = agentkeysArray[i]["agentKeyword"]
                item["agentName"] = agentkeysArray[i]["agentName"]
                item["agentNameI18n"] = agentkeysArray[i]["agentNameI18n"]
                item["agentType"] = agentkeysArray[i]["agentType"]
                item["modelUrlSlim"] = agentkeysArray[i]["modelUrlSlim"]
                item["modelUrlFat"] = agentkeysArray[i]["modelUrlFat"]
                item["modelUrlMedium"] = agentkeysArray[i]["modelUrlMedium"]
                item["modelUrlSymbols"] = agentkeysArray[i]["modelUrlSymbols"]
                treeModel.push(item)
            }
            agent_key_ctrl.updateData(treeModel)

            var agentkey = parentWidget.getAgentKey()
            instagent_key_ctrl.setContext(agentkey)
            instagent_key_ctrl.setEnable(false)

            instagent_name_ctrl.setContext(qsTr("undefined"))
            instagent_namei18n_ctrl.setContext(qsTr("undefined"))
            instagent_desc_ctrl.setContext(qsTr("undefined"))
//            agent_desc_ctrl.setContextWrapMode(true)

            // 这里可以处理按钮点击后的逻辑
            //var component = Qt.createComponent("addnewagent.qml");
            //var instance = component.createObject(parent);
        }
    }

}
