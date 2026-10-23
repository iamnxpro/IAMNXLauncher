import QtQuick

// An instance in the lower orbital arc. Initial + hue keeps every instance
// recognizable without loading legacy theme icons into the QML renderer.
Item {
    id: node

    property string instanceId: ""
    property string instanceName: ""
    property string letter: "?"
    property int hue: 20
    property bool selected: false
    property bool running: false

    signal activated()

    width: 92
    height: 82
    scale: hoverArea.containsMouse ? 1.1 : (selected ? 1.05 : 1.0)
    Behavior on scale { NumberAnimation { duration: 150; easing.type: Easing.OutCubic } }

    Accessible.role: Accessible.Button
    Accessible.name: instanceName

    Rectangle {
        anchors.horizontalCenter: parent.horizontalCenter
        y: 1
        width: 60
        height: 60
        radius: 30
        color: selected ? "#2A140C" : "#130C09"
        border.width: selected ? 2 : 1
        border.color: selected ? "#FF5E1A" : (hoverArea.containsMouse ? "#A85327" : "#38231A")

        Rectangle {
            anchors.centerIn: parent
            width: 48
            height: 48
            radius: 24
            color: Qt.hsla(node.hue / 360.0, 0.48, 0.26, 1.0)
            Text {
                anchors.centerIn: parent
                text: node.letter
                color: "#FFF3EA"
                font.family: "Segoe UI"
                font.pixelSize: 19
                font.weight: Font.Black
            }
        }

        Rectangle {
            visible: node.running
            anchors.right: parent.right
            anchors.top: parent.top
            anchors.rightMargin: 2
            anchors.topMargin: 2
            width: 9
            height: 9
            radius: 5
            color: "#7BE38B"
            border.width: 2
            border.color: "#130C09"
        }
    }

    Text {
        anchors.horizontalCenter: parent.horizontalCenter
        anchors.top: parent.top
        anchors.topMargin: 64
        width: 86
        text: node.instanceName
        color: node.selected ? "#FFB27A" : "#BDA99D"
        font.family: "Segoe UI"
        font.pixelSize: 9
        font.weight: node.selected ? Font.DemiBold : Font.Normal
        horizontalAlignment: Text.AlignHCenter
        elide: Text.ElideRight
    }

    MouseArea {
        id: hoverArea
        anchors.fill: parent
        hoverEnabled: true
        cursorShape: Qt.PointingHandCursor
        onClicked: node.activated()
    }
}
