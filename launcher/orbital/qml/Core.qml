import QtQuick

// The central reactor core. This is the only prominent call to action.
Item {
    id: core

    property string title: "PLAY"
    property string subtitle: ""
    property string hint: ""
    property bool busy: false
    property bool interactive: true
    property bool containsMouse: hoverArea.containsMouse

    signal activated()
    signal hoveredChanged(bool hovered)

    width: 280
    height: 300

    Item {
        id: pulse
        anchors.horizontalCenter: parent.horizontalCenter
        y: 3
        width: 226
        height: 226

        Rectangle {
            anchors.centerIn: parent
            width: 216
            height: 216
            radius: 108
            color: "transparent"
            border.width: 1
            border.color: "#55FF5E1A"
            opacity: 0.65
        }

        Rectangle {
            anchors.centerIn: parent
            width: 198
            height: 198
            radius: 99
            color: "transparent"
            border.width: 1
            border.color: "#33FF5E1A"
        }

        SequentialAnimation on scale {
            loops: Animation.Infinite
            NumberAnimation { from: 0.92; to: 1.08; duration: 1800; easing.type: Easing.InOutSine }
            NumberAnimation { from: 1.08; to: 0.92; duration: 1800; easing.type: Easing.InOutSine }
        }
        SequentialAnimation on opacity {
            loops: Animation.Infinite
            NumberAnimation { from: 0.55; to: 1.0; duration: 1800; easing.type: Easing.InOutSine }
            NumberAnimation { from: 1.0; to: 0.55; duration: 1800; easing.type: Easing.InOutSine }
        }
    }

    Item {
        id: spinner
        anchors.horizontalCenter: parent.horizontalCenter
        y: -4
        width: 240
        height: 240
        opacity: core.busy ? 1.0 : 0.0
        Behavior on opacity { NumberAnimation { duration: 250 } }

        property real spin: 0
        rotation: spin
        NumberAnimation on spin {
            loops: Animation.Infinite
            from: 0
            to: 360
            duration: core.busy ? 1200 : 9000
        }

        Repeater {
            model: 18
            delegate: Rectangle {
                width: index % 3 === 0 ? 5 : 3
                height: width
                radius: width / 2
                color: index % 3 === 0 ? "#FFD9B8" : "#FF5E1A"
                opacity: 0.25 + (index % 6) * 0.12
                x: 120 + Math.cos(index * Math.PI / 9) * 114 - width / 2
                y: 120 + Math.sin(index * Math.PI / 9) * 114 - height / 2
            }
        }
    }

    Rectangle {
        id: button
        anchors.horizontalCenter: parent.horizontalCenter
        y: 14
        width: 196
        height: 196
        radius: 98
        border.width: core.containsMouse && core.interactive ? 2 : 1
        border.color: core.interactive ? (core.containsMouse ? "#FFB27A" : "#FF5E1A") : "#5C3524"
        scale: core.containsMouse && core.interactive ? 1.04 : 1.0
        opacity: core.interactive ? 1.0 : 0.62

        gradient: Gradient {
            GradientStop { position: 0.0; color: core.containsMouse && core.interactive ? "#3A1A0D" : "#24120B" }
            GradientStop { position: 0.55; color: "#150B08" }
            GradientStop { position: 1.0; color: "#0C0705" }
        }

        Behavior on scale { NumberAnimation { duration: 170; easing.type: Easing.OutCubic } }
        Behavior on border.color { ColorAnimation { duration: 160 } }

        Rectangle {
            anchors.centerIn: parent
            width: 168
            height: 168
            radius: 84
            color: "transparent"
            border.width: 1
            border.color: "#2EFF5E1A"
        }

        Rectangle {
            anchors.centerIn: parent
            width: 144
            height: 144
            radius: 72
            color: "#10FF5E1A"
        }

        Text {
            anchors.horizontalCenter: parent.horizontalCenter
            anchors.verticalCenter: parent.verticalCenter
            anchors.verticalCenterOffset: -10
            text: core.title
            color: "#FFF3EA"
            font.family: "Segoe UI"
            font.pixelSize: core.title.length > 9 ? 20 : 30
            font.weight: Font.Black
            font.letterSpacing: 2.0
        }

        Text {
            anchors.horizontalCenter: parent.horizontalCenter
            anchors.verticalCenter: parent.verticalCenter
            anchors.verticalCenterOffset: 22
            text: core.subtitle
            color: "#C9A48F"
            font.family: "Segoe UI"
            font.pixelSize: 11
            font.weight: Font.DemiBold
            width: 150
            horizontalAlignment: Text.AlignHCenter
            elide: Text.ElideRight
        }

        MouseArea {
            id: hoverArea
            anchors.fill: parent
            hoverEnabled: true
            cursorShape: core.interactive ? Qt.PointingHandCursor : Qt.ArrowCursor
            onClicked: if (core.interactive) core.activated()
            onContainsMouseChanged: core.hoveredChanged(containsMouse)
        }
    }

    Text {
        anchors.horizontalCenter: parent.horizontalCenter
        anchors.top: parent.top
        anchors.topMargin: 232
        width: parent.width
        text: core.hint
        color: "#8F7869"
        font.family: "Segoe UI"
        font.pixelSize: 11
        horizontalAlignment: Text.AlignHCenter
        elide: Text.ElideRight
    }
}
