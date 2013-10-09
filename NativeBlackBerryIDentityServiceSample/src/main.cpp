#include <bb/cascades/Application>
#include "applicationui.hpp"
#include "bbids.hpp"

using namespace bb::cascades;

Q_DECL_EXPORT int main( int argc, char **argv ) {
    Application app( argc, argv );

    // start up the IDS library handling
    bbids *idsHandler = new bbids();

    // Create the Application UI object, this is where the main.qml file
    // is loaded and the application scene is set.
    new ApplicationUI( &app, idsHandler );

    // Enter the application main event loop.
    return Application::exec();
}

