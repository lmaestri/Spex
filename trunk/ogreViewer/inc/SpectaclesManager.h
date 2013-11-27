#ifndef SpectaclesManager_h__
#define SpectaclesManager_h__

#include <string>
#include <vector>
#include "OgreVector3.h"

//predeclarations
class Spectacles;
namespace Ogre
{
    class Entity;
    class Material;
    class SceneManager;
    class SceneNode;
}


class SpectaclesManager 
{
private:
    std::vector<Spectacles *>    m_spectacles;
    std::string                  m_configFileName;
    std::string                  m_facePlaceHolderMeshName;
    Ogre::Vector3                m_spectaclesPivotLocation;
    std::string                  m_coordinateCrossMeshName;
public:
    SpectaclesManager();
    ~SpectaclesManager();
    
    void load( const std::string& rConfigFile, Ogre::SceneManager* pSceneManager = 0, Ogre::SceneNode *pNode = 0, bool bCreateAndAttach = false ); 
    void clear( void );
    void reload( const std::string& rConfigFile = "" );

    Spectacles * getSpectacles( int index );
    unsigned int getNumSpectacles( void );
    std::string getFacePlaceHolderMeshName( void );
    Ogre::Vector3 getSpectaclesPivotLocation();
    std::string getCoordinateCrossMeshName();
};

#endif // SpectaclesManager_h__
