/* Copyright (c) 2012 Research In Motion Limited.
 * Copyright (c) 2012 Truphone Limited.
 *
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *
 * http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */
import bb.cascades 1.0

// creates one page with a label

Page {
    Container {
        layout: DockLayout {
        }
        background: Color.create("#DEE3E7")
        Container {
            verticalAlignment: VerticalAlignment.Center
            horizontalAlignment: HorizontalAlignment.Center
            Button {
                text: _audioControl.audioRunningProperty ? "Audio OFF" : "Audio ON"
                onClicked: {
                    _audioControl.toggleAudioOnOff()
                }
            }
            Button {
                enabled: _audioControl.audioRunningProperty ? true : false
                text: _audioControl.speakerOnProperty ? "Speaker OFF" : "Speaker ON"
                onClicked: {
                    _audioControl.toggleSpeaker()
                }
            }
            ImageButton {
                id: hashButton
                defaultImageSource: "asset:///hash.png"
                pressedImageSource: hashButton.defaultImageSource
                disabledImageSource: hashButton.defaultImageSource
                horizontalAlignment: HorizontalAlignment.Fill
                verticalAlignment: VerticalAlignment.Fill
                onTouch: {
                    if (event.isDown()) {
                        _audioControl.dtmfKeyDown()
                    } else if (event.isUp()) {
                        _audioControl.dtmfKeyUp()
                    }
                }
            }
        }
    }
}
