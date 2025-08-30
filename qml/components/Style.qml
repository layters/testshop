pragma Singleton

import QtQuick 2.12 //import QtQml

QtObject {
    // Fonts
    property QtObject fontFiraCodeBold: FontLoader { id: _fontFiraCodeBold; source: "qrc:/assets/fonts/FiraCode-Bold.ttf" }
    property QtObject fontFiraCodeLight: FontLoader { id: _fontFiraCodeLight; source: "qrc:/assets/fonts/FiraCode-Light.ttf" }
    property QtObject fontFiraCodeMedium: FontLoader { id: _fontFiraCodeMedium; source: "qrc:/assets/fonts/FiraCode-Medium.ttf" }
    property QtObject fontFiraCodeRegular: FontLoader { id: _fontFiraCodeRegular; source: "qrc:/assets/fonts/FiraCode-Regular.ttf" }
    property QtObject fontFiraCodeRetina: FontLoader { id: _fontFiraCodeRetina; source: "qrc:/assets/fonts/FiraCode-Retina.ttf" }
    property QtObject fontFiraCodeSemiBold: FontLoader { id: _fontFiraCodeSemiBold; source: "qrc:/assets/fonts/FiraCode-SemiBold.ttf" }
    ////readonly property QtObject font<name><style>: FontLoader { id: _font<name><style>; source: "qrc:/assets/fonts/" }
    // General settings
    property bool darkTheme: Settings.getBool("dark_theme")//true
    property string themeName: Settings.getString("theme")//"DefaultDark"
    // Colors    
    property string neroshopPurpleColor: "#6b5b95"
    property string neroshopPurpleTintedColor: "#8071a8"
    property string neroshopPurpleShadedColor: "#39304f"//"#50446f"//"#39304f"
    property string disabledColor: "#808080"
    property string moneroGrayColor: "#4c4c4c"
    property string moneroOrangeColor: "#ff6600"
    // Functions
    function getColorsFromTheme() {
        let primaryColor = ""
        let secondaryColor = ""
        let tertiaryColor = ""
        
        if(darkTheme) {
            if(themeName.match("PurpleDust")) {
                primaryColor = "#141419"
                secondaryColor = "#292933"
                tertiaryColor = "#515162"//"#4f4f63"
            }
            else { //"DefaultDark"
                primaryColor = "#131415"//"#1a1a1a"//"#202020"////"#141414"
                secondaryColor = "#202020"//"#2e2e2e"////"#1a1a1a"
                tertiaryColor = "#595959"
            }
        }
        else if(!darkTheme) { //"DefaultLight"
            primaryColor = "#c1c1c5"
            secondaryColor = "#909093"
            tertiaryColor = "#d7d7da"
        }
        return [primaryColor, secondaryColor, tertiaryColor];
    }    
    
}
