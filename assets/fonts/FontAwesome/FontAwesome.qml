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
    
    // some icons will only be displayed when the font.weight is Font.Bold, Font.ExtraBold or Font.Black :(
    property string monero: "\uf3d0"
    property string arrowAltCircleRight: "\uf35a"
    property string arrowAltCircleLeft: "\uf359"
    property string angleLeft: "\uf104"
    property string angleRight: "\uf105"
    property string cog: "\uf013"
    property string coin: "\uf85c"
    property string coins: "\uf51e"
    property string lock: "\uf023"
    property string lockOpen: "\uf3c1"
    property string eye: "\uf06e"
    property string eyeSlash: "\uf070"
    property string check: "\uf00c"
    property string xmark: "\uf00d"
    property string circleInfo: "\uf05a"
    property string triangleExclamation: "\uf071"
    property string circleExclamation: "\uf06a"
    property string seedling: "\uf4d8"
    property string heart: "\uf004"
    property string star: "\uf005"
    property string starHalf: "\uf089"; property string starHalfStroke: "\uf5c0"
    property string square: "\uf0c8"
    property string circle: "\uf111"
    property string cloud: "\uf0c2"
    property string house: "\uf015"
    property string ellipsis: "\uf141"
    property string cartShopping: "\uf07a"
    property string shop: "\uf54f"
    property string store: "\uf54e"
    property string tag: "\uf02b"
    property string tags: "\uf02c"
    property string truck: "\uf0d1"
    property string dolly: "\uf472"
    property string personDolly: "\uf4d0" // use this icon for item pickup option
    property string paperPlane: "\uf1d8"
    property string ship: "\uf21a"
    property string shield: "\uf132"
    property string shieldHalf: "\uf3ed"
    property string bolt: "\uf0e7"
    property string trash: "\uf1f8"
    property string inbox: "\uf01c"
    property string volume: "\uf6a8"
    property string barcode: "\uf02a"
    property string barcodeRead: "\uf464"
    //property string plus: "\u2b"
    property string pen: "\uf304"
    property string penToSquare: "\uf044"
    property string droplet: "\uf043"
    property string fire: "\uf06d"
    property string globe: "\uf0ac"
    property string bug: "\uf188"
    property string database: "\uf1c0"
    property string doorOpen: "\uf52b"
    property string mailbox: "\uf813"
    property string wallet: "\uf555"
    property string desktop: "\uf390"
    property string robot: "\uf544"
    property string handshake: "\uf2b5"
    ////property string terminal: ""
    ////property string upload: "\u"
    ////property string ?: "\u"
    ////property string user: "\uf007"
    ////property string ?: "\u"
    ////property string ?: "\u"
}    
        
