#include "FaceManager.h"
#include "OgreConfigFile.h"
#include "OgreException.h"
#include "Face.h"
#include "OgreVector3.h"
#include "OgreStringConverter.h"
#include "OgreStringVector.h"
#include "OgreResourceGroupManager.h"
#include <iosfwd>


FaceManager::FaceManager( void )
    : m_configFileName( "" )
{


}

FaceManager::~FaceManager( void )
{
    clear( );
}






void FaceManager::clear( void )
{
    std::map<std::string, Face*>::iterator iter = m_faces.begin();
    while( iter != m_faces.end() )
    {
        delete iter->second;
        iter->second = 0;
        ++ iter;
    }
}



void FaceManager::reload( const std::string& rConfigFile )
{
    clear();
    load( ""==rConfigFile ? m_configFileName : rConfigFile ); // either load from rConfigFile or use save config
}


std::string float3ToStream( float pVec3[] )
{
    std::stringstream st;
    st << pVec3[0] << ", " << pVec3[1] << ", " << pVec3[2];
    return st.str();
}

std::string vector3ToStream( Ogre::Vector3 vec ){
	std::stringstream st;
    st << vec.x << ", " << vec.y << ", " << vec.z;
    return st.str();
}

std::string vector2ToStream( Ogre::Vector2 vec ){
	std::stringstream st;
    st << vec.x << ", " << vec.y;
    return st.str();
}

void FaceManager::load( const std::string& rConfigFileName )
{
    Ogre::ConfigFile configFile;
    bool isLoaded = true;

    try
    {
        configFile.load( rConfigFileName );
    }
    catch( Ogre::FileNotFoundException * e)
    {
        // cannot load file
        e->what();
        isLoaded = false;	
    }
    catch( ... )
    {
        isLoaded = false;
    }

    if( isLoaded )
    {
        m_configFileName = rConfigFileName;

        // iterate over all sections in the file. 
        // each section is one face
        Ogre::ConfigFile::SectionIterator sectionIter = configFile.getSectionIterator();
        sectionIter.moveNext();
        while( sectionIter.end() != sectionIter.current() )
        {
            Ogre::String sectionName = sectionIter.peekNextKey();
            Ogre::String name = configFile.getSetting( "Name", sectionName );
            Ogre::String colorFile = configFile.getSetting( "ColorFile", sectionName );
            Ogre::String depthFile = configFile.getSetting( "DepthFile", sectionName );
            Ogre::Vector3 location = Ogre::StringConverter::parseVector3( configFile.getSetting( "Location", sectionName ) );
            Ogre::Vector3 rotation = Ogre::StringConverter::parseVector3( configFile.getSetting( "Rotation", sectionName ) );
            float scale = Ogre::StringConverter::parseReal( configFile.getSetting( "Scale", sectionName, "1.0" ), 1.0 );
            Ogre::Vector3 specLocation = Ogre::StringConverter::parseVector3( configFile.getSetting( "SpecLocation", sectionName ) );
            Ogre::Vector3 specRotation = Ogre::StringConverter::parseVector3( configFile.getSetting( "SpecRotation", sectionName ) );
            float specScale = Ogre::StringConverter::parseReal( configFile.getSetting( "SpecScale", sectionName, "1.0" ), 1.0 );
			Ogre::Vector3 leftEye3DLocation = Ogre::StringConverter::parseVector3( configFile.getSetting( "LeftEyeLocation3D", sectionName ) );
			Ogre::Vector3 rightEye3DLocation = Ogre::StringConverter::parseVector3( configFile.getSetting( "RightEyeLocation3D", sectionName ) );
			Ogre::Vector2 leftEye2DLocation = Ogre::StringConverter::parseVector2( configFile.getSetting( "LeftEyeLocation2D", sectionName ) );
			Ogre::Vector2 rightEye2DLocation = Ogre::StringConverter::parseVector2( configFile.getSetting( "RightEyeLocation2D", sectionName ) );
            Face * pFace = new Face(); 
            pFace->setPose( (location.ptr()), (rotation.ptr()), scale );
            pFace->setSpectaclesPose( (specLocation.ptr()), (specLocation.ptr()), specScale );
			pFace->setLeftEye3D(leftEye3DLocation.x, leftEye3DLocation.y, leftEye3DLocation.z);
			pFace->setRightEye3D(rightEye3DLocation.x, rightEye3DLocation.y, rightEye3DLocation.z);
			pFace->setLeftEye2D(leftEye2DLocation.x, leftEye2DLocation.y);
			pFace->setRightEye2D(rightEye2DLocation.x, rightEye2DLocation.y);
            try{
                loadImageFromFile( depthFile, pFace->m_depthImage );
                pFace->m_depthImageFileName = depthFile;
                pFace->m_bContainsDepthImage = true;
            } catch ( ... )
            {
                // catch if depth image fails to load, but keep face anyway!
                pFace->m_bContainsDepthImage = false;
            }
            try{
                loadImageFromFile( colorFile, pFace->m_colorImage );
                pFace->m_colorImageFileName = colorFile; // also only write if files could be loaded
                addFace( sectionName, pFace );
            } catch ( ... )
            {
                // if loading of color image fails, we also don't add the face
                delete pFace;
            }

            // continue iteration
            sectionIter.moveNext();
        }

    }
}

