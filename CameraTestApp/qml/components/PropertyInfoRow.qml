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

    Label {
        text: root.label + ":"
        Layout.preferredWidth: 80
        color: Material.color(Material.Grey, Material.Shade400)
    }

    Label {
        text: root.value
        Layout.fillWidth: true
        color: Material.foreground
    }
}
