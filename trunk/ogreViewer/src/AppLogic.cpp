#include "AppLogic.h"
#include "SpectaclesManager.h"
#include "Spectacles.h"


AppLogic::AppLogic(void)
    : m_isTiltSliderMoving( true ),
    m_currentSpectaclesIndex( -1 )
{
	m_lastCapturePoseName = "";

    // get configurations from the sensor
    FT_CAMERA_CONFIG * pDepth = m_faceTracker.GetDepthCameraConfig(),
                     * pColor = m_faceTracker.GetColorCameraConfig();

    if( !m_faceTracker.IsKinectPresent() )
    {
        throw new std::exception( "Kinect not present!" );
    }

    m_ogreRenderer.init( this, this, this, Ogre::Vector2( pColor->Width, pColor->Height ), 
        Ogre::Vector2( pDepth->Width, pDepth->Height ), NUI_CAMERA_COLOR_NOMINAL_VERTICAL_FOV );

    m_ogreRenderer.setDisplayFace( &m_currentFace );

    setupSpectacles( "spectacles.cfg" );
    m_faceManager.load( "faces.cfg" );
    m_ogreRenderer.updateViewableFaces( m_faceManager.getFaceNames() );
    
}


AppLogic::~AppLogic(void)
{
}



bool AppLogic::start( void )
{
    m_faceTracker.start();

    m_ogreRenderer.m_pKinectTiltSlider->setValue( m_faceTracker.getKinectSensor()->getElevationAngle() );
    m_ogreRenderer.m_pKinectTiltSlider->setValueCaption( Ogre::StringConverter::toString( m_faceTracker.getKinectSensor()->getElevationAngle() ) );
    m_ogreRenderer.m_pKinectTiltSlider->setCaption( "tilt" );
    
    // start ogre rendering, here. This is blocking until we quit the program!
    bool r = m_ogreRenderer.go();

    m_faceManager.store( "faces.cfg" );

    return r;
}

