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
    id: mainPage
    title: qsTr("Main Page")
    Rectangle {
        color: "transparent"
    }
    property int walletGenerationCount: 0
    ///////////////////////////
    function onAutoSync() {
        console.log("Auto-sync is turned " + ((settingsDialog.moneroDaemonAutoSync) ? "on" : "off"))
        if(!settingsDialog.moneroDaemonAutoSync) return;
        startWalletSync(); // connects to a monero daemon (either local or remote) then starts the sync process
    }
    ///////////////////////////
    function startWalletSync() {
        // Check if daemon is already synced
        if(Wallet.isDaemonSynced()) return;//{ console.log("Daemon is already connected and synced with the network"); return; }    
        // SettingsDialog should retrieve data  from settings.lua and all other components can retrieve from SettingsDialog
        console.log("******************************************************");
        console.log("Node type: ", (settingsDialog.moneroNodeType == 0) ? "remote node" : "local node")
        console.log("Path to monerod: ", settingsDialog.monerodPath)
        console.log("Selected node address: ", settingsDialog.moneroNodeAddress)
        console.log("Monero data dir: ", settingsDialog.moneroDataDir)
        console.log("Daemon username: ", settingsDialog.moneroDaemonUsername)
        console.log("Daemon Password: ", settingsDialog.moneroDaemonPassword)
        console.log("Confirm external bind: ", settingsDialog.confirmExternalBind)
        console.log("Restricted RPC: ", settingsDialog.restrictedRpc)
        console.log("Auto-sync: ", settingsDialog.moneroDaemonAutoSync)
        console.log("******************************************************");
        // start the monero daemon (local node) (optional)
        if(settingsDialog.moneroNodeType == 1) {
            // Dectect any running monero daemon (local)
            if(Backend.isWalletDaemonRunning()) {
                console.log("monerod is currently running in the background as detected. Now connecting to local node 127.0.0.1:" + settingsDialog.moneroNodeDefaultPort)
                Wallet.daemonConnect();//(moneroDaemonRpcLoginUser.text, moneroDaemonRpcLoginPwd.text)
                return; // exit function and do not proceed any further
            }
            
            if(settingsDialog.monerodPath.length < 1) { // Will not show when redirecting to HomePage
                messageBox.text = "You've selected Local node, but the path to monerod has not been specified"
                messageBox.open()
                return; // exit function and do not proceed any further
            }
            
            Wallet.daemonExecute(settingsDialog.monerodPath, settingsDialog.confirmExternalBind, settingsDialog.restrictedRpc, settingsDialog.moneroDataDir, 0/*Script.getNumber("neroshop.monero.wallet.restore_height")*/);
            Wallet.daemonConnect();//(moneroDaemonRpcLoginUser.text, moneroDaemonRpcLoginPwd.text)
            // Don't connect to daemon until we are sure that it has been launched/is ready (or else it will crash app)
        }
        // connect to a remote monero node (default)
        if(settingsDialog.moneroNodeType == 0) {
            let remote_node_ip = settingsDialog.moneroNodeAddress.split(":")[0]
            let remote_node_port = settingsDialog.moneroNodeAddress.split(":")[1]
            console.log("connecting to remote node " + remote_node_ip + ":" + remote_node_port)
            // Todo: use custom remote node
            Wallet.nodeConnect(remote_node_ip, remote_node_port)//, moneroDaemonRpcLoginUser.text, moneroDaemonRpcLoginPwd.text);//Wallet.nodeConnect(Script.getString("neroshop.monero.daemon.ip"), Script.getString("neroshop.monero.daemon.port"), moneroDaemonRpcLoginUser.text, moneroDaemonRpcLoginPwd.text);
        }
    }
    ///////////////////////////
    function generateWalletKeys() {
        // Convert folder url to string
        let folder = Backend.urlToLocalFile(walletFolderDialog.folder);////let folder = (isWindows) ? walletFolderDialog.folder.toString().replace("file:///", "") : walletFolderDialog.folder.toString().replace("file://", "")
        // Check whether wallet file exists
        if(Wallet.fileExists((walletNameField.text) ? qsTr(folder + "/%1").arg(walletNameField.text) : qsTr(folder + "/%1").arg(walletNameField.placeholderText))) {
            walletMessageArea.text = qsTr("A wallet file with the same name already exists")
            walletMessageArea.messageCode = 1
            return; // exit function and do not proceed any further
        }        
        // Close (destroy) the current monero_wallet first before re-creating a new monero_wallet (In case user decides to re-generate wallet keys)
        if(Wallet.isOpened()) Wallet.close();
        // Generate wallet
        Wallet.createRandomWallet(walletPasswordField.text, walletPasswordConfirmField.text, (walletNameField.text) ? qsTr(folder + "/%1").arg(walletNameField.text) : qsTr(folder + "/%1").arg(walletNameField.placeholderText))
        // In case wallet passwords do not match, display error message
        if(walletPasswordConfirmField.text != walletPasswordField.text || !walletPasswordField.acceptableInput) {
            walletMessageArea.text = (walletPasswordConfirmField.length > 0) ? qsTr("Wallet passwords do not match") : qsTr("Wallet password must be confirmed")
            walletMessageArea.messageCode = 1
        }
        // Exit function if wallet fails to generate
        if(!Wallet.isOpened()) return;
        // Increase the number of times a wallet has been generated this session (not necessary)
        walletGenerationCount = walletGenerationCount + 1
        // Display seed phrase in Repeater model
        walletSeedRepeater.model = Wallet.getMnemonicList()
        // Show wallet-related message(s)
        walletMessageArea.text = qsTr("\"%1\" has been created successfully.").arg((walletNameField.text) ? qsTr(folder + "/%1.keys").arg(walletNameField.text) : qsTr(folder + "/%1.keys").arg(walletNameField.placeholderText))
        walletMessageArea.messageCode = 0
        seedMessageArea.text = qsTr("These %1 words are the key to your account. Please store them safely before proceeding to the registration page.").arg(walletSeedRepeater.count)
        seedMessageArea.messageCode = 0//2
        // Clear wallet password fields
        walletPasswordField.text = ""
        walletPasswordConfirmField.text = ""
    }
    ///////////////////////////
    function registerWallet() {
        if(!Wallet.isOpened()) {
            messageBox.text = qsTr("Please generate your wallet keys before registering")
            messageBox.open()
            return; // exit function and do not proceed any further
        }
        // Do a regex check on the username to make sure that it is valid
        // ...
        // Make sure username is not taken (requires a database check)
        // ...
        // Register the wallet primary key to the database
        let register_result = Backend.registerUser(Wallet, optNameField.text, User)
        if(!register_result [0] ) {
            messageBox.text = register_result [1];
            messageBox.open()
            return; // exit function and do not proceed any further
        }
        //User.uploadAvatar("../images/appicons/LogoLight250x250.png");
        // Switch to HomePage
        pageLoader.source = "HomePage.qml"//stack.push(home_page)
        console.log("Primary address: ", Wallet.getPrimaryAddress())
        console.log("Balance: ", Wallet.getBalanceLocked().toFixed(12))
        console.log("Unlocked balance: ", Wallet.getBalanceUnlocked().toFixed(12))
        // start synching the monero node as soon we hit the register button (this confirms that we are satified with the wallet key that we've generated and won't turn back to re-generate a new one)
        // Todo: Add an auto-connect on login/registration button and only sync automatically if that option is turned on (will be turned on by default)
        onAutoSync()
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
            //Layout.alignment: Qt.AlignVCenter
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
                //rowSpacing: 5
            
                Text {
                    id: loginPageTitle
                    Layout.row: 0
                    Layout.column: 0
                    text: qsTr("Restore Wallet (Login)")
                    color: (NeroshopComponents.Style.darkTheme) ? "#ffffff" : "#000000"
                    font.bold: true                    
                    font.pointSize: 14
                }
                ButtonGroup {
                    id: walletRestoreMethodButtonGroup
                    exclusive: true // only one button selected at a time
                    onClicked: {
                        console.log("Selected", button.text, "button")
                        button.checked = true
                        if(button.text == restoreFromFileButton.text) {
                            walletRestoreStack.currentIndex = 0
                        }
                        if(button.text == restoreFromSeedButton.text) {
                            walletRestoreStack.currentIndex = 1
                        }
                        if(button.text == restoreFromKeysButton.text) {
                            walletRestoreStack.currentIndex = 2
                        }
                        if(button.text == restoreFromHWButton.text) {
                            walletRestoreStack.currentIndex = 3
                        }                                                                        
                    }
                }
                RowLayout {
                    id: walletRestoreButtonsRow
                    //Layout.preferredWidth: 
                    //Layout.preferredHeight: 
                    Layout.row: 1
                    Layout.column: 0    
                    Layout.fillWidth: true                
                    Layout.topMargin: 15
                    property real buttonWidth: (500 / 4)
                    property real buttonHeight: 40
                    // to add a button to the button group (within the Button object itself): ButtonGroup.group: walletRestoreMethodButtonGroup // attaches a button to a button group
                    Button {
                        id: restoreFromFileButton
                        ButtonGroup.group: walletRestoreMethodButtonGroup
                        checked: true
                        text: qsTr("Restore from file")//.arg("\uf8e9")
                        Layout.preferredHeight: parent.buttonHeight
                        Layout.preferredWidth: parent.buttonWidth//hovered ? 180 : parent.buttonWidth
                        //Layout.maximumWidth: 180//contentWidth + 20
                        icon.source: "qrc:/images/file.png" // keys (key.png), seed (sprout.png), file, hardware
                        //icon.color: "#ffffff"
                        display: AbstractButton.IconOnly//hovered ? AbstractButton.TextBesideIcon : AbstractButton.IconOnly//AbstractButton.TextUnderIcon
                        hoverEnabled: true
                        background: Rectangle {
                            color: (parent.checked) ? "#39304f" : "#6b5b95"
                            //border.color:
                            //border.width: 1
                            radius: 3
                        }
                         /*contentItem: Text { 
                             text: parent.text
                             color: "#ffffff"
                             horizontalAlignment: Text.AlignHCenter
                             verticalAlignment: Text.AlignVCenter
                             font.bold: true
                             //font.family: FontAwesome.fontFamilySolid
                         }*/
                         NeroshopComponents.Hint {
                             id: restoreFileHint
                             visible: parent.hovered; delay: 0
                             text: parent.text
                             pointer.visible: false
                             //y: parent.y - (parent.height + pointer.height)//(parent.height - this.height) / 2
                         }
                     }

                 Button {
                     id: restoreFromSeedButton
                     ButtonGroup.group: walletRestoreMethodButtonGroup
                     text: qsTr("Restore from seed")//.arg("\uf8e9")
                     Layout.preferredWidth: parent.buttonWidth//width: contentWidth + 20;
                     Layout.preferredHeight: parent.buttonHeight
                     icon.source: "qrc:/images/sprout.png" // keys (key.png), seed (sprout.png), file, hardware
                     //icon.color: "#ffffff"
                     display: AbstractButton.IconOnly
                     hoverEnabled: true
                     background: Rectangle {
                         color: (parent.checked) ? "#39304f" : "#6b5b95"
                         //border.color:
                         //border.width: 1
                         radius: 3
                     }
                         NeroshopComponents.Hint {
                             visible: parent.hovered; delay: 0
                             text: parent.text
                             pointer.visible: false
                             //y: parent.y - (parent.height + pointer.height)//(parent.height - this.height) / 2
                         }                     
                 }

                Button {
                    id: restoreFromKeysButton
                    ButtonGroup.group: walletRestoreMethodButtonGroup
                    text: qsTr("Restore from keys")//.arg("\uf8e9")
                    Layout.preferredWidth: parent.buttonWidth//width: contentWidth + 20;
                    Layout.preferredHeight: parent.buttonHeight
                    icon.source: "qrc:/images/key.png" // keys (key.png), seed (sprout.png), file, hardware
                    //icon.color: "#ffffff"
                    display: AbstractButton.IconOnly
                    hoverEnabled: true
                    background: Rectangle {
                        color: (parent.checked) ? "#39304f" : "#6b5b95"
                        //border.color:
                        //border.width: 1
                        radius: 3
                    }
                         NeroshopComponents.Hint {
                             visible: parent.hovered; delay: 0
                             text: parent.text
                             pointer.visible: false
                             //y: parent.y - (parent.height + pointer.height)//(parent.height - this.height) / 2
                         }                    
                }

                Button {
                    id: restoreFromHWButton
                    ButtonGroup.group: walletRestoreMethodButtonGroup
                    text: qsTr("Restore from HW")//("Restore from hardware")//.arg("\uf8e9")
                    Layout.preferredWidth: parent.buttonWidth//hovered ? Layout.maximumWidth : parent.buttonWidth//width: contentWidth + 20;
                    Layout.preferredHeight: parent.buttonHeight
                    //Layout.maximumWidth: 180//280
                    icon.source: "qrc:/images/usb.png" // keys (key.png), seed (sprout.png), file, hardware
                    //icon.color: "#ffffff"
                    display: AbstractButton.IconOnly
                    hoverEnabled: true
                    background: Rectangle {
                        color: (parent.checked) ? "#39304f" : "#6b5b95"
                        //border.color:
                        //border.width: 1
                        radius: 3
                    }
                         NeroshopComponents.Hint {
                             visible: parent.hovered; delay: 0
                             text: parent.text
                             pointer.visible: false
                             //y: parent.y - (parent.height + pointer.height)//(parent.height - this.height) / 2
                         }                    
                }         
            } // RowLayout
            // walletRestoreStack
            StackLayout {
                id: walletRestoreStack
                    Layout.row: 2
                    Layout.column: 0           
                    Layout.alignment: Qt.AlignHCenter | Qt.AlignVCenter
                    currentIndex: 0
                // WalletFileStackContent    
                ColumnLayout {
                    id: restoreFromWalletFile
                    Layout.alignment: Qt.AlignHCenter | Qt.AlignVCenter// | Qt.AlignTop; Layout.topMargin: 20               
                    //Layout.minimumHeight: 
                    Layout.preferredWidth: 500
                    Layout.maximumWidth: 600
                    //Layout.preferredHeight: 380
                    /*color: "transparent"
                    border.color: (NeroshopComponents.Style.darkTheme) ? "#a9a9a9" : "#696969"
                    //radius: 3*/

                // wallet file text
                Text {
                    id: walletFileText
                    Layout.row: 0
                    Layout.column: 0               
                    Layout.topMargin: 15
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
                Layout.fillWidth: true//Layout.alignment: Qt.AlignHCenter
                Layout.preferredWidth: 500
                Layout.preferredHeight: 50
                Layout.topMargin: (walletFileText.visible) ? 5 : 0
                text: walletFileDialog.file
                color: "#000000"////(NeroshopComponents.Style.darkTheme) ? "#ffffff" : "#000000" // textColor                
                selectByMouse: true
                readOnly: true
                
                background: Rectangle { 
                    color: "#708090"////(NeroshopComponents.Style.darkTheme) ? "#101010" : "#ffffff"
                    border.color: (NeroshopComponents.Style.darkTheme) ? "#a9a9a9" : "#696969"
                    radius: 3
                }                     
            }
            // wallet file upload or browse button
            Button {
                id: walletFileBrowseButton
                Layout.row: 2//1
                Layout.column: 0//1
                Layout.alignment: Qt.AlignHCenter
                ////Layout.fillWidth: true
                Layout.preferredWidth: 125////walletFileBrowseButtonText.contentWidth + 20  
                Layout.preferredHeight: walletFileField.height
                Layout.topMargin: 20
                text: qsTr("Browse")
                //display: AbstractButton.IconOnly//AbstractButton.TextBesideIcon//AbstractButton.TextOnly//AbstractButton.TextUnderIcon
                //icon.source: "qrc:/images/ellipsis.png"//"/upload.png"
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
                    
                TextField {
                    id: walletPasswordRestoreField
                    //Layout.alignment: Qt.AlignHCenter//Layout.fillWidth: true
                    Layout.fillWidth: true
                    Layout.preferredWidth: 500
                    Layout.preferredHeight: 50
                    Layout.row: 3
                    Layout.column: 0
                    Layout.topMargin: 15
                    placeholderText: qsTr("Wallet Password"); placeholderTextColor: (NeroshopComponents.Style.darkTheme) ? "#a9a9a9" : "#696969"
                    color: (NeroshopComponents.Style.darkTheme) ? "#ffffff" : "#000000" // textColor
                    echoMode: TextInput.Password
                    inputMethodHints: Qt.ImhSensitiveData
                    selectByMouse: true
                
                    background: Rectangle { 
                        color: (NeroshopComponents.Style.darkTheme) ? "#101010" : "#ffffff"
                        border.color: (NeroshopComponents.Style.darkTheme) ? "#a9a9a9" : "#696969"
                        radius: 3
                    }                     
                }                    
                }
               Rectangle {//ColumnLayout {
                    id: restoreFromMnemonicSeed
                    Layout.fillWidth: true
                    Layout.fillHeight: true
                    border.color: "blue"
                    
                    TextArea {
                        id: mnemonicSeedInput
                        Layout.alignment: Qt.AlignHCenter | Qt.AlignVCenter
                        Layout.preferredWidth: 500
                        Layout.preferredHeight: 500
                        //verticalAlignment: TextEdit.AlignVCenter // align the text within the center of TextArea item's height
                        wrapMode: TextEdit.Wrap
                        selectByMouse: true                        
                    }
                }
                Rectangle {
                    id: restoreFromKeys
                    //ColumnLayout {
                    //}                    
                }
                Rectangle {
                    id: restoreFromHardwareWallet
                    //ColumnLayout {
                    //}                    
                }                           
            }
                // confirm button
                Button {
                    id: loginButton
                    Layout.row: 3
                    Layout.column: 0
                    Layout.fillWidth: true
                    Layout.preferredWidth: 500
                    Layout.preferredHeight: 50
                    Layout.topMargin: 15
                    text: qsTr("Confirm")
                	hoverEnabled: true
                	////onClicked: login()
                	background: Rectangle {
                    	color: "#6b5b95"
                    	radius: 5
                	}
                	                
                	contentItem: Text {  
                    	//font.family: "Consolas"; //font.family: NeroshopComponents.Style.fontFiraCodeLight.name
                    	//font.pointSize: 10
                    	font.bold: true
                    	text: loginButton.text
                    	color: "#ffffff" // white
                    	horizontalAlignment: Text.AlignHCenter
                    	verticalAlignment: Text.AlignVCenter                    
                	}               
                	
                	onClicked: {
                	    // restore from file
                	    if(walletRestoreStack.currentIndex == 0) {
                	    // Todo: place this in a seperate function
                	        if(Wallet.isOpened()) {
                	            /*messageBox.text = qsTr("Wallet is already opened")
                	            messageBox.open()
                	            return;*/
                	            
                	            /*Wallet.close()
                	            if(!Wallet.isOpened()) console.log("old wallet closed");*/
                	        }                	        
                	        if(walletFileField.text.length == 0) {
                	            messageBox.text = qsTr("Wallet path must be specified")
                	            messageBox.open()
                	            return;
                	        }
                	        // Process login credentials
                	        //if(!Wallet.isOpened()) {
                	        if(!Backend.loginWithWalletFile(Wallet, Backend.urlToLocalFile(walletFileField.text), walletPasswordRestoreField.text)) {
                	                messageBox.text = qsTr("Invalid password or wallet network type")
                	                messageBox.open()
                	                return;                	        
                	        }
                            // Start synching the monero node as soon we hit the login button only sync automatically if auto-sync option is turned on (will be turned on by default)
                            onAutoSync();
                            // Switch to HomePage
                            pageLoader.source = "HomePage.qml"
                            console.log("Primary address:", Wallet.getPrimaryAddress())
                            console.log("Balance:", Wallet.getBalanceLocked().toFixed(12))
                            console.log("Unlocked balance:", Wallet.getBalanceUnlocked().toFixed(12))
                            //console.log("Mnemonic:", Wallet.getMnemonic())
                	    }
                	    // restore from seed
                	    if(walletRestoreStack.currentIndex == 1) {
                	        if(mnemonicSeedInput.text.length == 0) {
                	            messageBox.text = qsTr("No mnemonic was entered")
                	            messageBox.open()
                	            return;                	        
                	        }
                	        Wallet.restoreFromMnemonic(mnemonicSeedInput.text)
                	        // Process login credentials
                	        // ...
                	        onAutoSync();
                            // Switch to HomePage
                            ////pageLoader.source = "HomePage.qml"//stack.push(home_page)                	        
                	    }
                	}     
                }            
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
                Text {
                    id: walletGenPageTitle
                    Layout.row: 0
                    Layout.column: 0
                    text: qsTr("Create Wallet")
                    color: (NeroshopComponents.Style.darkTheme) ? "#ffffff" : "#000000"
                    font.bold: true                    
                    font.pointSize: 14
                }                
                // wallet name creation text
                Text {
                    id: walletNameText
                    Layout.row: 1
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
                    Layout.row: 2
                    Layout.column: 0
                    Layout.topMargin: (walletNameText.visible) ? 5 : 0
                    //visible: !walletSeedRepeater.model
                    placeholderText: qsTr("wallet"); placeholderTextColor: (NeroshopComponents.Style.darkTheme) ? "#a9a9a9" : "#696969"
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
                    Layout.row: 3
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
                    Layout.row: 4
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
                    Layout.row: 5
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
                    Layout.row: 6
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
                        border.color: (NeroshopComponents.Style.darkTheme) ? "#a9a9a9" : "#696969"
                        radius: 3
                    }                     
                }
                RowLayout {
                    Layout.row: 7
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
                //icon.source: "qrc:/images/change.png"
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
                    Layout.row: 8
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
                    ////implicitHeight: contentHeight + 20////Layout.preferredHeight: contentHeight + 20  // cause of QML TextArea: Binding loop detected for property "implicitWidth" error
                    ////Layout.topMargin: 20//15
                    selectByMouse: true
                    readOnly: true
                    verticalAlignment: TextEdit.AlignVCenter // align the text within the center of TextArea item's height
                    wrapMode: TextEdit.Wrap // move text to newline if it reaches the width of the TextArea
                    text: qsTr("")
                    //font.bold: true
                    color: (messageCode == 1) ? "#b22222" : ((NeroshopComponents.Style.darkTheme) ? "#ffffff" : "#404040")// text color
                    property int messageCode: 0 //0 = info; 1 = warning or error
                    background: Rectangle { 
                        color: "transparent"
                        border.color: (parent.messageCode == 1) ? "#b22222" : ((NeroshopComponents.Style.darkTheme) ? "#a9a9a9" : parent.color)////parent.color
                        radius: 3
                    }            
                    leftPadding: 25 + circleInfo.contentWidth
                    Text {
                        ////id: circleInfo
                        anchors.left: parent.left
                        anchors.leftMargin: 15
                        anchors.verticalCenter: parent.verticalCenter                         
                        text: (parent.messageCode == 1) ? qsTr(FontAwesome.triangleExclamation) : qsTr(FontAwesome.circleInfo)//(parent.messageCode == 2) ? qsTr(FontAwesome.seedling) : ((parent.messageCode == 1) ? qsTr(FontAwesome.triangleExclamation) : qsTr(FontAwesome.circleInfo))
                        color: (parent.messageCode == 1) ? "#b22222" : "#2196f3"//(parent.messageCode == 2) ? "#228b22" : ((parent.messageCode == 1) ? "#b22222" : "#2196f3")
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
                    icon.source: "qrc:/images/copy.png"
                    icon.color: "#ffffff"
                    display: AbstractButton.IconOnly//AbstractButton.TextBesideIcon//AbstractButton.TextOnly//AbstractButton.TextUnderIcon
                    hoverEnabled: true
                    onClicked: Backend.copyTextToClipboard(Wallet.getMnemonic())
                
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
                	placeholderText: qsTr("Display name (optional)")
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
