#include "SpectaclesManager.h"
#include "OgreConfigFile.h"
#include "Spectacles.h"
#include "OgreStringConverter.h"

SpectaclesManager::SpectaclesManager()
    :m_configFileName( "" ),
    m_facePlaceHolderMeshName( "" )
{

}
SpectaclesManager::~SpectaclesManager()
{

}



void SpectaclesManager::load( const std::string& rConfigFileName, Ogre::SceneManager* pSceneManager, Ogre::SceneNode *pSceneNode , bool bCreateAndAttach )
{
    Ogre::ConfigFile configFile;

    configFile.load( rConfigFileName );
    m_configFileName = rConfigFileName;

    // iterate over all sections in the file. 
    // each section is one glass
    Ogre::ConfigFile::SectionIterator sectionIter = configFile.getSectionIterator();
    sectionIter.moveNext();
    while( sectionIter.end() != sectionIter.current() )
    {
        Ogre::String sectionName = sectionIter.peekNextKey();
        Ogre::String mesh = configFile.getSetting ( "Mesh", sectionName );

        // check if "Mesh" is set, otherwise we don't create glasses!
        if( "" != mesh )
        {
            Ogre::String name = configFile.getSetting( "Name", sectionName );
            // if the name is empty use the section title
            if( "" == name )
            {
               name = sectionName; 
            }
            Ogre::String description = configFile.getSetting( "Desc", sectionName, "" ); 
            Ogre::String material= configFile.getSetting( "Material", sectionName, "" );
            Ogre::String thumbnail = configFile.getSetting( "Thumb", sectionName, "" );
            
            Spectacles *pGlass = new Spectacles( name, description, mesh, material );
            pGlass->m_thumbnail = thumbnail;
            if( bCreateAndAttach )
            {
                pGlass->createEntityAndAttach( pSceneManager, pSceneNode, false );
            }

            m_spectacles.push_back( pGlass );
        }
        sectionIter.getNext();
    }

    // we also store the faceplaceholder mesh in this config file (maybe later make a placeholder for each glass?)
    m_facePlaceHolderMeshName = configFile.getSetting( "FacePlaceHolder" );
    m_spectaclesPivotLocation = Ogre::StringConverter::parseVector3( configFile.getSetting( "PivotLocation" ) );
    m_coordinateCrossMeshName = configFile.getSetting( "CoordinateSpaceCross" );
}



void SpectaclesManager::clear( void )
{
    std::vector<Spectacles *>::iterator iter = m_spectacles.begin();
    while( iter != m_spectacles.end() )
    {
        delete (*iter);
        (*iter) = 0;
        iter ++;
    }

    m_spectacles.clear();
}


void SpectaclesManager::reload( const std::string& rConfigFile )
{
    clear();
    load( ""==rConfigFile ? m_configFileName : rConfigFile ); // either load from rConfigFile or use save config
}



Spectacles * SpectaclesManager::getSpectacles( int index )
{
    if( index >= 0 && index < (int)m_spectacles.size() )
    {
        return m_spectacles[index];
    }
    return 0; // oterwhise
}


unsigned int SpectaclesManager::getNumSpectacles( void )
{
    return m_spectacles.size();
}


std::string SpectaclesManager::getFacePlaceHolderMeshName( void )
{
    return m_facePlaceHolderMeshName;
}

Ogre::Vector3 SpectaclesManager::getSpectaclesPivotLocation()
{
    return m_spectaclesPivotLocation;
}

std::string SpectaclesManager::getCoordinateCrossMeshName()
{
   return m_coordinateCrossMeshName; 
}
