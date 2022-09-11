pragma Singleton

import QtQuick 2.12 //import QtQml

QtObject {
    //objectName: "Style"
    //id: attributes
    //property string name
    //property int size
    //property variant attributes
    //FontLoader { id: fixedFont; name: "Courier" }
    //FontLoader { id: webFont; source: "http://www.mysite.com/myfont.ttf" }    
    // General settings
    property bool darkTheme: true // or lightTheme
    // Catalog settings
    property bool gridView: true // or listView
    //property bool infiniteScroll: false // or loadMorePagination or numberPagination or prevNextPagination // infinite scroll will require a scrollview
    // Colors    
    property string disabledColor: "#808080"
    
    property string moneroGrayColor: "#4c4c4c"
    property string moneroOrangeColor: "#ff6600" // not sure if correct color
    
}
