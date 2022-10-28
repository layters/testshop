import QtQuick 2.12
import QtQuick.Shapes 1.12

Shape {
    id: root

    enum Direction {
        Up,
        Down,
        Left,
        Right
    }

    property int direction: Triangle.Direction.Up

    property alias strokeColor: shapePath.strokeColor
    property alias fillColor: shapePath.fillColor

    implicitWidth: 100
    implicitHeight: 50

    ShapePath {
        id: shapePath

        property var points: shapePath.pointsByDirection(root.direction)

        startX: shapePath.points[0].x
        startY: shapePath.points[0].y

        PathLine { x: shapePath.points[1].x; y: shapePath.points[1].y }
        PathLine { x: shapePath.points[2].x; y: shapePath.points[2].y }
        PathLine { x: shapePath.points[0].x; y: shapePath.points[0].y }

        function pointsByDirection(direction) {
            switch (direction) {
            case Triangle.Direction.Up:
                return [Qt.vector2d(0, root.height),
                        Qt.vector2d(root.width / 2.0, 0),
                        Qt.vector2d(root.width, root.height)
                ]
            case Triangle.Direction.Down:
                return [Qt.vector2d(0, 0),
                        Qt.vector2d(root.width / 2.0, root.height),
                        Qt.vector2d(root.width, 0)
                ]
            case Triangle.Direction.Left:
                return [Qt.vector2d(0, root.height / 2.0),
                        Qt.vector2d(root.width, 0),
                        Qt.vector2d(root.width, root.height)
                ]
            case Triangle.Direction.Right:
                return [Qt.vector2d(0, 0),
                        Qt.vector2d(0, root.height),
                        Qt.vector2d(root.width, root.height / 2.0)
                ]
            }
            return [Qt.vector2d(0, 0), Qt.vector2d(0, 0), Qt.vector2d(0, 0)]
        }
    }
}
