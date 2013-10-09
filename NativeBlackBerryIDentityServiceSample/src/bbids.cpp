#include <stdio.h>
#include <errno.h>
#include "bbids.hpp"

/**
 * The SLOT function that is triggered when the signal is triggered on the open socket
 */
void bbids::handleIO() {
    // Detected IDS data to be processed
    ids_process_msg( ids_fd );
}

/**
 * Constructor. This initializes the ids service. It is instantiated in the
 * main.cpp file "bbids *idsHandler = new bbids();"
 */
bbids::bbids() {
    // Step 1 - Initialize IDS
    ids_result_t result = ids_initialize();
    if( result != IDS_SUCCESS ) {
        fprintf( stderr, "Failured to initialize ids. errno %d", errno);
        return;
    }

    // OPTIONAL - Increase default logging level of the ids library for
    //            additional diagnostic information during development
    ids_set_option( IDS_OPT_VERBOSITY, IDS_LOG_VERBOSE );

    // Step 2 - Register to use BlackBerry ID as the Identity Provider
    result = ids_register_provider( BLACKBERRY_ID_PROVIDER, &_ids_provider, &ids_fd );
    if( result != IDS_SUCCESS ) {
        fprintf( stderr, "Failured to register ids provider. errno %d", errno);
        return;
    }

    // Step 3 - Add an FD handler for the IDS library communications
    notifier = new QSocketNotifier( ids_fd, QSocketNotifier::Read );
    notifier->setEnabled( true );
    QObject::connect( notifier, SIGNAL( activated(int) ), this, SLOT( handleIO() ) );
}

/**
 * Destructor. Clean-up resources.
 */
bbids::~bbids() {
    ids_shutdown();
}
