import QtQuick
import QtQuick.Controls
import QtQuick.Layouts
import QtQuick.Controls.Material

Rectangle {
    id: root
    color: Material.color(Material.Grey, Material.Shade900)
    radius: 5

    RowLayout {
        anchors.fill: parent
        anchors.margins: 10
        spacing: 15

        Label {
            text: "Camera:"
            font.pixelSize: 14
        }

        ComboBox {
            id: cameraCombo
            Layout.preferredWidth: 300
            model: cameraManager.cameras
            textRole: "name"
            currentIndex: cameraManager.currentIndex

            onCurrentIndexChanged: {
                if (currentIndex >= 0) {
                    cameraManager.setCurrentIndex(currentIndex)
                }
            }

            delegate: ItemDelegate {
                width: cameraCombo.width
                text: modelData.name
                highlighted: cameraCombo.highlightedIndex === index
            }
        }

        Button {
            text: "Refresh"
            icon.source: "qrc:/icons/refresh.svg"
            onClicked: cameraManager.refreshCameraList()
        }

        Rectangle {
            width: 1
            Layout.fillHeight: true
            Layout.topMargin: 5
            Layout.bottomMargin: 5
            color: Material.color(Material.Grey)
        }

        Button {
            id: connectBtn
            text: cameraManager.currentCamera && cameraManager.currentCamera.connected ? "Disconnect" : "Connect"
            highlighted: cameraManager.currentCamera && cameraManager.currentCamera.connected
            enabled: cameraManager.currentCamera !== null

            onClicked: {
                if (cameraManager.currentCamera.connected) {
                    cameraManager.disconnectCurrentCamera()
                } else {
                    cameraManager.connectCurrentCamera()
                }
            }
        }

        // Connection status indicator
        Rectangle {
            width: 16
            height: 16
            radius: 8
            color: {
                if (!cameraManager.currentCamera) return Material.color(Material.Grey)
                return cameraManager.currentCamera.connected ?
                    Material.color(Material.Green) :
                    Material.color(Material.Red)
            }

            ToolTip.visible: statusMouseArea.containsMouse
            ToolTip.text: {
                if (!cameraManager.currentCamera) return "No camera selected"
                return cameraManager.currentCamera.connected ? "Connected" : "Disconnected"
            }

            MouseArea {
                id: statusMouseArea
                anchors.fill: parent
                hoverEnabled: true
            }
        }

        Item { Layout.fillWidth: true }

        // Camera info
        Label {
            visible: cameraManager.currentCamera && cameraManager.currentCamera.connected
            text: cameraManager.currentCamera ? cameraManager.currentCamera.name : ""
            font.pixelSize: 12
            color: Material.color(Material.Grey, Material.Shade400)
        }
    }
}
