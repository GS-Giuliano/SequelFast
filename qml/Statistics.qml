import QtQuick
import QtQuick.Controls.Basic
import QtQuick.Layouts

Item {
    id: root
    width: 480
    height: 600

    readonly property color bgColor: omarchyPalette.background
    readonly property color panelColor: omarchyPalette.panel
    readonly property color fieldBgColor: omarchyPalette.field
    readonly property color textColor: omarchyPalette.foreground
    readonly property color mutedTextColor: omarchyPalette.disabledText
    readonly property color borderColor: omarchyPalette.selection
    readonly property color accentColor: omarchyPalette.accent

    Rectangle {
        anchors.fill: parent
        color: root.bgColor
    }

    ColumnLayout {
        anchors.fill: parent
        anchors.margins: 12
        spacing: 10

        Label {
            text: backend.title
            color: root.textColor
            font.bold: true
            font.pointSize: 12
        }

        // Schema metadata, laid out as small info cards (quickshell-style tiles)
        // rather than a form of read-only line edits.
        GridLayout {
            Layout.fillWidth: true
            columns: 3
            columnSpacing: 8
            rowSpacing: 8

            Repeater {
                model: [
                    { label: qsTr("Schema"), value: backend.schemaName },
                    { label: qsTr("Char set"), value: backend.charSet },
                    { label: qsTr("Collation"), value: backend.collation },
                    { label: qsTr("Encryption"), value: backend.encryption },
                    { label: qsTr("Size"), value: backend.size },
                    { label: qsTr("Tables"), value: backend.tableCount },
                ]

                delegate: Rectangle {
                    Layout.fillWidth: true
                    Layout.preferredHeight: 56
                    radius: 8
                    color: root.panelColor
                    border.width: 1
                    border.color: root.borderColor

                    ColumnLayout {
                        anchors.fill: parent
                        anchors.margins: 8
                        spacing: 2

                        Label {
                            text: modelData.label
                            color: root.mutedTextColor
                            font.pointSize: 8
                        }
                        Label {
                            text: modelData.value || "—"
                            color: root.textColor
                            font.pointSize: 9
                            font.bold: true
                            elide: Text.ElideRight
                            Layout.fillWidth: true
                        }
                    }
                }
            }
        }

        RowLayout {
            Layout.fillWidth: true
            spacing: 8

            Label {
                text: qsTr("Environment")
                color: root.textColor
                font.bold: true
            }
            Item { Layout.fillWidth: true }
            Label {
                text: qsTr("%1 variables").arg(backend.variablesModel.count)
                color: root.mutedTextColor
                font.pointSize: 8
            }
        }

        TextField {
            id: filterField
            Layout.fillWidth: true
            placeholderText: qsTr("Filter")
            color: root.textColor
            onTextEdited: backend.variablesModel.filterText = text
            background: Rectangle {
                color: root.fieldBgColor
                border.color: root.borderColor
                radius: 4
            }
        }

        ListView {
            id: variablesList
            Layout.fillWidth: true
            Layout.fillHeight: true
            clip: true
            model: backend.variablesModel
            ScrollBar.vertical: ScrollBar {}

            delegate: Rectangle {
                width: variablesList.width
                height: 40
                color: index % 2 === 0 ? "transparent" : root.panelColor

                MouseArea {
                    anchors.fill: parent
                    onClicked: valueDialog.showValue(varName, varValue)
                }

                RowLayout {
                    anchors.fill: parent
                    anchors.margins: 8
                    spacing: 8

                    Label {
                        text: varName
                        color: root.textColor
                        Layout.preferredWidth: parent.width * 0.4
                        elide: Text.ElideRight
                    }
                    Label {
                        text: varValue
                        color: root.mutedTextColor
                        Layout.fillWidth: true
                        elide: Text.ElideRight
                    }
                }
            }
        }

        RowLayout {
            Layout.fillWidth: true
            Item { Layout.fillWidth: true }
            Button {
                text: qsTr("Close")
                font.pointSize: 9
                padding: 4
                onClicked: backend.onClose()
            }
        }
    }

    Dialog {
        id: valueDialog
        property alias valueText: valueLabel.text
        title: qsTr("Value")
        modal: true
        anchors.centerIn: parent
        standardButtons: Dialog.Ok

        function showValue(name, value) {
            valueDialog.title = name
            valueText = value
            open()
        }

        contentItem: Label {
            id: valueLabel
            wrapMode: Text.WordWrap
            color: root.textColor
        }
    }
}
