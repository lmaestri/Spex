/*
-----------------------------------------------------------------------------
-----------------------------------------------------------------------------
*/
#include "OgreRender.h"
#include <FaceTrackLib.h>
#include "TrayGroup.h"
#include "SdkTrays.h"
#include "Face.h"
#include "TrayGroupManager.h"
#include "Spectacles.h"
#include "OISMouse.h"

const std::string   OgreRender::KINECT_COLOR_TEXTURE_NAME = "KinectColorTexture",
                    OgreRender::KINECT_DEPTH_TEXTURE_NAME = "KinectNameTexture";
//KINECT_DEPTH_TEXTURE_NAME = "KinectNameTexture";
 
//-------------------------------------------------------------------------------------
OgreRender::OgreRender(void)
    : mRoot(0),
    mCamera(0),
    mSceneMgr(0),
    mWindow(0),
    mResourcesCfg(Ogre::StringUtil::BLANK),
    mPluginsCfg(Ogre::StringUtil::BLANK),
    mTrayMgr(0),
    mCameraMan(0),
    mCursorWasVisible(false),
    mShutDown(false),
    mInputManager(0),
    mMouse(0),
    mKeyboard(0),
    m_FaceModelMeshName( "FaceModelMesh" ),
    m_faceModelDisplayType( Ogre::RenderOperation::OT_LINE_LIST ),
    m_numKinectSkeletons( 0 ),
    m_ppKinectSkeletonHeadNodes( 0 ),
    m_ppKinectSkeletonNeckNodes( 0 ),
    m_pFacePlaceHolderMesh( 0 ),
    m_isMousePressed( false ),
    m_bShowFaceModel( true ),
    m_isWriteObjKeyPressed( false ),
    m_bInCaptureMode( true ),
    m_pTrayGroupManager( 0 ),
    m_bUpdateTrayGroupManager( true ),
    m_isMouseDragged( false ),
    m_mouseDraggedViewSelectId( 0 ),
	m_guiLoaded(false),
	m_bShowEyeTracking(false),
	left_eye_selected(false),
	right_eye_selected(false)
{
}
//-------------------------------------------------------------------------------------
OgreRender::~OgreRender(void)
{
    destroyFaceModelMesh();

    if( m_pTrayGroupManager ) delete m_pTrayGroupManager;
    if (mTrayMgr) delete mTrayMgr;
    if (mCameraMan) delete mCameraMan;

    // free Material-Pointer! causes exceptions otherwise
    m_colorTexturePtr.setNull();
    m_depthTexturePtr.setNull();
 
    //Remove ourself as a Window listener
    Ogre::WindowEventUtilities::removeWindowEventListener(mWindow, this);
    windowClosed(mWindow);
    delete mRoot;
}

//--------------------------------------------------------------------------------
bool OgreRender::init( Ogre::FrameListener *pFrameListener, OgreBites::SdkTrayListener *pSdkTrayListener, OIS::KeyListener *pKeyListener,   
    const Ogre::Vector2& rColorInput, const Ogre::Vector2& rDepthInput, const float fovY )
{
#ifdef _DEBUG
    mResourcesCfg = "resources_d.cfg";
    mPluginsCfg = "plugins_d.cfg";
#else
    mResourcesCfg = "resources.cfg";
    mPluginsCfg = "plugins.cfg";
#endif

    // construct Ogre::Root
    mRoot = new Ogre::Root(mPluginsCfg);

    //-------------------------------------------------------------------------------------
    // setup resources
    // Load resource paths from config file
    Ogre::ConfigFile cf;
    cf.load(mResourcesCfg);

    // Go through all sections & settings in the file
    Ogre::ConfigFile::SectionIterator seci = cf.getSectionIterator();

    Ogre::String secName, typeName, archName;
    while (seci.hasMoreElements())
    {
        secName = seci.peekNextKey();
        Ogre::ConfigFile::SettingsMultiMap *settings = seci.getNext();
        Ogre::ConfigFile::SettingsMultiMap::iterator i;
        for (i = settings->begin(); i != settings->end(); ++i)
        {
            typeName = i->first;
            archName = i->second;
            Ogre::ResourceGroupManager::getSingleton().addResourceLocation(
                archName, typeName, secName);
        }
    }
    //-------------------------------------------------------------------------------------
    // configure
    // Show the configuration dialog and initialise the system
    // You can skip this and use root.restoreConfig() to load configuration
    // settings if you were sure there are valid ones saved in ogre.cfg
    if(mRoot->restoreConfig() || mRoot->showConfigDialog())
    {
        // If returned true, user clicked OK so initialise
        // Here we choose to let the system create a default rendering window by passing 'true'
        mWindow = mRoot->initialise(true, "Glasses");
    }
    else
    {
        return false;
    }
    //-------------------------------------------------------------------------------------
    // choose scenemanager
    // Get the SceneManager, in this case a generic one
    mSceneMgr = mRoot->createSceneManager(Ogre::ST_GENERIC);
    //-------------------------------------------------------------------------------------
    // create camera
    // Create the camera
    mCamera = mSceneMgr->createCamera("PlayerCam");

    // Position it at 500 in Z direction
    mCamera->setPosition(Ogre::Vector3(0,0,0));
    // Look back along -Z
    mCamera->setDirection( 0, 0, -1 );
    mCamera->setNearClipDistance( 0.001f );
    mCamera->setFOVy( Ogre::Radian( Ogre::Math::DegreesToRadians( fovY ) ) );

    mCameraMan = new OgreBites::SdkCameraMan( mCamera );   // create a default camera controller
    mCameraMan->setStyle( OgreBites::CS_MANUAL ); // <-- now camera doesn't move with mouse
    //-------------------------------------------------------------------------------------
    // create viewports
    // Create one viewport, entire window
    Ogre::Viewport* vp = mWindow->addViewport(mCamera);
    vp->setBackgroundColour(Ogre::ColourValue(0,0,0));

    // Alter the camera aspect ratio to match the viewport
    mCamera->setAspectRatio(
        Ogre::Real(vp->getActualWidth()) / Ogre::Real(vp->getActualHeight()));
    //-------------------------------------------------------------------------------------
    // Set default mipmap level (NB some APIs ignore this)
    Ogre::TextureManager::getSingleton().setDefaultNumMipmaps(5);
    //-------------------------------------------------------------------------------------
    // Create any resource listeners (for loading screens)
    //createResourceListener();
    //-------------------------------------------------------------------------------------
    // load resources
    Ogre::ResourceGroupManager::getSingleton().initialiseAllResourceGroups();
    //-------------------------------------------------------------------------------------
    // Create the scene

    // generate node which will hold the spectacle-meshes:
    m_pFacesNode = mSceneMgr->getRootSceneNode()->createChildSceneNode();
    m_pSpectaclesNode = m_pFacesNode->createChildSceneNode();
    m_pSpectaclesPivotNode = m_pSpectaclesNode->createChildSceneNode();


    // create a Mesh representing a sphere!
    createSphereMesh( "mySphereMesh", 0.01f );
    // Set ambient light
    mSceneMgr->setAmbientLight(Ogre::ColourValue(0.5, 0.5, 0.5));

    // Create a light
    Ogre::Light* l = mSceneMgr->createLight("MainLight");
    l->setPosition(0,0,0);
    l->setType( Ogre::Light::LT_POINT );


    //-------------------------------------------------------------------------------------
    // create stuff for KINECT buffers
    createKinectTextures( KINECT_COLOR_TEXTURE_NAME, KINECT_DEPTH_TEXTURE_NAME, rColorInput, rDepthInput );
    generateFaceModelMesh();
    //-------------------------------------------------------------------------------------
    //create FrameListener
    Ogre::LogManager::getSingletonPtr()->logMessage("*** Initializing OIS ***");
    OIS::ParamList pl;
    size_t windowHnd = 0;
    std::ostringstream windowHndStr;

    mWindow->getCustomAttribute("WINDOW", &windowHnd);
    windowHndStr << windowHnd;
    pl.insert(std::make_pair(std::string("WINDOW"), windowHndStr.str()));

    mInputManager = OIS::InputManager::createInputSystem( pl );

    mKeyboard = static_cast<OIS::Keyboard*>(mInputManager->createInputObject( OIS::OISKeyboard, true ));
    mMouse = static_cast<OIS::Mouse*>(mInputManager->createInputObject( OIS::OISMouse, true ));

    mMouse->setEventCallback(this);
    mKeyboard->setEventCallback(this);
    m_pAdditionalKeyListener = pKeyListener;

    //Set initial mouse clipping size
    windowResized(mWindow);

    //Register as a Window listener
    Ogre::WindowEventUtilities::addWindowEventListener(mWindow, this);

    

	// mTrayMgr->setListener( pSdkTrayListener );
    m_pAdditionalSdkTrayListener = pSdkTrayListener;

	initGUI();

    mRoot->addFrameListener( this );
    mRoot->addFrameListener( pFrameListener );

    return true;
}

