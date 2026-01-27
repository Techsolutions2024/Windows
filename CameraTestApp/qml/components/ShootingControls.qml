import QtQuick
import QtQuick.Controls
import QtQuick.Layouts
import QtQuick.Controls.Material

Rectangle {
    id: root
    color: Material.color(Material.Grey, Material.Shade900)
    radius: 5

    property var camera: cameraManager.currentCamera
    property bool isConnected: camera && camera.connected

    ColumnLayout {
        anchors.fill: parent
        anchors.margins: 10
        spacing: 15

        Label {
            text: "Shooting Controls"
            font.bold: true
            font.pixelSize: 16
        }

        // Main capture button
        Button {
            id: captureBtn
            Layout.fillWidth: true
            Layout.preferredHeight: 80
            text: "Take Picture"
            enabled: isConnected
            highlighted: true

            background: Rectangle {
                color: captureBtn.down ? Material.color(Material.Orange, Material.Shade700) :
                       captureBtn.hovered ? Material.color(Material.Orange, Material.Shade400) :
                       Material.color(Material.Orange)
                radius: 5
            }

            contentItem: Label {
                text: captureBtn.text
                font.pixelSize: 18
                font.bold: true
                color: "white"
                horizontalAlignment: Text.AlignHCenter
                verticalAlignment: Text.AlignVCenter
            }

            onClicked: {
                cameraManager.takePicture()
            }
        }

        Rectangle {
            Layout.fillWidth: true
            height: 1
            color: Material.color(Material.Grey, Material.Shade700)
        }

        Label {
            text: "Shutter Control"
            font.bold: true
            font.pixelSize: 14
            color: Material.color(Material.Grey, Material.Shade400)
        }

        // Shutter button states
        GridLayout {
            Layout.fillWidth: true
            columns: 2
            rowSpacing: 10
            columnSpacing: 10

            Button {
                Layout.fillWidth: true
                text: "Half Press"
                enabled: isConnected
                onClicked: camera.pressShutterButton(1)
            }

            Button {
                Layout.fillWidth: true
                text: "Full Press"
                enabled: isConnected
                onClicked: camera.pressShutterButton(2)
            }

            Button {
                Layout.fillWidth: true
                Layout.columnSpan: 2
                text: "Release"
                enabled: isConnected
                onClicked: camera.pressShutterButton(0)
            }
        }

        Rectangle {
            Layout.fillWidth: true
            height: 1
            color: Material.color(Material.Grey, Material.Shade700)
        }

        Label {
            text: "Save Location"
            font.bold: true
            font.pixelSize: 14
            color: Material.color(Material.Grey, Material.Shade400)
        }

        // Save to options
        ColumnLayout {
            Layout.fillWidth: true
            spacing: 5

            RadioButton {
                id: saveToHost
                text: "Save to PC"
                checked: true
                enabled: isConnected
                onClicked: camera.setSaveTo(2)
            }

            RadioButton {
                id: saveToCamera
                text: "Save to Camera"
                enabled: isConnected
                onClicked: camera.setSaveTo(1)
            }

            RadioButton {
                id: saveToBoth
                text: "Save to Both"
                enabled: isConnected
                onClicked: camera.setSaveTo(3)
            }
        }

        Rectangle {
            Layout.fillWidth: true
            height: 1
            color: Material.color(Material.Grey, Material.Shade700)
        }

        Label {
            text: "Quick Actions"
            font.bold: true
            font.pixelSize: 14
            color: Material.color(Material.Grey, Material.Shade400)
        }

        // Quick actions
        GridLayout {
            Layout.fillWidth: true
            columns: 2
            rowSpacing: 8
            columnSpacing: 8

            Button {
                Layout.fillWidth: true
                text: "Auto Focus"
                enabled: isConnected
                onClicked: {
                    camera.pressShutterButton(1) // Half press for AF
                }
            }

            Button {
                Layout.fillWidth: true
                text: "Burst Mode"
                enabled: isConnected
                onClicked: {
                    // Set drive mode to continuous
                    camera.setDriveMode(0x01)
                }
            }

            Button {
                Layout.fillWidth: true
                text: "Single Shot"
                enabled: isConnected
                onClicked: {
                    camera.setDriveMode(0x00)
                }
            }

            Button {
                Layout.fillWidth: true
                text: "Self Timer"
                enabled: isConnected
                onClicked: {
                    camera.setDriveMode(0x10) // 2 sec timer
                }
            }
        }

        Item { Layout.fillHeight: true }

        // Camera status
        Rectangle {
            Layout.fillWidth: true
            Layout.preferredHeight: 60
            color: Material.color(Material.Grey, Material.Shade800)
            radius: 3

            ColumnLayout {
                anchors.fill: parent
                anchors.margins: 8
                spacing: 4

                Label {
                    text: "Status"
                    font.pixelSize: 10
                    color: Material.color(Material.Grey, Material.Shade400)
                }

                Label {
                    text: {
                        if (!cameraManager.currentCamera) return "No camera"
                        if (!isConnected) return "Disconnected"
                        return camera.name
                    }
                    font.pixelSize: 12
                    elide: Text.ElideRight
                    Layout.fillWidth: true
                }

                Label {
                    text: isConnected ? "Shots available: " + camera.availableShots : ""
                    font.pixelSize: 10
                    color: Material.color(Material.Grey, Material.Shade400)
                    visible: isConnected
                }
            }
        }
    }
}
