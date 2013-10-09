import bb.cascades 1.0

Page {
    Container {
        Label {
            text: "BlackBerry ID - Core Properties"
            textStyle.color: Color.White
            textStyle.textAlign: TextAlign.Center
            horizontalAlignment: HorizontalAlignment.Center
        }
        background: Color.create("#2C2B2C")
        verticalAlignment: VerticalAlignment.Center

        TextField {
            id: screen_name
            hintText: "Select a button to get user info"
            inputMode: TextFieldInputMode.Text
            input.submitKey: SubmitKey.None
            horizontalAlignment: HorizontalAlignment.Center
            verticalAlignment: VerticalAlignment.Center
            text: buttonHandler.displayText
            clearButtonVisible: false
            focusPolicy: FocusPolicy.None
        }
        Button {
            text: "Get Screen Name"
            verticalAlignment: VerticalAlignment.Center
            horizontalAlignment: HorizontalAlignment.Fill
            topMargin: 20.0
            onClicked: {
                buttonHandler.get_ids_properties(1);
            }
        }
        Button {
            text: "Get First and Last Name"
            verticalAlignment: VerticalAlignment.Center
            horizontalAlignment: HorizontalAlignment.Fill
            topMargin: 20.0
            onClicked: {
                buttonHandler.get_ids_properties(2);
            }
        }
        Button {
            text: "Get User Name"
            verticalAlignment: VerticalAlignment.Center
            horizontalAlignment: HorizontalAlignment.Fill
            topMargin: 20.0
            onClicked: {
                buttonHandler.get_ids_properties(3);
            }
        }
        Button {
            text: "Get Unique Identifier"
            verticalAlignment: VerticalAlignment.Center
            horizontalAlignment: HorizontalAlignment.Fill
            topMargin: 20.0
            onClicked: {
                buttonHandler.get_ids_properties(4);
            }
        }
    }
}
