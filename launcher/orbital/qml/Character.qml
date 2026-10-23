import QtQuick
import QtQuick3D

// The companion. Built from the selected 64x64 skin crops so head, eyes,
// arms and legs can be posed independently without a skeletal asset.
Node {
    id: character

    property real lookX: 0
    property real lookY: 0
    property string mood: "idle"
    property bool rtl: false

    property real wave: 0
    property real bob: 0
    property real headYaw: {
        if (mood === "shy")
            return rtl ? -36 : 36;
        if (mood === "facepalm" || mood === "fail")
            return 0;
        if (mood === "reach")
            return rtl ? 26 : -26;
        return Math.max(-30, Math.min(30, lookX * 30));
    }
    property real headPitch: {
        if (mood === "facepalm")
            return 22;
        if (mood === "shy")
            return 12;
        if (mood === "reach")
            return 4;
        return Math.max(-15, Math.min(15, lookY * 12));
    }
    property real headTilt: mood === "facepalm" ? -12 : (mood === "wave" ? 5 : 0)
    property real lean: mood === "reach" ? 7 : (mood === "facepalm" ? 3 : 0)

    property real armRZ: {
        if (mood === "wave")
            return 138 + wave * 9;
        if (mood === "facepalm")
            return -105;
        if (mood === "shy")
            return -114;
        if (mood === "reach")
            return -80;
        if (mood === "fail")
            return 8;
        return 0;
    }
    property real armRX: (mood === "facepalm" || mood === "shy") ? 28 : (mood === "reach" ? 22 : 0)
    property real armLZ: mood === "shy" ? 114 : 0
    property real armLX: mood === "shy" ? 28 : 0

    Behavior on headYaw { NumberAnimation { duration: 130; easing.type: Easing.OutCubic } }
    Behavior on headPitch { NumberAnimation { duration: 150; easing.type: Easing.OutCubic } }
    Behavior on headTilt { NumberAnimation { duration: 180; easing.type: Easing.OutCubic } }
    Behavior on lean { NumberAnimation { duration: 260; easing.type: Easing.OutCubic } }
    Behavior on armRZ { NumberAnimation { duration: mood === "fail" ? 220 : 360; easing.type: Easing.OutBack } }
    Behavior on armRX { NumberAnimation { duration: 300; easing.type: Easing.OutCubic } }
    Behavior on armLZ { NumberAnimation { duration: 300; easing.type: Easing.OutBack } }
    Behavior on armLX { NumberAnimation { duration: 300; easing.type: Easing.OutCubic } }

    SequentialAnimation on bob {
        loops: Animation.Infinite
        NumberAnimation { from: 0; to: 0.32; duration: 1700; easing.type: Easing.InOutSine }
        NumberAnimation { from: 0.32; to: 0; duration: 1700; easing.type: Easing.InOutSine }
    }

    SequentialAnimation {
        id: waveAnim
        loops: Animation.Infinite
        NumberAnimation { target: character; property: "wave"; from: -1; to: 1; duration: 320; easing.type: Easing.InOutSine }
        NumberAnimation { target: character; property: "wave"; from: 1; to: -1; duration: 320; easing.type: Easing.InOutSine }
    }

    onMoodChanged: {
        if (mood === "wave") {
            waveAnim.restart();
        } else {
            waveAnim.stop();
            wave = 0;
        }
    }

    Node {
        id: bodyRoot
        position: Qt.vector3d(0, bob, 0)
        eulerRotation: Qt.vector3d(0, 0, lean)

        // Legs
        TexturedBox {
            position: Qt.vector3d(-2, -6, 0)
            boxWidth: 4
            boxHeight: 12
            boxDepth: 4
            baseColor: "#3B4652"
            frontTexture: "qrc:/orbital/assets/character/leg_l_front.png"
            backTexture: "qrc:/orbital/assets/character/leg_l_back.png"
            leftTexture: "qrc:/orbital/assets/character/leg_l_left.png"
            rightTexture: "qrc:/orbital/assets/character/leg_l_right.png"
        }
        TexturedBox {
            position: Qt.vector3d(2, -6, 0)
            boxWidth: 4
            boxHeight: 12
            boxDepth: 4
            baseColor: "#3B4652"
            frontTexture: "qrc:/orbital/assets/character/leg_r_front.png"
            backTexture: "qrc:/orbital/assets/character/leg_r_back.png"
            leftTexture: "qrc:/orbital/assets/character/leg_r_left.png"
            rightTexture: "qrc:/orbital/assets/character/leg_r_right.png"
        }

        // Body
        TexturedBox {
            position: Qt.vector3d(0, 6, 0)
            boxWidth: 8
            boxHeight: 12
            boxDepth: 4
            baseColor: "#54555C"
            frontTexture: "qrc:/orbital/assets/character/body_front.png"
            backTexture: "qrc:/orbital/assets/character/body_back.png"
            leftTexture: "qrc:/orbital/assets/character/body_left.png"
            rightTexture: "qrc:/orbital/assets/character/body_right.png"
            topTexture: "qrc:/orbital/assets/character/body_top.png"
            bottomTexture: "qrc:/orbital/assets/character/body_bottom.png"
        }

        // Right arm (character's +X side)
        Node {
            id: rightArm
            position: Qt.vector3d(6, 12, 0)
            eulerRotation: Qt.vector3d(character.armRX, 0, character.armRZ)
            TexturedBox {
                position: Qt.vector3d(0, -6, 0)
                boxWidth: 4
                boxHeight: 12
                boxDepth: 4
                baseColor: "#6A4B3C"
                frontTexture: "qrc:/orbital/assets/character/arm_r_front.png"
                backTexture: "qrc:/orbital/assets/character/arm_r_back.png"
                leftTexture: "qrc:/orbital/assets/character/arm_r_left.png"
                rightTexture: "qrc:/orbital/assets/character/arm_r_right.png"
            }
        }

        // Left arm
        Node {
            id: leftArm
            position: Qt.vector3d(-6, 12, 0)
            eulerRotation: Qt.vector3d(character.armLX, 0, character.armLZ)
            TexturedBox {
                position: Qt.vector3d(0, -6, 0)
                boxWidth: 4
                boxHeight: 12
                boxDepth: 4
                baseColor: "#6A4B3C"
                frontTexture: "qrc:/orbital/assets/character/arm_l_front.png"
                backTexture: "qrc:/orbital/assets/character/arm_l_back.png"
                leftTexture: "qrc:/orbital/assets/character/arm_l_left.png"
                rightTexture: "qrc:/orbital/assets/character/arm_l_right.png"
            }
        }

        // Head
        Node {
            id: headPivot
            position: Qt.vector3d(0, 12, 0)
            eulerRotation: Qt.vector3d(character.headPitch, character.headYaw, character.headTilt)

            TexturedBox {
                position: Qt.vector3d(0, 4, 0)
                boxWidth: 8
                boxHeight: 8
                boxDepth: 8
                baseColor: "#8D624C"
                frontTexture: "qrc:/orbital/assets/character/head_front.png"
                backTexture: "qrc:/orbital/assets/character/head_back.png"
                leftTexture: "qrc:/orbital/assets/character/head_left.png"
                rightTexture: "qrc:/orbital/assets/character/head_right.png"
                topTexture: "qrc:/orbital/assets/character/head_top.png"
                bottomTexture: "qrc:/orbital/assets/character/head_bottom.png"
            }

            // Eye whites and movable pupils sit just in front of the eyeless
            // face texture. They follow the cursor and look away when the PIN
            // is revealed.
            Model {
                source: "#Cube"
                position: Qt.vector3d(-1.55, 4.55, 4.085)
                scale: Qt.vector3d(0.0165, 0.0115, 0.0025)
                materials: PrincipledMaterial {
                    baseColor: "#FFF8EF"
                    lighting: PrincipledMaterial.NoLighting
                }
            }
            Model {
                source: "#Cube"
                position: Qt.vector3d(1.55, 4.55, 4.085)
                scale: Qt.vector3d(0.0165, 0.0115, 0.0025)
                materials: PrincipledMaterial {
                    baseColor: "#FFF8EF"
                    lighting: PrincipledMaterial.NoLighting
                }
            }
            Model {
                source: "#Cube"
                position: Qt.vector3d(-1.55 + character.lookX * 0.42, 4.55 - character.lookY * 0.28, 4.105)
                scale: Qt.vector3d(0.0072, 0.0078, 0.0025)
                materials: PrincipledMaterial {
                    baseColor: "#1A100D"
                    lighting: PrincipledMaterial.NoLighting
                }
            }
            Model {
                source: "#Cube"
                position: Qt.vector3d(1.55 + character.lookX * 0.42, 4.55 - character.lookY * 0.28, 4.105)
                scale: Qt.vector3d(0.0072, 0.0078, 0.0025)
                materials: PrincipledMaterial {
                    baseColor: "#1A100D"
                    lighting: PrincipledMaterial.NoLighting
                }
            }
        }
    }
}
