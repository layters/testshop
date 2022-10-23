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
import FontAwesome 1.0
import "../components" as NeroshopComponents

Page {
    id: main_page
    title: qsTr("Main Page")
    Rectangle {
        color: "transparent"
    }
    ///////////////////////////
    function startWalletSync() {
        // start local monero daemon or node synchronization (optional)
        /*Wallet.daemonExecute(Script.getString("neroshop.monero.daemon.ip"), 
            Script.getString("neroshop.monero.daemon.port"),
            Script.getBoolean("neroshop.monero.daemon.confirm_external_bind"),
            Script.getBoolean("neroshop.monero.daemon.restricted_rpc"),
            Script.getBoolean("neroshop.monero.daemon.remote"),
            Script.getString("neroshop.monero.daemon.data_dir"),
            Script.getString("neroshop.monero.daemon.network_type"),
            Script.getNumber("neroshop.monero.daemon.restore_height")
        );*/
        // connect to a remote monero node (default)
        let remote_node = Script.getTableStrings("neroshop.monero.nodes.stagenet")[1]
        let remote_node_ip = remote_node.split(":")[0]
        let remote_node_port = remote_node.split(":")[1]
        console.log("connecting to remote node " + remote_node_ip + " (port: " + remote_node_port + ")")
        Wallet.daemonConnect(remote_node_ip, remote_node_port);//Wallet.daemonConnect(Script.getString("neroshop.monero.daemon.ip"), Script.getString("neroshop.monero.daemon.port"));    
    }
    ///////////////////////////
    function generateWalletKeys() {
        // check if wallet has already been generated so that this function does not repeat
        // todo: if user decides to re-generate wallet keys, then destroy the current monero_wallet object and recreate it
        if(Wallet.getMnemonic().length > 0) {
            walletMessageArea.text = qsTr("Wallet has already been generated. One wallet per session to reduce spam")
            walletMessageArea.messageCode = 1            
            return;
        }
        // generate a unique wallet seed (mnemonic)
        let folderUrlToString = walletFolderDialog.folder.toString().replace("file://","")
        let error = Wallet.createRandomWallet(walletPasswordField.text, walletPasswordConfirmField.text, (walletNameField.text) ? qsTr(folderUrlToString + "/%1").arg(walletNameField.text) : qsTr(folderUrlToString + "/%1").arg(walletNameField.placeholderText))
        // if wallet passwords don't match, display error message
        let WALLET_PASSWORD_NO_MATCH = 2
        let WALLET_ALREADY_EXISTS = 3;
        if(error == WALLET_PASSWORD_NO_MATCH) {//if(walletPasswordConfirmField.text != walletPasswordField.text || !walletPasswordField.acceptableInput) {
            walletMessageArea.text = (walletPasswordConfirmField.length > 0) ? qsTr("Wallet passwords do not match") : qsTr("Wallet password must be confirmed")
            walletMessageArea.messageCode = 1
        }
        else if(error == WALLET_ALREADY_EXISTS) {
            walletMessageArea.text = qsTr("A wallet file with the same name already exists")
            walletMessageArea.messageCode = 1
        }
        
        if(!Wallet.isGenerated()) return;
        // assign the mnemonic model to the repeater model
        walletSeedRepeater.model = Wallet.getMnemonicList()
        // show wallet and seed message
        walletMessageArea.text = qsTr("\"%1\" has been created successfully.").arg((walletNameField.text) ? qsTr(folderUrlToString + "/%1.keys").arg(walletNameField.text) : qsTr(folderUrlToString + "/%1.keys").arg(walletNameField.placeholderText))
        walletMessageArea.messageCode = 0
        seedMessageArea.text = qsTr("These %1 words are the key to your account. Please store them safely!").arg(walletSeedRepeater.count)
        seedMessageArea.messageCode = 2
        // clear wallet name and password text fields
        walletNameField.text = ""
        walletPasswordField.text = ""
        walletPasswordConfirmField.text = ""
        // hide backButton if wallet has already been generated (not sure if this is a good idea? User may forget to copy their seed phrase)
        ////registrationPageBackButton.visible = false
        // start synching the monero node as soon we generate a wallet
        startWalletSync();            
    }
    ///////////////////////////
    function registerWallet() {
        if(!Wallet.isGenerated()) {
            messageBox.text = qsTr("Please generate your wallet keys before registering")
            messageBox.open()
            return; // exit function and do not proceed any further
        }
        // do a regex check on the username before proceeding
        // make sure username is not taken (requires a database check)
        // register the wallet primary key to the database
        // switch (login) to home page
        //stack.push(home_page)
        pageLoader.source = "HomePage.qml"
        console.log("Primary address: ", Wallet.getPrimaryAddress())
        console.log("Balance: ", Wallet.getBalanceLocked(0))
        console.log("Unlocked balance: ", Wallet.getBalanceUnlocked(0))
        //console.log("subaddress: ", (!Wallet.isGenerated()) ? "" : Wallet.createUniqueSubaddressObject(0).address)//console.log(Wallet.isGenerated() ? Wallet.getAddressesAll() : "no addresses found")
        //console.log("subaddress balance: ", (!Wallet.isGenerated()) ? "" : Wallet.createUniqueSubaddressObject(0).balance)        
    }
    ///////////////////////////
    NeroshopComponents.MessageBox {////MessageDialog {
        id: messageBox
        title: "message"
        x: mainWindow.x + (mainWindow.width - this.width) / 2
        y: mainWindow.y + (mainWindow.height - this.height) / 2
    }
    ///////////////////////////
    // for login page
    FileDialog {
        id: walletFileDialog
        fileMode: FileDialog.OpenFile
        currentFile: walletFileField.text // currentFile is deprecated since Qt 6.3. Use selectedFile instead
        folder: (isWindows) ? StandardPaths.writableLocation(StandardPaths.DocumentsLocation) + "/neroshop" : StandardPaths.writableLocation(StandardPaths.HomeLocation) + "/neroshop"//StandardPaths.writableLocation(StandardPaths.AppDataLocation) // refer to https://doc.qt.io/qt-5/qstandardpaths.html#StandardLocation-enum
        nameFilters: ["Wallet files (*.keys)"]
        ////options: FileDialog.ReadOnly // will not allow you to create folders while file dialog is opened
    }
    ///////////////////////////
    // for registration page
    FolderDialog {
        id: walletFolderDialog
        currentFolder: walletPathField.text
        folder: (isWindows) ? StandardPaths.writableLocation(StandardPaths.DocumentsLocation) + "/neroshop" : StandardPaths.writableLocation(StandardPaths.HomeLocation) + "/neroshop"
    }
    ///////////////////////////
    StackLayout { // Perfect for a stack of items where only one item is visible at a time//ColumnLayout { // From top to bottom
        id: mainPageStack // auth_menu inside home menu
        anchors.fill: parent // will fill entire Window area
        currentIndex: 0//buttonsBar.currentIndex//currentIndex: 1

        Rectangle {
            // todo: place login section right next to wallet generation section so long as the wallet seed model is not visible
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
            Button {
                id: loginPageNextButton
                anchors.verticalCenter: parent.verticalCenter//Layout.alignment: Qt.AlignVCenter | Qt::AlignRight
                anchors.right: parent.right
                anchors.rightMargin: 20
                implicitWidth: 60; height: implicitWidth
                text: qsTr(FontAwesome.angleRight)
                hoverEnabled: true
            
                background: Rectangle {
                    color: "#121212"//"#6b5b95"//
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
                
            GridLayout {
                anchors.horizontalCenter: parent.horizontalCenter
                anchors.top: parent.top
                anchors.topMargin: 20
                // wallet file text
                Text {
                    id: walletFileText
                    Layout.row: 0
                    Layout.column: 0               
                    text: qsTr("Wallet file")
                    //visible: false
                    color: (NeroshopComponents.Style.darkTheme) ? "#ffffff" : "#000000"
                    font.bold: true
                }
            // wallet file field
            TextField {
                id: walletFileField
                Layout.row: 1
                Layout.column: 0
                Layout.preferredWidth: 500; Layout.preferredHeight: 50
                Layout.topMargin: (walletFileText.visible) ? 5 : 0
                text: walletFileDialog.file
                color: "#000000"////(NeroshopComponents.Style.darkTheme) ? "#ffffff" : "#000000" // textColor                
                selectByMouse: true
                readOnly: true
                
                background: Rectangle { 
                    color: "#708090"////(NeroshopComponents.Style.darkTheme) ? "#101010" : "#ffffff"
                    border.color: (NeroshopComponents.Style.darkTheme) ? "#a9a9a9" : "#696969" // if path exists, make border.color green, if not then make red
                    radius: 3
                }                     
            }
            // wallet file upload or browse button
            Button {
                id: walletFileBrowseButton
                Layout.row: 1
                Layout.column: 1        
                Layout.preferredWidth: walletFileBrowseButtonText.contentWidth + 20  
                Layout.preferredHeight: walletFileField.height
                text: qsTr("Browse")
                //display: AbstractButton.IconOnly//AbstractButton.TextBesideIcon//AbstractButton.TextOnly//AbstractButton.TextUnderIcon
                //icon.source: "file:///" + neroshopResourcesDir + "/ellipsis.png"//"/upload.png"
                //icon.color: "#ffffff"
                hoverEnabled: true
                onClicked: walletFileDialog.open()
                
                background: Rectangle {
                    color: "#808080"
                    radius: 5
                    border.color: walletFileBrowseButton.hovered ? "#ffffff" : this.color//"#ffffff"//control.down ? "#17a81a" : "#21be2b"
                }
                // this removes the icon unfortunately :( but it gives us more control over the text
                contentItem: Text {
                    id: walletFileBrowseButtonText
                    text: parent.text
                    color: "#ffffff"
                    horizontalAlignment: Text.AlignHCenter
                    verticalAlignment: Text.AlignVCenter
                    font.bold: true                              
                }            
                ToolTip.delay: 1000
                ToolTip.visible: hovered
                ToolTip.text: qsTr("Browse wallet file")
            }                
            
                ButtonGroup {
                    id: walletRestoreMethodButtonGroup
                    exclusive: true // only one button selected at a time
                    onClicked: {
                        console.log("Selected", button.text, "button")
                        button.checked = true
                        /*if(button.text == restoreFromFileButton.text) {
                            walletRestoreStack.currentIndex = 0
                        }*/
                        /*if(button.text == restoreFromSeedButton.text) {
                            walletRestoreStack.currentIndex = 1
                        }*/
                        /*if(button.text == restoreFromKeysButton.text) {
                            walletRestoreStack.currentIndex = 2
                        }*/
                        /*if(button.text == restoreFromHWButton.text) {
                            walletRestoreStack.currentIndex = 3
                        }*/                                                                        
                    }
                }
                RowLayout {
                    id: walletRestoreButtonsRow
                    //Layout.preferredWidth: 
                    //Layout.preferredHeight: 
                    Layout.row: 2
                    Layout.column: 0                    
                    Layout.topMargin: 15
                    // to add a button to the button group (within the Button object itself): ButtonGroup.group: walletRestoreMethodButtonGroup // attaches a button to a button group
                    Button {
                        id: restoreFromFileButton
                        ButtonGroup.group: walletRestoreMethodButtonGroup
                        checked: true
                        text: qsTr("Restore from file")//.arg("\uf8e9")
                        Layout.preferredHeight: 40
                        icon.source: "file:///" + neroshopResourcesDir + "/file.png" // keys (key.png), seed (sprout.png), file, hardware
                        //icon.color: "#ffffff"
                        display: AbstractButton.IconOnly//hovered ? AbstractButton.TextBesideIcon : AbstractButton.IconOnly//AbstractButton.TextUnderIcon
                        hoverEnabled: true
                        background: Rectangle {
                            color: (parent.checked) ? "#39304f" : "#6b5b95"
                            //border.color:
                            //border.width: 1
                            radius: 3
                        }
                         //contentItem: Text { 
                         //    text: parent.text
                         //    color: "#ffffff"
                         //    horizontalAlignment: Text.AlignHCenter
                         //    verticalAlignment: Text.AlignVCenter
                         //    font.bold: true
                         //    font.family: FontAwesome.fontFamilySolid
                         //}
                     }

                 Button {
                     id: restoreFromSeedButton
                     ButtonGroup.group: walletRestoreMethodButtonGroup
                     text: qsTr("Restore from seed")//.arg("\uf8e9")
                     //width: contentWidth + 20;
                     Layout.preferredHeight: 40
                     icon.source: "file:///" + neroshopResourcesDir + "/sprout.png" // keys (key.png), seed (sprout.png), file, hardware
                     //icon.color: "#ffffff"
                     display: AbstractButton.IconOnly//hovered ? AbstractButton.TextBesideIcon : AbstractButton.IconOnly//AbstractButton.TextUnderIcon
                     hoverEnabled: true
                     background: Rectangle {
                         color: (parent.checked) ? "#39304f" : "#6b5b95"
                         //border.color:
                         //border.width: 1
                         radius: 3
                     }
                 }

                Button {
                    id: restoreFromKeysButton
                    ButtonGroup.group: walletRestoreMethodButtonGroup
                    text: qsTr("Restore from keys")//.arg("\uf8e9")
                    //width: contentWidth + 20;
                    Layout.preferredHeight: 40
                    icon.source: "file:///" + neroshopResourcesDir + "/key.png" // keys (key.png), seed (sprout.png), file, hardware
                    //icon.color: "#ffffff"
                    display: AbstractButton.IconOnly//hovered ? AbstractButton.TextBesideIcon : AbstractButton.IconOnly//AbstractButton.TextUnderIcon
                    hoverEnabled: true
                    background: Rectangle {
                        color: (parent.checked) ? "#39304f" : "#6b5b95"
                        //border.color:
                        //border.width: 1
                        radius: 3
                    }
                }

                Button {
                    id: restoreFromHWButton
                    ButtonGroup.group: walletRestoreMethodButtonGroup
                    text: qsTr("Restore from hardware wallet")//.arg("\uf8e9")
                    //width: contentWidth + 20;
                    Layout.preferredHeight: 40
                    icon.source: "file:///" + neroshopResourcesDir + "/usb.png" // keys (key.png), seed (sprout.png), file, hardware
                    //icon.color: "#ffffff"
                    display: AbstractButton.IconOnly//hovered ? AbstractButton.TextBesideIcon : AbstractButton.IconOnly//AbstractButton.TextUnderIcon
                    hoverEnabled: true
                    background: Rectangle {
                        color: (parent.checked) ? "#39304f" : "#6b5b95"
                        //border.color:
                        //border.width: 1
                        radius: 3
                    }
                }         
            } // RowLayout
            StackLayout {
                id: walletRestoreStack
                    Layout.row: 3
                    Layout.column: 0                     
                    currentIndex: 0
                Rectangle {
                    id: restoreFromWalletFile
                    visible: restoreFromFileButton.checked                    
                    //Layout.minimumHeight: 
                    Layout.preferredWidth: 500
                    Layout.preferredHeight: 300
                    Layout.topMargin: 5
                    //GridLayout {}
                    color: "transparent"
                    border.color: (NeroshopComponents.Style.darkTheme) ? "#a9a9a9" : "#696969"
                    //radius: 3
                }
            ////    Rectangle {
            ////        id: restoreFromMnemonicSeed
            ////        visible: restoreFrom?Button.checked
            ////    }
            ////    Rectangle {
            ////        id: restoreFromKeys
            ////        visible: restoreFrom?Button.checked
            ////    }
            ////    Rectangle {
            ////        id: restoreFromHardwareWallet
            ////        visible: restoreFrom?Button.checked
            ////    }                                    
            }
            //ScrollView {
            //    id: wallet_upload_scrollview
            /*             
            //}
            // upload button
            Button {
                id: walletPathButton
                text: restoreFromFileButton.checked ? qsTr("Browse") : qsTr("Change")//qsTr("Browse")//qsTr("...")//qsTr("Upload")
                onClicked: walletFileDialog.open()
                display: AbstractButton.IconOnly//AbstractButton.TextBesideIcon//AbstractButton.TextOnly
                //hoverEnabled: true
                x: walletFileUploadField.x + walletFileUploadField.width + 5
                y: walletFileUploadField.y            
                width: 50
                height: walletFileUploadField.height
                
                icon.source: "file:///" + neroshopResourcesDir + "/upload.png"//neroshopResourceDir + "/upload.png"
                icon.color: "#ffffff"
                // can only have 1 contentItem at a time (a contentItem is not needed for Button)
                //contentItem: Image {
                //    source: neroshopResourceDir + "/upload.png"
                //    height: 24; width: 24 // has no effect since image is scaled
                //    fillMode:Image.PreserveAspectFit; // the image is scaled uniformly to fit button without cropping // https://doc.qt.io/qt-6/qml-qtquick-image.html#fillMode-prop
                //}          
                //contentItem: Text {  
                //    text: walletPathButton.text
                //    color: "#ffffff"
                //    horizontalAlignment: Text.AlignHCenter
                //    verticalAlignment: Text.AlignVCenter
                //}
        
                background: Rectangle {
                    color: "#808080"//parent.down ? "#bbbbbb" : (parent.hovered ? "#d6d6d6" : "#f6f6f6")
                    radius: 0
                    border.color: walletPathButton.hovered ? "#ffffff" : this.color//"#ffffff"//control.down ? "#17a81a" : "#21be2b"
                }            
            
                ToolTip.delay: 500 // shows tooltip after hovering for 0.5 second
                ToolTip.visible: hovered
                ToolTip.text: qsTr("Upload wallet file")            
            }        
            //Label {
            //    id: wallet_file_password_label
            //    text: qsTr("Wallet file password")
            //}            
            // wallet password textfield
            TextField {//TextInput {
                id: wallet_password_edit
                x: walletFileUploadField.x
                y: walletFileUploadField.y + walletFileUploadField.height + 10
                width: walletFileUploadField.width//; height: 
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
            }*/
            } // GridLayout
        } // eof wallet_file_authentication_page
                
        // generate auth keys page
        Rectangle {
            id: walletGenerationPage
            color: NeroshopComponents.Style.getColorsFromTheme()[0]

            Button {
                id: walletPageBackButton
                anchors.verticalCenter: parent.verticalCenter//Layout.alignment: Qt.AlignVCenter | Qt::AlignRight
                anchors.left: parent.left
                anchors.leftMargin: 20
                implicitWidth: 60; height: implicitWidth
                text: qsTr(FontAwesome.angleLeft)
                hoverEnabled: true
                visible: (walletSeedRepeater.model == null)
            
                background: Rectangle {
                    color: "#121212"//"#6b5b95"//
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
                    mainPageStack.currentIndex = mainPageStack.currentIndex - 1
                }
            }
            
            Button {
                id: walletPageNextButton
                anchors.verticalCenter: parent.verticalCenter//Layout.alignment: Qt.AlignVCenter | Qt::AlignRight
                anchors.right: parent.right
                anchors.rightMargin: 20
                implicitWidth: 60; height: implicitWidth
                text: qsTr(FontAwesome.angleRight)
                hoverEnabled: true
                visible: (walletSeedRepeater.model != null)
            
                background: Rectangle {
                    color: "#121212"//"#6b5b95"//
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

            GridLayout {
                anchors.horizontalCenter: parent.horizontalCenter//(walletSeedRepeater.model) ? this.horizontalCenter : parent.horizontalCenter
                anchors.verticalCenter: parent.verticalCenter
                anchors.top: parent.top
                anchors.topMargin: 20
                //anchors.left: parent.left
                //anchors.leftMargin: walletPageNextButton.width + 20
                //width: 800
                ////anchors.rightMargin: walletPageNextButton.width + 20//anchors.margins: 50
                columns: 2 // Set column limit to 2
                columnSpacing: 100 // The default value is 5. Same with rowSpacing
                ////rows: 10
                ////rowSpacing: 15
                // Reminder: Layout. functions work for items inside Layouts but not for the Layout itself!
                // wallet name creation text
                Text {
                    id: walletNameText
                    Layout.row: 0
                    Layout.column: 0               
                    text: qsTr("Wallet name")
                    //visible: !walletSeedRepeater.model
                    color: (NeroshopComponents.Style.darkTheme) ? "#ffffff" : "#000000"
                    font.bold: true
                }                
                // wallet name creation field
                TextField {
                    id: walletNameField
                    Layout.preferredWidth: 500
                    Layout.preferredHeight: 50//width: 500; height: 50
                    Layout.row: 1
                    Layout.column: 0
                    Layout.topMargin: (walletNameText.visible) ? 5 : 0
                    //visible: !walletSeedRepeater.model
                    placeholderText: qsTr("auth"); placeholderTextColor: (NeroshopComponents.Style.darkTheme) ? "#a9a9a9" : "#696969"
                    color: (NeroshopComponents.Style.darkTheme) ? "#ffffff" : "#000000" // textColor                
                    selectByMouse: true
                    // todo: validator regex for preventing special characters like: * . " / \ [ ] : ; | , from being added to the wallet name
                
                    background: Rectangle { 
                        color: (NeroshopComponents.Style.darkTheme) ? "#101010" : "#ffffff"
                        border.color: (NeroshopComponents.Style.darkTheme) ? "#a9a9a9" : "#696969"
                        radius: 3
                    }                     
                }
                // wallet password creation field
                TextField {
                    id: walletPasswordField
                    Layout.preferredWidth: 500
                    Layout.preferredHeight: 50//width: 500; height: 50
                    Layout.row: 2
                    Layout.column: 0                
                    //visible: !walletSeedRepeater.model
                    placeholderText: qsTr("Wallet Password")
                    placeholderTextColor: (NeroshopComponents.Style.darkTheme) ? "#a9a9a9" : "#696969"
                    color: NeroshopComponents.Style.moneroOrangeColor // textColor
                    echoMode: (!walletPasswordVisibilityToggle.checked) ? TextInput.Normal : TextInput.Password//TextInput.PasswordEchoOnEdit
                    inputMethodHints: Qt.ImhSensitiveData
                    selectByMouse: true
                    //validator: RegularExpressionValidator { regularExpression: "^(?=.*?[A-Z])(?=.*?[a-z])(?=.*?[0-9])(?=.*?[#?!@$%^&*-]).{8,}$" }
                    background: Rectangle { 
                        color: walletNameField.background.color
                        border.color: (!parent.acceptableInput) ? walletPasswordStatusMark.invalidColor : parent.placeholderTextColor//walletPasswordConfirmField.background.border.color////((parent.length > 0 && walletPasswordConfirmField.length == 0) ? ((walletPasswordConfirmField.text != walletPasswordField.text || !walletPasswordField.acceptableInput) ? walletPasswordStatusMark.invalidColor : walletPasswordStatusMark.validColor) : walletPasswordConfirmField.background.border.color)
                        border.width: (!parent.acceptableInput) ? 2 : 1//walletPasswordConfirmField.background.border.width////(parent.length > 0 && walletPasswordConfirmField.length == 0) ? 2 : walletPasswordConfirmField.background.border.width
                        radius: 3
                    }         
                    // todo: maybe add a regex validator
                    rightPadding: 15 + walletPasswordVisibilityToggle.width
                    // wallet password visibility toggle
                    Button {
                        id: walletPasswordVisibilityToggle
                        text: (checked) ? FontAwesome.eye : FontAwesome.eyeSlash//icon.source:
                        anchors.right: parent.right
                        anchors.rightMargin: 15
                        anchors.verticalCenter: parent.verticalCenter
                        implicitWidth: 24; implicitHeight: 24
                        checkable: true
                        checked: true//false
                        hoverEnabled: true
                        // checked = show, unchecked = hide. Passwords are hidden by default
                        background: Rectangle {
                            color: "transparent"
                        }
                        contentItem: Text {
                            text: parent.text
                            color: (parent.checked) ? "#a9a9a9" : "#696969"
                            verticalAlignment: Text.AlignVCenter
                            font.bold: true
                        }
                    }
                }                
                // wallet password confirmation field
                TextField {
                    id: walletPasswordConfirmField
                    Layout.preferredWidth: 500
                    Layout.preferredHeight: 50//width: 500; height: 50
                    Layout.row: 3
                    Layout.column: 0
                    //Layout.topMargin: 5
                    //rightPadding
                    //visible: !walletSeedRepeater.model
                    placeholderText: qsTr("Confirm Wallet Password")
                    placeholderTextColor: (NeroshopComponents.Style.darkTheme) ? "#a9a9a9" : "#696969"
                    color: walletPasswordField.color//NeroshopComponents.Style.moneroOrangeColor // textColor
                    echoMode: (!walletPasswordVisibilityToggle.checked) ? TextInput.Normal : TextInput.Password
                    inputMethodHints: Qt.ImhSensitiveData
                    selectByMouse: true
                    background: Rectangle { 
                        color: walletPasswordField.background.color//NeroshopComponents.Style.moneroGrayColor
                        border.color: (parent.length > 0) ? walletPasswordStatusMark.color : parent.placeholderTextColor
                        border.width: (parent.length > 0) ? 2 : 1
                        radius: 3
                    }
                    rightPadding: 30 + walletPasswordStatusMark.contentWidth//double rightmargin size
                    // wallet password validation status mark
                    Text {
                        id: walletPasswordStatusMark
                        property string validColor: "#50b954"
                        property string invalidColor: "#f17982"
                        anchors.right: parent.right
                        anchors.rightMargin: 15
                        anchors.verticalCenter: parent.verticalCenter                        
                        text: (walletPasswordConfirmField.text != walletPasswordField.text || !walletPasswordField.acceptableInput) ? qsTr(FontAwesome.xmark) : qsTr(FontAwesome.check)
                        visible: (walletPasswordConfirmField.length > 0)
                        font.bold: true
                        font.family: FontAwesome.fontFamily
                        color: (walletPasswordConfirmField.text != walletPasswordField.text || !walletPasswordField.acceptableInput) ? invalidColor : validColor
                    }                              
                }
                // wallet path text
                Text {
                    id: walletPathText
                    Layout.row: 4
                    Layout.column: 0               
                    Layout.topMargin: 10
                    text: qsTr("Wallet path")
                    //visible: !walletSeedRepeater.model
                    color: (NeroshopComponents.Style.darkTheme) ? "#ffffff" : "#000000"
                    font.bold: true                
                }                
                // wallet path field
                TextField {
                    id: walletPathField
                    Layout.row: 5
                    Layout.column: 0
                    Layout.preferredWidth: 500; Layout.preferredHeight: 50
                    Layout.topMargin: (walletPathText.visible) ? 5 : 0
                    //visible: !walletSeedRepeater.model
                    text: walletFolderDialog.folder////(walletNameField.text) ? qsTr(walletFolderDialog.folder + "/%1.keys").arg(walletNameField.text) : qsTr(walletFolderDialog.folder + "/%1.keys").arg(walletNameField.placeholderText)
                    color: "#000000"//(NeroshopComponents.Style.darkTheme) ? "#ffffff" : "#000000" // textColor                
                    selectByMouse: true
                    readOnly: true
                
                    background: Rectangle { 
                        color: "#708090"//(NeroshopComponents.Style.darkTheme) ? "#101010" : "#ffffff"
                        border.color: (NeroshopComponents.Style.darkTheme) ? "#a9a9a9" : "#696969" // if path exists, make border.color green, if not then make red
                        radius: 3
                    }                     
                }
                RowLayout {
                    Layout.row: 6
                    Layout.column: 0
                    //visible: !walletSeedRepeater.model
                // wallet path change or upload button
                Button {
                    id: walletPathChangeButton
                Layout.topMargin: 10
                Layout.preferredWidth: walletPathField.width / 4//walletPathChangeButtonText.contentWidth + 20  
                Layout.preferredHeight: 50//walletPathField.Layout.preferredHeight
                text: qsTr("Change")
                //display: AbstractButton.IconOnly//AbstractButton.TextBesideIcon//AbstractButton.TextOnly
                //icon.source: "file:///" + neroshopResourcesDir + "/change.png"
                //icon.color: "#ffffff"
                hoverEnabled: true                
                onClicked: walletFolderDialog.open()
                
                background: Rectangle {
                    color: NeroshopComponents.Style.moneroGrayColor
                    radius: 5
                    border.color: walletPathChangeButton.hovered ? "#ffffff" : this.color//"#ffffff"//control.down ? "#17a81a" : "#21be2b"
                }            
                
                contentItem: Text {
                    id: walletPathChangeButtonText
                    text: parent.text
                    color: "#ffffff"
                    horizontalAlignment: Text.AlignHCenter
                    verticalAlignment: Text.AlignVCenter
                    font.bold: true                              
                }
            
                    ToolTip.delay: 1000
                    ToolTip.visible: hovered
                    ToolTip.text: qsTr("Change wallet path")
                }                  
                // generate key button
                Button {
                    id: generateKeysButton      
                Layout.topMargin: 10//20      
                Layout.preferredWidth: (walletPathChangeButton.width * 3) - parent.spacing//150
                Layout.preferredHeight: 50          
                text: qsTr("Generate")//("Generate Keys")
                hoverEnabled: true
                onClicked: generateWalletKeys()
                
                contentItem: Text {  
                    //id: generateKeysButtonText
                    font.bold: true
                    text: generateKeysButton.text
                    color: "#ffffff" // white
                    horizontalAlignment: Text.AlignHCenter
                    verticalAlignment: Text.AlignVCenter                    
                }
                
                    background: Rectangle {
                        color: NeroshopComponents.Style.moneroOrangeColor
                        radius: 5
                    }
                }
                } // RowLayout containing both walletPathChangeButton and generateKeys button
                // important wallet message box
                TextArea {
                    id: walletMessageArea
                    visible: true////false
                    Layout.row: 7
                    Layout.column: 0
                    Layout.fillWidth: true // extends the TextArea's width to the width of the Layout
                    Layout.maximumWidth: walletPathField.width // keeps textarea from going past grid bounds when text is added
                    Layout.preferredHeight: contentHeight + 20
                    Layout.topMargin: 20//15
                    selectByMouse: true
                    readOnly: true
                    verticalAlignment: TextEdit.AlignVCenter // align the text within the center of TextArea item's height
                    wrapMode: TextEdit.Wrap // move text to newline if it reaches the width of the TextArea
                    text: qsTr("Please generate your wallet keys before registering your wallet.")
                    color: (messageCode == 1) ? "#b22222" : ((NeroshopComponents.Style.darkTheme) ? "#ffffff" : "#404040")// text color
                    property int messageCode: 0 //0 = info; 1 = warning or error
                    background: Rectangle { 
                        color: "transparent"
                        border.color: (parent.messageCode == 1) ? "#b22222" : "#2196f3"////parent.color//(NeroshopComponents.Style.darkTheme) ? "#ffffff" : "#404040"
                        radius: 3
                    }            
                    leftPadding: 30 + circleInfo.contentWidth
                    Text {
                        id: circleInfo
                        anchors.left: parent.left
                        anchors.leftMargin: 15
                        anchors.verticalCenter: parent.verticalCenter                         
                        text: (parent.messageCode == 1) ? qsTr(FontAwesome.triangleExclamation) : qsTr(FontAwesome.circleInfo)
                        color: (parent.messageCode == 1) ? "#b22222" : "#2196f3"
                        font.bold: true
                        font.family: FontAwesome.fontFamily
                    }
                }
                // wallet seed message box
                TextArea {
                    id: seedMessageArea
                    visible: (walletSeedRepeater.model != null)
                    Layout.row: 0
                    Layout.column: 1
                    Layout.fillWidth: true // extends the TextArea's width to the width of the Layout
                    Layout.maximumWidth: 550////walletPathField.width // keeps textarea from going past grid bounds when text is added
                    Layout.preferredHeight: contentHeight + 20
                    Layout.topMargin: 20//15
                    selectByMouse: true
                    readOnly: true
                    verticalAlignment: TextEdit.AlignVCenter // align the text within the center of TextArea item's height
                    wrapMode: TextEdit.Wrap // move text to newline if it reaches the width of the TextArea
                    text: qsTr("")
                    color: (messageCode == 1) ? "#b22222" : ((NeroshopComponents.Style.darkTheme) ? "#ffffff" : "#404040")// text color
                    property int messageCode: 0 //0 = info; 1 = warning or error
                    background: Rectangle { 
                        color: "transparent"
                        border.color: (parent.messageCode == 2) ? "#228b22" : ((parent.messageCode == 1) ? "#b22222" : "#2196f3")////parent.color//(NeroshopComponents.Style.darkTheme) ? "#ffffff" : "#404040"
                        radius: 3
                    }            
                    leftPadding: 30 + circleInfo.contentWidth
                    Text {
                        ////id: circleInfo
                        anchors.left: parent.left
                        anchors.leftMargin: 15
                        anchors.verticalCenter: parent.verticalCenter                         
                        text: (parent.messageCode == 2) ? qsTr(FontAwesome.seedling) : ((parent.messageCode == 1) ? qsTr(FontAwesome.triangleExclamation) : qsTr(FontAwesome.circleInfo))
                        color: (parent.messageCode == 2) ? "#228b22" : ((parent.messageCode == 1) ? "#b22222" : "#2196f3")
                        font.bold: true
                        font.family: FontAwesome.fontFamily
                    }
                }              
                // testing listview for walletseed display   
                Flow {////RowLayout {
                    id: walletSeedDisplay
                    //Layout.preferredHeight: 200
                    Layout.rowSpan: 7////8 // fill the row 8 times?
                    Layout.row: 1
                    Layout.column: 1
                    ////Layout.fillWidth: true // no reason to fill width since we set the maximum width
                    //Layout.preferredHeight: 40 * Layout.rowSpan // remove
                    Layout.maximumWidth: 550//walletMessageArea.width
                    Layout.topMargin: 20//15
                    /****orientation: ListView.Horizontal****/
                    spacing: 5

                    Repeater {
                        id: walletSeedRepeater
                        ////model: null
                        delegate: Rectangle {
                            width: 130; height: 40
                            Text { 
                                text: (index + 1) + ".   " + modelData
                                color: (NeroshopComponents.Style.darkTheme) ? "#ffffff" : "#000000"
                                anchors.verticalCenter: parent.verticalCenter//verticalAlignment: Text.AlignVCenter
                                anchors.left: parent.left
                                anchors.leftMargin: 15
                            }
                            color: "transparent"
                            border.color: (NeroshopComponents.Style.darkTheme) ? "#a9a9a9" : "#696969"
                            radius: 3
                            //MouseArea {
                            //    anchors.fill: parent
                            //    onClicked: {}
                            //}
                        }
                    }
                } // Flow?
                // wallet seed copy button  
                Button {
                    id: seedCopyButton
                    Layout.row: 9
                    Layout.column: 1
                    Layout.fillWidth: true////width: contentWidth + 20; height: 40
                    visible: (walletSeedRepeater.model != null)
                    text: qsTr("Copy")
                    icon.source: "file:///" + neroshopResourcesDir + "/copy.png"
                    icon.color: "#ffffff"
                    display: AbstractButton.IconOnly//AbstractButton.TextBesideIcon//AbstractButton.TextOnly//AbstractButton.TextUnderIcon
                    hoverEnabled: true
                    onClicked: Wallet.copyMnemonicToClipboard()////copyToClipboard()
                
                    background: Rectangle {
                        color: "#404040"
                        radius: 5
                        border.color: parent.hovered ? "#ffffff" : this.color//"#ffffff"//control.down ? "#17a81a" : "#21be2b"
                    }

                    ToolTip.delay: 1000
                    ToolTip.visible: hovered
                    ToolTip.text: qsTr("Copy seed phrase")////qsTr("Copy to clipboard")                    
                } // copyButton                                                                    
            } // GridLayout for walletGenerationPage                    
        } // eof walletGenerationPage
        Rectangle {
            id: registrationPage
            color: NeroshopComponents.Style.getColorsFromTheme()[0]

            Button {
                id: registrationPageBackButton
                anchors.verticalCenter: parent.verticalCenter//Layout.alignment: Qt.AlignVCenter | Qt::AlignLeft
                anchors.left: parent.left
                anchors.leftMargin: 20
                implicitWidth: 60; height: implicitWidth
                text: qsTr(FontAwesome.angleLeft)
                hoverEnabled: true
            
                background: Rectangle {
                    color: "#121212"//"#6b5b95"//
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
            
            GridLayout {
                anchors.centerIn: parent
            	// optional pseudonym edit
            	TextField {
                	id: optNameField
                	Layout.row: 0
                	Layout.column: 0
                	Layout.preferredWidth: 500
                	Layout.preferredHeight: 50
                	placeholderText: qsTr("Pseudonym (optional)")
                	placeholderTextColor: "#696969" // dim gray
                	color: "#6b5b95"////(NeroshopComponents.Style.darkTheme) ? "#ffffff" : "#000000" // textColor
                	selectByMouse: true
                	background: Rectangle { 
                        color: (NeroshopComponents.Style.darkTheme) ? "#101010" : "#ffffff"
                        border.color: (NeroshopComponents.Style.darkTheme) ? "#a9a9a9" : "#696969"
                        radius: 3
                    }              
                	//validator: RegExpValidator { regExp: /^(?=.{8,20}$)(?![_.])(?!.*[_.]{2})[a-zA-Z0-9._]+(?<![_.])$/ } // validator: RegularExpressionValidator { regularExpression: /[0-9A-F]+/ } // since Qt  5.14
                    
                    ////Layout.topMargin: (optNameText.visible) ? 5 : 0
                    ////placeholderTextColor: (NeroshopComponents.Style.darkTheme) ? "#a9a9a9" : "#696969" 	
            	}
            	// register button
            	Button {
                	id: registerButton
                	Layout.row: 1
                	Layout.column: 0         
                	Layout.fillWidth: true
                	Layout.preferredHeight: 50
                	Layout.topMargin: 15
                	text: qsTr("Register")
                	hoverEnabled: true
                	onClicked: registerWallet()
                	background: Rectangle {
                    	color: "#6b5b95"
                    	radius: 5
                	}
                	                
                	contentItem: Text {  
                    	//font.family: "Consolas"; //font.family: NeroshopComponents.Style.fontFiraCodeLight.name
                    	//font.pointSize: 10
                    	font.bold: true
                    	text: registerButton.text
                    	color: "#ffffff" // white
                    	horizontalAlignment: Text.AlignHCenter
                    	verticalAlignment: Text.AlignVCenter                    
                	}                
            	}
            } // GridLayout for registrationPage
        } // eof registrationPage  
        // walletfile auth page
        // Upload button with read-only textfield
    }    
}    