//--------------------------------------------------------------------------------
void OgreRender::createKinectTextures(const std::string& colorTextureName, 
    const std::string& depthTextureName, const Ogre::Vector2& rColorInput, const Ogre::Vector2& rDepthInput )
{
    m_colorTexturePtr = Ogre::TextureManager::getSingleton().createManual(
        colorTextureName, 
        Ogre::ResourceGroupManager::DEFAULT_RESOURCE_GROUP_NAME, 
        Ogre::TEX_TYPE_2D, 
        rColorInput.x, 
        rColorInput.y,
        1,
        0,
        // old: Ogre::PF_R8G8B8, 
        Ogre::PF_X8R8G8B8,
        Ogre::TU_DYNAMIC_WRITE_ONLY );

    m_depthTexturePtr = Ogre::TextureManager::getSingleton().createManual(
        depthTextureName, 
        Ogre::ResourceGroupManager::DEFAULT_RESOURCE_GROUP_NAME, 
        Ogre::TEX_TYPE_2D, 
        rDepthInput.x, 
        rDepthInput.y, 
        0,
        Ogre::PF_L16, 
        Ogre::TU_DYNAMIC_WRITE_ONLY_DISCARDABLE );

    //Create Color Overlay
    { /*
        //Create Overlay
        Ogre::OverlayManager& overlayManager = Ogre::OverlayManager::getSingleton();
        Ogre::Overlay* overlay = overlayManager.create("KinectColorOverlay");

        //Create Material
        const std::string materialName = "KinectColorMaterial";
        Ogre::MaterialPtr material = Ogre::MaterialManager::getSingleton().create(materialName, Ogre::ResourceGroupManager::DEFAULT_RESOURCE_GROUP_NAME);
        material->getTechnique(0)->getPass(0)->setLightingEnabled(false);
        material->getTechnique(0)->getPass(0)->setDepthWriteEnabled(false);
        material->getTechnique(0)->getPass(0)->createTextureUnitState(colorTextureName);

        //Create Panel
        Ogre::PanelOverlayElement* panel = static_cast<Ogre::PanelOverlayElement*>(overlayManager.createOverlayElement("Panel", "KinectColorPanel"));
        panel->setMetricsMode(Ogre::GMM_PIXELS);
        panel->setMaterialName(materialName);
        panel->setDimensions((float)rColorInput.x, (float)rColorInput.y);
        panel->setPosition(0.0f, 0.0f);
        overlay->add2D(panel);		
        overlay->setZOrder(300);
        overlay->show(); // */
    }
    //Create Color Overlay
    {
        //Create Material
        const std::string materialName = "KinectColorMaterial";
        Ogre::MaterialPtr material = Ogre::MaterialManager::getSingleton().create(materialName, Ogre::ResourceGroupManager::DEFAULT_RESOURCE_GROUP_NAME);
        material->getTechnique(0)->getPass(0)->setLightingEnabled( false );
        material->getTechnique(0)->getPass(0)->setDepthWriteEnabled( false );
        material->getTechnique(0)->getPass(0)->setDepthCheckEnabled( false );
        material->getTechnique(0)->getPass(0)->createTextureUnitState(colorTextureName);

        // Create background rectangle covering the whole screen
        m_pColorBackgroundRectangle = new Ogre::Rectangle2D(true);
        m_pColorBackgroundRectangle->setCorners(-1.0, 1.0, 1.0, -1.0);
        m_pColorBackgroundRectangle->setMaterial( materialName );

        // Render the background before everything else
        m_pColorBackgroundRectangle->setRenderQueueGroup(Ogre::RENDER_QUEUE_BACKGROUND);

        // Use infinite AAB to always stay visible
        Ogre::AxisAlignedBox aabInf;
        aabInf.setInfinite();
        m_pColorBackgroundRectangle->setBoundingBox(aabInf);

        // Attach background to the scene
        m_pColorBackgroundNode = mSceneMgr->getRootSceneNode()->createChildSceneNode("ColorBackground");
        m_pColorBackgroundNode->attachObject( m_pColorBackgroundRectangle );
        m_pColorBackgroundNode->setVisible( true );
    }

    //Create Depth Overlay 
    {  
        //Create Material
        const std::string materialName = "KinectDepthMaterial";
        Ogre::MaterialPtr material = Ogre::MaterialManager::getSingleton().create(materialName, Ogre::ResourceGroupManager::DEFAULT_RESOURCE_GROUP_NAME);
        material->getTechnique(0)->getPass(0)->setLightingEnabled(false);
        material->getTechnique(0)->getPass(0)->setDepthWriteEnabled(false);
       // material->getTechnique(0)->getPass(0)->setAlphaRejectSettings(Ogre::CMPF_GREATER, 127);

        material->getTechnique(0)->getPass(0)->createTextureUnitState(depthTextureName);
        //material->getTechnique(0)->getPass(0)->setVertexProgram("Ogre/Compositor/StdQuad_vp");
        //material->getTechnique(0)->getPass(0)->setFragmentProgram("KinectDepth");

        // Create background rectangle covering the whole screen
        m_pDepthBackgroundRectangle = new Ogre::Rectangle2D(true);
        m_pDepthBackgroundRectangle->setCorners(-1.0, 1.0, 1.0, -1.0);
        m_pDepthBackgroundRectangle->setMaterial( materialName );

        // Render the background before everything else
        m_pDepthBackgroundRectangle->setRenderQueueGroup(Ogre::RENDER_QUEUE_BACKGROUND);

        // Use infinite AAB to always stay visible
        Ogre::AxisAlignedBox aabInf;
        aabInf.setInfinite();
        m_pDepthBackgroundRectangle->setBoundingBox(aabInf);

        // Attach background to the scene
        m_pDepthBackgroundNode = mSceneMgr->getRootSceneNode()->createChildSceneNode("DepthBackground");
        m_pDepthBackgroundNode->attachObject( m_pDepthBackgroundRectangle );
        m_pDepthBackgroundNode->setVisible( false );

    }// */
    
    //Create quad which writes depth information 
    {  
        //Create Material
        Ogre::MaterialPtr material = Ogre::MaterialManager::getSingleton().getByName( "DepthTexMaterial" );
       // material->getTechnique(0)->getPass(0)->setAlphaRejectSettings(Ogre::CMPF_GREATER, 127);

        if( !material.isNull() )
        {
            material->getTechnique(0)->getPass(0)->getTextureUnitState( 0 )->setTextureName( m_colorTexturePtr->getName() );
            material->getTechnique(0)->getPass(0)->getTextureUnitState( 1 )->setTextureName( m_depthTexturePtr->getName() );
            //material->getTechnique(0)->getPass(0)->setVertexProgram("Ogre/Compositor/StdQuad_vp");
            //material->getTechnique(0)->getPass(0)->setFragmentProgram("KinectDepth");

            // Create background rectangle covering the whole screen
            m_pDepthTexRectangle = new Ogre::Rectangle2D(true);
            m_pDepthTexRectangle->setCorners(-1.0, 1.0, 1.0, -1.0);
            m_pDepthTexRectangle->setMaterial( material->getName() );

            // Render the background before everything else
            m_pDepthTexRectangle->setRenderQueueGroup(Ogre::RENDER_QUEUE_BACKGROUND);

            // Use infinite AAB to always stay visible
            Ogre::AxisAlignedBox aabInf;
            aabInf.setInfinite();
            m_pDepthTexRectangle->setBoundingBox(aabInf);
            // Attach background to the scene
            m_pDepthTexNode = mSceneMgr->getRootSceneNode()->createChildSceneNode( "DepthTexNode" );
            m_pDepthTexNode->attachObject( m_pDepthTexRectangle );
            m_pDepthTexNode->setVisible( false );
        }

    }// */
}


void OgreRender::updatePDLabel(float pupillaryDistance, float zdistance){
	Ogre::DisplayString caption = "";
	std::ostringstream ss;
	ss << pupillaryDistance;
	caption.append(ss.str());
	caption.append(" mm");
	m_pPDLabel->setCaption(caption);
}


//--------------------------------------------------------------------------------
void OgreRender::setFaceTrackerDetails( IFTResult * pResult, bool isConverged )
{
    if( pResult && SUCCEEDED( pResult->GetStatus() ) )
    {
        //m_pFTLabel->show();
        Ogre::DisplayString caption = "Face detected"; 
        caption.append( ( isConverged? " (+)" : "" ) );
        m_pFTLabel->setCaption( caption );

        float scale;
        float rotation[3], translation[3];

        pResult->Get3DPose( &scale, rotation, translation );

        // scale 0
        m_pFTDetailsPanel->setParamValue( 0, Ogre::StringConverter::toString( scale, 2 ) );
        // rotation 1
        m_pFTDetailsPanel->setParamValue( 1, Ogre::StringConverter::toString( rotation[0], 2 ) + ", " +  
            Ogre::StringConverter::toString( rotation[1], 2 ) + ", " +
            Ogre::StringConverter::toString( rotation[2], 2 ) );
        // translation 2
        m_pFTDetailsPanel->setParamValue( 2, Ogre::StringConverter::toString( translation[0], 2 ) + ", " +  
            Ogre::StringConverter::toString( translation[1], 2 ) + ", " +
            Ogre::StringConverter::toString( translation[2], 2 ) );

    }
    else if( pResult )
    {
        // Error handling
        HRESULT r = pResult->GetStatus();
        switch( pResult->GetStatus() )
        {
        case FT_ERROR_FACE_DETECTOR_FAILED:
                m_pFTLabel->setCaption( "Face detector failed!" );
            break;
        case FT_ERROR_AAM_FAILED:
                m_pFTLabel->setCaption( "failed to track face parts" );
            break;
        case FT_ERROR_NN_FAILED: 
                m_pFTLabel->setCaption( "Neural network failed" );
            break;
        case FT_ERROR_EVAL_FAILED:
                m_pFTLabel->setCaption( "quality too poor" );
            break;
        default:
                // -1884553204
                // error number 
                // 8FAC000C
                m_pFTLabel->setCaption( Ogre::StringConverter::toString(r) );
            break;
            /*
        case        FT_FACILITY                                 :
        case       FT_ERROR_INVALID_MODELS                 :
        case       FT_ERROR_INVALID_INPUT_IMAGE            :
        case       FT_ERROR_UNINITIALIZED                  :
        case       FT_ERROR_INVALID_MODEL_PATH             :
        case       FT_ERROR_INVALID_CAMERA_CONFIG          :
            m_pFTLabel->setCaption( Ogre::StringConverter::toString(r) );
            break;*/
        case       FT_ERROR_INVALID_3DHINT                 :
            m_pFTLabel->setCaption( "invalid 3D hint" );
            break;
        case       FT_ERROR_HEAD_SEARCH_FAILED             :
            m_pFTLabel->setCaption( "head search failed" );
            break;
        case       FT_ERROR_USER_LOST                      :
            m_pFTLabel->setCaption( "user lost" );
            break;
        case       FT_ERROR_KINECT_DLL_FAILED              :
            m_pFTLabel->setCaption( "Kinect-DLL error!" );
            break;
        case       FT_ERROR_KINECT_NOT_CONNECTED           :
            m_pFTLabel->setCaption( "Kinect not connected" );
            break;
        }
    }
}

 
//--------------------------------------------------------------------------------
bool OgreRender::go(void)
{
    mRoot->startRendering();
 
    return true;
}
 