void AppLogic::update( void )
{
    m_faceTracker.update();

    if( m_faceTracker.IsFaceDetected() )
    {
        // get the model for visualizing the face-model
        m_ogreRenderer.updateFaceModelMesh( m_faceTracker.m_pTriangles, m_faceTracker.m_numTriangles, m_faceTracker.m_pVertices, m_faceTracker.m_numVertices );
        
        float scale = 1, pRot[3] = { 0,0,0 }, pTrans[3] = { 0,0,0 };
        m_faceTracker.GetResult()->Get3DPose( &scale, pRot, pTrans );
		
		m_currentFace.setPose( pTrans, pRot, scale );
		
		//Eyetracking

		//Pupil Bounding Box (Candide-3)
		int LEFT_OUTER_UPPER = 67;
		int LEFT_OUTER_LOWER = 68;
		int LEFT_INNER_UPPER = 71;
		int LEFT_INNER_LOWER = 72;

		int RIGHT_OUTER_UPPER = 69;
		int RIGHT_OUTER_LOWER = 70;
		int RIGHT_INNER_UPPER = 73;
		int RIGHT_INNER_LOWER = 74;

		
		//printf("Left3D: %f | %f | %f \n", left_xs, left_ys, left_zs);
		//printf("Right3D: %f | %f | %f \n", right_xs, right_ys, right_zs);
				
		if(m_ogreRenderer.m_bInCaptureMode){

			//Left Pupilcenter
			float left_xs = 0.25f * (m_faceTracker.m_pVertices[LEFT_OUTER_UPPER].x + m_faceTracker.m_pVertices[LEFT_OUTER_LOWER].x + m_faceTracker.m_pVertices[LEFT_INNER_UPPER].x + m_faceTracker.m_pVertices[LEFT_INNER_LOWER].x);
			float left_ys = 0.25f * (m_faceTracker.m_pVertices[LEFT_OUTER_UPPER].y + m_faceTracker.m_pVertices[LEFT_OUTER_LOWER].y + m_faceTracker.m_pVertices[LEFT_INNER_UPPER].y + m_faceTracker.m_pVertices[LEFT_INNER_LOWER].y);
			float left_zs = 0.25f * (m_faceTracker.m_pVertices[LEFT_OUTER_UPPER].z + m_faceTracker.m_pVertices[LEFT_OUTER_LOWER].z + m_faceTracker.m_pVertices[LEFT_INNER_UPPER].z + m_faceTracker.m_pVertices[LEFT_INNER_LOWER].z);

			//Left Pupilcenter
			float right_xs = 0.25f * (m_faceTracker.m_pVertices[RIGHT_OUTER_UPPER].x + m_faceTracker.m_pVertices[RIGHT_OUTER_LOWER].x + m_faceTracker.m_pVertices[RIGHT_INNER_UPPER].x + m_faceTracker.m_pVertices[RIGHT_INNER_LOWER].x);
			float right_ys = 0.25f * (m_faceTracker.m_pVertices[RIGHT_OUTER_UPPER].y + m_faceTracker.m_pVertices[RIGHT_OUTER_LOWER].y + m_faceTracker.m_pVertices[RIGHT_INNER_UPPER].y + m_faceTracker.m_pVertices[RIGHT_INNER_LOWER].y);
			float right_zs = 0.25f * (m_faceTracker.m_pVertices[RIGHT_OUTER_UPPER].z + m_faceTracker.m_pVertices[RIGHT_OUTER_LOWER].z + m_faceTracker.m_pVertices[RIGHT_INNER_UPPER].z + m_faceTracker.m_pVertices[RIGHT_INNER_LOWER].z);
		
			float dx = pow((left_xs - right_xs), 2);
			float dy = pow((left_ys - right_ys), 2);
			float dz = pow((left_zs - right_zs), 2);
			float pupillarydistance = sqrt(dx + dy + dz) * 1000;
			float zdistance = 0.5f * (left_zs + right_zs);

			m_currentFace.setLeftEye3D(left_xs, left_ys, left_zs);
			m_currentFace.setRightEye3D(right_xs, right_ys, right_zs);
			
			m_ogreRenderer.updatePDLabel(pupillarydistance, zdistance);

			float left_xs2D = 0.25f * (m_faceTracker.m_pPoints[LEFT_OUTER_UPPER].x + m_faceTracker.m_pPoints[LEFT_OUTER_LOWER].x + m_faceTracker.m_pPoints[LEFT_INNER_UPPER].x + m_faceTracker.m_pPoints[LEFT_INNER_LOWER].x);
			float left_ys2D = 0.25f * (m_faceTracker.m_pPoints[LEFT_OUTER_UPPER].y + m_faceTracker.m_pPoints[LEFT_OUTER_LOWER].y + m_faceTracker.m_pPoints[LEFT_INNER_UPPER].y + m_faceTracker.m_pPoints[LEFT_INNER_LOWER].y);

			float right_xs2D = 0.25f * (m_faceTracker.m_pPoints[RIGHT_OUTER_UPPER].x + m_faceTracker.m_pPoints[RIGHT_OUTER_LOWER].x + m_faceTracker.m_pPoints[RIGHT_INNER_UPPER].x + m_faceTracker.m_pPoints[RIGHT_INNER_LOWER].x);
			float right_ys2D = 0.25f * (m_faceTracker.m_pPoints[RIGHT_OUTER_UPPER].y + m_faceTracker.m_pPoints[RIGHT_OUTER_LOWER].y + m_faceTracker.m_pPoints[RIGHT_INNER_UPPER].y + m_faceTracker.m_pPoints[RIGHT_INNER_LOWER].y);

			m_currentFace.setLeftEye2D(left_xs2D/m_faceTracker.GetColorImage()->GetWidth(), left_ys2D/m_faceTracker.GetColorImage()->GetHeight());
			m_currentFace.setRightEye2D(right_xs2D/m_faceTracker.GetColorImage()->GetWidth(), right_ys2D/m_faceTracker.GetColorImage()->GetHeight());
		}

        // do some debug output:
        static bool s_isFirstRun = true;
        if( m_ogreRenderer.m_isWriteObjKeyPressed )
        {
            m_faceTracker.write3DFaceModelObjToFile( "FaceModel_transformed.obj", true );
            m_faceTracker.write3DFaceModelObjToFile( "FaceModel_neutral.obj", false , 0, false, false, false );
            m_faceTracker.writeTransform3DFaceModelToFile( "Transformation.txt" );
            s_isFirstRun = false;
        }
    }

    // get skeleton Data from Kinectsensor and present it in Ogre
    m_ogreRenderer.updateSkeletonPoints( m_faceTracker.getKinectSensor()->m_SkeletonTracked, m_faceTracker.getKinectSensor()->m_HeadPoint,
        m_faceTracker.getKinectSensor()->m_NeckPoint, NUI_SKELETON_COUNT );
   
    // update the status-overlay
    m_ogreRenderer.setFaceTrackerDetails( m_faceTracker.GetResult(), m_faceTracker.m_isFaceModelConverged );

    if( m_ogreRenderer.m_bInCaptureMode )
    {
        m_ogreRenderer.setDisplayFace( &m_currentFace );
    }
    else if( m_ogreRenderer.m_pViewsSelectMenu->getNumItems() > 0 )
    {
        std::string selectedView = m_ogreRenderer.m_pViewsSelectMenu->getSelectedItem();
        m_ogreRenderer.setDisplayFace( m_faceManager.getFaceByName( selectedView ) );
    }

    if( m_faceTracker.m_isTextureUpdated && m_ogreRenderer.m_bInCaptureMode )
    {   
        // update the overlay textures
        m_currentFace.updateImages( m_faceTracker.GetColorImage(), m_faceTracker.GetDepthImage(), m_faceTracker.IsFaceDetected() );
        m_faceTracker.m_isTextureUpdated = false;
    }
    
    // if the kinectTiltSlider has been moved and the mouse isn't pressed anymore!
    if( m_isTiltSliderMoving && !m_ogreRenderer.isMousePressed() )
    {
        float value = m_ogreRenderer.m_pKinectTiltSlider->getValue();
        m_faceTracker.getKinectSensor()->setElevationAngle( m_ogreRenderer.m_pKinectTiltSlider->getValue() );  
        m_isTiltSliderMoving = false;
    }
}




