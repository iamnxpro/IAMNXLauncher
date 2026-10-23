import QtQuick
import QtQuick3D

// A single textured quad used to build a Minecraft-style box out of the
// individual skin faces. Keeping faces separate means the 64x64 skin atlas
// can be used without custom vertex UV generation.
Node {
    id: face

    property string faceTexture: ""
    property real faceWidth: 8
    property real faceHeight: 8
    property vector3d facePosition: Qt.vector3d(0, 0, 0)
    property vector3d faceRotation: Qt.vector3d(0, 0, 0)
    property bool faceVisible: true

    visible: faceVisible && faceTexture !== ""

    Model {
        source: "#Rectangle"
        position: face.facePosition
        eulerRotation: face.faceRotation
        scale: Qt.vector3d(face.faceWidth / 100, face.faceHeight / 100, 1)

        materials: PrincipledMaterial {
            baseColorMap: Texture {
                source: face.faceTexture
                magFilter: Texture.Nearest
                minFilter: Texture.Nearest
                mipFilter: Texture.None
            }
            lighting: PrincipledMaterial.NoLighting
        }
    }
}