//--------------------------------------------------------------------------------
// generating the mesh for visualization
// howto: http://www.ogre3d.org/tikiwiki/tiki-index.php?page=ManualObject#How_to_create_3D_objects_by_code_
void OgreRender::generateFaceModelMesh(  )
{
    // only create it here don't add any data
    m_pFaceModelMesh = mSceneMgr->createManualObject( m_FaceModelMeshName );


    m_pFaceModelNode = mSceneMgr->getRootSceneNode()->createChildSceneNode();
    m_pFaceModelNode->attachObject( m_pFaceModelMesh );
    // m_pFaceModelNode->scale( mWindow->getWidth() / m_colorTexturePtr->getWidth(),
    //   mWindow->getHeight() / m_colorTexturePtr->getHeight(), 1 ); // do scaling to convert from different texture resolutions
    //m_pFaceModelNode->scale( 10, 10, 10 );
}

//--------------------------------------------------------------------------------
void OgreRender::destroyFaceModelMesh( )
{
    // detacht
    m_pFaceModelNode->detachObject( m_pFaceModelMesh );
    // and destroy
    mSceneMgr->destroyManualObject( m_FaceModelMeshName );
}

//--------------------------------------------------------------------------------
void OgreRender::updateFaceModelMesh( const FT_TRIANGLE * pTriangles, const unsigned int numTriangles,
    const FT_VECTOR3D * pVertices, const unsigned int numVertices )
{
    m_pFaceModelMesh->clear();

    m_pFaceModelMesh->begin( "FaceModel/Wires", m_faceModelDisplayType );


    // vertices
    for( UINT i = 0; i < numVertices; ++ i )
    {
        // ATTENTION: we invert the z-component here!
        m_pFaceModelMesh->position( pVertices[i].x, pVertices[i].y, -pVertices[i].z );
    }

    // indices
    for( UINT i = 0; i < numTriangles; ++ i )
    {
        // keep kji order, so meshes will be shown (backface culling)
        m_pFaceModelMesh->index( pTriangles[i].k );
        m_pFaceModelMesh->index( pTriangles[i].j );
        m_pFaceModelMesh->index( pTriangles[i].i );
        if( m_faceModelDisplayType == Ogre::RenderOperation::OT_LINE_LIST )
        {
            m_pFaceModelMesh->index( pTriangles[i].k );
        }
    }
    

    m_pFaceModelMesh->end();

    // GUI stuff ..
    if( !m_bShowFaceModel )
    {
        m_pShowFaceModelDisplayTypeSelectMenu->selectItem( "none" );
    }
    else
    {
        switch( m_faceModelDisplayType )
        {
        case Ogre::RenderOperation::OT_LINE_LIST:
            m_pShowFaceModelDisplayTypeSelectMenu->selectItem( "wires" );
            break;
        case Ogre::RenderOperation::OT_TRIANGLE_LIST:
            m_pShowFaceModelDisplayTypeSelectMenu->selectItem( "triangles" );
            break;
        case Ogre::RenderOperation::OT_POINT_LIST:
            m_pShowFaceModelDisplayTypeSelectMenu->selectItem( "points" );
            break;
        }
    }
}

//--------------------------------------------------------------------------------
void OgreRender::updateSkeletonPoints( const bool * pIsTracked, const FT_VECTOR3D * pHeadPoints,
    const FT_VECTOR3D* pNeckPoints, const unsigned int numSkeletons )
{
    // lazy initialization
    if( 0 == m_ppKinectSkeletonHeadNodes )
    {
        m_ppKinectSkeletonHeadNodes = new Ogre::SceneNode * [ numSkeletons ];
        m_ppKinectSkeletonNeckNodes = new Ogre::SceneNode * [ numSkeletons ];
        m_numKinectSkeletons = numSkeletons;


        for( unsigned int i = 0; i < numSkeletons; ++ i )
        {
            {
                Ogre::Entity* sphereEntity = mSceneMgr->createEntity( "mySphereMesh" );
                m_ppKinectSkeletonHeadNodes[i] = mSceneMgr->getRootSceneNode()->createChildSceneNode();
                m_ppKinectSkeletonHeadNodes[i]->attachObject(sphereEntity);
                sphereEntity->setMaterialName("Skeleton/Node"); 
            }
            {
                Ogre::Entity* sphereEntity = mSceneMgr->createEntity( "mySphereMesh" );
                m_ppKinectSkeletonNeckNodes[i] = mSceneMgr->getRootSceneNode()->createChildSceneNode();
                m_ppKinectSkeletonNeckNodes[i]->attachObject(sphereEntity);
                sphereEntity->setMaterialName("Skeleton/Node"); 
            }
        }
    }

    // update positions
    for( unsigned int i = 0; i < numSkeletons; ++ i )
    {
        // check if this skeleton is actually tracked!
        if( pIsTracked[i] )
        { // update position
            m_ppKinectSkeletonHeadNodes[i]->setPosition( pHeadPoints[i].x, pHeadPoints[i].y, - pHeadPoints[i].z );
            m_ppKinectSkeletonHeadNodes[i]->setVisible( m_pShowSkeletonNodesCheckbox->isChecked() );
            m_ppKinectSkeletonNeckNodes[i]->setPosition( pNeckPoints[i].x, pNeckPoints[i].y, - pNeckPoints[i].z );
            m_ppKinectSkeletonNeckNodes[i]->setVisible( m_pShowSkeletonNodesCheckbox->isChecked() );
        }
        else
        { // hide the mesh
            m_ppKinectSkeletonHeadNodes[i]->setVisible( false );
            m_ppKinectSkeletonNeckNodes[i]->setVisible( false );
        }

    }
}


