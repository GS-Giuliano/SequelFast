import QtQuick
import QtQuick.Controls.Basic
import QtQuick.Layouts

Item {
    id: root
    width: 480
    height: 560

    readonly property bool dark: omarchyPalette.dark
    readonly property color bgColor: omarchyPalette.background
    readonly property color fieldBgColor: omarchyPalette.field
    readonly property color textColor: omarchyPalette.foreground
    readonly property color borderColor: omarchyPalette.selection
    readonly property color accentColor: omarchyPalette.accent

    Rectangle {
        anchors.fill: parent
        color: root.bgColor
    }

    Connections {
        target: backend
        function onConnectionFailed(message) {
            errorDialog.text = message
            errorDialog.open()
        }
        function onConnectionSucceeded() {
            errorDialog.text = qsTr("Connected successfully!")
            errorDialog.open()
        }
    }

    Dialog {
        id: errorDialog
        property alias text: errorLabel.text
        title: qsTr("Connection")
        modal: true
        anchors.centerIn: parent
        standardButtons: Dialog.Ok
        contentItem: Label {
            id: errorLabel
            wrapMode: Text.WordWrap
            color: root.textColor
        }
    }

    ColumnLayout {
        anchors.fill: parent
        anchors.margins: 12
        spacing: 8

        RowLayout {
            Layout.fillWidth: true
            spacing: 8

            Label { text: qsTr("Name"); color: root.textColor }
            TextField {
                Layout.fillWidth: true
                text: backend.name
                onTextEdited: backend.name = text
                background: Rectangle { color: root.fieldBgColor; border.color: root.borderColor; radius: 4 }
                color: root.textColor
            }
        }

        TabBar {
            id: tabBar
            Layout.fillWidth: true
            TabButton { text: qsTr("Connection") }
            TabButton { text: qsTr("SSH") }
        }

        StackLayout {
            Layout.fillWidth: true
            Layout.fillHeight: true
            currentIndex: tabBar.currentIndex

            GridLayout {
                columns: 2
                columnSpacing: 8
                rowSpacing: 6

                Label { text: qsTr("Host"); color: root.textColor }
                TextField {
                    Layout.fillWidth: true
                    text: backend.host
                    placeholderText: "127.0.0.1"
                    onTextEdited: backend.host = text
                    background: Rectangle { color: root.fieldBgColor; border.color: root.borderColor; radius: 4 }
                    color: root.textColor
                }

                Label { text: qsTr("Port"); color: root.textColor }
                TextField {
                    Layout.fillWidth: true
                    text: backend.port
                    placeholderText: "3306"
                    onTextEdited: backend.port = text
                    background: Rectangle { color: root.fieldBgColor; border.color: root.borderColor; radius: 4 }
                    color: root.textColor
                }

                Label { text: qsTr("Username"); color: root.textColor }
                TextField {
                    Layout.fillWidth: true
                    text: backend.user
                    placeholderText: "root"
                    onTextEdited: backend.user = text
                    background: Rectangle { color: root.fieldBgColor; border.color: root.borderColor; radius: 4 }
                    color: root.textColor
                }

                Label { text: qsTr("Password"); color: root.textColor }
                TextField {
                    Layout.fillWidth: true
                    text: backend.pass
                    echoMode: TextInput.Password
                    onTextEdited: backend.pass = text
                    background: Rectangle { color: root.fieldBgColor; border.color: root.borderColor; radius: 4 }
                    color: root.textColor
                }

                Label { text: qsTr("Schema"); color: root.textColor }
                TextField {
                    Layout.fillWidth: true
                    text: backend.schema
                    onTextEdited: backend.schema = text
                    background: Rectangle { color: root.fieldBgColor; border.color: root.borderColor; radius: 4 }
                    color: root.textColor
                }

                Label { text: qsTr("Color"); color: root.textColor; Layout.columnSpan: 2 }
                Flow {
                    Layout.columnSpan: 2
                    Layout.fillWidth: true
                    spacing: 6

                    Repeater {
                        model: backend.colorList
                        delegate: Rectangle {
                            width: 26
                            height: 26
                            radius: 13
                            color: modelData.rgb
                            border.width: index === backend.colorIndex ? 3 : 1
                            border.color: index === backend.colorIndex ? root.accentColor : root.borderColor

                            MouseArea {
                                anchors.fill: parent
                                onClicked: backend.colorIndex = index
                            }
                        }
                    }
                }
            }

            GridLayout {
                columns: 2
                columnSpacing: 8
                rowSpacing: 6

                Label { text: qsTr("Host"); color: root.textColor }
                TextField {
                    Layout.fillWidth: true
                    text: backend.sshHost
                    onTextEdited: backend.sshHost = text
                    background: Rectangle { color: root.fieldBgColor; border.color: root.borderColor; radius: 4 }
                    color: root.textColor
                }

                Label { text: qsTr("Port"); color: root.textColor }
                TextField {
                    Layout.fillWidth: true
                    text: backend.sshPort
                    onTextEdited: backend.sshPort = text
                    background: Rectangle { color: root.fieldBgColor; border.color: root.borderColor; radius: 4 }
                    color: root.textColor
                }

                Label { text: qsTr("Username"); color: root.textColor }
                TextField {
                    Layout.fillWidth: true
                    text: backend.sshUser
                    onTextEdited: backend.sshUser = text
                    background: Rectangle { color: root.fieldBgColor; border.color: root.borderColor; radius: 4 }
                    color: root.textColor
                }

                Label { text: qsTr("Password"); color: root.textColor }
                TextField {
                    Layout.fillWidth: true
                    text: backend.sshPass
                    echoMode: TextInput.Password
                    onTextEdited: backend.sshPass = text
                    background: Rectangle { color: root.fieldBgColor; border.color: root.borderColor; radius: 4 }
                    color: root.textColor
                }

                Label { text: qsTr("Key file"); color: root.textColor }
                TextField {
                    Layout.fillWidth: true
                    text: backend.sshKeyFile
                    onTextEdited: backend.sshKeyFile = text
                    background: Rectangle { color: root.fieldBgColor; border.color: root.borderColor; radius: 4 }
                    color: root.textColor
                }
            }
        }

        CheckBox {
            text: qsTr("Save shared favorites here")
            checked: backend.shared
            onToggled: backend.shared = checked
            contentItem: Label {
                text: qsTr("Save shared favorites here")
                color: root.textColor
                leftPadding: 24
                verticalAlignment: Text.AlignVCenter
            }
        }

        RowLayout {
            Layout.fillWidth: true
            spacing: 8

            Button {
                text: qsTr("Save")
                highlighted: true
                font.pointSize: 9
                padding: 4
                onClicked: backend.onSave()
            }
            Button {
                text: qsTr("Connect")
                font.pointSize: 9
                padding: 4
                onClicked: backend.onConnect()
            }
            Button {
                text: qsTr("Remove")
                font.pointSize: 9
                padding: 4
                onClicked: backend.onRemove()
            }
            Item { Layout.fillWidth: true }
            Button {
                text: qsTr("Cancel")
                font.pointSize: 9
                padding: 4
                onClicked: backend.onCancel()
            }
        }
    }
}
