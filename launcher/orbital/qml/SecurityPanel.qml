import QtQuick

// Local PIN management. It is intentionally separate from account passwords.
Item {
    id: panel

    property string message: ""
    property bool messageIsError: false

    signal closed()

    function reset() {
        newPin.text = "";
        confirmPin.text = "";
        message = "";
        messageIsError = false;
    }

    function save() {
        message = "";
        messageIsError = false;
        if (newPin.text.trim().length < 4 || newPin.text.length > 64) {
            message = Lang.t("security.short", Lang.revision);
            messageIsError = true;
            return;
        }
        if (newPin.text !== confirmPin.text) {
            message = Lang.t("security.mismatch", Lang.revision);
            messageIsError = true;
            return;
        }
        if (Backend.setPin(newPin.text)) {
            panel.closed();
        } else {
            message = Lang.t("security.short", Lang.revision);
            messageIsError = true;
        }
    }

    visible: false
    z: 210

    onVisibleChanged: {
        if (visible)
            reset();
    }

    Rectangle {
        anchors.fill: parent
        color: "#CC090604"
        MouseArea { anchors.fill: parent }
    }

    Rectangle {
        id: card
        anchors.centerIn: parent
        width: Math.min(panel.width - 56, 510)
        height: 454
        radius: 28
        color: "#140C09"
        border.width: 1
        border.color: "#5A2B16"

        gradient: Gradient {
            GradientStop { position: 0.0; color: "#20110B" }
            GradientStop { position: 1.0; color: "#0B0706" }
        }

        Rectangle {
            anchors.top: parent.top
            anchors.horizontalCenter: parent.horizontalCenter
            anchors.topMargin: -1
            width: 110
            height: 3
            radius: 2
            color: "#FF5E1A"
        }

        Image {
            anchors.horizontalCenter: parent.horizontalCenter
            anchors.top: parent.top
            anchors.topMargin: 34
            width: 34
            height: 34
            source: "qrc:/orbital/assets/icons/lock.png"
            opacity: 0.9
        }

        Text {
            anchors.horizontalCenter: parent.horizontalCenter
            anchors.top: parent.top
            anchors.topMargin: 82
            text: Lang.t("security.title", Lang.revision)
            color: "#FFF3EA"
            font.family: "Segoe UI"
            font.pixelSize: 24
            font.weight: Font.Bold
        }

        Text {
            anchors.horizontalCenter: parent.horizontalCenter
            anchors.top: parent.top
            anchors.topMargin: 120
            width: parent.width - 80
            text: Lang.t("security.desc", Lang.revision)
            color: "#A98F80"
            font.family: "Segoe UI"
            font.pixelSize: 12
            horizontalAlignment: Text.AlignHCenter
            wrapMode: Text.WordWrap
        }

        Rectangle {
            id: newFrame
            anchors.horizontalCenter: parent.horizontalCenter
            anchors.top: parent.top
            anchors.topMargin: 184
            width: parent.width - 72
            height: 48
            radius: 12
            color: "#0D0806"
            border.width: newPin.activeFocus ? 1.5 : 1
            border.color: newPin.activeFocus ? "#FF5E1A" : "#3A241A"

            TextInput {
                id: newPin
                anchors.fill: parent
                anchors.leftMargin: 16
                anchors.rightMargin: 16
                verticalAlignment: TextInput.AlignVCenter
                color: "#FFF3EA"
                font.family: "Segoe UI"
                font.pixelSize: 15
                echoMode: TextInput.Password
                passwordCharacter: "●"
                selectByMouse: true
                activeFocusOnTab: true
                horizontalAlignment: Lang.rtl ? TextInput.AlignRight : TextInput.AlignLeft
                placeholderText: Lang.t("security.new", Lang.revision)
                placeholderTextColor: "#6D574B"
                onAccepted: confirmPin.forceActiveFocus()
            }
        }

        Rectangle {
            id: confirmFrame
            anchors.horizontalCenter: parent.horizontalCenter
            anchors.top: newFrame.bottom
            anchors.topMargin: 12
            width: parent.width - 72
            height: 48
            radius: 12
            color: "#0D0806"
            border.width: confirmPin.activeFocus ? 1.5 : 1
            border.color: confirmPin.activeFocus ? "#FF5E1A" : "#3A241A"

            TextInput {
                id: confirmPin
                anchors.fill: parent
                anchors.leftMargin: 16
                anchors.rightMargin: 16
                verticalAlignment: TextInput.AlignVCenter
                color: "#FFF3EA"
                font.family: "Segoe UI"
                font.pixelSize: 15
                echoMode: TextInput.Password
                passwordCharacter: "●"
                selectByMouse: true
                activeFocusOnTab: true
                horizontalAlignment: Lang.rtl ? TextInput.AlignRight : TextInput.AlignLeft
                placeholderText: Lang.t("security.confirm", Lang.revision)
                placeholderTextColor: "#6D574B"
                onAccepted: panel.save()
            }
        }

        Text {
            anchors.horizontalCenter: parent.horizontalCenter
            anchors.top: confirmFrame.bottom
            anchors.topMargin: 8
            width: parent.width - 80
            text: panel.message !== "" ? panel.message
                                        : (Backend.hasPin ? Lang.t("security.enabled", Lang.revision) : Lang.t("security.none", Lang.revision))
            color: panel.message !== "" ? (panel.messageIsError ? "#FF8066" : "#9ED9A6") : "#816A5D"
            font.family: "Segoe UI"
            font.pixelSize: 11
            horizontalAlignment: Text.AlignHCenter
            wrapMode: Text.WordWrap
        }

        Rectangle {
            id: saveButton
            anchors.horizontalCenter: parent.horizontalCenter
            anchors.top: confirmFrame.bottom
            anchors.topMargin: 42
            width: 160
            height: 46
            radius: 13
            color: saveArea.containsMouse ? "#FF7A35" : "#FF5E1A"
            Text {
                anchors.centerIn: parent
                text: Lang.t("security.save", Lang.revision)
                color: "#210D05"
                font.family: "Segoe UI"
                font.pixelSize: 14
                font.weight: Font.Bold
            }
            MouseArea {
                id: saveArea
                anchors.fill: parent
                hoverEnabled: true
                cursorShape: Qt.PointingHandCursor
                onClicked: panel.save()
            }
        }

        Rectangle {
            anchors.left: saveButton.right
            anchors.leftMargin: 12
            anchors.verticalCenter: saveButton.verticalCenter
            width: 130
            height: 46
            radius: 13
            color: removeArea.containsMouse ? "#2A140D" : "transparent"
            border.width: 1
            border.color: "#5A2B16"
            visible: Backend.hasPin
            Text {
                anchors.centerIn: parent
                text: Lang.t("security.remove", Lang.revision)
                color: "#D9A08B"
                font.family: "Segoe UI"
                font.pixelSize: 12
                font.weight: Font.DemiBold
            }
            MouseArea {
                id: removeArea
                anchors.fill: parent
                hoverEnabled: true
                cursorShape: Qt.PointingHandCursor
                onClicked: {
                    Backend.clearPin();
                    panel.closed();
                }
            }
        }

        Rectangle {
            anchors.right: parent.right
            anchors.top: parent.top
            anchors.rightMargin: 16
            anchors.topMargin: 14
            width: 32
            height: 32
            radius: 9
            color: closeArea.containsMouse ? "#2B160D" : "transparent"
            Image {
                anchors.centerIn: parent
                width: 16
                height: 16
                source: "qrc:/orbital/assets/icons/close.png"
                opacity: 0.7
            }
            MouseArea {
                id: closeArea
                anchors.fill: parent
                hoverEnabled: true
                cursorShape: Qt.PointingHandCursor
                onClicked: panel.closed()
            }
        }
    }
}