//--------------------------------------------------------------------------------
void OgreRender::updateGlassesPose( )
{
    m_pFacesNode->resetToInitialState();
    if( m_pDisplayFace )
    {
        // update node containing the spectacles-node and facemesh
        m_pFacesNode->setPosition( m_pDisplayFace->getPose().location[0], m_pDisplayFace->getPose().location[1], - m_pDisplayFace->getPose().location[2] );
        m_pFacesNode->scale( m_pDisplayFace->getPose().scaling, m_pDisplayFace->getPose().scaling, m_pDisplayFace->getPose().scaling );
        m_pFacesNode->pitch( Ogre::Radian( Ogre::Math::DegreesToRadians( - m_pDisplayFace->getPose().rotation[0] ) ) ); // X <-- works!
        m_pFacesNode->yaw( Ogre::Radian( Ogre::Math::DegreesToRadians( - m_pDisplayFace->getPose().rotation[1] ) ) );  // Y <-- works!
        m_pFacesNode->roll( Ogre::Radian( Ogre::Math::DegreesToRadians( m_pDisplayFace->getPose().rotation[2] ) ) ); // Z

        // update the child node of m_pFacesNode which only contains the spectacles
		m_pSpectaclesNode->resetToInitialState();
        m_pSpectaclesNode->translate( m_spectaclesPivotLocation );
        m_pSpectaclesNode->scale( Ogre::Vector3( m_pDisplayFace->getSpectaclesPose().scaling ) );
        m_pSpectaclesNode->pitch( Ogre::Radian( Ogre::Math::DegreesToRadians( m_pDisplayFace->getSpectaclesPose().rotation[0] ) ) ); // X <-- works!
        m_pSpectaclesNode->yaw( Ogre::Radian( Ogre::Math::DegreesToRadians( m_pDisplayFace->getSpectaclesPose().rotation[1] ) ) );  // Y <-- works!
        m_pSpectaclesNode->roll( Ogre::Radian( Ogre::Math::DegreesToRadians( m_pDisplayFace->getSpectaclesPose().rotation[2] ) ) ); // Z
        m_pSpectaclesNode->translate( m_pDisplayFace->getSpectaclesPose().location[0], m_pDisplayFace->getSpectaclesPose().location[1], m_pDisplayFace->getSpectaclesPose().location[2] );
		
        m_pSpectaclesPivotNode->resetToInitialState();
        m_pSpectaclesPivotNode->translate( - m_spectaclesPivotLocation );

		//Update Eyetracking
		setLeftEyePosition(m_pDisplayFace->getLeftEye2DLocation().x,m_pDisplayFace->getLeftEye2DLocation().y);
		setRightEyePosition(m_pDisplayFace->getRightEye2DLocation().x,m_pDisplayFace->getRightEye2DLocation().y);
		
		//Update 3D coords and calculate new distance
		
		if(left_eye_selected){
			Ogre::Vector4 worldPos(m_pDisplayFace->getLeftEye3DLocation().x, m_pDisplayFace->getLeftEye3DLocation().y, m_pDisplayFace->getLeftEye3DLocation().z, 1.0f);
			Ogre::Vector4 screenPosW = mCamera->getProjectionMatrix() * mCamera->getViewMatrix() * worldPos;
			screenPosW.x = screenPosW.x / screenPosW.w;
			screenPosW.y = screenPosW.y / screenPosW.w;
			screenPosW.z = screenPosW.z / screenPosW.w;
			Ogre::Vector2 screenPos((-1.0f*screenPosW.x)*0.5f + 0.5f, 1 - (-1.0f*screenPosW.y*0.5f + 0.5f));
			m_pDisplayFace->setLeftEye2D(screenPos.x, screenPos.y);
			setZoomPanelPosition(screenPos.x, screenPos.y);
			zoom_panel->setUV(screenPos.x - 0.07, screenPos.y - 0.07, screenPos.x + 0.07, screenPos.y + 0.07);
			updatePDLabel(m_pDisplayFace->getPupillaryDistance(),0.0f);
		}
		if(right_eye_selected){
			Ogre::Vector4 worldPos(m_pDisplayFace->getRightEye3DLocation().x, m_pDisplayFace->getRightEye3DLocation().y, m_pDisplayFace->getRightEye3DLocation().z, 1.0f);
			Ogre::Vector4 screenPosW = mCamera->getProjectionMatrix() * mCamera->getViewMatrix() * worldPos;
			screenPosW.x = screenPosW.x / screenPosW.w;
			screenPosW.y = screenPosW.y / screenPosW.w;
			screenPosW.z = screenPosW.z / screenPosW.w;
			Ogre::Vector2 screenPos((-1.0f*screenPosW.x)*0.5f + 0.5f, 1 - (-1.0f*screenPosW.y*0.5f + 0.5f));
			m_pDisplayFace->setRightEye2D(screenPos.x, screenPos.y);
			setZoomPanelPosition(screenPos.x, screenPos.y);
			zoom_panel->setUV(screenPos.x - 0.07, screenPos.y - 0.07, screenPos.x + 0.07, screenPos.y + 0.07);
			updatePDLabel(m_pDisplayFace->getPupillaryDistance(),0.0f);
		}
		
        // update gui
        m_pUserScaleSlider->setValue( m_pDisplayFace->getSpectaclesPose().scaling, false ); 
        m_pUserRotationXSlider->setValue( m_pDisplayFace->getSpectaclesPose().rotation[0], false );
        m_pUserRotationYSlider->setValue( m_pDisplayFace->getSpectaclesPose().rotation[1], false );
        m_pUserRotationZSlider->setValue( m_pDisplayFace->getSpectaclesPose().rotation[2], false );
        m_pUserLocationXSlider->setValue( m_pDisplayFace->getSpectaclesPose().location[0], false );
        m_pUserLocationYSlider->setValue( m_pDisplayFace->getSpectaclesPose().location[1], false );
        m_pUserLocationZSlider->setValue( m_pDisplayFace->getSpectaclesPose().location[2], false );
    }
}

//--------------------------------------------------------------------------------
// called from frameRenderingQueed
void OgreRender::update( )
{
    updateGlassesPose();
    if( m_bUpdateTrayGroupManager )
    {
        m_pTrayGroupManager->update();
        m_bUpdateTrayGroupManager = false;
    }


    if( m_pDisplayFace && m_pDisplayFace->areTexturesChanged() ) // <-- bug if changing the static images!
    {
        // copy images which contain the color and depth images from Kinect into Ogre::Textures
        m_colorTexturePtr->getBuffer()->blitFromMemory( m_pDisplayFace->getColorImage().getPixelBox() );
        if( m_pDisplayFace->hasDepthImage() )
        {
            m_depthTexturePtr->getBuffer()->blitFromMemory( m_pDisplayFace->getDepthImage().getPixelBox() );
        }
        m_pDisplayFace->setAreTexturesChanged( false );
    }

    // automatically set which face position to capture, if we are in capture mode
    if( m_bInCaptureMode )
    {
        updateCaptureFacePosition( );
    }
}
//void OgreRender::updateKinectTextures( IFTImage * pSrcColorImage, IFTImage * pSrcDepthImage )
//{

//    if( pSrcColorImage )
//    {
//        //FTIMAGEFORMAT format = pSrcColorImage->GetFormat();
//        // if the FTIMAGEFORMAT is FTIMAGEFORMAT_UINT8_B8G8R8X8 then use Ogre::PF_X8R8G8B8!
//        Ogre::PixelBox srcColorPix = Ogre::PixelBox( pSrcColorImage->GetWidth(), pSrcColorImage->GetHeight(), 1, Ogre::PF_X8R8G8B8, pSrcColorImage->GetBuffer() );
//        m_colorTexturePtr->getBuffer()->blitFromMemory( srcColorPix );
//    }

//    if( pSrcDepthImage )
//    {
//        //FTIMAGEFORMAT format = pSrcDepthImage->GetFormat();
//        Ogre::PixelBox srcDepthPix = Ogre::PixelBox( pSrcDepthImage->GetWidth(), pSrcDepthImage->GetHeight(), 1, Ogre::PF_L16, pSrcDepthImage->GetBuffer() );
//        m_depthTexturePtr->getBuffer()->blitFromMemory( srcDepthPix );
//    }

