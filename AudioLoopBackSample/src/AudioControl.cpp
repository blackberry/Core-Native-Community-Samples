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

#include "AudioControl.hpp"
#include <audio/audio_manager_routing.h>
#include <QDebug>
#include <stdio.h>

extern void startPcmAudio();
extern void stopPcmAudio();
extern void toggleSpeaker(bool enable);

AudioControl::AudioControl(QObject* parent) : QObject(parent)
	, m_audioRunning(false)
	, m_speakerOn(false)
	, m_dtmfAudioManager(0)
{
}

AudioControl::~AudioControl()
{
	releaseAudioManagerHandle();
}

void AudioControl::toggleSpeaker()
{
	m_speakerOn ? ::toggleSpeaker(false) : ::toggleSpeaker(true);
    m_speakerOn = !m_speakerOn;
    emit speakerOnSignal(m_speakerOn);
}

void AudioControl::toggleAudioOnOff()
{
	if(m_audioRunning){
		m_speakerOn = false;
		emit speakerOnSignal(m_speakerOn);
	}
	m_audioRunning ? stopPcmAudio() : startPcmAudio();
	m_audioRunning = !m_audioRunning;
	emit audioRunningSignal(m_audioRunning);
}

bool AudioControl::isAudioRunning()
{
	return m_audioRunning;
}

bool AudioControl::isSpeakerOn()
{
	return m_speakerOn;
}

void AudioControl::releaseAudioManagerHandle()
{
	if(m_dtmfAudioManager != 0){
		int ret = audio_manager_free_handle(m_dtmfAudioManager);
		if(ret != 0){
			qDebug() << "AudioControl::releaseHandle() = " << ret;
		}
		m_dtmfAudioManager = 0;
	}
}

void AudioControl::dtmfKeyDown()
{
	lazyInitializDtmfPlayer();
	m_mediaPlayer.play();
}

void AudioControl::lazyInitializDtmfPlayer()
{
	if(m_dtmfAudioManager == 0){
		int ret = audio_manager_get_handle(AUDIO_TYPE_VOICE_TONES,0,true,&m_dtmfAudioManager);
		if(ret != 0){
			qDebug() << "AudioControl::audioManagerHandle() = " << ret;
		}
		ret = audio_manager_set_handle_routing_conditions(
				m_dtmfAudioManager, SETTINGS_RESET_ON_DEVICE_DISCONNECTION | SETTINGS_RESET_ON_DEVICE_CONNECTION);

		if(ret != 0){
			qDebug() << "AudioControl::audioManagerHandle() = " << ret;
		}
		m_mediaPlayer.setSourceUrl(QUrl("asset:///DTMFG.WAV"));
		m_mediaPlayer.setAudioManagerHandle(m_dtmfAudioManager);
	}
}

void AudioControl::dtmfKeyUp()
{
	m_mediaPlayer.stop();
}
