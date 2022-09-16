// todo: rename this file to LoginPage.qml
// requires Qt version 5.12 (latest is 5.15 as of this writing). See https://doc.qt.io/qt-5/qt5-intro.html
import QtQuick 2.12//2.7 //(QtQuick 2.7 is lowest version for Qt 5.7)
import QtQuick.Controls 2.12//2.0 // (requires at least Qt 5.12 where QtQuick.Controls 1 is deprecated. See https://doc.qt.io/qt-5/qtquickcontrols-index.html#versions) // needed for built-in styles // Page, TextField, TextArea (multi-lined TextField), TextFieldStyle//import QtQuick.Controls.Material 2.12 // Other styles: 
//import QtQuick.Controls.Material 2.0
//import QtQuick.Controls.Styles 1.4 // ApplicationWindowStyle, TextFieldStyle
import QtQuick.Layouts 1.12//1.15 // The module is new in Qt 5.1 and requires Qt Quick 2.1. // RowLayout, ColumnLayout, GridLayout, StackLayout, Layout
import QtQuick.Dialogs 1.3 // MessageDialog (since Qt 5.2)
import QtGraphicalEffects 1.12 // LinearGradient
import Qt.labs.platform 1.1 // FileDialog (since Qt 5.8) // change to "import QtQuick.Dialogs" if using Qt 6.2

//import neroshop.Wallet 1.0
import "../components" as NeroshopComponents