//}
//--------------------------------------------------------------------------------
void OgreRender::initGUI() 
{

    // Trays for showing Info, UI, etc ...
    mTrayMgr = new OgreBites::SdkTrayManager("InterfaceName", mWindow, mMouse, this);
    mTrayMgr->showFrameStats(OgreBites::TL_TOP);
    mTrayMgr->toggleAdvancedFrameStats(); // hide advanced frame stats
    mTrayMgr->setListener( this );

	//Eyeoverlay
	Ogre::OverlayManager& overlayManager = Ogre::OverlayManager::getSingleton();
    // Create an overlay
    overlay = overlayManager.create( "OverlayName" );
	
	// Create Left Eye
	left_eye = static_cast<Ogre::OverlayContainer*>( overlayManager.createOverlayElement( "Panel", "LeftEye" ) );
	left_eye->setPosition( 0.45f, 0.45f );
	left_eye->setDimensions( 0.07f, 0.07f );
	left_eye->setMaterialName( "Crosshair" );

	zoom_panel = static_cast<Ogre::BorderPanelOverlayElement *> (Ogre::OverlayManager::getSingleton().createOverlayElement("BorderPanel", "Zoom"));
	
	zoom_panel->setParameter("uv_coords","0 0 1 1");
	zoom_panel->hide();
	zoom_panel->setDimensions( 0.3f, 0.3f );
	zoom_panel->setMaterialName( "KinectColorMaterial" );

	// Create Right Eye
	right_eye = static_cast<Ogre::OverlayContainer*>( overlayManager.createOverlayElement( "Panel", "RightEye" ) );
	right_eye->setPosition( 0.45f, 0.45f );
	right_eye->setDimensions( 0.07f, 0.07f );
	right_eye->setMaterialName( "Crosshair" );
	
	// Add the panel to the overlay
	overlay->add2D( zoom_panel );
	overlay->add2D( left_eye );
	overlay->add2D( right_eye );
	

    //Hide the overlay
    overlay->hide();
	//mTrayMgr->showLogo(OgreBites::TL_BOTTOMRIGHT);
    mTrayMgr->showCursor();

	//PupillaryDistance Label
	m_pPDLabel = mTrayMgr->createLabel(OgreBites::TL_TOP,"PupillaryDistance", "");
	
    // TrayGroupManager, managing the TrayGroups
    m_pTrayGroupManager = new TrayGroupManager( mTrayMgr );
    m_pFTStatusTrayGroup = m_pTrayGroupManager->createTrayGroup( OgreBites::TL_TOPRIGHT, "Tracker Status", true );
    m_pSpectaclesTrayGroup = m_pTrayGroupManager->createTrayGroup( OgreBites::TL_TOPLEFT, "Glasses", false );
    m_pViewTrayGroup =       m_pTrayGroupManager->createTrayGroup( OgreBites::TL_BOTTOMLEFT, "Views", false );
    m_pCaptureTrayGroup =    m_pTrayGroupManager->createTrayGroup( OgreBites::TL_BOTTOMLEFT, "Capture", false );
    m_pKinectSettingsTrayGroup = m_pTrayGroupManager->createTrayGroup( OgreBites::TL_TOPRIGHT, "Kinect", true );
    m_pDebugTrayGroup = m_pTrayGroupManager->createTrayGroup( OgreBites::TL_TOPRIGHT, "Debug", true );
    m_pUserControlTrayGroup = m_pTrayGroupManager->createTrayGroup( OgreBites::TL_BOTTOMRIGHT, "Manual Adjustment", true );
    m_pViewSliderTrayGroup = m_pTrayGroupManager->createTrayGroup( OgreBites::TL_BOTTOM, "ViewSlider", false );

    // selection Menu for glasses:
    m_pSpectaclesSelectMenu = mTrayMgr->createThickSelectMenu( OgreBites::TL_TOPLEFT, "SpectaclesSelectMenu", "Glasses", 200, 10, Ogre::StringVector() ); 
    m_pSpectaclesSelectMenu->show();
    m_pSpectaclesTrayGroup->addWidget( m_pSpectaclesSelectMenu );
    m_pSpectaclesTrayGroup->setUnfold( true );

    // different views:
    Ogre::StringVector views; 
    m_pViewsSelectMenu = mTrayMgr->createThickSelectMenu( OgreBites::TL_BOTTOMLEFT, "ViewsSelectMenu", "Views", 200, 10, views );
    m_pDeleteViewButton = mTrayMgr->createButton( OgreBites::TL_BOTTOMLEFT, "DeleteViewButton", "delete!", 200 );

    Ogre::StringVector positions; 
    positions.push_back( "left" );
    positions.push_back( "front" );
    positions.push_back( "right" );
    m_pCaptureSelectMenu = mTrayMgr->createThickSelectMenu( OgreBites::TL_BOTTOMLEFT, "CaptureSelectMenu", "Capture", 200, 10, positions );
    m_pCaptureButton = mTrayMgr->createButton( OgreBites::TL_BOTTOMLEFT, "CaptureButton", "capture!", 200 );
	m_pLastCaptureButton = mTrayMgr->createButton( OgreBites::TL_BOTTOMLEFT, "LastCaptureButton", "Last capture", 200 );

    //TrayGroup
	m_pViewTrayGroup->addWidget( m_pLastCaptureButton );
    m_pViewTrayGroup->addWidget( m_pViewsSelectMenu );
    m_pViewTrayGroup->addWidget( m_pDeleteViewButton );
    m_pViewTrayGroup->setUnfold( false );

    m_pCaptureTrayGroup->addWidget( m_pCaptureSelectMenu );
    m_pCaptureTrayGroup->addWidget( m_pCaptureButton );
    m_pCaptureTrayGroup->setUnfold( true );

    // create a label which is visible when the facetracker finds a face!
    m_pFTLabel = mTrayMgr->createLabel( OgreBites::TL_TOPRIGHT, "FaceTrackerLabel", "face found", 200 );
    m_pFTStatusTrayGroup->addWidget( m_pFTLabel );
    m_pFTLabel->show();

    // details of the FT Model
    Ogre::StringVector ftItems;
    ftItems.push_back( "scale" );
    ftItems.push_back( "rotation" ); 
    ftItems.push_back( "translation" );

    m_pFTDetailsPanel = mTrayMgr->createParamsPanel( OgreBites::TL_TOPRIGHT, "FaceTracker", 200, ftItems );
    m_pFTStatusTrayGroup->addWidget( m_pFTDetailsPanel );
    m_pFTDetailsPanel->show();


    //m_pKinectTiltUpButton = mTrayMgr->createButton( OgreBites::TL_TOPRIGHT, "KinectTiltUp", "^" );
    //m_pKinectTiltDownButton = mTrayMgr->createButton( OgreBites::TL_TOPRIGHT, "KinectTiltDown", "v" );
    m_pKinectTiltSlider = mTrayMgr->createThickSlider( OgreBites::TL_TOPRIGHT, "KinectTiltSlider", "tilt", 200, 70, -27, 27, 27*2+1 );
    m_pKinectSettingsTrayGroup->addWidget( m_pKinectTiltSlider );

    m_pFovySlider = mTrayMgr->createThickSlider( OgreBites::TL_TOPRIGHT, "FovySlider", "FOVy", 200, 100, 30, 90, 90-30 ); 
    m_pFovySlider->setValue(  mCamera->getFOVy().valueDegrees(), false );
    m_pKinectSettingsTrayGroup->addWidget( m_pFovySlider );
    m_pKinectSettingsTrayGroup->setUnfold( false );

    // Display Settings
    m_pShowKinectDepthCheckbox            = mTrayMgr->createCheckBox( OgreBites::TL_TOPRIGHT, "ShowKinectDepthCheckbox", "show depth", 200 );
    m_pShowKinectDepthCheckbox->setChecked( false, false );
    m_pShowGlassesCheckbox                = mTrayMgr->createCheckBox( OgreBites::TL_TOPRIGHT, "ShowGlassesCheckbox", "show glasses", 200 );
    m_pShowGlassesCheckbox->setChecked( true, true );
	m_pShowEyeTrackingCheckbox			  = mTrayMgr->createCheckBox( OgreBites::TL_TOPRIGHT, "ShowEyeTrackingCheckbox", "show eyes", 200 );
	m_pShowEyeTrackingCheckbox->setChecked( false, false );
    m_pShowSkeletonNodesCheckbox          = mTrayMgr->createCheckBox( OgreBites::TL_TOPRIGHT, "ShowSkeletonNodesCheckbox", "show skeletons", 200 );
    m_pShowFacePlaceholderCheckbox = mTrayMgr->createCheckBox( OgreBites::TL_TOPRIGHT, "ShowFacePlaceholderCheckbox", "show faceplaceholder", 200 );
    Ogre::StringVector displayTypeItems;
    displayTypeItems.push_back( "none" );
    displayTypeItems.push_back( "triangles" ); //Ogre::RenderOperation::OperationType::
    displayTypeItems.push_back( "wires" );
    displayTypeItems.push_back( "points" );
    m_pShowFaceModelDisplayTypeSelectMenu = mTrayMgr->createThickSelectMenu( OgreBites::TL_TOPRIGHT, "ShowFaceModelDisplayTypeSelectMenu", "FaceMesh ", 200, 5, displayTypeItems );
    m_pShowFaceModelDisplayTypeSelectMenu->selectItem( "none" );

    m_pDebugTrayGroup->addWidget( m_pShowKinectDepthCheckbox );
    m_pDebugTrayGroup->addWidget( m_pShowGlassesCheckbox );
    m_pDebugTrayGroup->addWidget( m_pShowSkeletonNodesCheckbox );
    m_pDebugTrayGroup->addWidget( m_pShowFacePlaceholderCheckbox );
    m_pDebugTrayGroup->addWidget( m_pShowFaceModelDisplayTypeSelectMenu );
	m_pDebugTrayGroup->addWidget( m_pShowEyeTrackingCheckbox );
    m_pDebugTrayGroup->setUnfold( false );


    m_pUserScaleSlider = mTrayMgr->createThickSlider( OgreBites::TL_BOTTOMRIGHT, "UserScalingSlider", "scale", 200, 50, 0.5, 1.5, (unsigned int)((1.5 - 0.5)/0.01) );
	m_pUserRotationXSlider = mTrayMgr->createThickSlider( OgreBites::TL_BOTTOMRIGHT, "UserRotationXSlider", "rotation X", 200, 50, -45, +45, 90+1 );
	m_pUserRotationYSlider = mTrayMgr->createThickSlider( OgreBites::TL_BOTTOMRIGHT, "UserRotationYSlider", "rotation Y", 200, 50, -45, +45, 90+1 );
	m_pUserRotationZSlider = mTrayMgr->createThickSlider( OgreBites::TL_BOTTOMRIGHT, "UserRotationZSlider", "rotation Z", 200, 50, -45, +45, 90+1 );
	m_pUserLocationXSlider = mTrayMgr->createThickSlider( OgreBites::TL_BOTTOMRIGHT, "UserLocationXSlider", "translation X", 200, 50, -0.5, 0.5, 1001 );
	m_pUserLocationYSlider = mTrayMgr->createThickSlider( OgreBites::TL_BOTTOMRIGHT, "UserLocationYSlider", "translation Y", 200, 50, -0.5, 0.5, 1001 );
	m_pUserLocationZSlider = mTrayMgr->createThickSlider( OgreBites::TL_BOTTOMRIGHT, "UserLocationZSlider", "translation Z", 200, 50, -0.5, 0.5, 1001 );

    m_pUserControlTrayGroup->addWidget( m_pUserScaleSlider );
    m_pUserControlTrayGroup->addWidget( m_pUserRotationXSlider);
    m_pUserControlTrayGroup->addWidget( m_pUserRotationYSlider);
    m_pUserControlTrayGroup->addWidget( m_pUserRotationZSlider);
    m_pUserControlTrayGroup->addWidget( m_pUserLocationXSlider);
    m_pUserControlTrayGroup->addWidget( m_pUserLocationYSlider);
    m_pUserControlTrayGroup->addWidget( m_pUserLocationZSlider);
    m_pUserControlTrayGroup->setUnfold( false );

    m_pViewSlider = mTrayMgr->createLongSlider( OgreBites::TL_BOTTOM, "ViewSlider", "", 240, 60, 0, 10, 10 );
    m_pViewSliderTrayGroup->addWidget( m_pViewSlider );
    m_pViewSliderTrayGroup->setVisible( false );
    m_pViewSliderTrayGroup->setUnfold( true );

	m_guiLoaded = true;
}