void FaceManager::store( const std::string& rFileName )
{
    std::ofstream file;
    file.open( rFileName.c_str() );
    file << "# Ogre Config file exported by FaceManager " << std::endl;

    FaceMap::iterator iter = m_faces.begin();
    while( iter != m_faces.end() )
    {
        file << "[" << iter->first << "]" << std::endl;

        const Face::Pose& pose = iter->second->getPose();
        const Face::Pose& specPose = iter->second->getSpectaclesPose();
		Face* face = iter->second;

        file << "Name = " << iter->first << std::endl;
        file << "ColorFile = " << face->m_colorImageFileName << std::endl; 
        file << "DepthFile = " << face->m_depthImageFileName << std::endl; 
        file << "Location = " << float3ToStream( (float*) pose.location ) << std::endl;
        file << "Rotation = " << float3ToStream( (float*) pose.rotation ) << std::endl;
        file << "Scale = " << pose.scaling << std::endl;
        file << "SpecLocation = " << float3ToStream( (float*) specPose.location ) << std::endl;
        file << "SpecRotation = " << float3ToStream( (float*) specPose.rotation ) << std::endl;
        file << "SpecScale = " << specPose.scaling << std::endl;
		file << "LeftEyeLocation3D = " << vector3ToStream( face->getLeftEye3DLocation() ) << std::endl;
		file << "RightEyeLocation3D = " << vector3ToStream( face->getRightEye3DLocation() ) << std::endl;
		file << "LeftEyeLocation2D = " << vector2ToStream( face->getLeftEye2DLocation() ) << std::endl;
		file << "RightEyeLocation2D = " << vector2ToStream( face->getRightEye2DLocation() ) << std::endl;
		file << "PupillaryDistance = " << face->getPupillaryDistance() << std::endl;
        iter->second = 0;
        ++ iter;
    }

    // vertices
    file.close();
}



void FaceManager::addFaceCopy( const std::string& rName, const Face&  rFace, bool bOverWrite, bool bWriteToFile )
{
    if( bOverWrite || getFaceByName( rName ) == 0 )
    {
        removeFaceByName( rName ); // delete before inserting new one
        Face * pFace = new Face( rFace );
        m_faces.insert( FacePair( rName, pFace ) );
        if( bWriteToFile )
        {
            std::string colorFileName( rName );
            std::string depthFileName( rName );

            // write to depthImage and colorImage to File (harddisk) ... do it now.
            pFace->m_colorImage.save( colorFileName.append( "_color.png" ) );
            pFace->m_depthImage.save( depthFileName.append( "_depth.bmp" ) );

            pFace->m_colorImageFileName = colorFileName;
            pFace->m_depthImageFileName = depthFileName;
        }
    }
}

void FaceManager::addFace( const std::string& rName, Face  *pFace, bool bOverWrite )
{
    if( bOverWrite || getFaceByName( rName ) == 0 )
    {
        removeFaceByName( rName ); // delete before inserting new one
        m_faces.insert( FacePair( rName, ( pFace ) ) );
    }
}

bool FaceManager::removeFaceByName( const std::string& rName )
{
    std::map<std::string, Face*>::iterator iter = m_faces.find( rName );
    if( iter != m_faces.end() )
    {
        m_faces.erase( rName );
        return true;
    }

    return false;
}

Face* FaceManager::getFaceByName( const std::string& rName )
{
    std::map<std::string, Face*>::iterator iter = m_faces.find( rName );
    if( iter != m_faces.end() )
    {
        return iter->second;
    }

    return 0;
}

bool compareFacesByYRotation( const Face* face1, const Face* face2 )
{
    // compare Y rotation
    return face1->getPose().rotation[1] < face2->getPose().rotation[1];
}

const Ogre::StringVector FaceManager::getFaceNames()
{
    std::vector< Face *> faces;

    // put all faces in a vector
    {
        std::map<std::string, Face*>::iterator iter = m_faces.begin();
        while( iter != m_faces.end() )
        {
            faces.push_back( iter->second );
            ++ iter;
        }
    }

    // sort faces by Y rotation
    std::sort( faces.begin(), faces.end(), compareFacesByYRotation );

    Ogre::StringVector faceNames;
    // get sorted names
    {
        std::vector<Face*>::iterator iter = faces.begin();
        while( iter != faces.end() )
        {
            faceNames.push_back( (*iter)->getCurrentPoseName() ); 
            ++ iter;
        }
    }


    return faceNames;
}

void FaceManager::loadImageFromFile( Ogre::String &fileName, Ogre::Image &rImage )
{
    std::ifstream fileStream;
    fileStream.open( fileName.c_str(), std::ios::binary );
    Ogre::DataStreamPtr dataStreamPtr( new Ogre::FileStreamDataStream( &fileStream, false ) );
    rImage.load( dataStreamPtr ); //, "Renderings" );
    fileStream.close();
}