Page {
    id: main_page
    title: qsTr("Main Page")
    //Rectangle {
    //    color:"red"
    //}
    ///////////////////////////
    function copy_to_clipboard() { 
        // If text edit string is empty, exit function
        if(!seed_display_edit.text) return;
        // Select all text from edit then copy the selected text
        seed_display_edit.selectAll()
        seed_display_edit.copy()
        console.log("Copied to clipboard");
    }
    ///////////////////////////
    //Wallet {
    //    id: wallet
    //}
    ///////////////////////////
    function generate_keys() {
        // generate a unique wallet seed (mnemonic)
        let error = Wallet.create_random_wallet(wallet_password_create_edit.text, wallet_password_confirm_edit.text, neroshopWalletDir + "/auth")//"wallet")
        // if wallet passwords don't match, display error message
        let WALLET_PASSWORD_NO_MATCH = 2
        let WALLET_ALREADY_EXISTS = 3;
        if(error == WALLET_PASSWORD_NO_MATCH) {//if(wallet_password_confirm_edit.text != wallet_password_create_edit.text) {
            //important_message_field.x = wallet_password_confirm_edit.x
            important_message_field.text = qsTr("Wallet passwords do not match")
            important_message_field.visible = true
            /*message_box.text = qsTr("Wallet passwords do not match")
            message_box.open()*/
        }
        else if(error == WALLET_ALREADY_EXISTS) {
            important_message_field.text = qsTr("A wallet file with the same name already exists")
            important_message_field.visible = true
        }
        // then copy the mnemonic to the seed display edit
        seed_display_edit.text = Wallet.get_mnemonic()
        // show important message (only if wallet keys were successfully created)
        if(seed_display_edit.text) {
            //important_message_field.x = (genkey_page.width / 2) - (important_message_field.width / 2)// place at center of genkey_page//generate_key_button.x
            //important_message_field.y = generate_key_button.y + generate_key_button.height + 20        
            important_message_field.text = qsTr("These words are the key to your account. Please store them safely!")
            important_message_field.visible = true       
            // clear wallet password text fields
            wallet_password_create_edit.text = "";
            wallet_password_confirm_edit.text = "";
        }
    }
    ///////////////////////////
    function register_wallet() {
        // if not key generated, then generate key
        if(!seed_display_edit.text) {
            message_box.text = qsTr("Please generate your keys before registering")
            message_box.open()
            return; // exit function and do not proceed any further
        }
        // do a regex check on the username before proceeding
        // make sure username is not taken (requires a database check)
        // register the wallet primary key to the database
        // switch (login) to home page
        //stack.push(home_page)
        page_loader.source = "home_page.qml"
    }
    ///////////////////////////    
// consists of login and registration menus
    MessageDialog {
        id: message_box
        //visible: false
        title: "message"
        text: "It's so cool that you are using Qt Quick."
        //detailedText:
        //icon: StandardIcon.Question
        ////standardButtons: StandardButton.Ok | StandardButton.Cancel
        //modal: true // blocks input to other content beneath the dialog.

        //onAccepted: console.log("Ok clicked")
        //onRejected: console.log("Cancel clicked")
    }
    ///////////////////////////
    FileDialog {
        id: wallet_file_dialog
        fileMode: FileDialog.OpenFile
        currentFile: wallet_upload_edit.text // currentFile is deprecated since Qt 6.3. Use selectedFile instead
        folder: neroshopWalletDir//StandardPaths.writableLocation(StandardPaths.HomeLocation) + "/neroshop"//StandardPaths.writableLocation(StandardPaths.AppDataLocation) // refer to https://doc.qt.io/qt-5/qstandardpaths.html#StandardLocation-enum
        nameFilters: ["Wallet files (*.keys)"]
        //options: FileDialog.ReadOnly // will not allow you to create folders while file dialog is opened
    }
    ///////////////////////////
    StackLayout { // Perfect for a stack of items where only one item is visible at a time//ColumnLayout { // From top to bottom
        id: auth_stack // auth_menu inside home menu
        //anchors.fill: parent // will fill entire Window area
        currentIndex: 1       
        width: 800//500
        height: 500//300 
        x: parent.width / 2 - this.width / 2 // window is the id of ApplicationWindow
        y: parent.height / 2 - this.height / 2

        // generate auth keys page
        Rectangle {
            id: genkey_page
            color: (NeroshopComponents.Style.darkTheme) ? "#2e2e2e" : "#a0a0a0"//160, 160, 160
            // optional pseudonym edit
            TextField {
                id: opt_username_edit
                placeholderText: qsTr("Pseudonym (optional)")
                placeholderTextColor: "#696969" // dim gray
                color:"#6b5b95"
                //x: seed_display_scrollview.x
                anchors.left: seed_display_scrollview.left
                //y: seed_display_scrollview.y + seed_display_scrollview.height + 30
                anchors.top: seed_display_scrollview.bottom
                anchors.topMargin: 30
                selectByMouse: true
                
                width: 300
                background: Rectangle { 
                    color: (NeroshopComponents.Style.darkTheme) ? "transparent": "#101010"//"#101010" = rgb(16, 16, 16)
                    border.color: "#696969" // dim gray//"#ffffff"
                    border.width: (NeroshopComponents.Style.darkTheme) ? 1 : 0
                }                
                //validator: RegExpValidator { regExp: /^(?=.{8,20}$)(?![_.])(?!.*[_.]{2})[a-zA-Z0-9._]+(?<![_.])$/ } // validator: RegularExpressionValidator { regularExpression: /[0-9A-F]+/ } // since Qt  5.14
            }
            // register button
            Button {
                id: wallet_register_button
                text: qsTr("Register")
                //x: copy_button.x//opt_username_edit.x + opt_username_edit.width + 50
                anchors.left: opt_username_edit.right//copy_button.left
                anchors.leftMargin: 20
                //y: opt_username_edit.y + verticalCenter of username_edit
                anchors.top: opt_username_edit.top
                anchors.topMargin: (opt_username_edit.height / 2) - (this.height / 2)
                width: 180//copy_button.width
                height: 60 + 5//50 // width will be set automatically based on text length
                onClicked: register_wallet()
                
                contentItem: Text {  
                    font.family: "Consolas"; //font.family: NeroshopComponents.Style.fontFiraCodeLight.name
                    //font.pointSize: 10
                    font.bold: true
                    text: wallet_register_button.text
                    color: "#ffffff" // white
                    horizontalAlignment: Text.AlignHCenter
                    verticalAlignment: Text.AlignVCenter                    
                }
                
                background: Rectangle {
                    color: "#6b5b95" // #ff6600 is the monero orange color
                    radius: 0
                }                
            }
            // change wallet path edit
            // ...
            // wallet password create edit
            TextField {
                id: wallet_password_create_edit
                placeholderText: qsTr("Wallet Password")
                placeholderTextColor: "#a9a9a9" // darkgray
                color: NeroshopComponents.Style.moneroOrangeColor//"#ff6600" // textColor
                echoMode: TextInput.Password//TextInput.PasswordEchoOnEdit
                inputMethodHints: Qt.ImhSensitiveData
                selectByMouse: true
                //validator: RegularExpressionValidator { regularExpression: "^(?=.*?[A-Z])(?=.*?[a-z])(?=.*?[0-9])(?=.*?[#?!@$%^&*-]).{8,}$" }
                //x: generate_key_button.x + generate_key_button.width + 15
                anchors.left: generate_key_button.right // generate_key_button.x + the width of generate_key_button
                anchors.leftMargin: 15
                //y: generate_key_button.y
                anchors.top: generate_key_button.top//generate_key_button.verticalCenter = will place text field at center of generate_key_button vertically (y)
                
                width: 300//; height: generate_key_button.height / 2         
                background: Rectangle { 
                    color: NeroshopComponents.Style.moneroGrayColor//#4c4c4c = monero gray color//"#404040" = 64, 64, 64
                    border.color: parent.placeholderTextColor
                    border.width: (NeroshopComponents.Style.darkTheme) ? 1 : 0
                }         
                // todo: maybe add a regex validator                    
            }
            // wallet password confirm edit
            TextField {
                id: wallet_password_confirm_edit
                placeholderText: qsTr("Confirm Wallet Password")
                placeholderTextColor: "#a9a9a9" // darkgray
                color: NeroshopComponents.Style.moneroOrangeColor//"#ff6600" // textColor
                echoMode: TextInput.Password
                inputMethodHints: Qt.ImhSensitiveData
                selectByMouse: true
                x: wallet_password_create_edit.x
                y: wallet_password_create_edit.y + wallet_password_create_edit.height + 5
                width: wallet_password_create_edit.width; height: wallet_password_create_edit.height   
                background: Rectangle { 
                    color: NeroshopComponents.Style.moneroGrayColor
                    border.color: parent.placeholderTextColor
                    border.width: (NeroshopComponents.Style.darkTheme) ? 1 : 0
                }                              
            }
            // generate key button
            Button {
                id: generate_key_button
                text: qsTr("Generate Keys")
                x: 20
                y: 20
                width: 150
                height: 60 + 5//50 // width will be set automatically based on text length
                
                onClicked: generate_keys()
                
                contentItem: Text {  
                    font.family: "Consolas";
                    font.pointSize: 10; 
                    font.bold: true
                    text: generate_key_button.text
                    color: "#ffffff" // white
                    horizontalAlignment: Text.AlignHCenter
                    verticalAlignment: Text.AlignVCenter                    
                }
                
                background: Rectangle {
                    color: '#ff6600' // #ff6600 is the monero orange color
                    radius: 0
                }
            }
            // Important message box
            TextField {
                id: important_message_field
                visible: false
                x: (genkey_page.width / 2) - (this.width / 2)// place at center of genkey_page//generate_key_button.x
                y: generate_key_button.y + generate_key_button.height + 20                
                /*x: seed_display_scrollview.x
                y: seed_display_scrollview.y + seed_display_scrollview.height + 10*/
                readOnly: true
                //text: qsTr("")
                color: "#ffffff"// text color
                background: Rectangle { 
                    color: "firebrick"
                }            
            }            
            // wallet seed display
            ScrollView {
                id: seed_display_scrollview
                //anchors.fill: parent
                width: 500//300
                height: 250//150    
                x: generate_key_button.x//20       
                y: (important_message_field.visible) ? important_message_field.y + important_message_field.height + 20 : generate_key_button.y + generate_key_button.height + 20//genkey_page.height - this.height - 20                     
                
                TextArea {
                    id: seed_display_edit
                    readOnly: true
                    //text: qsTr("")
                    color: "#000000" // text color
                    //selectionColor: "transparent"
                    // Style
                    background: Rectangle { 
                        color: "#708090"
                        border.color:"#ffffff" // white (since dark mode is default)//border.width: 1//radius: 2
                    }        
                    // on Left mouse clicked
                    MouseArea {
                        id: seed_display_mouse_area
                        anchors.fill: parent
                        acceptedButtons: Qt.LeftButton | Qt.RightButton
                        //cursorShape: Qt.IBeamCursor
                        onClicked: (mouse)=> {
                            // left mouse = displayListOptions()
                            if ((mouse.button == Qt.LeftButton)) {
                                //console.log("Left mouse clicked on seed_display_edit");
                            }
                            // right mouse = selectAll()
                            if ((mouse.button == Qt.RightButton)) {
                                //console.log("Right mouse clicked on seed_display_edit");
                            }
                        }
                        /*onEntered: {
                            this.cursorShape = Qt.IBeamCursor
                        }*/
                    }
                    // text options for copy, paste, etc. (experimental)
                    /*ListView {
                        anchors.fill: parent
                        ////model: ContactModel {}
                        delegate: contactDelegate
                        highlight: Rectangle { color: "lightsteelblue"; radius: 5 }
                        focus: true
                        x: seed_display_mouse_area.mouseX
                        y: seed_display_mouse_area.mouseY
                    }*/                           
                }
            }
            // copy_button (copies to the clipboard)
            Button {
                id: copy_button
                width: 100; height: seed_display_scrollview.height//50
                //x: (seed_display_scrollview.x + seed_display_scrollview.width) + (genkey_page.width / 2) - ((this.width + seed_display_scrollview.width + 10) / 2) //(seed_display_scrollview.x + seed_display_scrollview.width) + 5
                anchors.left: seed_display_scrollview.right//seed_display_scrollview.horizontalCenter
                anchors.leftMargin: (parent.width / 2) - ((this.width + seed_display_scrollview.width + 10) / 2)
                //y: seed_display_scrollview.y + (seed_display_scrollview.height / 2) - (this.height / 2)
                anchors.top: seed_display_scrollview.top
                anchors.topMargin: (seed_display_scrollview.height / 2) - (this.height / 2)
                text: qsTr("Copy") // rather have an icon with a tooltip than a text
                display: AbstractButton.IconOnly // will only show the icon and not the text
                // this only works on selected text
                onClicked: copy_to_clipboard()//seed_display_edit.copy()//copy_to_clipboard()
                
                ToolTip.delay: 500 // shows tooltip after hovering for 0.5 second
                ToolTip.visible: hovered
                ToolTip.text: qsTr("Copy to clipboard")
                
                icon.source: "file:///" + neroshopResourcesDir + "/copy.png"//neroshopResourceDir + "/copy.png"
                icon.color: "#ffffff"
                //icon.height: 24; icon.width: 24
                
                background: Rectangle {
                    color: "#808080"
                    radius: 0
                    border.color: this.parent.hovered ? "#ffffff" : this.color
                }                   
            }
        } // eof genkey_page
        // walletfile auth page
        // Upload button with read-only textfield
        Rectangle {
            id: wallet_file_auth_page
            color: (NeroshopComponents.Style.darkTheme) ? "#2e2e2e" : "#a0a0a0"
            //gradient: Gradient {
            //    GradientStop { position: 0.0; color: "white" }
            //    GradientStop { position: 1.0; color: "black" }
            //}            
            //anchors.right: parent.right//implicitWidth: 200
            //implicitHeight: 200      
            // add spacing from parent (padding - located inside the borders of an element)
            //anchors.margins: 50//anchors.leftPadding: 20
            // wallet_file name edit
            //ScrollView {
            //    id: wallet_upload_scrollview
            TextField {
                id: wallet_upload_edit // for displaying wallet file name or path
                x: (wallet_file_auth_page.width - wallet_upload_button.width - this.width) / 2//20
                y: 20 // top_margin
                width: 300; height: 30
                readOnly: true
                text: wallet_file_dialog.file//property url source: wallet_file_dialog.file; text: this.source
                ////placeholderText: qsTr("...") // eh ... probably not necessary (better to just leave it blank)
                // change TextField color
                background: Rectangle { 
                    color: "#708090"
                    border.color:"#ffffff" // white (since dark mode is default)//border.width: 1//radius: 2
                }
            }              
            //}
            // upload button
            Button {
                id: wallet_upload_button
                text: qsTr("Upload")
                onClicked: wallet_file_dialog.open()
                display: AbstractButton.IconOnly//AbstractButton.TextBesideIcon//AbstractButton.TextOnly
                //hoverEnabled: true
                x: wallet_upload_edit.x + wallet_upload_edit.width + 5
                y: wallet_upload_edit.y            
                width: 50
                height: wallet_upload_edit.height
                
                icon.source: "file:///" + neroshopResourcesDir + "/upload.png"//neroshopResourceDir + "/upload.png"
                icon.color: "#ffffff"
                // can only have 1 contentItem at a time (a contentItem is not needed for Button)
                /*contentItem: Image {
                    source: neroshopResourceDir + "/upload.png"
                    height: 24; width: 24 // has no effect since image is scaled
                    fillMode:Image.PreserveAspectFit; // the image is scaled uniformly to fit button without cropping // https://doc.qt.io/qt-6/qml-qtquick-image.html#fillMode-prop
                }          
                contentItem: Text {  
                    text: wallet_upload_button.text
                    color: "#ffffff"
                    horizontalAlignment: Text.AlignHCenter
                    verticalAlignment: Text.AlignVCenter
                }*/
        
                background: Rectangle {
                    color: "#808080"/*parent.down ? "#bbbbbb" :
                        (parent.hovered ? "#d6d6d6" : "#f6f6f6")*/
                    radius: 0
                    border.color: wallet_upload_button.hovered ? "#ffffff" : this.color//"#ffffff"//control.down ? "#17a81a" : "#21be2b"
                }            
            
                ToolTip.delay: 500 // shows tooltip after hovering for 0.5 second
                ToolTip.visible: hovered
                ToolTip.text: qsTr("Upload wallet file")            
            }        
            /*Label {
                id: wallet_file_password_label
                text: qsTr("Wallet file password")
            }*/            
            // wallet password textfield
            TextField {//TextInput {
                id: wallet_password_edit
                x: wallet_upload_edit.x
                y: wallet_upload_edit.y + wallet_upload_edit.height + 10
                width: wallet_upload_edit.width//; height: 
                placeholderText: qsTr("Wallet Password")
                placeholderTextColor: "#696969" // dim gray
                color: "#5f3dc4"//"#402ef7"//"orange" // textColor
                echoMode: TextInput.Password//TextInput.PasswordEchoOnEdit // hide sensative text
                selectByMouse: true // can select parts of or all of text with mouse
                // change TextField color (only works with TextFields not TextInputs)
                background: Rectangle { 
                    color: wallet_file_auth_page.color
                    //opacity: 0.5
                    border.color: "#696969" // dim gray//"#ffffff"
                }      
            }
            // confirm button
            Button {
                id: wallet_file_confirm_button
                text: qsTr("Confirm")
                x: wallet_password_edit.x
                y: wallet_file_auth_page.height - this.height - 20//wallet_password_edit.y + wallet_password_edit.height + 0
                width: 150; height: 60 
                //onClicked: Authenticator.auth_walletfile()

                contentItem: Text {  
                    text: wallet_file_confirm_button.text
                    color: "#ffffff"
                    // place text at center of button
                    horizontalAlignment: Text.AlignHCenter//anchors.centerIn: parent // no work :(
                    verticalAlignment: Text.AlignVCenter
                }
                                
                background: Rectangle {
                    color: '#6b5b95'//#5f3dc4 = (95, 61, 196)//add to cart button colors =>//"#6b5b95"//#6b5b95 = (107, 91, 149)//"#483d8b"//#483d8b = (72, 61, 139)//"#ff6600"//#ff6600 is the monero orange color
                    radius: 0
                }            
            }
        } // eof wallet_file_authentication_page
        Rectangle {
            id: seed_auth_page
            color: (NeroshopComponents.Style.darkTheme) ? "#2e2e2e" : "#a0a0a0"
        }
        
        Rectangle {
            id: keys_auth_page
            color: (NeroshopComponents.Style.darkTheme) ? "#2e2e2e" : "#a0a0a0"
        }        
    }
    ///////////////////////////
    RowLayout {//TabBar {
        //id: access_buttons_container
        x: auth_stack.x + (auth_stack.width / 2) - (this.width / 2) // center the x
        y: (auth_stack.y + auth_stack.height) + 5// 5 is the padding // lower the y
        
        Button {//TabButton { // must be used in conjunction with a TabBar according to: https://doc.qt.io/qt-5/qml-qtquick-controls2-tabbutton.html
            text: qsTr("register (genkey)")
            onClicked: auth_stack.currentIndex = 0
        
            background: Rectangle {
                color: "#00aebf"
            }                    
        }
        Button {//TabButton {
            id: login_button//auth_walletfile_button
            text: qsTr("auth_with_walletfile")
            onClicked: auth_stack.currentIndex = 1
            
            contentItem: Text {  
                font.family: "Consolas"; 
                font.pointSize: 10; 
                font.bold: true
                
                text: login_button.text
                color: "#ffffff" // white text
            }
        
            background: Rectangle {
                color: NeroshopComponents.Style.moneroOrangeColor//"#ff6600"//parent.down ? "#bbbbbb" :
                        //(parent.hovered ? "#d6d6d6" : "#f6f6f6")
                radius: 0
                border.color: login_button.hovered ? "#ffffff" : this.color//"#ffffff"//control.down ? "#17a81a" : "#21be2b"
            }            
        }
        Button {//TabButton {
            text: qsTr("auth_with_seed")
            onClicked: auth_stack.currentIndex = 2
        
            background: Rectangle {
                color: NeroshopComponents.Style.moneroGrayColor
            }                    
        }  
        Button {//TabButton {
            text: qsTr("auth_with_keys")
            onClicked: auth_stack.currentIndex = 3
        
            background: Rectangle {
                color: "#402ef7"
            }                    
        }          
        Button {//TabButton {
            text: qsTr("auth_with_hw")
            //onClicked: auth_stack.currentIndex = 4
        
            background: Rectangle {
                color: "red"
            }                    
        }                                
    }    
}    