//--------------------------------------------------------------------------------
void OgreRender::updateViewableFaces( const Ogre::StringVector& faceNames )
{
    unsigned int idx = m_pViewsSelectMenu->getSelectionIndex();
    m_pViewsSelectMenu->clearItems();
    m_pViewsSelectMenu->setItems( faceNames );
    m_pViewSlider->setRange( 0, faceNames.size()-1, faceNames.size(), false );
    if( idx >= m_pViewsSelectMenu->getNumItems() )
    {
        idx = m_pViewsSelectMenu->getNumItems() - 1;
    }
    if( idx >= 0 && idx < m_pViewsSelectMenu->getNumItems() )
    {
        m_pViewsSelectMenu->selectItem( idx, false );
        m_pViewSlider->setValue( idx, false );
        //m_pViewSlider->setCaption( m_pViewsSelectMenu->getSelectedItem() );
    }
}

//--------------------------------------------------------------------------------
void OgreRender::setDisplayFace( Face* pFace )
{
    m_pDisplayFace = pFace;
    if( m_pDisplayFace )
    {
		m_pDisplayFace->setAreTexturesChanged( true );
		updatePDLabel(m_pDisplayFace->getPupillaryDistance(), 0.0f);
    }
}

//--------------------------------------------------------------------------------
void OgreRender::updateCaptureFacePosition()
{
   std::string positionName = m_pDisplayFace->getCurrentPoseName();

   try{ 
        m_pCaptureSelectMenu->selectItem( positionName );
   } catch ( Ogre::Exception& e ){
	   e.getDescription();
       // if item not in the list we add it!
       m_pCaptureSelectMenu->addItem( positionName );
       m_pCaptureSelectMenu->selectItem( positionName );
   }
}

//--------------------------------------------------------------------------------
void OgreRender::setFacePlaceHolderMesh( std::string meshName )
{
    if( meshName == "" )
    {   //default mesh.
        meshName = "facePlaceHolder.mesh"; 
    }
    if( m_pFacePlaceHolderMesh != 0 )
    {
        m_pFacesNode->detachObject( m_pFacePlaceHolderMesh );
        mSceneMgr->destroyEntity( m_pFacePlaceHolderMesh->getName() );    
    }
    // create new Entity-Mesh
    m_pFacePlaceHolderMesh = mSceneMgr->createEntity( "facePlaceHolder", meshName );
    m_pFacePlaceHolderMesh->setMaterial( Ogre::MaterialManager::getSingleton().getByName( "FaceModel/Transparent" ) );
    m_pFacePlaceHolderMesh->setRenderQueueGroupAndPriority( Ogre::RENDER_QUEUE_BACKGROUND, 1000  );
    m_pSpectaclesPivotNode->attachObject( m_pFacePlaceHolderMesh );
}


void OgreRender::setSpectaclesPivotPoint( const Ogre::Vector3& specPivotPoint )
{
    m_spectaclesPivotLocation = specPivotPoint;
}

void OgreRender::addSpectacles( Spectacles * pSpectacles )
{
    OgreBites::DecorWidget *pDecor = mTrayMgr->createDecorWidget( OgreBites::TL_TOPLEFT, pSpectacles->getName(), "Spectacles/Thumb" );
    Ogre::OverlayElement *pOverlay = pDecor->getOverlayElement();
    Ogre::TexturePtr texture = Ogre::TextureManager::getSingleton().load( pSpectacles->getThumbNail(), "Popular" );
    Ogre::MaterialPtr material = ( Ogre::MaterialManager::getSingleton().create( pSpectacles->getName(), Ogre::ResourceGroupManager::DEFAULT_RESOURCE_GROUP_NAME ) );
    material->getTechnique(0)->getPass(0)->setLightingEnabled( false );
    material->getTechnique(0)->getPass(0)->setDepthWriteEnabled( false );
    material->getTechnique(0)->getPass(0)->setDepthCheckEnabled( false );
    material->getTechnique(0)->getPass(0)->createTextureUnitState( texture->getName() );
    pOverlay->setMaterialName( material->getName() );
    m_pSpectaclesTrayGroup->addWidget( pDecor );

    m_spectaclesThumbs.push_back( pDecor );

}

//-------------------------------------------------------------------------------------
// LISTENER FUNCTIONS
//-------------------------------------------------------------------------------------
// Ogre::FrameListener
bool OgreRender::frameRenderingQueued(const Ogre::FrameEvent& evt)
{
    if(mWindow->isClosed())
        return false;

    if(mShutDown)
        return false;

    //Need to capture/update each device
    mKeyboard->capture();
    mMouse->capture();

    mTrayMgr->frameRenderingQueued(evt);

    if (!mTrayMgr->isDialogVisible())
    {
        mCameraMan->frameRenderingQueued(evt);   // if dialog isn't up, then update the camera
    }

    // call the OgreRender::update method
    update( );

    return true;
}


bool OgreRender::keyPressed( const OIS::KeyEvent &arg )
{
    if (mTrayMgr->isDialogVisible()) return true;   // don't process any more keys if dialog is up
 
    if (arg.key == OIS::KC_F)   // toggle visibility of advanced frame stats
    {
        mTrayMgr->toggleAdvancedFrameStats();
    }
    else if (arg.key == OIS::KC_R)   // cycle polygon rendering mode
    {
        Ogre::String newVal;
        Ogre::PolygonMode pm;
 
        switch (mCamera->getPolygonMode())
        {
        case Ogre::PM_SOLID:
            newVal = "Wireframe";
            pm = Ogre::PM_WIREFRAME;
            break;
        case Ogre::PM_WIREFRAME:
            newVal = "Points";
            pm = Ogre::PM_POINTS;
            break;
        default:
            newVal = "Solid";
            pm = Ogre::PM_SOLID;
        }
 
        mCamera->setPolygonMode(pm);
    }
    else if( arg.key == OIS::KC_O  )
    {
        m_isWriteObjKeyPressed = true;
    }
    else if(arg.key == OIS::KC_F5)   // refresh all textures
    {
        Ogre::TextureManager::getSingleton().reloadAll();
        Ogre::MaterialManager::getSingleton().reloadAll();
    }
    else if (arg.key == OIS::KC_SYSRQ)   // take a screenshot
    {
        mWindow->writeContentsToTimestampedFile("screenshot", ".jpg");

    }
    else if (arg.key == OIS::KC_ESCAPE)
    {
        mShutDown = true;
    }
    // change spectacles with arrow up/down keys
    else if( arg.key == OIS::KC_UP )
    {
        int idx = m_pSpectaclesSelectMenu->getSelectionIndex();
        -- idx;
        if( idx >= 0 )
        {
            m_pSpectaclesSelectMenu->selectItem( idx );
        }
    }
    // change spectacles with arrow up/down keys
    else if( arg.key == OIS::KC_DOWN )
    {
        unsigned int idx = m_pSpectaclesSelectMenu->getSelectionIndex();
        ++ idx;
        if( idx < m_pSpectaclesSelectMenu->getNumItems() )
        {
            m_pSpectaclesSelectMenu->selectItem( idx );
        }
    }
    // change viewing poses with arrow left/right keys
    else if( arg.key == OIS::KC_LEFT )
    {
        unsigned int idx = m_pViewsSelectMenu->getSelectionIndex();
        -- idx;
        if( idx >= 0 )
        {
            m_pViewsSelectMenu->selectItem( idx );
        }
    }
    // change spectacles with arrow up/down keys
    else if( arg.key == OIS::KC_RIGHT )
    {
        unsigned int idx = m_pViewsSelectMenu->getSelectionIndex();
        ++ idx;
        if( idx < m_pViewsSelectMenu->getNumItems() )
        {
            m_pViewsSelectMenu->selectItem( idx );
        }
    }
    else if( arg.key == OIS::KC_SPACE )
    {
        // capture with this key so we forward it!
        m_pAdditionalKeyListener->keyPressed( arg );
    }

    mCameraMan->injectKeyDown(arg);
    return true;
}
 
bool OgreRender::keyReleased( const OIS::KeyEvent &arg )
{
    m_isWriteObjKeyPressed = false;
    mCameraMan->injectKeyUp(arg);
    return true;
}
 
