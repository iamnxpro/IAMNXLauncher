import QtQuick
import QtQuick3D

// The 3D layer: a low-poly sci-fi portal, a rotating energy ring, sparks, and
// the block companion. It deliberately avoids shadows, post-processing and
// temporal AA so it stays comfortable on integrated GPUs.
Item {
    id: scene

    property real lookX: 0
    property real lookY: 0
    property bool rtl: false
    property string mood: "idle"
    property bool portalActive: false
    property bool portalClosing: false
    property real suckProgress: 0

    property real portalScale: portalActive ? (portalClosing ? 0.01 : 1.0) : 0.34
    property real portalOpacity: portalActive ? (portalClosing ? 0.0 : 1.0) : 0.13
    property real cameraZ: portalActive ? (portalClosing ? 700 : 605) : 700

    Behavior on portalScale {
        NumberAnimation {
            duration: scene.portalClosing ? 260 : 950
            easing.type: scene.portalClosing ? Easing.InCubic : Easing.OutCubic
        }
    }
    Behavior on portalOpacity {
        NumberAnimation {
            duration: scene.portalClosing ? 240 : 800
            easing.type: Easing.OutCubic
        }
    }
    Behavior on cameraZ { NumberAnimation { duration: 900; easing.type: Easing.InOutCubic } }
    Behavior on suckProgress { NumberAnimation { duration: 1000; easing.type: Easing.InCubic } }

    function suck() {
        suckProgress = 1;
    }

    function reset() {
        suckProgress = 0;
    }

    View3D {
        anchors.fill: parent
        renderMode: View3D.Offscreen

        environment: SceneEnvironment {
            backgroundMode: SceneEnvironment.Transparent
            clearColor: "transparent"
            antialiasingMode: SceneEnvironment.NoAA
            temporalAAEnabled: false
        }

        PerspectiveCamera {
            id: camera
            position: Qt.vector3d(0, 0, scene.cameraZ)
            fieldOfView: 45
            clipNear: 5
            clipFar: 4000
        }

        DirectionalLight {
            eulerRotation: Qt.vector3d(-24, -32, 0)
            brightness: 1.15
            ambientColor: "#4A2418"
        }

        PointLight {
            position: Qt.vector3d(180, 150, 320)
            color: "#FF7A2A"
            brightness: 2.8
            quadraticFade: 1.0
        }

        PointLight {
            position: Qt.vector3d(-120, 120, 260)
            color: "#FFD7B0"
            brightness: 1.2
            quadraticFade: 1.2
        }

        // ------------------------------------------------------------------
        // Portal
        // ------------------------------------------------------------------
        Node {
            id: portalRoot
            position: Qt.vector3d(0, 0, -250)
            scale: Qt.vector3d(scene.portalScale, scene.portalScale, scene.portalScale)
            opacity: scene.portalOpacity

            Model {
                source: "#Rectangle"
                scale: Qt.vector3d(6.6, 6.6, 1)
                position: Qt.vector3d(0, 0, -6)

                materials: PrincipledMaterial {
                    baseColorMap: Texture { source: "qrc:/orbital/assets/fx/portal.png" }
                    alphaMode: PrincipledMaterial.Blend
                    lighting: PrincipledMaterial.NoLighting
                }
            }

            // Inner darkness gives the portal depth instead of a flat decal.
            Model {
                source: "#Sphere"
                scale: Qt.vector3d(1.18, 1.18, 0.18)
                position: Qt.vector3d(0, 0, -18)
                materials: PrincipledMaterial {
                    baseColor: "#120704"
                    roughness: 0.35
                    metalness: 0.15
                }
            }

            Node {
                id: ring
                property real spin: 0
                eulerRotation: Qt.vector3d(0, 0, spin)
                NumberAnimation on spin {
                    loops: Animation.Infinite
                    from: 0
                    to: 360
                    duration: 15000
                }

                Repeater3D {
                    model: 48
                    delegate: Model {
                        property real angle: index * 7.5
                        source: "#Cube"
                        position: Qt.vector3d(Math.cos(angle * Math.PI / 180) * 255,
                                              Math.sin(angle * Math.PI / 180) * 255,
                                              0)
                        eulerRotation: Qt.vector3d(0, 0, angle)
                        scale: Qt.vector3d(0.07, 0.62, 0.07)
                        materials: PrincipledMaterial {
                            baseColor: index % 3 === 0 ? "#FFD9B8" : "#FF5E1A"
                            emissiveFactor: Qt.vector3d(1.0, 0.25, 0.03)
                            lighting: PrincipledMaterial.NoLighting
                        }
                    }
                }
            }

            Node {
                id: innerRing
                property real spin: 0
                eulerRotation: Qt.vector3d(0, 0, spin)
                NumberAnimation on spin {
                    loops: Animation.Infinite
                    from: 360
                    to: 0
                    duration: 9500
                }

                Repeater3D {
                    model: 26
                    delegate: Model {
                        property real angle: index * (360 / 26)
                        source: "#Cube"
                        position: Qt.vector3d(Math.cos(angle * Math.PI / 180) * 184,
                                              Math.sin(angle * Math.PI / 180) * 184,
                                              2)
                        eulerRotation: Qt.vector3d(0, 0, angle + 45)
                        scale: Qt.vector3d(0.045, 0.42, 0.045)
                        materials: PrincipledMaterial {
                            baseColor: "#FFB27A"
                            emissiveFactor: Qt.vector3d(1.0, 0.38, 0.08)
                            lighting: PrincipledMaterial.NoLighting
                        }
                    }
                }
            }

            Node {
                id: sparks
                property real spin: 0
                eulerRotation: Qt.vector3d(0, 0, spin)
                NumberAnimation on spin {
                    loops: Animation.Infinite
                    from: 0
                    to: 360
                    duration: 22000
                }

                Repeater3D {
                    model: 22
                    delegate: Model {
                        property real angle: index * (360 / 22)
                        property real radius: 105 + (index % 5) * 33
                        source: "#Cube"
                        position: Qt.vector3d(Math.cos(angle * Math.PI / 180) * radius,
                                              Math.sin(angle * Math.PI / 180) * radius,
                                              (index % 4) * 9 - 5)
                        eulerRotation: Qt.vector3d(angle, angle * 0.5, 0)
                        scale: Qt.vector3d(0.055, 0.055, 0.055)
                        materials: PrincipledMaterial {
                            baseColor: "#FFE0C8"
                            emissiveFactor: Qt.vector3d(1.0, 0.45, 0.1)
                            lighting: PrincipledMaterial.NoLighting
                        }
                    }
                }
            }
        }

        // ------------------------------------------------------------------
        // Companion
        // ------------------------------------------------------------------
        Node {
            id: characterNode
            property real baseX: scene.rtl ? -285 : 285
            property real baseYaw: scene.rtl ? 16 : -16

            position: Qt.vector3d(baseX * (1 - scene.suckProgress),
                                  -112 + 112 * scene.suckProgress,
                                  120 - 380 * scene.suckProgress)
            scale: {
                const s = 5.0 * (1 - 0.94 * scene.suckProgress);
                return Qt.vector3d(s, s, s);
            }
            eulerRotation: Qt.vector3d(0,
                                       baseYaw * (1 - scene.suckProgress) + 720 * scene.suckProgress,
                                       620 * scene.suckProgress)

            Character {
                lookX: scene.lookX
                lookY: scene.lookY
                mood: scene.mood
                rtl: scene.rtl
            }
        }
    }
}
