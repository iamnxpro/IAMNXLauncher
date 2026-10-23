import QtQuick

// IAMNX orbital launcher shell.
//
// Layout map:
//   center   - launch reactor
//   upper arc - navigation nodes
//   lower arc - instance orbit
//   corner   - live 3D companion
Item {
    id: root

    width: 1360
    height: 860
    focus: true

    LayoutMirroring.enabled: Lang.rtl
    LayoutMirroring.childrenInherit: true

    property real lookX: 0
    property real lookY: 0
    property string mood: "idle"

    property bool unlocked: true
    property bool lockVisible: false
    property bool securityVisible: false

    property bool launching: false
    property bool portalActive: false
    property bool portalClosing: false

    property string toastKey: ""
    property bool toastIsError: false

    property real uiSuck: launching ? 1 : 0
    property real navRadius: Math.min(width, height) * 0.355
    property real instanceRadius: Math.min(width, height) * 0.315

    property var navModel: [
        { icon: "qrc:/orbital/assets/icons/plus.png", key: "nav.add", action: "add" },
        { icon: "qrc:/orbital/assets/icons/user.png", key: "nav.accounts", action: "accounts" },
        { icon: "qrc:/orbital/assets/icons/palette.png", key: "nav.skins", action: "skins" },
        { icon: "qrc:/orbital/assets/icons/folder.png", key: "nav.folders", action: "folder" },
        { icon: "qrc:/orbital/assets/icons/arrow.png", key: "nav.manage", action: "manage" },
        { icon: "qrc:/orbital/assets/icons/gear.png", key: "nav.settings", action: "settings" },
        { icon: "qrc:/orbital/assets/icons/help.png", key: "nav.help", action: "help" }
    ]

    property var instanceItems: {
        var source = Backend.instances;
        var out = [];
        var total = Math.min(6, source.length);
        for (var i = 0; i < total; ++i)
            out.push(source[i]);
        return out;
    }

    property string bubbleKey: {
        switch (mood) {
        case "wave": return "bubble.wave";
        case "shy": return "bubble.shy";
        case "facepalm": return "bubble.facepalm";
        case "reach": return "bubble.reach";
        case "fail": return "bubble.fail";
        case "sucked": return "bubble.portal";
        default: return "";
        }
    }

    function orbitX(angle, radius) {
        var direction = Lang.rtl ? -1 : 1;
        return width / 2 + direction * Math.cos(angle * Math.PI / 180) * radius;
    }

    function orbitY(angle, radius) {
        return height / 2 + Math.sin(angle * Math.PI / 180) * radius;
    }

    function updateLook(x, y) {
        lookX = Math.max(-1, Math.min(1, (x / Math.max(1, width) - 0.5) * 2));
        lookY = Math.max(-1, Math.min(1, (y / Math.max(1, height) - 0.5) * 2));
    }

    function showToast(key, isError) {
        toastKey = key;
        toastIsError = isError;
        toastTimer.restart();
    }

    function deny(key) {
        showToast(key, true);
        mood = "facepalm";
        moodReset.restart();
    }

    function statusLabel() {
        if (launching || Backend.busy)
            return Lang.t("status.launching", Lang.revision);
        switch (Backend.selectedInstanceStatus) {
        case "running": return Lang.t("core.running", Lang.revision);
        case "ready": return Lang.t("status.ready", Lang.revision);
        case "unavailable": return Lang.t("error.cannotLaunch", Lang.revision);
        default: return Lang.t("core.noInstance", Lang.revision);
        }
    }

    function navEnabled(action) {
        if (action === "folder" || action === "manage")
            return Backend.selectedInstanceId !== "";
        return true;
    }

    function runNav(action) {
        switch (action) {
        case "add": Backend.addInstance(); break;
        case "accounts": Backend.openAccounts(); break;
        case "skins": Backend.openSkins(); break;
        case "folder":
            if (Backend.selectedInstanceId !== "")
                Backend.openInstanceFolder();
            break;
        case "manage":
            if (Backend.selectedInstanceId !== "")
                Backend.openInstanceSettings();
            break;
        case "settings": Backend.openSettings(); break;
        case "help": Backend.openHelp(); break;
        }
    }

    function beginLaunch() {
        if (!unlocked || lockVisible || securityVisible || launching)
            return;
        if (Backend.selectedInstanceId === "") {
            deny("error.noInstance");
            return;
        }
        if (Backend.accountName === "") {
            deny("error.noAccount");
            return;
        }
        if (!Backend.canLaunchSelected) {
            deny("error.cannotLaunch");
            return;
        }
        launching = true;
        portalClosing = false;
        portalActive = true;
        mood = "reach";
        launchTimer.restart();
    }

    function cancelLaunch(code) {
        launching = false;
        portalActive = false;
        portalClosing = false;
        scene.reset();
        mood = "fail";
        if (code !== "")
            showToast(code, true);
        moodReset.restart();
    }

    function toggleLanguage() {
        Lang.toggleLanguage();
    }

    Component.onCompleted: {
        Backend.refresh();
        if (Backend.hasPin) {
            unlocked = false;
            lockVisible = true;
        } else {
            unlocked = true;
            mood = "wave";
            waveReset.restart();
        }
    }

    HoverHandler {
        id: pointerTracker
        onPointChanged: {
            var p = pointerTracker.point.position;
            root.updateLook(p.x, p.y);
        }
    }

    Timer {
        id: waveReset
        interval: 3300
        onTriggered: if (mood === "wave" && !launching) mood = "idle"
    }

    Timer {
        id: moodReset
        interval: 2400
        onTriggered: {
            if (!launching && unlocked && !lockVisible && mood !== "wave")
                mood = "idle";
        }
    }

    Timer {
        id: reachTimer
        interval: 230
        onTriggered: if (!launching && unlocked && !lockVisible) mood = "reach"
    }

    Timer {
        id: launchTimer
        interval: 620
        onTriggered: {
            if (!launching)
                return;
            scene.suck();
            mood = "sucked";
            Backend.launchSelected();
        }
    }

    Timer {
        id: quitTimer
        interval: 880
        onTriggered: Backend.quitLauncher()
    }

    Timer {
        id: toastTimer
        interval: 4800
        onTriggered: root.toastKey = ""
    }

    Connections {
        target: Backend

        function onLaunchStarted() {
            root.launching = true;
        }

        function onGameStarted() {
            root.portalClosing = true;
            root.mood = "sucked";
            root.quitTimer.restart();
        }

        function onLaunchFailed(code) {
            root.cancelLaunch(code);
        }

        function onPinAccepted() {
            root.unlocked = true;
            root.lockVisible = false;
            root.mood = "wave";
            root.waveReset.restart();
        }

        function onPinRejected(remaining) {
            root.mood = "facepalm";
            root.moodReset.restart();
        }

        function onToast(key) {
            root.showToast(key, false);
        }
    }

    // ---------------------------------------------------------------------
    // Background
    // ---------------------------------------------------------------------
    Rectangle {
        anchors.fill: parent
        gradient: Gradient {
            GradientStop { position: 0.0; color: "#0A0705" }
            GradientStop { position: 0.55; color: "#0E0907" }
            GradientStop { position: 1.0; color: "#070504" }
        }
    }

    Rectangle {
        anchors.horizontalCenter: parent.horizontalCenter
        anchors.verticalCenter: parent.verticalCenter
        width: parent.height * 1.25
        height: width
        radius: width / 2
        color: "#0CFF5E1A"
        opacity: 0.9
    }

    Rectangle {
        x: -parent.height * 0.35
        y: -parent.height * 0.45
        width: parent.height * 0.9
        height: width
        radius: width / 2
        color: "#07FF5E1A"
    }

    Repeater {
        model: 18
        delegate: Rectangle {
            x: root.width * index / 18
            y: 0
            width: 1
            height: root.height
            color: "#08FFFFFF"
        }
    }

    Repeater {
        model: 12
        delegate: Rectangle {
            x: 0
            y: root.height * index / 12
            width: root.width
            height: 1
            color: "#07FFFFFF"
        }
    }

    // ---------------------------------------------------------------------
    // 3D layer
    // ---------------------------------------------------------------------
    OrbitalScene {
        id: scene
        anchors.fill: parent
        lookX: root.lookX
        lookY: root.lookY
        rtl: Lang.rtl
        mood: root.mood
        portalActive: root.portalActive
        portalClosing: root.portalClosing
    }

    // ---------------------------------------------------------------------
    // 2D interface (sucked into the portal on launch)
    // ---------------------------------------------------------------------
    Item {
        id: uiLayer
        anchors.fill: parent
        enabled: !root.lockVisible && !root.securityVisible && !root.launching
        opacity: root.lockVisible || root.securityVisible ? 0.10 : (1 - root.uiSuck)
        scale: 1 - 0.94 * root.uiSuck
        rotation: (Lang.rtl ? -1 : 1) * 22 * root.uiSuck
        transformOrigin: Item.Center

        Behavior on opacity { NumberAnimation { duration: 360 } }
        Behavior on scale { NumberAnimation { duration: root.launching ? 900 : 460; easing.type: Easing.InCubic } }
        Behavior on rotation { NumberAnimation { duration: root.launching ? 900 : 460; easing.type: Easing.InCubic } }

        // Header ---------------------------------------------------------
        Item {
            id: header
            anchors.left: parent.left
            anchors.right: parent.right
            anchors.top: parent.top
            height: 82

            Rectangle {
                anchors.left: parent.left
                anchors.right: parent.right
                anchors.bottom: parent.bottom
                height: 1
                color: "#18FF5E1A"
            }

            Item {
                id: brand
                anchors.left: parent.left
                anchors.leftMargin: 32
                anchors.verticalCenter: parent.verticalCenter
                width: 270
                height: 52

                Rectangle {
                    anchors.left: parent.left
                    anchors.verticalCenter: parent.verticalCenter
                    width: 38
                    height: 38
                    radius: 9
                    rotation: 45
                    gradient: Gradient {
                        GradientStop { position: 0.0; color: "#FF9A5C" }
                        GradientStop { position: 1.0; color: "#D83F08" }
                    }
                    Text {
                        anchors.centerIn: parent
                        rotation: -45
                        text: "I"
                        color: "#190A04"
                        font.family: "Segoe UI"
                        font.pixelSize: 21
                        font.weight: Font.Black
                    }
                }

                Column {
                    anchors.left: parent.left
                    anchors.leftMargin: 58
                    anchors.verticalCenter: parent.verticalCenter
                    spacing: 1
                    Text {
                        text: "IAMNX"
                        color: "#FFF3EA"
                        font.family: "Segoe UI"
                        font.pixelSize: 22
                        font.weight: Font.Black
                        font.letterSpacing: 3.0
                    }
                    Text {
                        text: Lang.t("app.tagline", Lang.revision)
                        color: "#B47755"
                        font.family: "Segoe UI"
                        font.pixelSize: 9
                        font.weight: Font.DemiBold
                        font.letterSpacing: 2.2
                    }
                }
            }

            Row {
                id: headerActions
                anchors.right: parent.right
                anchors.rightMargin: 26
                anchors.verticalCenter: parent.verticalCenter
                spacing: 10
                layoutDirection: Lang.rtl ? Qt.RightToLeft : Qt.LeftToRight

                Rectangle {
                    id: accountChip
                    width: 208
                    height: 42
                    radius: 21
                    color: accountArea.containsMouse ? "#21130C" : "#130C09"
                    border.width: 1
                    border.color: accountArea.containsMouse ? "#6B3A22" : "#332018"

                    Behavior on color { ColorAnimation { duration: 150 } }

                    Rectangle {
                        id: accountAvatar
                        anchors.left: parent.left
                        anchors.leftMargin: 7
                        anchors.verticalCenter: parent.verticalCenter
                        width: 28
                        height: 28
                        radius: 14
                        color: Backend.accountName !== "" ? "#5C2A13" : "#241710"
                        Text {
                            anchors.centerIn: parent
                            text: Backend.accountName.length > 0 ? Backend.accountName.charAt(0).toUpperCase() : "?"
                            color: "#FFB27A"
                            font.family: "Segoe UI"
                            font.pixelSize: 12
                            font.weight: Font.Bold
                        }
                    }

                    Column {
                        anchors.left: accountAvatar.right
                        anchors.leftMargin: 9
                        anchors.right: parent.right
                        anchors.rightMargin: 12
                        anchors.verticalCenter: parent.verticalCenter
                        spacing: 0
                        Text {
                            width: parent.width
                            text: Backend.accountName !== "" ? Backend.accountName
                                                             : Lang.t("account.none", Lang.revision)
                            color: "#E8D8CE"
                            font.family: "Segoe UI"
                            font.pixelSize: 11
                            font.weight: Font.DemiBold
                            elide: Text.ElideRight
                            horizontalAlignment: Lang.rtl ? Text.AlignRight : Text.AlignLeft
                        }
                        Text {
                            width: parent.width
                            text: Backend.accountType !== "" ? Lang.t("account." + Backend.accountType, Lang.revision)
                                                            : Lang.t("account.manage", Lang.revision)
                            color: "#8B6B59"
                            font.family: "Segoe UI"
                            font.pixelSize: 9
                            elide: Text.ElideRight
                            horizontalAlignment: Lang.rtl ? Text.AlignRight : Text.AlignLeft
                        }
                    }

                    MouseArea {
                        id: accountArea
                        anchors.fill: parent
                        hoverEnabled: true
                        cursorShape: Qt.PointingHandCursor
                        onClicked: Backend.openAccounts()
                    }
                }

                Rectangle {
                    id: languageButton
                    width: 64
                    height: 38
                    radius: 19
                    color: languageArea.containsMouse ? "#21130C" : "#130C09"
                    border.width: 1
                    border.color: "#332018"

                    Image {
                        anchors.left: parent.left
                        anchors.leftMargin: 12
                        anchors.verticalCenter: parent.verticalCenter
                        width: 14
                        height: 14
                        source: "qrc:/orbital/assets/icons/globe.png"
                        opacity: 0.72
                    }
                    Text {
                        anchors.right: parent.right
                        anchors.rightMargin: 11
                        anchors.verticalCenter: parent.verticalCenter
                        text: Lang.rtl ? "AR" : "EN"
                        color: "#D9A98F"
                        font.family: "Segoe UI"
                        font.pixelSize: 10
                        font.weight: Font.Bold
                    }

                    MouseArea {
                        id: languageArea
                        anchors.fill: parent
                        hoverEnabled: true
                        cursorShape: Qt.PointingHandCursor
                        onClicked: root.toggleLanguage()
                    }
                }

                Rectangle {
                    id: lockButton
                    width: 38
                    height: 38
                    radius: 19
                    color: lockArea.containsMouse ? "#21130C" : "#130C09"
                    border.width: Backend.hasPin ? 1 : 1
                    border.color: Backend.hasPin ? "#8A3E1D" : "#332018"

                    Image {
                        anchors.centerIn: parent
                        width: 16
                        height: 16
                        source: "qrc:/orbital/assets/icons/lock.png"
                        opacity: Backend.hasPin ? 0.95 : 0.55
                    }

                    Rectangle {
                        visible: Backend.hasPin
                        anchors.right: parent.right
                        anchors.top: parent.top
                        anchors.rightMargin: 3
                        anchors.topMargin: 3
                        width: 7
                        height: 7
                        radius: 4
                        color: "#FF5E1A"
                    }

                    MouseArea {
                        id: lockArea
                        anchors.fill: parent
                        hoverEnabled: true
                        cursorShape: Qt.PointingHandCursor
                        onClicked: if (root.unlocked) root.securityVisible = true
                    }
                }
            }
        }

        // Upper navigation arc -------------------------------------------
        Repeater {
            model: root.navModel
            delegate: OrbitalNode {

                property real orbitAngle: 195 + index * 25

                x: root.orbitX(orbitAngle, root.navRadius) - width / 2
                y: root.orbitY(orbitAngle, root.navRadius) - height / 2
                icon: modelData.icon
                label: Lang.t(modelData.key, Lang.revision)
                interactive: root.navEnabled(modelData.action) && !root.launching
                onActivated: root.runNav(modelData.action)
            }
        }

        // Instance orbit -------------------------------------------------
        Repeater {
            model: root.instanceItems
            delegate: InstanceNode {

                property int count: root.instanceItems.length
                property real orbitAngle: count <= 1 ? 132 : 95 + index * (80 / (count - 1))

                x: root.orbitX(orbitAngle, root.instanceRadius) - width / 2
                y: root.orbitY(orbitAngle, root.instanceRadius) - height / 2
                instanceId: modelData.id
                instanceName: modelData.name
                letter: modelData.letter
                hue: modelData.hue
                running: modelData.running
                selected: Backend.selectedInstanceId === modelData.id
                onActivated: Backend.selectInstance(modelData.id)
            }
        }

        Text {
            visible: root.instanceItems.length === 0 && !root.launching
            anchors.left: parent.left
            anchors.bottom: parent.bottom
            anchors.leftMargin: 34
            anchors.bottomMargin: 62
            width: 300
            text: Lang.t("library.empty", Lang.revision) + "  |  "
                  + Lang.t("library.emptyHint", Lang.revision)
            color: "#8F7566"
            font.family: "Segoe UI"
            font.pixelSize: 11
            lineHeight: 1.3
            wrapMode: Text.WordWrap
            horizontalAlignment: Lang.rtl ? Text.AlignRight : Text.AlignLeft
        }

        // Core -----------------------------------------------------------
        Core {
            id: core
            anchors.centerIn: parent
            anchors.verticalCenterOffset: -18

            title: root.launching ? Lang.t("core.launching", Lang.revision)
                                  : (Backend.selectedInstanceId !== ""
                                     ? Lang.t("core.play", Lang.revision)
                                     : Lang.t("core.noInstance", Lang.revision))
            subtitle: Backend.selectedInstanceId !== ""
                      ? Backend.selectedInstanceName
                      : Lang.t("core.select", Lang.revision)
            hint: root.launching ? Lang.t("status.starting", Lang.revision)
                                 : (Backend.selectedInstanceId !== ""
                                    ? Lang.t("core.hint", Lang.revision)
                                    : Lang.t("library.emptyHint", Lang.revision))
            busy: root.launching || Backend.busy
            interactive: root.unlocked && !root.lockVisible && !root.securityVisible && !root.launching

            onActivated: root.beginLaunch()
            onHoveredChanged: function(hovered) {
                if (hovered && !root.launching && root.unlocked && !root.lockVisible)
                    root.reachTimer.restart();
                else {
                    root.reachTimer.stop();
                    if (root.mood === "reach" && !root.launching)
                        root.mood = "idle";
                }
            }
        }

        // Companion speech bubble ----------------------------------------
        Rectangle {
            id: bubble
            visible: root.bubbleKey !== "" && root.unlocked && !root.lockVisible
                     && !root.securityVisible && !root.launching
            anchors.right: parent.right
            anchors.rightMargin: 356
            anchors.bottom: parent.bottom
            anchors.bottomMargin: 244
            width: 236
            height: bubbleText.implicitHeight + 26
            radius: 16
            color: "#E6140C09"
            border.width: 1
            border.color: "#5A2B16"
            opacity: visible ? 1 : 0

            Behavior on opacity { NumberAnimation { duration: 220 } }

            Text {
                id: bubbleText
                anchors.left: parent.left
                anchors.right: parent.right
                anchors.verticalCenter: parent.verticalCenter
                anchors.leftMargin: 15
                anchors.rightMargin: 15
                text: Lang.t(root.bubbleKey, Lang.revision)
                color: "#F2DED2"
                font.family: "Segoe UI"
                font.pixelSize: 12
                lineHeight: 1.25
                wrapMode: Text.WordWrap
                horizontalAlignment: Lang.rtl ? Text.AlignRight : Text.AlignLeft
            }

            Rectangle {
                anchors.right: parent.right
                anchors.bottom: parent.bottom
                anchors.rightMargin: 34
                anchors.bottomMargin: -5
                width: 10
                height: 10
                rotation: 45
                color: "#E6140C09"
                border.width: 1
                border.color: "#5A2B16"
            }
        }

        // Bottom status ---------------------------------------------------
        Item {
            anchors.left: parent.left
            anchors.right: parent.right
            anchors.bottom: parent.bottom
            height: 46

            Row {
                anchors.left: parent.left
                anchors.leftMargin: 30
                anchors.verticalCenter: parent.verticalCenter
                spacing: 9
                layoutDirection: Lang.rtl ? Qt.RightToLeft : Qt.LeftToRight

                Rectangle {
                    width: 7
                    height: 7
                    radius: 4
                    anchors.verticalCenter: parent.verticalCenter
                    color: root.launching ? "#FF9A5C"
                                        : (Backend.selectedInstanceStatus === "ready" ? "#7BE38B"
                                        : (Backend.selectedInstanceStatus === "running" ? "#FF5E1A" : "#6B574C"))
                }

                Text {
                    text: root.statusLabel()
                    color: "#A78D7D"
                    font.family: "Segoe UI"
                    font.pixelSize: 10
                    font.weight: Font.DemiBold
                    font.letterSpacing: 0.8
                }

                Text {
                    text: "|"
                    color: "#4A3429"
                    font.pixelSize: 10
                }

                Text {
                    text: Lang.t("common.version", Lang.revision) + " " + Backend.version
                    color: "#6F5A4E"
                    font.family: "Segoe UI"
                    font.pixelSize: 10
                }
            }

            Text {
                anchors.right: parent.right
                anchors.rightMargin: 30
                anchors.verticalCenter: parent.verticalCenter
                text: "OFFLINE-FIRST  |  GPL-3.0  |  PRISM / MULTIMC LINEAGE"
                color: "#4F3D34"
                font.family: "Segoe UI"
                font.pixelSize: 8
                font.letterSpacing: 0.8
            }
        }
    }

    // Toast -------------------------------------------------------------
    Rectangle {
        id: toast
        anchors.horizontalCenter: parent.horizontalCenter
        anchors.top: parent.top
        anchors.topMargin: 94
        width: Math.min(parent.width - 64, 540)
        height: 42
        radius: 21
        color: root.toastIsError ? "#2A0E09" : "#16100C"
        border.width: 1
        border.color: root.toastIsError ? "#A53E27" : "#4A2E1F"
        opacity: root.toastKey !== "" ? 1 : 0
        visible: opacity > 0.01
        z: 50

        Behavior on opacity { NumberAnimation { duration: 220 } }

        Text {
            anchors.fill: parent
            anchors.leftMargin: 18
            anchors.rightMargin: 18
            text: root.toastKey !== "" ? Lang.t(root.toastKey, Lang.revision) : ""
            color: root.toastIsError ? "#FFB09B" : "#E5D2C6"
            font.family: "Segoe UI"
            font.pixelSize: 12
            font.weight: Font.DemiBold
            verticalAlignment: Text.AlignVCenter
            horizontalAlignment: Text.AlignHCenter
            elide: Text.ElideRight
        }

        MouseArea {
            anchors.fill: parent
            cursorShape: Qt.PointingHandCursor
            onClicked: root.toastKey = ""
        }
    }

    // Local launcher lock ----------------------------------------------
    PinScreen {
        id: pinScreen
        anchors.fill: parent
        visible: root.lockVisible
        z: 200

        onRevealChanged: function(revealed) {
            if (revealed)
                root.mood = "shy";
            else if (root.lockVisible)
                root.mood = "idle";
        }

        onFailed: {
            root.mood = "facepalm";
            root.moodReset.restart();
        }
    }

    // Local PIN management ---------------------------------------------
    SecurityPanel {
        id: securityPanel
        anchors.fill: parent
        visible: root.securityVisible
        z: 210
        onClosed: root.securityVisible = false
    }
}