bool OgreRender::mouseMoved( const OIS::MouseEvent &arg )
{
    if (mTrayMgr->injectMouseMove(arg)) return true;
    mCameraMan->injectMouseMove(arg);
	
	
	if( m_isMouseDragged && !m_bInCaptureMode && m_bShowEyeTracking)
    {
		if(left_eye_selected){
			Ogre::Vector2 cursorPos(mTrayMgr->getCursorContainer()->getLeft()/mCamera->getViewport()->getActualWidth(), mTrayMgr->getCursorContainer()->getTop()/mCamera->getViewport()->getActualHeight());
			//cursorPos.x = cursorPos.x - offsetX + 0.035;
			//cursorPos.y = cursorPos.y - offsetY + 0.035;
			Ogre::Ray mouseRay = mCamera->getCameraToViewportRay(cursorPos.x, cursorPos.y);
			Ogre::Vector3 newPos = mouseRay.getPoint(m_pDisplayFace->getLeftEye3DLocation().z);
			m_pDisplayFace->setLeftEye3D(newPos.x, newPos.y, m_pDisplayFace->getLeftEye3DLocation().z);
			//printf("%f | %f | %f \n", newPos.x, newPos.y, m_pDisplayFace->getLeftEye3DLocation().z);
		}
		if(right_eye_selected){
			Ogre::Vector2 cursorPos(mTrayMgr->getCursorContainer()->getLeft()/mCamera->getViewport()->getActualWidth(), mTrayMgr->getCursorContainer()->getTop()/mCamera->getViewport()->getActualHeight());
			//cursorPos.x = cursorPos.x - offsetX + 0.035;
			//cursorPos.y = cursorPos.y - offsetY + 0.035;
			Ogre::Ray mouseRay = mCamera->getCameraToViewportRay(cursorPos.x, cursorPos.y);
			Ogre::Vector3 newPos = mouseRay.getPoint(m_pDisplayFace->getRightEye3DLocation().z);
			m_pDisplayFace->setRightEye3D(newPos.x, newPos.y, m_pDisplayFace->getLeftEye3DLocation().z);
		}
	}
	// set view based on mouse movement
	if( m_isMouseDragged && !m_bInCaptureMode && m_bShowGlasses)
    {
        Ogre::Vector2 movement = calculateMovement( m_mouseDraggedStartState, arg.state, 10 );

        if( arg.state.buttonDown( OIS::MB_Left ) )
        {
            // change translation
            m_pDisplayFace->setSpectaclesPose(
                m_beforeMouseDraggedSpectaclesPose.location[0] + movement.x, 
                m_beforeMouseDraggedSpectaclesPose.location[1] - movement.y, 
                m_beforeMouseDraggedSpectaclesPose.location[2], 
                m_beforeMouseDraggedSpectaclesPose.rotation[0], 
                m_beforeMouseDraggedSpectaclesPose.rotation[1], 
                m_beforeMouseDraggedSpectaclesPose.rotation[2],
                m_beforeMouseDraggedSpectaclesPose.scaling ); 
        }
        if( arg.state.buttonDown( OIS::MB_Right ) )
        {
            // rotation
            m_pDisplayFace->setSpectaclesPose(
                m_beforeMouseDraggedSpectaclesPose.location[0], 
                m_beforeMouseDraggedSpectaclesPose.location[1], 
                m_beforeMouseDraggedSpectaclesPose.location[2], 
                m_beforeMouseDraggedSpectaclesPose.rotation[0] + movement.y * 10, 
                m_beforeMouseDraggedSpectaclesPose.rotation[1] + movement.x * 10, 
                m_beforeMouseDraggedSpectaclesPose.rotation[2],
                m_beforeMouseDraggedSpectaclesPose.scaling ); 
        }
    }

    return true;
}
 
bool OgreRender::mousePressed( const OIS::MouseEvent &arg, OIS::MouseButtonID id )
{
    m_isMousePressed = true;
    mCameraMan->injectMouseDown(arg, id);
	Ogre::Vector2 cursorPos(mTrayMgr->getCursorContainer()->getLeft()/mCamera->getViewport()->getActualWidth(), mTrayMgr->getCursorContainer()->getTop()/mCamera->getViewport()->getActualHeight());
	float l_top = left_eye->getTop();
	float l_left = left_eye->getLeft();
	float r_top = right_eye->getTop();
	float r_left = right_eye->getLeft();

	float width = left_eye->getWidth();
	float height = left_eye->getHeight();

	if(l_top < cursorPos.y && (l_top+height) > cursorPos.y && l_left < cursorPos.x && (l_left+width) > cursorPos.x){
		left_eye_selected = !left_eye_selected;
		if(left_eye_selected){
			right_eye->hide();
			zoom_panel->show();
			left_eye->setDimensions(0.3f, 0.3f);
			//offsetX = cursorPos.x - l_left;
			//offsetY = cursorPos.y - l_top;
		}
	}else if(r_top < cursorPos.y && (r_top+height) > cursorPos.y && r_left < cursorPos.x && (r_left+width) > cursorPos.x){
		right_eye_selected = !right_eye_selected;
		if(right_eye_selected){
			left_eye->hide();
			zoom_panel->show();
			//offsetX = cursorPos.x - r_left;
			//offsetY = cursorPos.y - r_top;
			right_eye->setDimensions(0.3f, 0.3f);
		}
	}

    // selection of spectacles (has to be treated this ugly way!)
    for (unsigned int i = 0; i < m_spectaclesThumbs.size(); i++)
    {
		if (m_spectaclesThumbs[i]->isVisible() && OgreBites::Widget::isCursorOver(m_spectaclesThumbs[i]->getOverlayElement(), Ogre::Vector2(mTrayMgr->getCursorContainer()->getLeft(), mTrayMgr->getCursorContainer()->getTop()), 0))
        {
            m_pSpectaclesSelectMenu->selectItem(i);
            break;
        }
    }

    if (mTrayMgr->injectMouseDown(arg, id)) return true;

	m_mouseDraggedStartState = arg.state;
	m_isMouseDragged = true;
	m_mouseDraggedViewSelectId = m_pViewsSelectMenu->getSelectionIndex();
	m_beforeMouseDraggedSpectaclesPose = m_pDisplayFace->getSpectaclesPose();

	//do things here for changing faces per mouse
	if( !m_bInCaptureMode && m_bShowGlasses )
    {
		m_pCoordinateCrossMesh->setVisible( true );
    }

    return true;
}
 
bool OgreRender::mouseReleased( const OIS::MouseEvent &arg, OIS::MouseButtonID id )
{
    m_isMousePressed = false;
    m_isMouseDragged = false;
    if (mTrayMgr->injectMouseUp(arg, id)) return true;
    mCameraMan->injectMouseUp(arg, id);
    m_pCoordinateCrossMesh->setVisible( false );
	if(left_eye_selected) right_eye->show();
	if(right_eye_selected) left_eye->show();
	zoom_panel->hide();
	left_eye_selected = false;
	right_eye_selected = false;
	left_eye->setDimensions(0.07f, 0.07f);
	right_eye->setDimensions(0.07f, 0.07f);
    return true;
}
 
//Adjust mouse clipping area
void OgreRender::windowResized(Ogre::RenderWindow* rw)
{
    unsigned int width, height, depth;
    int left, top;
    rw->getMetrics(width, height, depth, left, top);
 
    const OIS::MouseState &ms = mMouse->getMouseState();
    ms.width = width;
    ms.height = height;
    
}
 
//Unattach OIS before window shutdown (very important under Linux)
void OgreRender::windowClosed(Ogre::RenderWindow* rw)
{
    //Only close for window that created OIS (the main window in these demos)
    if( rw == mWindow )
    {
        if( mInputManager )
        {
            mInputManager->destroyInputObject( mMouse );
            mInputManager->destroyInputObject( mKeyboard );
 
            OIS::InputManager::destroyInputSystem(mInputManager);
            mInputManager = 0;
        }
    }
}

// Ogre::SdkTrayListener
void OgreRender::sliderMoved( OgreBites::Slider* slider)
{
    if( "FovySlider" == slider->getName() )
    {
        mCamera->setFOVy( Ogre::Radian( Ogre::Math::DegreesToRadians( slider->getValue() ) ) );
    }
    else if(
        slider == m_pUserScaleSlider || 
        slider == m_pUserRotationXSlider ||
        slider == m_pUserRotationYSlider ||
        slider == m_pUserRotationZSlider ||
        slider == m_pUserLocationXSlider ||
        slider == m_pUserLocationYSlider ||
        slider == m_pUserLocationZSlider )
    {
        m_pDisplayFace->setSpectaclesPose( 
         	m_pUserLocationXSlider->getValue(), m_pUserLocationYSlider->getValue(), m_pUserLocationZSlider->getValue(),
            m_pUserRotationXSlider->getValue(), m_pUserRotationYSlider->getValue(), m_pUserRotationZSlider->getValue(),
         	m_pUserScaleSlider->getValue() );

    }
    else if( slider == m_pViewSlider )
    {
        m_pViewsSelectMenu->selectItem( (int) m_pViewSlider->getValue(), true );
    }
    // delegate event to another Listener (e.g. AppLogic)
    m_pAdditionalSdkTrayListener->sliderMoved( slider );
}


void OgreRender::buttonHit( OgreBites::Button * button )
{
    // delegate event to another Listener (e.g. AppLogic)
    m_pAdditionalSdkTrayListener->buttonHit( button );
}

void OgreRender::itemSelected( OgreBites::SelectMenu * selectMenu )
{
    if( m_pShowFaceModelDisplayTypeSelectMenu == selectMenu )
    {
        if( 0 == selectMenu->getSelectedItem().compare( "triangles" ) )
        {
            m_faceModelDisplayType = Ogre::RenderOperation::OT_TRIANGLE_LIST;
        }
        else if( 0 == selectMenu->getSelectedItem().compare( "wires" ) )
        {
            m_faceModelDisplayType = Ogre::RenderOperation::OT_LINE_LIST;
        }
        else if( 0 == selectMenu->getSelectedItem().compare( "points" ) )
        {
            m_faceModelDisplayType = Ogre::RenderOperation::OT_POINT_LIST;
        }
        m_bShowFaceModel = true;
        if( 0 == selectMenu->getSelectedItem().compare( "none" ) )
        {
            m_bShowFaceModel = false;
        }

        m_pFaceModelMesh->setVisible( m_bShowFaceModel );
    }
    // delegate event to another Listener (e.g. AppLogic)
    m_pAdditionalSdkTrayListener->itemSelected( selectMenu );
}

