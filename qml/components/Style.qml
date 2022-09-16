pragma Singleton

import QtQuick 2.12 //import QtQml

QtObject {
    // Fonts
    property QtObject fontFiraCodeBold: FontLoader { id: _fontFiraCodeBold; source: "qrc:/fonts/FiraCode-Bold.ttf" }
    property QtObject fontFiraCodeLight: FontLoader { id: _fontFiraCodeLight; source: "qrc:/fonts/FiraCode-Light.ttf" }
    property QtObject fontFiraCodeMedium: FontLoader { id: _fontFiraCodeMedium; source: "qrc:/fonts/FiraCode-Medium.ttf" }
    property QtObject fontFiraCodeRegular: FontLoader { id: _fontFiraCodeRegular; source: "qrc:/fonts/FiraCode-Regular.ttf" }
    property QtObject fontFiraCodeRetina: FontLoader { id: _fontFiraCodeRetina; source: "qrc:/fonts/FiraCode-Retina.ttf" }
    property QtObject fontFiraCodeSemiBold: FontLoader { id: _fontFiraCodeSemiBold; source: "qrc:/fonts/FiraCode-SemiBold.ttf" }
    ////readonly property QtObject font<name><style>: FontLoader { id: _font<name><style>; source: "qrc:/fonts/" }
    // General settings
    property bool darkTheme: true // or lightTheme
    // Catalog settings
    property bool gridView: true // or listView
    //property bool infiniteScroll: false // or loadMorePagination or numberPagination or prevNextPagination // infinite scroll will require a scrollview
    // Colors    
    property string neroshopPurpleColor: "#6b5b95"
    property string neroshopPurpleTintedColor: "#8071a8"
    property string neroshopPurpleShadedColor: "#39304f"//"#50446f"//"#39304f"
    
    property string disabledColor: "#808080"
    property string pageDarkBackgroundColor: "#121212" // #121212 = rgb(18, 18, 18)//property string pageDimBackgroundColor: "#2e2e2e" // #2e2e2e = rgb(46, 46, 46);
    property string pageLightBackgroundColor: "#a0a0a0" // = rgb(160, 160, 160)
    
    property string moneroGrayColor: "#4c4c4c"
    property string moneroOrangeColor: "#ff6600" // not sure if correct color
    
}
