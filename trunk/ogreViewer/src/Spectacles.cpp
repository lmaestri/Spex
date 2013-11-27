#include "Spectacles.h"
#include "OgreEntity.h"
#include "OgreMaterial.h"
#include <string>
#include "OgreSceneManager.h"
#include "OgreSceneNode.h"
#include "OgreException.h"
#include "OgreLog.h"
#include "OgrePrerequisites.h"

Spectacles::Spectacles( const std::string &rName, const std::string &rDescription, const std::string &rMeshName, const std::string &rMaterial )
    : m_name( rName ), m_description( rDescription ), m_ogreMeshName( rMeshName ), m_ogreMaterialName( rMaterial ), m_isMeshLoaded( false )
{

}

Spectacles::~Spectacles( void )
{


}

void Spectacles::createEntityAndAttach( Ogre::SceneManager* pSceneManager, Ogre::SceneNode *pSceneNode, bool bIsVisible )
{
    try
    {
        m_pMesh = pSceneManager->createEntity( m_name, m_ogreMeshName );
        pSceneNode->attachObject( m_pMesh ); 
        m_pMesh->setVisible( bIsVisible );

        m_isMeshLoaded = true;
    }
    catch( Ogre::Exception& e )
    {
       Ogre::String message = e.what();
       //Ogre::LogManager.getSingleton().logMessage( message );
    }
}

Ogre::Entity *      Spectacles::getMesh( void ) const
{
    return m_pMesh;
}

Ogre::Material *    Spectacles::getMaterial( void ) const
{
    return m_pMaterial;
}

const std::string&  Spectacles::getName( void ) const
{
    return m_name;
}

const std::string&  Spectacles::getDescription( void ) const
{
    return m_description;
}

bool Spectacles::isVisible( void ) const
{
    if( m_isMeshLoaded )
    {
        return m_pMesh->isVisible();
    }
    else
    {
        return false;
    }
}

void Spectacles::setVisible( bool bIsVisible )
{
    if( m_isMeshLoaded )
    {
        m_pMesh->setVisible( bIsVisible );
    }
}

const std::string& Spectacles::getThumbNail() const
{
    return m_thumbnail;
}