void AppLogic::setCurrentSpectacles( int index )
{
    if( index != m_currentSpectaclesIndex )
    {
        if( m_currentSpectaclesIndex >= 0 )
        { 
            m_spectaclesManager.getSpectacles( m_currentSpectaclesIndex )->setVisible( false );
        }
        //m_ogreRenderer.setGlassesMesh( m_spectaclesManager.getSpectacles( index )->getMesh() );
        m_currentSpectaclesIndex = index;
        m_spectaclesManager.getSpectacles( m_currentSpectaclesIndex )->setVisible( true );
    }
    
}


// Ogre::FrameListener
bool AppLogic::frameRenderingQueued(const Ogre::FrameEvent& evt)
{
    // call update each frame
    update( );

    return true;
}

// Ogre::SdkTrayListener
void AppLogic::sliderMoved( OgreBites::Slider* slider)
{
    // if the kinectTiltSlider has been moved and the mouse isn't pressed anymore!
    if( "KinectTiltSlider" == slider->getName() )
    {
        m_isTiltSliderMoving = true; 
    }
}

void AppLogic::checkBoxToggled(OgreBites::CheckBox* pCheckBox){
	if(pCheckBox->getName() == "ShowGlassesCheckbox" && m_ogreRenderer.m_guiLoaded){
		m_spectaclesManager.getSpectacles(m_currentSpectaclesIndex)->setVisible(pCheckBox->isChecked());
	}
}

