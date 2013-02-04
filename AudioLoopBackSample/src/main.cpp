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

#include <bb/cascades/Application>
#include <bb/cascades/QmlDocument>
#include <bb/cascades/AbstractPane>

#include <QLocale>
#include <QTranslator>
#include <Qt/qdeclarativedebug.h>
#include <iostream>
#include "AudioLoopBackSample.hpp"

using namespace bb::cascades;

static void customMessageHandler(QtMsgType type, const char *message)
{
	switch (type) {
		case QtDebugMsg:
			std::cout << "Debug: " << message << std::endl;
			break;
		case QtWarningMsg:
			std::cout << "Warning: " << message << std::endl;
			break;
		case QtCriticalMsg:
			std::cout << "Critical: " << message << std::endl;
			break;
		case QtFatalMsg:
			std::cout << "Fatal: " << message << std::endl;
			std::abort();
			break;
	}
}

Q_DECL_EXPORT int main(int argc, char **argv)
{
	qInstallMsgHandler(customMessageHandler);
    // this is where the server is started etc
    Application app(argc, argv);

    // localization support
    QTranslator translator;
    QString locale_string = QLocale().name();
    QString filename = QString( "AudioLoopBackSample_%1" ).arg( locale_string );
    if (translator.load(filename, "app/native/qm")) {
        app.installTranslator( &translator );
    }

    new AudioLoopBackSample(&app);

    // we complete the transaction started in the app constructor and start the client event loop here
    return Application::exec();
    // when loop is exited the Application deletes the scene which deletes all its children (per qt rules for children)
}
