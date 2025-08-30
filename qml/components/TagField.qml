import QtQuick 2.12
import QtQuick.Controls 2.12
import QtQuick.Layouts 1.12

import FontAwesome 1.0

Item {
    id: root
    implicitWidth: 500
    implicitHeight: childrenRect.height

    property var tagList: ListModel {}//[]
    property int maxTagCount: 9 // Maximum number of tags allowed
    
    property alias textField: tagInput
    
    function tags() {
        var stringList = []
        for (let i = 0; i < repeater.count; i++) {
            let item = repeater.itemAt(i)
            let text = item.children[0].children[0].text
            stringList.push(text)
        }
        return stringList
    }
    
    function addTag(tagName) {
        if (isTagDuplicate(tagName)) {
            ////console.log("addTag: Duplicate tag name:", tagName)
            return;
        }
        if (tagList.count >= maxTagCount) {
            console.log("addTag: Maximum tags exceeded:", tagList.count)
            return;
        }
        // Append tag to tagList model
        tagList.append({ text: tagName });
    }
    
    function removeTag(tagName) {
        // Remove tag from tagList model
        for (let i = tagList.count - 1; i >= 0; i--) {
            if (tagList.get(i).text === tagName) {
                tagList.remove(i)
                break; // assuming tags are unique, stop after removing
            }
        }
    }
    
    function clearTags() {
        tagList.clear()
    }
    
    function isTagDuplicate(tag) {
        for (let i = 0; i < tagList.count; i++) {
            if (tagList.get(i).text === tag) {
                return true; // Found a duplicate tag
            }
        }
        return false; // No duplicate tag found
    }
    
    function containsTag(tag) {
        return isTagDuplicate(tag)
    }

    ColumnLayout {
        width: parent.width
        spacing: 5
        
        Flow {
            Layout.fillWidth: true//width: parent.width//Layout.maximumWidth: parent.width//Layout.maximumHeight: parent.height * 0.8
            spacing: 5
        
            Repeater {
                id: repeater
                model: tagList
                //onItemRemoved: {}
                delegate: Rectangle {
                    width: removeTagButton.visible ? (tagLabel.width + 15) + 20 : tagLabel.width + 20
                    height: tagLabel.height + 10
                    radius: 12//20
                    color: "lightblue"

                    border.color: "black"
                    border.width: 1

                    Row {
                        anchors.fill: parent
                        spacing: 5
                        Text {
                            id: tagLabel
                            anchors.verticalCenter: parent.verticalCenter
                            leftPadding: 10
                            text: model.text//modelData
                        }
                    
                        Button {
                            id: removeTagButton
                            anchors.verticalCenter: parent.verticalCenter
                            width: 20; height: 20
                            text: qsTr(FontAwesome.xmark)//("x")
                            hoverEnabled: true
                            
                            contentItem: Text {
                                horizontalAlignment: Text.AlignHCenter
                                verticalAlignment: Text.AlignVCenter
                                text: removeTagButton.text
                                color: removeTagButton.hovered ? "#ffffff" : "#000000"
                                font.bold: true
                                font.family: FontAwesome.fontFamily
                            }
                        
                            background: Rectangle {
                                width: parent.width
                                height: parent.height
                                radius: 50
                                color: removeTagButton.hovered ? "firebrick" : "transparent"
                                opacity: 0.7
                            }
                        
                            onClicked: {
                                tagList.remove(index)
                            }
                            
                            MouseArea {
                                anchors.fill: parent
                                onPressed: mouse.accepted = false
                                cursorShape: Qt.PointingHandCursor
                            }
                        }
                    } // Row
                }
            }
        }

        TextField {
            id: tagInput
            Layout.fillWidth: true//width: parent.width
            Layout.preferredHeight: 50//height: 30
            placeholderText: "Add tags (comma-separated)"
    
            onEditingFinished: {
                if (tagInput.text.trim() !== "") {
                    let tags = tagInput.text.split(",")
                    tags = tags.map(function(tag) { return tag.trim() })
                    
                    // Calculate the number of tags to add based on the maximum tag count
                    let tagsToAdd = Math.min(tags.length, maxTagCount - tagList.count)
                    
                    for (let i = 0; i < tagsToAdd; i++) {
                        if (tags[i] !== "" && !root.isTagDuplicate(tags[i])) { // Skip duplicate tags and empty tags after comma
                            tagList.append({ text: tags[i] })
                        }
                    }
                    tagInput.text = ""
                }
            }
        }
    } // ColumnLayout
}

