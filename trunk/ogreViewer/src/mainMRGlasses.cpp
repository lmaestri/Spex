#include "AppLogic.h"


int main(int argc, char *argv[])
{
    // Create application object
    AppLogic mainApp;

    
    try {
        mainApp.start();
    } catch( Ogre::Exception& e ) {
        std::cerr << "An exception has occured: " <<
            e.getFullDescription().c_str() << std::endl;
    }

    return 0;
}