void OgreRender::labelHit( OgreBites::Label * label )
{
    if( label == m_pCaptureTrayGroup->getLabel() || label == m_pViewTrayGroup->getLabel() )
    {
        m_pViewTrayGroup->toggleTrayGroup();
        m_pCaptureTrayGroup->toggleTrayGroup();
        // if we want to capture something we have to switch to live view!
        if( m_pCaptureTrayGroup->isUnfold() )
        {
            // we are in capture mode and show live view!
            m_bInCaptureMode = true;
            m_pViewSliderTrayGroup->setVisible( false );
        }
        else
        {
            // we are in view mode and show the current face
            m_bInCaptureMode = false;
            m_pViewSliderTrayGroup->setVisible( true );
            m_pDisplayFace->setAreTexturesChanged( true ); // ... force update of texures
        }
    }
    else if( label == m_pSpectaclesTrayGroup->getLabel() )
    {
        m_pSpectaclesTrayGroup->toggleTrayGroup();
    }
    else if( label == m_pDebugTrayGroup->getLabel() )
    {
        m_pDebugTrayGroup->toggleTrayGroup();        
    }
    else if( label == m_pKinectSettingsTrayGroup->getLabel() )
    {
        m_pKinectSettingsTrayGroup->toggleTrayGroup();
    }
    else if( label == m_pUserControlTrayGroup->getLabel() )
    {
        m_pUserControlTrayGroup->toggleTrayGroup();
        m_pCoordinateCrossMesh->setVisible( m_pUserControlTrayGroup->isUnfold() );
    }
    else if( label == m_pFTStatusTrayGroup->getLabel() )
    {
       m_pFTStatusTrayGroup->toggleTrayGroup();
    }
    // update the TrayGroupManager after we hit a symbol
    m_bUpdateTrayGroupManager = true;

    // delegate event to another Listener (e.g. AppLogic)
    m_pAdditionalSdkTrayListener->labelHit( label );
}


void OgreRender::checkBoxToggled( OgreBites::CheckBox * checkbox )
{
    if( checkbox == m_pShowKinectDepthCheckbox )
    {
        m_pColorBackgroundNode->setVisible( !checkbox->isChecked() );
        m_pDepthBackgroundNode->setVisible( checkbox->isChecked() );
    }
    else if( checkbox == m_pShowGlassesCheckbox )
    {
		m_bShowGlasses = checkbox->isChecked();
    }
	else if ( checkbox == m_pShowEyeTrackingCheckbox )
	{
		m_bShowEyeTracking = m_pShowEyeTrackingCheckbox->isChecked();
		if(m_pShowEyeTrackingCheckbox->isChecked()){
			overlay->show();
		}else{
			overlay->hide();
		}
	}
    else if( checkbox == m_pShowSkeletonNodesCheckbox )
    {
        for( unsigned int i = 0; i < m_numKinectSkeletons; ++ i )
        {
            m_ppKinectSkeletonHeadNodes[i]->setVisible( checkbox->isChecked() );
            m_ppKinectSkeletonNeckNodes[i]->setVisible( checkbox->isChecked() );
        }
    }
    else if( checkbox == m_pShowFacePlaceholderCheckbox )
    {
        if( !checkbox->isChecked() )
        {  
            m_pFacePlaceHolderMesh->setMaterial( Ogre::MaterialManager::getSingleton().getByName( "FaceModel/Transparent" ) );
        }
        else
        {
            m_pFacePlaceHolderMesh->setMaterial( Ogre::MaterialManager::getSingleton().getByName( "FaceModel/Solid" ) );
        }
    }
    // delegate event to another Listener (e.g. AppLogic)
    m_pAdditionalSdkTrayListener->checkBoxToggled( checkbox );
}



//-----------------------------------------------------------------------------
// UTILITY FUNCTIONS
//----------------------------------------------------------------------------- 
using namespace Ogre;

// creating a sphere-mesh from code with VBOs etc.
// from: http://www.ogre3d.org/tikiwiki/ManualSphereMeshes
void OgreRender::createSphereMesh(const std::string& strName, const float r, const int nRings , const int nSegments)
{
    MeshPtr pSphere = MeshManager::getSingleton().createManual(strName, ResourceGroupManager::DEFAULT_RESOURCE_GROUP_NAME);
    SubMesh *pSphereVertex = pSphere->createSubMesh();

    pSphere->sharedVertexData = new VertexData();
    VertexData* vertexData = pSphere->sharedVertexData;

    // define the vertex format
    VertexDeclaration* vertexDecl = vertexData->vertexDeclaration;
    size_t currOffset = 0;
    // positions
    vertexDecl->addElement(0, currOffset, VET_FLOAT3, VES_POSITION);
    currOffset += VertexElement::getTypeSize(VET_FLOAT3);
    // normals
    vertexDecl->addElement(0, currOffset, VET_FLOAT3, VES_NORMAL);
    currOffset += VertexElement::getTypeSize(VET_FLOAT3);
    // two dimensional texture coordinates
    vertexDecl->addElement(0, currOffset, VET_FLOAT2, VES_TEXTURE_COORDINATES, 0);
    currOffset += VertexElement::getTypeSize(VET_FLOAT2);

    // allocate the vertex buffer
    vertexData->vertexCount = (nRings + 1) * (nSegments+1);
    HardwareVertexBufferSharedPtr vBuf = HardwareBufferManager::getSingleton().createVertexBuffer(vertexDecl->getVertexSize(0), vertexData->vertexCount, HardwareBuffer::HBU_STATIC_WRITE_ONLY, false);
    VertexBufferBinding* binding = vertexData->vertexBufferBinding;
    binding->setBinding(0, vBuf);
    float* pVertex = static_cast<float*>(vBuf->lock(HardwareBuffer::HBL_DISCARD));

    // allocate index buffer
    pSphereVertex->indexData->indexCount = 6 * nRings * (nSegments + 1);
    pSphereVertex->indexData->indexBuffer = HardwareBufferManager::getSingleton().createIndexBuffer(HardwareIndexBuffer::IT_16BIT, pSphereVertex->indexData->indexCount, HardwareBuffer::HBU_STATIC_WRITE_ONLY, false);
    HardwareIndexBufferSharedPtr iBuf = pSphereVertex->indexData->indexBuffer;
    unsigned short* pIndices = static_cast<unsigned short*>(iBuf->lock(HardwareBuffer::HBL_DISCARD));

    float fDeltaRingAngle = (Math::PI / nRings);
    float fDeltaSegAngle = (2 * Math::PI / nSegments);
    unsigned short wVerticeIndex = 0 ;

    // Generate the group of rings for the sphere
    for( int ring = 0; ring <= nRings; ring++ ) {
        float r0 = r * sinf (ring * fDeltaRingAngle);
        float y0 = r * cosf (ring * fDeltaRingAngle);

        // Generate the group of segments for the current ring
        for(int seg = 0; seg <= nSegments; seg++) {
            float x0 = r0 * sinf(seg * fDeltaSegAngle);
            float z0 = r0 * cosf(seg * fDeltaSegAngle);

            // Add one vertex to the strip which makes up the sphere
            *pVertex++ = x0;
            *pVertex++ = y0;
            *pVertex++ = z0;

            Vector3 vNormal = Vector3(x0, y0, z0).normalisedCopy();
            *pVertex++ = vNormal.x;
            *pVertex++ = vNormal.y;
            *pVertex++ = vNormal.z;

            *pVertex++ = (float) seg / (float) nSegments;
            *pVertex++ = (float) ring / (float) nRings;

            if (ring != nRings) {
                // each vertex (except the last) has six indices pointing to it
                *pIndices++ = wVerticeIndex + nSegments + 1;
                *pIndices++ = wVerticeIndex;               
                *pIndices++ = wVerticeIndex + nSegments;
                *pIndices++ = wVerticeIndex + nSegments + 1;
                *pIndices++ = wVerticeIndex + 1;
                *pIndices++ = wVerticeIndex;
                wVerticeIndex ++;
            }
        }; // end for seg
    } // end for ring

    // Unlock
    vBuf->unlock();
    iBuf->unlock();
    // Generate face list
    pSphereVertex->useSharedVertices = true;

    // the original code was missing this line:
    pSphere->_setBounds( AxisAlignedBox( Vector3(-r, -r, -r), Vector3(r, r, r) ), false );
    pSphere->_setBoundingSphereRadius(r);
    // this line makes clear the mesh is loaded (avoids memory leaks)
    pSphere->load();
}

Ogre::Vector2 OgreRender::calculateMovement( const OIS::MouseState& startState, const OIS::MouseState& endState, unsigned int numItems )
{

    return Ogre::Vector2( ( endState.X.abs - startState.X.abs ) / ( endState.width / 2.0 ),
            ( endState.Y.abs - startState.Y.abs ) / ( endState.height / 2.0 ) );
}

void OgreRender::setCoordinateCross( const std::string& name )
{
    m_pCoordinateCrossMesh = mSceneMgr->createEntity( name );
    m_pSpectaclesNode->attachObject( m_pCoordinateCrossMesh );
    m_pCoordinateCrossMesh->setRenderQueueGroup( Ogre::RENDER_QUEUE_OVERLAY );
    m_pCoordinateCrossMesh->setVisible( false );
}

void OgreRender::setLeftEyePosition( float x, float y ){
	float centered_x = x - (left_eye->getWidth()/2.0f);
	float centered_y = y - (left_eye->getHeight()/2.0f);
	left_eye->setPosition(centered_x,centered_y);
}

void OgreRender::setRightEyePosition( float x, float y ){
	float centered_x = x - (right_eye->getWidth()/2.0f);
	float centered_y = y - (right_eye->getHeight()/2.0f);
	right_eye->setPosition(centered_x,centered_y);
}

void OgreRender::setZoomPanelPosition( float x, float y ){
	float centered_x = x - (zoom_panel->getWidth()/2.0f);
	float centered_y = y - (zoom_panel->getHeight()/2.0f);
	zoom_panel->setPosition(centered_x, centered_y);
}
