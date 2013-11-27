#ifndef Spectacle_h__
#define Spectacle_h__

#include <string>

//predecrlarations
namespace Ogre
{
    class Entity;
    class Material;
    class SceneManager;
    class SceneNode;
}

// Base class representing a renderable glass
// contains name, description, mesh ...
class Spectacles 
{
    friend class SpectaclesManager;

private:
    std::string         m_name;
    std::string         m_description;
    std::string         m_ogreMeshName;
    std::string         m_ogreMaterialName;
    std::string         m_thumbnail;
    Ogre::Entity      * m_pMesh;
    Ogre::Material    * m_pMaterial;
    bool                m_isMeshLoaded;

    void createEntityAndAttach( Ogre::SceneManager* pSceneManager, Ogre::SceneNode *pSceneNode, bool bIsVisible );

public:
    Spectacles( const std::string &rName, const std::string &rDescription, const std::string &rMeshName, const std::string &rMaterial = "" );
    ~Spectacles( void );

    Ogre::Entity *      getMesh( void ) const;
    Ogre::Material *    getMaterial( void ) const;
    const std::string&  getName( void ) const;
    const std::string&  getDescription( void ) const;
    bool                isVisible( void ) const;
    void setVisible( bool isVisible );
    const std::string&  getThumbNail() const;
};

#endif // Spectacle_h__
