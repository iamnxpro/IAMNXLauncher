import QtQuick
import QtQuick3D

// Blocky Minecraft-style body part with the appropriate skin crops on each
// side. The solid cube underneath provides depth and hides seams.
Node {
    id: box

    property real boxWidth: 8
    property real boxHeight: 12
    property real boxDepth: 4
    property color baseColor: "#4A3027"
    property string frontTexture: ""
    property string backTexture: ""
    property string leftTexture: ""
    property string rightTexture: ""
    property string topTexture: ""
    property string bottomTexture: ""

    Model {
        source: "#Cube"
        scale: Qt.vector3d(box.boxWidth / 100, box.boxHeight / 100, box.boxDepth / 100)

        materials: PrincipledMaterial {
            baseColor: box.baseColor
            roughness: 0.95
            metalness: 0.0
        }
    }

    TexturedFace {
        faceTexture: box.frontTexture
        faceWidth: box.boxWidth
        faceHeight: box.boxHeight
        facePosition: Qt.vector3d(0, 0, box.boxDepth / 2 + 0.025)
    }

    TexturedFace {
        faceTexture: box.backTexture
        faceWidth: box.boxWidth
        faceHeight: box.boxHeight
        facePosition: Qt.vector3d(0, 0, -box.boxDepth / 2 - 0.025)
        faceRotation: Qt.vector3d(0, 180, 0)
    }

    TexturedFace {
        faceTexture: box.rightTexture
        faceWidth: box.boxDepth
        faceHeight: box.boxHeight
        facePosition: Qt.vector3d(box.boxWidth / 2 + 0.025, 0, 0)
        faceRotation: Qt.vector3d(0, 90, 0)
    }

    TexturedFace {
        faceTexture: box.leftTexture
        faceWidth: box.boxDepth
        faceHeight: box.boxHeight
        facePosition: Qt.vector3d(-box.boxWidth / 2 - 0.025, 0, 0)
        faceRotation: Qt.vector3d(0, -90, 0)
    }

    TexturedFace {
        faceTexture: box.topTexture
        faceWidth: box.boxWidth
        faceHeight: box.boxDepth
        facePosition: Qt.vector3d(0, box.boxHeight / 2 + 0.025, 0)
        faceRotation: Qt.vector3d(-90, 0, 0)
    }

    TexturedFace {
        faceTexture: box.bottomTexture
        faceWidth: box.boxWidth
        faceHeight: box.boxDepth
        facePosition: Qt.vector3d(0, -box.boxHeight / 2 - 0.025, 0)
        faceRotation: Qt.vector3d(90, 0, 0)
    }
}
