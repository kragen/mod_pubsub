/**
 * Test globals
 */
#if !defined(GLOBALS_H)
#define GLOBALS_H


/**
 * SERVER_SERVICE_NAME is the name of the KnowNow server if it is installed
 * as a MS service by the installer.  Earlier releases used "KnowNow 
 * Notification Server", while later releases use "KnowNow Server". If the
 * server was not installed as a service use "NONE" so the test will know it
 * cannot stop and start the server.
 *
 * <PRE>
 * To get a list of services go to:
 * Contol Panel - Administration - Services
 * KnowNow Notification Server
 * KnowNow Server
 * </PRE>
 *
 * In stead of re-compiling use the -servers param to override the default.
 * <P>
 * NOTE: IF "NONE" is set then any tests that require to start or stop the
 * server should execute CPPUNIT_FAIL(ERR_SERVER_SERVICE_NOT_AVAILABLE);
 * <P>
 * This methodology will have to be changed to be portable to other non-MS platforms.
 */
#define DEFAULT_SERVER_NAME "KnowNow Server"
//#define DEFAULT_SERVER_NAME "KnowNow Notification Server"
//#define DEFAULT_SERVER_NAME "NONE"

/*
 * This is the default server URI. You can use the -servername param to override it
 * or add a list of URIs.
 */
#define DEFAULT_SERVER_URI "http://localhost:8000/kn"




// error messages.
#define ERR_SERVER_SERVICE_NOT_AVAILABLE "Test deferred: cannot start/stop KnowNow server."





#endif // !defined(GLOBALS_H)

