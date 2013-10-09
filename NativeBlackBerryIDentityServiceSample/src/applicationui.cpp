#include <errno.h>
#include <bb/cascades/Application>
#include <bb/cascades/QmlDocument>
#include <bb/cascades/AbstractPane>
#include "applicationui.hpp"
#include "bbids.hpp"

using namespace bb::cascades;

static QmlDocument *qml = NULL;

/**
 * Initializes the UI for the application.
 */
ApplicationUI::ApplicationUI( bb::cascades::Application *app, bbids *handler ) :
        QObject( app ), idsHandler( handler ) {

    // Create scene document from main.qml asset, the parent is set
    // to ensure the document gets destroyed properly at shut down.
    qml = QmlDocument::create( "asset:///main.qml" ).parent( this );

    //Adding an event handler to the button "get information"
    qml->setContextProperty( "buttonHandler", this );

    // Create root object for the UI
    AbstractPane *root = qml->createRootObject<AbstractPane>();

    // Set created root object as the application scene(home page in this instance)
    app->setScene( root );
}

/**
 * Get the current display text.
 * @return The text that application handler should display
 */
QString ApplicationUI::displayText() {
    return displayValue;
}

/**
 * Update the text that the application should be displaying.  Emits the signal
 * that the property has changed so that the QML ui is updated.
 * @param txt The new text that the application is to display.
 */
void ApplicationUI::setDisplayText( QString txt ) {
    displayValue = txt;
    emit displayTextChanged( displayValue ); // signal value was updated
}

/**
 * This function is called asynchronously if the BBIDS responds with success.
 * This will simply set the text box in the QML file to the property value provided.
 */
extern "C" void my_success_handler( ids_request_id_t request_id,
                                    int property_count,
                                    const ids_property_t* properties,
                                    void* cb_data ) {
    (void)request_id; // suppress unused warning
    ApplicationUI* uiApp = (ApplicationUI*)cb_data;
    QString value;
    // concat all of the properties returned together for displaying
    for( int i = 0; i < property_count; i++ ) {
        value.append( properties[i].value );
        value.append( " " );
    }
    uiApp->setDisplayText( value );
}

/**
 * This function is called asynchronously if the BBIDS responds with a failure.
 */
extern "C" void my_failure_handler( ids_request_id_t request_id,
                                    ids_result_t result,
                                    const char* info,
                                    void* cb_data ) {
    ApplicationUI* uiApp = (ApplicationUI*)cb_data;
    char failureText[ 32 ];
    sprintf( failureText, "Failure: %d", result );
    uiApp->setDisplayText( QString( failureText ) );

    fprintf( stderr, "Req %u: %s [%s]\n", request_id, failureText, info?info:"NULL" );
}

/**
 * Triggered from QML when the Get User Information buttons are pressed.
 * Each value of x passed in is from the button in the QML indicating which user detail
 * being requested.
 * The request for the property(s) is sent by this function, but the updates with the
 * actual content is performed asynchronously in the success or failure handlers.
 */
void ApplicationUI::get_ids_properties( int x ) {
    switch( x ) {
        case 1: {
            const char* properties[] = { IDS_BBID_PROP_SCREENNAME };
            // Other valid values include IDS_BBID_PROP_SCREENNAME, IDS_BBID_PROP_FIRSTNAME, IDS_BBID_PROP_LASTNAME, IDS_BBID_PROP_USERNAME
            int result = ids_get_properties( idsHandler->_ids_provider, BBID_PROPERTY_CORE, 1,
                    properties, my_success_handler, my_failure_handler, this, NULL );
            if( result != IDS_SUCCESS ) {
                fprintf( stderr, "Failed to send ids_get_properties request. errno %d", errno );
            }
            break;
        }
        case 2: {
            // retrieves multiple properties at the same time
            const char* properties[] = { IDS_BBID_PROP_FIRSTNAME, IDS_BBID_PROP_LASTNAME };
            int property_count = 2;
            int result = ids_get_properties( idsHandler->_ids_provider, BBID_PROPERTY_CORE, property_count,
                    properties, my_success_handler, my_failure_handler, this, NULL );
            if( result != IDS_SUCCESS ) {
                fprintf( stderr, "Failed to send ids_get_properties request. errno %d", errno );
            }
            break;
        }
        case 3: {
            const char* properties[] = { IDS_BBID_PROP_USERNAME };
            int result = ids_get_properties( idsHandler->_ids_provider, BBID_PROPERTY_CORE, 1,
                    properties, my_success_handler, my_failure_handler, this, NULL );
            if( result != IDS_SUCCESS ) {
                fprintf( stderr, "Failed to send ids_get_properties request. errno %d", errno );
            }
            break;
        }
        case 4: {
            const char* properties[] = { IDS_BBID_PROP_UID };
            int result = ids_get_properties( idsHandler->_ids_provider, BBID_PROPERTY_CORE, 1,
                    properties, my_success_handler, my_failure_handler, this, NULL );
            if( result != IDS_SUCCESS ) {
                fprintf( stderr, "Failed to send ids_get_properties request. errno %d", errno );
            }
            break;
        }
    }
}
