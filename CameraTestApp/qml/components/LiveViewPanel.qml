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
    property bool liveViewActive: camera && camera.liveViewActive

    // Live view update timer
    Timer {
        id: liveViewTimer
        interval: 33 // ~30 fps
        repeat: true
        running: liveViewActive

        onTriggered: {
            if (camera && camera.liveViewActive) {
                var image = camera.downloadLiveViewImage()
                if (image) {
                    liveViewImage.source = ""
                    imageProvider.setImage(image)
                    liveViewImage.source = "image://live/frame"
                }
            }
        }
    }

    ColumnLayout {
        anchors.fill: parent
        anchors.margins: 10
        spacing: 10

        // Title bar
        RowLayout {
            Layout.fillWidth: true

            Label {
                text: "Live View"
                font.bold: true
                font.pixelSize: 16
            }

            Item { Layout.fillWidth: true }

            Label {
                text: liveViewActive ? "Active" : "Inactive"
                color: liveViewActive ? Material.color(Material.Green) : Material.color(Material.Grey)
            }
        }

        // Live view display area
        Rectangle {
            Layout.fillWidth: true
            Layout.fillHeight: true
            color: "#000000"
            radius: 3

            // Placeholder when no live view
            Label {
                anchors.centerIn: parent
                text: {
                    if (!isConnected) return "Connect camera to start"
                    if (!liveViewActive) return "Press 'Start Live View' to begin"
                    return ""
                }
                color: Material.color(Material.Grey)
                font.pixelSize: 14
                visible: !liveViewActive
            }

            // Live view image
            Image {
                id: liveViewImage
                anchors.fill: parent
                anchors.margins: 2
                fillMode: Image.PreserveAspectFit
                cache: false
                visible: liveViewActive
            }

            // Overlay info
            Rectangle {
                anchors.bottom: parent.bottom
                anchors.left: parent.left
                anchors.right: parent.right
                height: 30
                color: Qt.rgba(0, 0, 0, 0.6)
                visible: liveViewActive

                RowLayout {
                    anchors.fill: parent
                    anchors.margins: 5
                    spacing: 15

                    Label {
                        text: isConnected ? propertyHelper.isoToString(camera.iso) : ""
                        font.pixelSize: 12
                    }
                    Label {
                        text: isConnected ? propertyHelper.apertureToString(camera.aperture) : ""
                        font.pixelSize: 12
                    }
                    Label {
                        text: isConnected ? propertyHelper.shutterSpeedToString(camera.shutterSpeed) : ""
                        font.pixelSize: 12
                    }
                    Item { Layout.fillWidth: true }
                    Label {
                        text: isConnected ? propertyHelper.batteryLevelToString(camera.batteryLevel) : ""
                        font.pixelSize: 12
                    }
                }
            }
        }

        // Live view controls
        RowLayout {
            Layout.fillWidth: true
            spacing: 10

            Button {
                text: liveViewActive ? "Stop Live View" : "Start Live View"
                enabled: isConnected
                Layout.fillWidth: true
                highlighted: liveViewActive

                onClicked: {
                    if (liveViewActive) {
                        camera.stopLiveView()
                    } else {
                        camera.startLiveView()
                    }
                }
            }
        }
    }
}
