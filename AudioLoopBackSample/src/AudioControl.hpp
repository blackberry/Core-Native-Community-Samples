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

#ifndef AUDIOCONTROL_HPP_
#define AUDIOCONTROL_HPP_

#include <bb/multimedia/MediaPlayer>
#include <QObject>
#include <QVariant>

using namespace bb::multimedia;

class AudioControl : public QObject{
Q_OBJECT
Q_PROPERTY(bool audioRunningProperty READ isAudioRunning NOTIFY audioRunningSignal)
Q_PROPERTY(bool speakerOnProperty READ isSpeakerOn NOTIFY speakerOnSignal)

public:
	AudioControl(QObject* parent = NULL);
	virtual ~AudioControl();
public slots:
	void toggleSpeaker();
	void toggleAudioOnOff();
	void dtmfKeyDown();
	void dtmfKeyUp();
signals:
	void audioRunningSignal(bool);
	void speakerOnSignal(bool);
private:
	bool isAudioRunning();
	bool isSpeakerOn();
	void releaseAudioManagerHandle();
	void lazyInitializDtmfPlayer();

private:
	bool m_audioRunning;
	bool m_speakerOn;
	unsigned int m_dtmfAudioManager;
	MediaPlayer m_mediaPlayer;

};

#endif /* AUDIOCONTROL_HPP_ */
