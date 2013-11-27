#ifndef FaceManager_h__
#define FaceManager_h__
#include <string>
#include <map>
#include <OgreVector3.h>
#include "OgreStringVector.h"

// predeclarations
class Face;

class FaceManager
{
private:
    std::string m_configFileName;
    typedef std::map<std::string, Face *> FaceMap;
    typedef std::pair<std::string, Face *> FacePair; 
    FaceMap m_faces;

    void addFace( const std::string& rName, Face *pFace, bool bOverWrite = true );
    void loadImageFromFile( Ogre::String &fileName, Ogre::Image &rImage );
    
public: 
    FaceManager( void );
    ~FaceManager( void );
    void load( const std::string& rConfigFile ); 

    void clear( void );
    void reload( const std::string& rConfigFile = "" );
    void store( const std::string& rFileName );
    void addFaceCopy( const std::string& rName, const Face& rFace, bool bOverWrite = true, bool bWriteToFile = true );
    bool removeFaceByName( const std::string& rName );
    Face* getFaceByName( const std::string& rName );
    const Ogre::StringVector getFaceNames();
};

#endif // FaceManager_h__
