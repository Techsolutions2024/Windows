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

    ScrollView {
        anchors.fill: parent
        anchors.margins: 10
        contentWidth: availableWidth
        clip: true

        ColumnLayout {
            width: parent.width
            spacing: 15

            Label {
                text: "Camera Properties"
                font.bold: true
                font.pixelSize: 16
            }

            // ISO
            PropertyRow {
                label: "ISO"
                value: isConnected ? propertyHelper.isoToString(camera.iso) : "-"
                values: isConnected ? camera.isoValues : []
                currentValue: isConnected ? camera.iso : 0
                enabled: isConnected
                onValueSelected: function(val) { camera.setIso(val) }
            }

            // Aperture
            PropertyRow {
                label: "Aperture"
                value: isConnected ? propertyHelper.apertureToString(camera.aperture) : "-"
                values: isConnected ? camera.apertureValues : []
                currentValue: isConnected ? camera.aperture : 0
                enabled: isConnected
                onValueSelected: function(val) { camera.setAperture(val) }
            }

            // Shutter Speed
            PropertyRow {
                label: "Shutter"
                value: isConnected ? propertyHelper.shutterSpeedToString(camera.shutterSpeed) : "-"
                values: isConnected ? camera.shutterSpeedValues : []
                currentValue: isConnected ? camera.shutterSpeed : 0
                enabled: isConnected
                onValueSelected: function(val) { camera.setShutterSpeed(val) }
            }

            // Exposure Compensation
            PropertyRow {
                label: "Exp. Comp"
                value: isConnected ? propertyHelper.exposureCompToString(camera.exposureCompensation) : "-"
                values: [] // Manual entry often needed
                currentValue: isConnected ? camera.exposureCompensation : 0
                enabled: isConnected
                onValueSelected: function(val) { camera.setExposureCompensation(val) }
            }

            // White Balance
            PropertyRow {
                label: "WB"
                value: isConnected ? propertyHelper.whiteBalanceToString(camera.whiteBalance) : "-"
                values: isConnected ? camera.whiteBalanceValues : []
                currentValue: isConnected ? camera.whiteBalance : 0
                enabled: isConnected
                onValueSelected: function(val) { camera.setWhiteBalance(val) }
            }

            // Drive Mode
            PropertyRow {
                label: "Drive"
                value: isConnected ? propertyHelper.driveModeToString(camera.driveMode) : "-"
                values: isConnected ? camera.driveModeValues : []
                currentValue: isConnected ? camera.driveMode : 0
                enabled: isConnected
                onValueSelected: function(val) { camera.setDriveMode(val) }
            }

            // Image Quality
            PropertyRow {
                label: "Quality"
                value: isConnected ? propertyHelper.imageQualityToString(camera.imageQuality) : "-"
                values: isConnected ? camera.imageQualityValues : []
                currentValue: isConnected ? camera.imageQuality : 0
                enabled: isConnected
                onValueSelected: function(val) { camera.setImageQuality(val) }
            }

            Rectangle {
                Layout.fillWidth: true
                height: 1
                color: Material.color(Material.Grey, Material.Shade700)
            }

            Label {
                text: "Read-Only Properties"
                font.bold: true
                font.pixelSize: 14
                color: Material.color(Material.Grey, Material.Shade400)
            }

            // AE Mode (read only)
            PropertyInfoRow {
                label: "AE Mode"
                value: isConnected ? propertyHelper.aeModeToString(camera.aeMode) : "-"
            }

            // AF Mode (read only)
            PropertyInfoRow {
                label: "AF Mode"
                value: isConnected ? propertyHelper.afModeToString(camera.afMode) : "-"
            }

            // Battery Level (read only)
            PropertyInfoRow {
                label: "Battery"
                value: isConnected ? propertyHelper.batteryLevelToString(camera.batteryLevel) : "-"
            }

            // Available Shots (read only)
            PropertyInfoRow {
                label: "Avail. Shots"
                value: isConnected ? camera.availableShots.toString() : "-"
            }

            Item { Layout.fillHeight: true }

            // Refresh button
            Button {
                Layout.fillWidth: true
                text: "Refresh Properties"
                enabled: isConnected
                onClicked: {
                    camera.refreshProperties()
                    camera.refreshPropertyDescriptors()
                }
            }
        }
    }
}
