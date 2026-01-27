import QtQuick
import QtQuick.Controls
import QtQuick.Layouts
import QtQuick.Controls.Material
import CameraTest 1.0

import "components"

ApplicationWindow {
    id: window
    visible: true
    width: 1280
    height: 800
    minimumWidth: 1024
    minimumHeight: 600
    title: qsTr("Canon Camera Test App")

    Material.theme: Material.Dark
    Material.accent: Material.Orange

    // Status message
    property string statusMessage: ""

    Component.onCompleted: {
        // Initialize SDK on startup
        if (cameraManager.initializeSDK()) {
            statusMessage = "SDK initialized successfully"
        } else {
            statusMessage = "Failed to initialize SDK"
        }
    }

    // Error handling
    Connections {
        target: cameraManager
        function onError(message) {
            errorPopup.text = message
            errorPopup.open()
        }
    }

    Connections {
        target: cameraManager.currentCamera
        function onError(message) {
            errorPopup.text = message
            errorPopup.open()
        }
        function onPictureDownloaded(path) {
            statusMessage = "Picture saved: " + path
        }
    }

    // Main layout
    ColumnLayout {
        anchors.fill: parent
        anchors.margins: 10
        spacing: 10

        // Top bar: Camera selection and connection
        CameraSelector {
            id: cameraSelector
            Layout.fillWidth: true
            Layout.preferredHeight: 60
        }

        // Main content area
        RowLayout {
            Layout.fillWidth: true
            Layout.fillHeight: true
            spacing: 10

            // Left panel: Properties
            PropertyPanel {
                id: propertyPanel
                Layout.preferredWidth: 300
                Layout.fillHeight: true
            }

            // Center: Live View
            LiveViewPanel {
                id: liveViewPanel
                Layout.fillWidth: true
                Layout.fillHeight: true
            }

            // Right panel: Shooting controls
            ShootingControls {
                id: shootingControls
                Layout.preferredWidth: 250
                Layout.fillHeight: true
            }
        }

        // Bottom: Status bar
        StatusBar {
            id: statusBar
            Layout.fillWidth: true
            Layout.preferredHeight: 30
            message: statusMessage
        }
    }

    // Error popup
    Popup {
        id: errorPopup
        anchors.centerIn: parent
        width: 400
        height: 150
        modal: true
        closePolicy: Popup.CloseOnEscape | Popup.CloseOnPressOutside

        property alias text: errorText.text

        ColumnLayout {
            anchors.fill: parent
            anchors.margins: 20
            spacing: 20

            Label {
                text: "Error"
                font.bold: true
                font.pixelSize: 18
                color: Material.color(Material.Red)
            }

            Label {
                id: errorText
                Layout.fillWidth: true
                wrapMode: Text.WordWrap
            }

            Button {
                text: "OK"
                Layout.alignment: Qt.AlignRight
                onClicked: errorPopup.close()
            }
        }
    }
}
