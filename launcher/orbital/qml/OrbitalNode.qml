import QtQuick

// One node in the radial navigation ring.
Item {
    id: node

    property string icon: ""
    property string label: ""
    property bool active: false
    property bool interactive: true
    property bool running: false

    signal activated()

    width: 108
    height: 92
    opacity: interactive ? 1.0 : 0.42
    scale: hoverArea.containsMouse && interactive ? 1.08 : 1.0
    Behavior on scale { NumberAnimation { duration: 160; easing.type: Easing.OutCubic } }

    Accessible.role: Accessible.Button
    Accessible.name: label

    Rectangle {
        id: halo
        anchors.horizontalCenter: parent.horizontalCenter
        y: 4
        width: 68
        height: 68
        radius: 34
        color: "transparent"
        border.width: node.active || hoverArea.containsMouse ? 1.5 : 0
        border.color: node.active ? "#FF5E1A" : "#66FF5E1A"
        opacity: node.active ? 0.85 : 0.55
    }

    Rectangle {
        anchors.horizontalCenter: parent.horizontalCenter
        y: 8
        width: 60
        height: 60
        radius: 30
        color: hoverArea.containsMouse ? "#26130C" : "#160D09"
        border.width: 1
        border.color: node.active ? "#FF5E1A" : (hoverArea.containsMouse ? "#A85327" : "#39231A")
        Behavior on color { ColorAnimation { duration: 150 } }
        Behavior on border.color { ColorAnimation { duration: 150 } }

        Image {
            anchors.centerIn: parent
            width: 28
            height: 28
            source: node.icon
            fillMode: Image.PreserveAspectFit
            opacity: node.interactive ? 0.94 : 0.55
            smooth: true
        }

        Rectangle {
            visible: node.running
            anchors.right: parent.right
            anchors.top: parent.top
            anchors.rightMargin: 4
            anchors.topMargin: 4
            width: 10
            height: 10
            radius: 5
            color: "#7BE38B"
            border.width: 2
            border.color: "#160D09"
        }
    }

    Text {
        anchors.horizontalCenter: parent.horizontalCenter
        anchors.top: parent.top
        anchors.topMargin: 72
        width: parent.width
        text: node.label
        color: node.active ? "#FFB27A" : "#D9CEC8"
        font.family: "Segoe UI"
        font.pixelSize: 11
        font.weight: Font.DemiBold
        horizontalAlignment: Text.AlignHCenter
        elide: Text.ElideRight
        wrapMode: Text.WordWrap
        maximumLineCount: 2
    }

    MouseArea {
        id: hoverArea
        anchors.fill: parent
        hoverEnabled: true
        enabled: node.interactive
        cursorShape: node.interactive ? Qt.PointingHandCursor : Qt.ArrowCursor
        onClicked: node.activated()
    }
}
