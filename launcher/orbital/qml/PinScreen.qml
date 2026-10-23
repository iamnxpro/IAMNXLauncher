import QtQuick

// Local launcher lock. This PIN never touches the Minecraft account code.
Item {
    id: lock

    property bool revealPassword: false
    property string errorText: ""

    signal revealChanged(bool revealed)
    signal failed()
    signal submitted()

    function reset() {
        pinInput.text = "";
        revealPassword = false;
        errorText = "";
        Qt.callLater(function() { pinInput.forceActiveFocus(); });
    }

    function submit() {
        if (pinInput.text.length === 0)
            return;
        const attempt = pinInput.text;
        pinInput.text = "";
        errorText = "";
        const ok = Backend.verifyPin(attempt);
        if (!ok) {
            pinInput.forceActiveFocus();
        }
        submitted();
    }

    function showError(seconds) {
        errorText = seconds > 0 ? Lang.t("pin.locked", Lang.revision) + " (" + seconds + "s)"
                                : Lang.t("pin.error", Lang.revision);
        shake.restart();
        failed();
    }

    visible: false
    z: 200

    onVisibleChanged: {
        if (visible)
            reset();
    }

    Connections {
        target: Backend
        function onPinRejected(remaining) { lock.showError(remaining); }
    }

    Rectangle {
        anchors.fill: parent
        color: "#E60A0604"

        MouseArea {
            anchors.fill: parent
            acceptedButtons: Qt.AllButtons
        }
    }

    Rectangle {
        id: card
        anchors.centerIn: parent
        width: Math.min(lock.width - 56, 460)
        height: 452
        radius: 30
        color: "#140C09"
        border.width: 1
        border.color: "#5A2B16"

        transform: Translate { id: shaker; x: 0 }

        gradient: Gradient {
            GradientStop { position: 0.0; color: "#20110B" }
            GradientStop { position: 0.45; color: "#130C09" }
            GradientStop { position: 1.0; color: "#0B0706" }
        }

        SequentialAnimation {
            id: shake
            NumberAnimation { target: shaker; property: "x"; from: 0; to: -9; duration: 45 }
            NumberAnimation { target: shaker; property: "x"; from: -9; to: 9; duration: 45 }
            NumberAnimation { target: shaker; property: "x"; from: 9; to: -7; duration: 45 }
            NumberAnimation { target: shaker; property: "x"; from: -7; to: 7; duration: 45 }
            NumberAnimation { target: shaker; property: "x"; from: 7; to: 0; duration: 45 }
        }

        Rectangle {
            anchors.top: parent.top
            anchors.horizontalCenter: parent.horizontalCenter
            anchors.topMargin: -1
            width: 120
            height: 3
            radius: 2
            color: "#FF5E1A"
        }

        Rectangle {
            id: brand
            anchors.horizontalCenter: parent.horizontalCenter
            anchors.top: parent.top
            anchors.topMargin: 42
            width: 58
            height: 58
            radius: 18
            rotation: 45
            gradient: Gradient {
                GradientStop { position: 0.0; color: "#FF8A4A" }
                GradientStop { position: 1.0; color: "#D63C08" }
            }
            Text {
                anchors.centerIn: parent
                rotation: -45
                text: "I"
                color: "#1A0A04"
                font.family: "Segoe UI"
                font.pixelSize: 27
                font.weight: Font.Black
            }
        }

        Text {
            anchors.horizontalCenter: parent.horizontalCenter
            anchors.top: brand.bottom
            anchors.topMargin: 32
            text: Lang.t("pin.title", Lang.revision)
            color: "#FFF3EA"
            font.family: "Segoe UI"
            font.pixelSize: 28
            font.weight: Font.Bold
            horizontalAlignment: Text.AlignHCenter
        }

        Text {
            anchors.horizontalCenter: parent.horizontalCenter
            anchors.top: parent.top
            anchors.topMargin: 176
            text: Lang.t("pin.subtitle", Lang.revision)
            color: "#BCA79A"
            font.family: "Segoe UI"
            font.pixelSize: 13
            horizontalAlignment: Text.AlignHCenter
        }

        Rectangle {
            id: inputFrame
            anchors.horizontalCenter: parent.horizontalCenter
            anchors.top: parent.top
            anchors.topMargin: 214
            width: parent.width - 72
            height: 54
            radius: 14
            color: "#0D0806"
            border.width: pinInput.activeFocus ? 1.5 : 1
            border.color: pinInput.activeFocus ? "#FF5E1A" : (errorText !== "" ? "#B43A2B" : "#3A241A")

            TextInput {
                id: pinInput
                anchors.left: parent.left
                anchors.right: eyeButton.left
                anchors.verticalCenter: parent.verticalCenter
                anchors.leftMargin: 18
                anchors.rightMargin: 10
                height: parent.height
                verticalAlignment: TextInput.AlignVCenter
                color: "#FFF3EA"
                selectionColor: "#8A3A17"
                selectedTextColor: "#FFF3EA"
                font.family: "Segoe UI"
                font.pixelSize: 16
                font.letterSpacing: revealPassword ? 0 : 5
                echoMode: revealPassword ? TextInput.Normal : TextInput.Password
                passwordCharacter: "●"
                passwordMaskDelay: 0
                selectByMouse: true
                activeFocusOnTab: true
                horizontalAlignment: Lang.rtl ? TextInput.AlignRight : TextInput.AlignLeft
                placeholderText: Lang.t("pin.placeholder", Lang.revision)
                placeholderTextColor: "#6D574B"
                onAccepted: lock.submit()
                onTextChanged: if (errorText !== "") errorText = ""
            }

            Rectangle {
                id: eyeButton
                anchors.right: parent.right
                anchors.verticalCenter: parent.verticalCenter
                anchors.rightMargin: 8
                width: 38
                height: 38
                radius: 10
                color: eyeArea.containsMouse ? "#2B160D" : "transparent"

                Image {
                    anchors.centerIn: parent
                    width: 20
                    height: 20
                    source: lock.revealPassword ? "qrc:/orbital/assets/icons/eye_off.png"
                                                 : "qrc:/orbital/assets/icons/eye.png"
                    opacity: 0.75
                }

                MouseArea {
                    id: eyeArea
                    anchors.fill: parent
                    hoverEnabled: true
                    cursorShape: Qt.PointingHandCursor
                    onClicked: {
                        lock.revealPassword = !lock.revealPassword;
                        lock.revealChanged(lock.revealPassword);
                        pinInput.forceActiveFocus();
                    }
                }
            }
        }

        Text {
            anchors.horizontalCenter: parent.horizontalCenter
            anchors.top: inputFrame.bottom
            anchors.topMargin: 8
            width: parent.width - 80
            text: lock.errorText
            color: "#FF8066"
            font.family: "Segoe UI"
            font.pixelSize: 12
            horizontalAlignment: Text.AlignHCenter
            wrapMode: Text.WordWrap
            visible: text !== ""
        }

        Rectangle {
            id: unlockButton
            anchors.horizontalCenter: parent.horizontalCenter
            anchors.top: inputFrame.bottom
            anchors.topMargin: 34
            width: parent.width - 72
            height: 52
            radius: 14
            color: unlockArea.containsMouse ? "#FF7A35" : "#FF5E1A"
            opacity: pinInput.text.length > 0 ? 1.0 : 0.52
            scale: unlockArea.containsMouse && pinInput.text.length > 0 ? 1.015 : 1.0

            Behavior on scale { NumberAnimation { duration: 130 } }

            Text {
                anchors.centerIn: parent
                text: Lang.t("pin.unlock", Lang.revision)
                color: "#210D05"
                font.family: "Segoe UI"
                font.pixelSize: 15
                font.weight: Font.Bold
            }

            MouseArea {
                id: unlockArea
                anchors.fill: parent
                hoverEnabled: true
                cursorShape: Qt.PointingHandCursor
                onClicked: lock.submit()
            }
        }

        Text {
            anchors.horizontalCenter: parent.horizontalCenter
            anchors.bottom: parent.bottom
            anchors.bottomMargin: 24
            width: parent.width - 76
            text: Lang.t("pin.localOnly", Lang.revision)
            color: "#816A5D"
            font.family: "Segoe UI"
            font.pixelSize: 10
            horizontalAlignment: Text.AlignHCenter
            wrapMode: Text.WordWrap
        }
    }
}
