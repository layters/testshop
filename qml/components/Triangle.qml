import QtQuick 2.12
import QtQuick.Controls 2.12
import QtQuick.Shapes 1.3 // (since Qt 5.10) // Shape
import QtGraphicalEffects 1.12//Qt5Compat.GraphicalEffects 1.15//= Qt6// ColorOverlay

Item {
    id: triangleTip
    property string direction: "up"//"down"
    property string color: "white"
    readonly property int parentWidth: parent.width // get the Hint (Tooltip)'s width
    readonly property int parentHeight: parent.height // get the Hint (Tooltip)'s height
    readonly property int parentX: parent.x
    readonly property int parentY: parent.y
    
    Image {
        id: triangle
        source: "file:///" + neroshopResourcesDir + "/triangle.png"//neroshopResourceDir + "/triangle.png"
        // direction: up
        x: parent.parentX + (parent.parentWidth - this.width) / 2
        y: parent.parentY - this.height// + 4
        //transform: Rotation { origin.x: triangle.x; origin.y: triangle.y; axis { x: 0; y: 0; z: 1 } angle: 180 }
        height: 24; width: 24
        visible: triangleTip.visible
    }
    ColorOverlay {
        anchors.fill: triangle
        source: triangle
        color: parent.color
        visible: triangle.visible
    }
}
/*Shape { 
    id: triangle
    property string direction: "down" // todo: add this property to custom Tooltip instead
    property string color: "#ffffff" // default color is set to white
    property string borderColor: "black"
    ////property string borderWidth: 1
    
    function get_offsetX() {
        if(direction == "up") return -20;
        if(direction == "down") return 20;
        if(direction == "left") return 3;
        if(direction == "right") return 4;
        return 20; // "down" value (default)
    }
    
    function get_offsetY() {
        if(direction == "up") return -30;
        if(direction == "down") return 30;
        if(direction == "left") return 3;
        if(direction == "right") return 4;
        return 30; // "down" value (default)
    }    
    readonly property real offsetX: get_offsetX()
    readonly property real offsetY: get_offsetY()

    ShapePath {
        //capStyle: ShapePath.FlatCap//ShapePath.RoundCap
        strokeStyle: ShapePath.SolidLine
        strokeWidth: 1;
        strokeColor: triangle.borderColor
        fillColor: triangle.color
        PathLine { x: -triangle.offsetX ; y: -triangle.offsetY }
        PathLine { x: triangle.offsetX; y: -triangle.offsetY }
        PathLine { x: 0; y: 0 }
    }
}*/          
/*
// source: https://github.com/clogwog/qml-triangle/blob/master/Triangle.qml
import QtQuick 2.4

// Cnavas is said to be slow so Shape is recommended over it
Canvas {
              id: triangle
              antialiasing: true

              property int triangleWidth: 60
              property int triangleHeight: 60
              property color strokeStyle:  "#ffffff"
              property color fillStyle: "#ffffff"
              property int lineWidth: 3
              property bool fill: false
              property bool stroke: true
              property real alpha: 1.0
              states: [
                  State {
                      name: "pressed"; when: ma1.pressed
                      PropertyChanges { target: triangle; fill: true; }
                  }
              ]

              onLineWidthChanged:requestPaint();
              onFillChanged:requestPaint();
              onStrokeChanged:requestPaint();

              signal clicked()

              onPaint: {
                  var ctx = getContext("2d");
                  ctx.save();
                  ctx.clearRect(0,0,triangle.width, triangle.height);
                  ctx.strokeStyle = triangle.strokeStyle;
                  ctx.lineWidth = triangle.lineWidth
                  ctx.fillStyle = triangle.fillStyle
                  ctx.globalAlpha = triangle.alpha
                  ctx.lineJoin = "round";
                  ctx.beginPath();

                  // put rectangle in the middle
                  ctx.translate( (0.5 *width - 0.5*triangleWidth), (0.5 * height - 0.5 * triangleHeight))

                  // draw the rectangle
                  ctx.moveTo(0,triangleHeight/2 ); // left point of triangle
                  ctx.lineTo(triangleWidth, 0);
                  ctx.lineTo(triangleWidth,triangleHeight);

                  ctx.closePath();
                  if (triangle.fill)
                      ctx.fill();
                  if (triangle.stroke)
                      ctx.stroke();
                  ctx.restore();
              }
              MouseArea{
                  id: ma1
                  anchors.fill: parent
                  onClicked: parent.clicked()
              }
}
*/
