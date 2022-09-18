pragma Singleton
import QtQuick 2.12

Object {

    //fontawesome-free-6.2.0-desktop // source: https://fontawesome.com/v6/download
    FontLoader { 
        id: regular
        source: "otfs/Font Awesome 6 Free-Regular-400.otf"
    }
    
    FontLoader { 
        id: solid
        source: "otfs/Font Awesome 6 Free-Solid-900.otf"
    }
    
    FontLoader { 
        id: brands
        source: "otfs/Font Awesome 6 Brands-Regular-400.otf" 
    }
    
    property string fontFamily: regular.name
    property string fontFamilyBrands: brands.name
    property string fontFamilySolid: solid.name

    // Icons used in neroshop GUI (Font Awesome version 6.2.0)
    // To add new icons, check unicodes in Font Awesome Free's Cheatsheet:
    // https://fontawesome.com/v5/cheatsheet/free/solid
    // https://fontawesome.com/v5/cheatsheet/free/regular
    // https://fontawesome.com/v5/cheatsheet/free/brands
    
    property string arrowAltCircleRight: "\uf35a"
    property string arrowAltCircleLeft: "\uf359"
    property string angleLeft: "\uf104"
    property string angleRight: "\uf105"
    ////property string upload: "\u"
    ////property string ?: "\u"
    ////property string user: "\uf007"
    ////property string ?: "\u"
    ////property string heart: "\uf004"
    ////property string star: "\uf005"
    ////property string starHalf: "\uf089"
    ////property string ?: "\u"
}    
        