void AppLogic::itemSelected(  OgreBites::SelectMenu * pSelectMenu )
{
    // set the spectacles
    if( "SpectaclesSelectMenu" == pSelectMenu->getName() )
    {
        setCurrentSpectacles( pSelectMenu->getSelectionIndex() );
    }

    // set the view to show
    if( m_ogreRenderer.m_pViewsSelectMenu == pSelectMenu )
    {
        {
            Face * pFace =  m_faceManager.getFaceByName( pSelectMenu->getSelectedItem() );
            if( pFace )
            {
                m_ogreRenderer.setDisplayFace( pFace );
                pFace->setAreTexturesChanged( true );
                m_ogreRenderer.updateViewableFaces( m_faceManager.getFaceNames() );
            }
        }
    }
}


void AppLogic::buttonHit( OgreBites::Button * pButton )
{
    if( pButton->getName() == "CaptureButton" )
    {
        capture();
    }else if( pButton->getName() == "DeleteViewButton" && m_ogreRenderer.m_pViewsSelectMenu->getNumItems() > 0 ){
       m_faceManager.removeFaceByName( m_ogreRenderer.m_pViewsSelectMenu->getSelectedItem() );
       m_ogreRenderer.updateViewableFaces( m_faceManager.getFaceNames() );
       if( m_ogreRenderer.m_pViewsSelectMenu->getNumItems() > 0 ){
            m_ogreRenderer.setDisplayFace( m_faceManager.getFaceByName(  m_ogreRenderer.m_pViewsSelectMenu->getSelectedItem() ) );
       }
    }else if ( pButton->getName() == "LastCaptureButton" ){
		 if( m_ogreRenderer.m_pViewsSelectMenu->getNumItems() > 0 ){
            m_ogreRenderer.setDisplayFace(  m_faceManager.getFaceByName(m_lastCapturePoseName) );
			m_ogreRenderer.m_pViewsSelectMenu->selectItem(m_lastCapturePoseName);
		}else{
			m_lastCapturePoseName = "";
		 }
	}
}

void AppLogic::setupSpectacles( const std::string & rFileName )
{
    m_spectaclesManager.load( rFileName, m_ogreRenderer.mSceneMgr, m_ogreRenderer.m_pSpectaclesPivotNode, true );
    for( unsigned int i = 0; i < m_spectaclesManager.getNumSpectacles(); ++ i )
    {
        m_ogreRenderer.m_pSpectaclesSelectMenu->addItem( m_spectaclesManager.getSpectacles(i)->getName() );
        m_ogreRenderer.addSpectacles( m_spectaclesManager.getSpectacles(i) );
    }
    m_ogreRenderer.m_pSpectaclesSelectMenu->selectItem( 0, true );
    m_ogreRenderer.setFacePlaceHolderMesh( m_spectaclesManager.getFacePlaceHolderMeshName() );
    m_ogreRenderer.setSpectaclesPivotPoint( m_spectaclesManager.getSpectaclesPivotLocation() );
    m_ogreRenderer.setCoordinateCross( m_spectaclesManager.getCoordinateCrossMeshName() );
}

void AppLogic::capture()
{
    if( m_faceTracker.IsFaceDetected() )
    {
        // the head rotation is encrypted in the PoseName calculated in the class Face.
        // make a copy of currentFace and store it in faceManager
		//printf("Captured: %f, %f, %f \n", m_currentFace.getLeftEyeLocation().x, m_currentFace.getLeftEyeLocation().y, m_currentFace.getLeftEyeLocation().z);
        m_faceManager.addFaceCopy( m_currentFace.getCurrentPoseName(), m_currentFace );
		m_lastCapturePoseName = m_currentFace.getCurrentPoseName();
        // update the list of available faces in the view!
        m_ogreRenderer.updateViewableFaces( m_faceManager.getFaceNames() );
    }
}

bool AppLogic::keyPressed( const OIS::KeyEvent &arg )
{
    // gets called when we want to capture something
    if( arg.key == OIS::KC_SPACE )
    {
        capture();
    }

    return true;
}

bool AppLogic::keyReleased( const OIS::KeyEvent &arg )
{
    // not needed until now
    return true;
}
