import QtQuick
import QtQuick.Controls
import QtQuick.Layouts
import QtQuick.Controls.Material

Rectangle {
    id: root
    color: Material.color(Material.Grey, Material.Shade800)
    radius: 3

    property string message: ""

    RowLayout {
        anchors.fill: parent
        anchors.leftMargin: 10
        anchors.rightMargin: 10
        spacing: 15

        // Status message
        Label {
            id: messageLabel
            text: root.message
            font.pixelSize: 12
            elide: Text.ElideRight
            Layout.fillWidth: true

            Behavior on text {
                SequentialAnimation {
                    PropertyAnimation { target: messageLabel; property: "opacity"; to: 0; duration: 100 }
                    PropertyAction { target: messageLabel; property: "text" }
                    PropertyAnimation { target: messageLabel; property: "opacity"; to: 1; duration: 100 }
                }
            }
        }

        // SDK status
        Rectangle {
            width: 8
            height: 8
            radius: 4
            color: cameraManager.sdkInitialized ?
                Material.color(Material.Green) :
                Material.color(Material.Red)

            ToolTip.visible: sdkStatusMouse.containsMouse
            ToolTip.text: cameraManager.sdkInitialized ? "SDK Initialized" : "SDK Not Initialized"

            MouseArea {
                id: sdkStatusMouse
                anchors.fill: parent
                hoverEnabled: true
            }
        }

        Label {
            text: "SDK"
            font.pixelSize: 10
            color: Material.color(Material.Grey, Material.Shade400)
        }

        // Separator
        Rectangle {
            width: 1
            height: parent.height - 10
            color: Material.color(Material.Grey, Material.Shade600)
        }

        // Camera count
        Label {
            text: cameraManager.cameras.length + " camera(s)"
            font.pixelSize: 12
            color: Material.color(Material.Grey, Material.Shade400)
        }

        // Separator
        Rectangle {
            width: 1
            height: parent.height - 10
            color: Material.color(Material.Grey, Material.Shade600)
        }

        // Current time
        Label {
            id: timeLabel
            font.pixelSize: 12
            color: Material.color(Material.Grey, Material.Shade400)

            Timer {
                interval: 1000
                running: true
                repeat: true
                triggeredOnStart: true
                onTriggered: {
                    timeLabel.text = Qt.formatDateTime(new Date(), "hh:mm:ss")
                }
            }
        }
    }
}
