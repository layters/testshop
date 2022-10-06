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
import "../../fonts/FontAwesome"//import FontAwesome 1.0
import "../components" as NeroshopComponents

Page {
    id: main_page
    title: qsTr("Main Page")
    Rectangle {
        color: "transparent"
    }
    ///////////////////////////
    function copyToClipboard() { 
        // If text edit string is empty, exit function
        if(!seedDisplayEdit.text) return;
        // Select all text from edit then copy the selected text
        seedDisplayEdit.selectAll()
        seedDisplayEdit.copy()
        console.log("Copied to clipboard");
    }
    ///////////////////////////
    //Wallet {
    //    id: wallet
    //}
    ///////////////////////////
    function generateWalletKeys() {
        // generate a unique wallet seed (mnemonic)
        let error = Wallet.create_random_wallet(walletPasswordField.text, walletPasswordConfirmField.text, neroshopWalletDir + "/auth")//"wallet")
        // if wallet passwords don't match, display error message
        let WALLET_PASSWORD_NO_MATCH = 2
        let WALLET_ALREADY_EXISTS = 3;
        if(error == WALLET_PASSWORD_NO_MATCH) {//if(walletPasswordConfirmField.text != walletPasswordField.text) {
            //walletMessageField.x = walletPasswordConfirmField.x
            walletHint.show(qsTr("Wallet passwords do not match"), -1)
        }
        else if(error == WALLET_ALREADY_EXISTS) {
            walletHint.show(qsTr("A wallet file with the same name already exists"), -1)
        }
        // then copy the mnemonic to the seed display edit
        seedDisplayEdit.text = Wallet.get_mnemonic()
        // show important message (only if wallet keys were successfully created)
        if(seedDisplayEdit.text) {
            //walletMessageField.x = (registerPage.width / 2) - (walletMessageField.width / 2)// place at center of registerPage//generate_key_button.x
            //walletMessageField.y = generate_key_button.y + generate_key_button.height + 20        
            walletMessageField.text = qsTr("These words are the key to your account. Please store them safely!")
            walletMessageField.visible = true       
            // clear wallet password text fields
            walletPasswordField.text = "";
            walletPasswordConfirmField.text = "";
        }
    }
    ///////////////////////////
    function registerWallet() {
        // if not key generated, then generate key
        if(!seedDisplayEdit.text) {
            messageBox.text = qsTr("Please generate your keys before registering")
            messageBox.open()
            return; // exit function and do not proceed any further
        }
        // do a regex check on the username before proceeding
        // make sure username is not taken (requires a database check)
        // register the wallet primary key to the database
        // switch (login) to home page
        //stack.push(home_page)
        pageLoader.source = "HomePage.qml"
        // start local monero daemon or node synchronization (optional)
        /*Wallet.daemonOpen(Script.getString("neroshop.monero.daemon.ip"), 
            Script.getString("neroshop.monero.daemon.port"),
            Script.getBoolean("neroshop.monero.daemon.confirm_external_bind"),
            Script.getBoolean("neroshop.monero.daemon.restricted_rpc"),
            Script.getBoolean("neroshop.monero.daemon.remote"),
            Script.getString("neroshop.monero.daemon.data_dir"),
            Script.getString("neroshop.monero.daemon.network_type"),
            Script.getNumber("neroshop.monero.daemon.restore_height")
        );*/
        // connect to a remote monero node (default)(causes segfault)
        let remote_node = Script.getTableStrings("neroshop.monero.nodes.stagenet")[1]//Script.getTableStrings("neroshop.monero.nodes.mainnet")[0]
        let remote_node_ip = remote_node.split(":")[0]
        let remote_node_port = remote_node.split(":")[1]
        console.log("connecting to remote node " + remote_node_ip + " at port " + remote_node_port)
        Wallet.daemonConnect(remote_node_ip, remote_node_port);//Wallet.daemonConnect(Script.getString("neroshop.monero.daemon.ip"), Script.getString("neroshop.monero.daemon.port"));
    }
    ///////////////////////////    
// consists of login and registration menus
    MessageDialog {
        id: messageBox
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
        id: walletFileDialog
        fileMode: FileDialog.OpenFile
        currentFile: wallet_upload_edit.text // currentFile is deprecated since Qt 6.3. Use selectedFile instead
        folder: neroshopWalletDir//StandardPaths.writableLocation(StandardPaths.HomeLocation) + "/neroshop"//StandardPaths.writableLocation(StandardPaths.AppDataLocation) // refer to https://doc.qt.io/qt-5/qstandardpaths.html#StandardLocation-enum
        nameFilters: ["Wallet files (*.keys)"]
        //options: FileDialog.ReadOnly // will not allow you to create folders while file dialog is opened
    }
    ///////////////////////////        
    /*    Button { // must be used in conjunction with a TabBar according to: https://doc.qt.io/qt-5/qml-qtquick-controls2-tabbutton.html
            text: qsTr("Register")
            width: implicitWidth
            onClicked: mainPageStack.currentIndex = 0
        
            background: Rectangle {
                color: buttonsBar.buttonColor//"#00aebf"
            }                    
        }
        Button {
            id: login_button//auth_walletfile_button
            text: qsTr("Login with Wallet file")
            width: implicitWidth
            onClicked: mainPageStack.currentIndex = 1
            
            contentItem: Text {  
                //font.family: "Consolas"; 
                //font.pointSize: 10; 
                //font.bold: true
                
                text: login_button.text
                color: "#ffffff" // white text
            }
        
            background: Rectangle {
                color: buttonsBar.buttonColor//NeroshopComponents.Style.moneroOrangeColor//"#ff6600"//parent.down ? "#bbbbbb" :
                        //(parent.hovered ? "#d6d6d6" : "#f6f6f6")
                radius: 0
                //border.color: login_button.hovered ? "#ffffff" : this.color//"#ffffff"//control.down ? "#17a81a" : "#21be2b"
            }            
        }
        Button {
            text: qsTr("Login with Seed (Mnemonic)")
            width: implicitWidth
            onClicked: mainPageStack.currentIndex = 2
        
            background: Rectangle {
                color: buttonsBar.buttonColor//NeroshopComponents.Style.moneroGrayColor
            }                    
        }  
        Button {
            text: qsTr("Login with Keys")
            width: implicitWidth
            onClicked: mainPageStack.currentIndex = 3
        
        
            background: Rectangle {
                color: buttonsBar.buttonColor//"#402ef7"
            }                    
        }          
        Button {
            text: qsTr("Login with Hardware wallet")
            width: implicitWidth
            //onClicked: mainPageStack.currentIndex = 4
        
            background: Rectangle {
                color: buttonsBar.buttonColor//"red"
            }                    
        }       */                         
    ///////////////////////////
    StackLayout { // Perfect for a stack of items where only one item is visible at a time//ColumnLayout { // From top to bottom
        id: mainPageStack // auth_menu inside home menu
        anchors.fill: parent // will fill entire Window area
        currentIndex: 0//buttonsBar.currentIndex//currentIndex: 1
                
        // generate auth keys page
        Rectangle {
            id: registerPage
            color: NeroshopComponents.Style.getColorsFromTheme()[0]
            
    ///////////////////////////
        Button {
            id: pageRedirectButton
            anchors.verticalCenter: parent.verticalCenter//Layout.alignment: Qt.AlignVCenter// | Qt::AlignRight //or Qt::AlignLeft
            anchors.right: parent.right//anchors.left: parent.left
            anchors.rightMargin: 20
            implicitWidth: 60; height: implicitWidth
            text: qsTr(FontAwesome.angleRight)//qsTr(FontAwesome.angleLeft)
            
            background: Rectangle {
                color: "#6b5b95"//"#121212"
                radius: 100
                border.color: parent.contentItem.color//(NeroshopComponents.Style.darkTheme) ? parent.contentItem.color : "#000000"
                border.width: (parent.hovered) ? 1 : 0
            }
            
            contentItem: Text {
                text: parent.text
                color: "#ffffff"
                horizontalAlignment: Text.AlignHCenter
                verticalAlignment: Text.AlignVCenter
                font.bold: true
                font.family: FontAwesome.fontFamily
                font.pixelSize: (parent.width / 2)
            }
            
            onClicked: {
                mainPageStack.currentIndex = mainPageStack.currentIndex + 1
            }
        }            
            // change wallet path edit
            // ...
            // wallet password create edit
            TextField {
                id: walletPasswordField
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
                id: walletPasswordConfirmField
                placeholderText: qsTr("Confirm Wallet Password")
                placeholderTextColor: "#a9a9a9" // darkgray
                color: NeroshopComponents.Style.moneroOrangeColor//"#ff6600" // textColor
                echoMode: TextInput.Password
                inputMethodHints: Qt.ImhSensitiveData
                selectByMouse: true
                x: walletPasswordField.x
                y: walletPasswordField.y + walletPasswordField.height + 5
                width: walletPasswordField.width; height: walletPasswordField.height   
                background: Rectangle { 
                    color: NeroshopComponents.Style.moneroGrayColor
                    border.color: parent.placeholderTextColor
                    border.width: (NeroshopComponents.Style.darkTheme) ? 1 : 0
                }                              
            }
            // tooltip
            NeroshopComponents.Hint {
                id: walletHint
                text: "This is the wallet password and psuedonym error message tooltip"
                height: walletPasswordField.height + walletPasswordConfirmField.height + 5 // 5 is the gap between the two
                x: walletPasswordField.x + walletPasswordField.width + 5//anchors.left: walletPasswordField.right
                //anchors.leftMargin: 0
                y: walletPasswordField.y//anchors.top: walletPasswordField.top
                visible: false
                color: "firebrick"
                rect.opacity: 1.0
                //textColor: "#ffffff"
                //borderColor:
                //borderWidth:
                radius: 0
                
            }
            // generate key button
            Button {
                id: generate_key_button
                text: qsTr("Generate Keys")
                x: 20
                y: 20
                width: 150
                height: 60 + 5//50 // width will be set automatically based on text length
                
                onClicked: generateWalletKeys()
                
                contentItem: Text {  
                    //font.family: "Consolas";
                    //font.pointSize: 10; 
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
                id: walletMessageField
                visible: false
                anchors.horizontalCenter: registerPage.horizontalCenter// place at center of registerPage//generate_key_button.x
                anchors.top: generate_key_button.bottom
                anchors.topMargin: 20              
                /*x: seedDisplayScrollView.x
                y: seedDisplayScrollView.y + seedDisplayScrollView.height + 10*/
                readOnly: true
                //text: qsTr("")
                color: "#ffffff"// text color
                background: Rectangle { 
                    color: "firebrick"
                }            
            }            
            // wallet seed display
            ScrollView {
                id: seedDisplayScrollView
                //anchors.fill: parent
                width: 800//500//300
                height: 250//150    
                x: generate_key_button.x//20       
                y: (walletMessageField.visible) ? walletMessageField.y + walletMessageField.height + 20 : generate_key_button.y + generate_key_button.height + 20//registerPage.height - this.height - 20                     
                clip: true
                
                TextArea {
                    id: seedDisplayEdit
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
                                //console.log("Left mouse clicked on seedDisplayEdit");
                            }
                            // right mouse = selectAll()
                            if ((mouse.button == Qt.RightButton)) {
                                //console.log("Right mouse clicked on seedDisplayEdit");
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
            // seedCopyButton (copies to the clipboard)
                Button {
                    id: seedCopyButton
                    x: seedDisplayScrollView.width - seedDisplayScrollView.ScrollBar.horizontal.visualPosition//(seedDisplayScrollView.width - width)//anchors.left: parent.left//anchors.right: parent.right
                    //y: seedDisplayScrollView.ScrollBar.vertical.visualPosition//(seedDisplayScrollView.height - height)//anchors.leftMargin: (parent.width - this.width - 10)//anchors.rightMargin: 10
                    anchors.bottom: parent.bottom
                    anchors.bottomMargin: 10
                    width: 32; height: this.width//50
                    text: qsTr("Copy")
                    display: AbstractButton.IconOnly
                    onClicked: copyToClipboard() // this only works on selected text
                    icon.source: "file:///" + neroshopResourcesDir + "/copy.png"
                    icon.color: "#ffffff"
                    //clip: false
                    //icon.height: 24; icon.width: 24
                    background: Rectangle {
                        color: "transparent"
                        radius: 0
                        border.color: this.parent.hovered ? "#ffffff" : this.color
                    }
                    ToolTip.delay: 500 // shows tooltip after hovering for 0.5 second
                    ToolTip.visible: hovered
                    ToolTip.text: qsTr("Copy to clipboard")                                   
                }                                                         
                }
            }
            // optional pseudonym edit
            TextField {
                id: optNameEdit
                placeholderText: qsTr("Pseudonym (optional)")
                placeholderTextColor: "#696969" // dim gray
                color:"#6b5b95"
                //x: seedDisplayScrollView.x
                anchors.left: seedDisplayScrollView.left
                //y: seedDisplayScrollView.y + seedDisplayScrollView.height + 30
                anchors.top: seedDisplayScrollView.bottom
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
                anchors.left: optNameEdit.right
                anchors.leftMargin: 20
                anchors.verticalCenter: optNameEdit.verticalCenter
                
                width: 180//seedCopyButton.width
                height: 60 + 5//50 // width will be set automatically based on text length
                onClicked: registerWallet()
                
                contentItem: Text {  
                    //font.family: "Consolas"; //font.family: NeroshopComponents.Style.fontFiraCodeLight.name
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
        } // eof registerPage
        // walletfile auth page
        // Upload button with read-only textfield
        Rectangle {
            // todo: create buttons: restore from keys, restore from mnemonic, restore from hardware wallet
            id: loginPage
            color: NeroshopComponents.Style.getColorsFromTheme()[0]
            //gradient: Gradient {
            //    GradientStop { position: 0.0; color: "white" }
            //    GradientStop { position: 1.0; color: "black" }
            //}            
            //anchors.right: parent.right//implicitWidth: 200
            //implicitHeight: 200      
            // add spacing from parent (padding - located inside the borders of an element)
            //anchors.margins: 50//anchors.leftPadding: 20
    ///////////////////////////
        Button {
            id: pageRedirectButton1
            anchors.verticalCenter: parent.verticalCenter//Layout.alignment: Qt.AlignVCenter | Qt::AlignLeft
            anchors.left: parent.left
            anchors.leftMargin: 20
            implicitWidth: 60; height: implicitWidth
            text: qsTr(FontAwesome.angleLeft)
            
            background: Rectangle {
                color: "#6b5b95"//"#121212"
                radius: 100
                border.color: parent.contentItem.color
                border.width: (parent.hovered) ? 1 : 0
            }
            
            contentItem: Text {
                text: parent.text
                color: "#ffffff"
                horizontalAlignment: Text.AlignHCenter
                verticalAlignment: Text.AlignVCenter
                font.bold: true
                font.family: FontAwesome.fontFamily
                font.pixelSize: (parent.width / 2)
            }
            
            onClicked: {
                mainPageStack.currentIndex = mainPageStack.currentIndex - 1
            }
        }            
            // wallet_file name edit
            // ...
            /*    ButtonGroup {
                    id: walletRestoreMethodButtonGroup
                    exclusive: true // only one button selected at a time
                    onClicked: {
                        console.log("Selected", button.text, "button")
                        button.checked = true
                        //onClicked: walletRestoreStack.currentIndex = 0
                    }
                }
            // to add a button to the button group (within the Button object itself): ButtonGroup.group: walletRestoreMethodButtonGroup // attaches a button to a button group
            */
            ////StackLayout {
            ////    anchors.fill: loginPage
            ////    id: walletRestoreStack
            ////    Rectangle {
            ////        id: restoreFromWalletFile
            ////    }
            ////    Rectangle {
            ////        id: restoreFromMnemonicSeed
            ////    }
            ////    Rectangle {
            ////        id: restoreFromKeys
            ////    }
            ////    Rectangle {
            ////        id: restoreFromHardwareWallet
            ////    }                                    
            ////}
            //ScrollView {
            //    id: wallet_upload_scrollview
            TextField {
                id: wallet_upload_edit // for displaying wallet file name or path
                x: (loginPage.width - wallet_upload_button.width - this.width) / 2//20
                y: 20 // top_margin
                width: 300; height: 30
                readOnly: true
                text: walletFileDialog.file//property url source: walletFileDialog.file; text: this.source
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
                onClicked: walletFileDialog.open()
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
                    color: loginPage.color
                    //opacity: 0.5
                    border.color: "#696969" // dim gray//"#ffffff"
                }      
            }
            // confirm button
            Button {
                id: wallet_file_confirm_button
                text: qsTr("Confirm")
                x: wallet_password_edit.x
                y: loginPage.height - this.height - 20//wallet_password_edit.y + wallet_password_edit.height + 0
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
    }    
}    
