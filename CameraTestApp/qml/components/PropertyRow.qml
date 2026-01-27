import QtQuick
import QtQuick.Controls
import QtQuick.Layouts
import QtQuick.Controls.Material

RowLayout {
    id: root
    Layout.fillWidth: true
    spacing: 10

    property string label: ""
    property string value: "-"
    property var values: []
    property int currentValue: 0
    property bool enabled: true

    signal valueSelected(int val)

    Label {
        text: root.label + ":"
        Layout.preferredWidth: 80
        color: root.enabled ? Material.foreground : Material.color(Material.Grey)
    }

    Label {
        text: root.value
        Layout.fillWidth: true
        font.bold: true
        color: root.enabled ? Material.accent : Material.color(Material.Grey)
    }

    Button {
        text: "..."
        Layout.preferredWidth: 40
        enabled: root.enabled && root.values.length > 0
        flat: true

        onClicked: {
            valueMenu.open()
        }

        Menu {
            id: valueMenu
            y: parent.height

            Repeater {
                model: root.values
                MenuItem {
                    text: getDisplayText(modelData)
                    highlighted: modelData === root.currentValue
                    onTriggered: root.valueSelected(modelData)
                }
            }
        }
    }

    function getDisplayText(val) {
        // Try to get a display string based on the label
        switch (root.label) {
            case "ISO": return propertyHelper.isoToString(val)
            case "Aperture": return propertyHelper.apertureToString(val)
            case "Shutter": return propertyHelper.shutterSpeedToString(val)
            case "Exp. Comp": return propertyHelper.exposureCompToString(val)
            case "WB": return propertyHelper.whiteBalanceToString(val)
            case "Drive": return propertyHelper.driveModeToString(val)
            case "Quality": return propertyHelper.imageQualityToString(val)
            default: return val.toString()
        }
    }
}
