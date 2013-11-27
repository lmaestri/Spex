/*
mofified from an Ogre sample
*/
#ifndef __MinimalOgre_h_
#define __MinimalOgre_h_
 
#include <OgreCamera.h>
#include <OgreEntity.h>
#include <OgreLogManager.h>
#include <OgreRoot.h>
#include <OgreViewport.h>
#include <OgreSceneManager.h>
#include <OgreRenderWindow.h>
#include <OgreConfigFile.h>
#include <OgreOverlay.h>
#include <OgreOverlayManager.h>
#include <OgreMath.h>

#include <OISEvents.h>
#include <OISInputManager.h>
#include <OISKeyboard.h>
#include <OISMouse.h>
 
#include <SdkTrays.h>
#include <SdkCameraMan.h>
#include "Face.h"

// predeclarations
struct IFTImage;
struct IFTModel;
struct IFTResult;
struct FT_TRIANGLE;
struct FT_VECTOR3D;
class TrayGroup;
class TrayGroupManager;
class Face;
class Spectacles;

class OgreRender : public Ogre::FrameListener, public Ogre::WindowEventListener, public OIS::KeyListener, public OIS::MouseListener, OgreBites::SdkTrayListener
{
friend class AppLogic;

protected:
    Ogre::Root *mRoot;
    Ogre::Camera* mCamera;
    Ogre::SceneManager* mSceneMgr;
    Ogre::RenderWindow* mWindow;
    Ogre::String mResourcesCfg;
    Ogre::String mPluginsCfg;

    // Textures, Overlay for Kinect color & depth inputs
    Ogre::TexturePtr    m_colorTexturePtr,
                        m_depthTexturePtr;
    // Kinect Color & Depth textures background nodes, rects
    Ogre::SceneNode*	m_pColorBackgroundNode;
    Ogre::SceneNode    *m_pDepthBackgroundNode;
	Ogre::SceneNode    *m_pDepthTexNode;
    Ogre::Rectangle2D  *m_pColorBackgroundRectangle;    
    Ogre::Rectangle2D  *m_pDepthBackgroundRectangle;    
    Ogre::Rectangle2D  *m_pDepthTexRectangle;
	
    

    // stuff for showing the facemodel
    Ogre::ManualObject* m_pFaceModelMesh;
    Ogre::RenderOperation::OperationType m_faceModelDisplayType;
    const Ogre::String  m_FaceModelMeshName;
    
	Ogre::SceneNode*    m_pFaceModelNode;
	// the node containing the spectacles and the the facemesh
    Ogre::SceneNode*    m_pFacesNode;
    // the node containing the spectacles, which is a child of m_pFacesNode 
    Ogre::SceneNode*    m_pSpectaclesNode;
    Ogre::SceneNode*    m_pSpectaclesPivotNode;
    
	// the faceplaceholder for correct transparency
    Ogre::Entity *      m_pFacePlaceHolderMesh;
    Ogre::Vector3       m_spectaclesPivotLocation;
    Ogre::Entity        *m_pCoordinateCrossMesh;

    // stuff showing Kinect-skeleton
    unsigned int        m_numKinectSkeletons;
    Ogre::SceneNode**   m_ppKinectSkeletonHeadNodes;
    Ogre::SceneNode**   m_ppKinectSkeletonNeckNodes;

    // handling user input
    bool                m_bShowKinectDepth;
    bool                m_bShowGlasses;			//maybe not needed
    bool                m_bShowSkeletonNodes;
    bool                m_bShowFaceModel;		//wire frame model!
	bool				m_bShowEyeTracking;		//show eye boundingboxes
	//Offset between cursor and zoom panel for eyetracking
	//float				offsetX;
	//float				offsetY;

    // OgreBites
    OgreBites::SdkTrayManager* mTrayMgr;
    // basic camera controller
	OgreBites::SdkCameraMan* mCameraMan;      
    // additional tray listener, except (this)
    OgreBites::SdkTrayListener* m_pAdditionalSdkTrayListener; 
    OIS::KeyListener *m_pAdditionalKeyListener;
    // FaceTrack feedback
    OgreBites::Label*       m_pFTLabel;
	OgreBites::Label*       m_pPDLabel;
    OgreBites::ParamsPanel* m_pFTDetailsPanel;
    // Kinect Controls
    OgreBites::Label       *m_pKinectSettingsLabel;
    OgreBites::Slider *     m_pKinectTiltSlider;
    // FOVy slider
    OgreBites::Slider *     m_pFovySlider;

	//EyeTracking
	Ogre::Overlay* overlay;
	Ogre::OverlayContainer* left_eye;
	Ogre::BorderPanelOverlayElement* zoom_panel;
	Ogre::OverlayContainer* right_eye;
	bool left_eye_selected;
	bool right_eye_selected;

    // debug stuff
    OgreBites::Label       *m_pDisplaySettingsLabel;
    OgreBites::CheckBox *   m_pShowKinectDepthCheckbox;
    OgreBites::CheckBox *   m_pShowGlassesCheckbox;
	OgreBites::CheckBox *	m_pShowEyeTrackingCheckbox;
    OgreBites::CheckBox *   m_pShowSkeletonNodesCheckbox;
    OgreBites::CheckBox *   m_pShowFacePlaceholderCheckbox;
    OgreBites::SelectMenu * m_pShowFaceModelDisplayTypeSelectMenu;
    
    // glasses & views
    OgreBites::SelectMenu* m_pSpectaclesSelectMenu;
    OgreBites::SelectMenu* m_pViewsSelectMenu;
    OgreBites::SelectMenu* m_pCaptureSelectMenu;
    OgreBites::Button     *m_pCaptureButton;
	OgreBites::Button	  *m_pLastCaptureButton;
    OgreBites::Button     *m_pDeleteViewButton;
    OgreBites::Slider     *m_pViewSlider;

    // user control stuff
    OgreBites::Slider     *m_pUserScaleSlider;
    OgreBites::Slider     *m_pUserRotationXSlider;
    OgreBites::Slider     *m_pUserRotationYSlider;
    OgreBites::Slider     *m_pUserRotationZSlider;
    OgreBites::Slider     *m_pUserLocationXSlider;
    OgreBites::Slider     *m_pUserLocationYSlider;
    OgreBites::Slider     *m_pUserLocationZSlider;


    // tray groups for guis
    TrayGroup             *m_pSpectaclesTrayGroup;
    TrayGroup             *m_pViewTrayGroup;
    TrayGroup             *m_pCaptureTrayGroup;
    TrayGroup             *m_pKinectSettingsTrayGroup;
    TrayGroup             *m_pDebugTrayGroup;
    TrayGroup             *m_pUserControlTrayGroup;
    TrayGroup             *m_pFTStatusTrayGroup;
    TrayGroup             *m_pViewSliderTrayGroup;

    TrayGroupManager      *m_pTrayGroupManager;
    bool                  m_bUpdateTrayGroupManager;


    // mouse dragging
    OIS::MouseState       m_mouseDraggedStartState;
    bool                  m_isMouseDragged; 
    int                   m_mouseDraggedViewSelectId;

    std::vector<OgreBites::DecorWidget *> m_spectaclesThumbs;
    

    
    bool mCursorWasVisible;						// was cursor visible before dialog appeared
    bool mShutDown;
    bool m_isMousePressed;
    bool m_isWriteObjKeyPressed;
    Face* m_pDisplayFace;						// current Face, storing color and depth image and the glassposition
    bool m_bInCaptureMode;
	bool m_guiLoaded;

    // OIS Input devices
    OIS::InputManager* mInputManager;
    OIS::Mouse*    mMouse;
    OIS::Keyboard* mKeyboard;
    Face::Pose m_beforeMouseDraggedSpectaclesPose;

    // --- FUNCTIONS ---

    void OgreRender::createKinectTextures( const std::string& colorTextureName, const std::string& depthTextureName,
        const Ogre::Vector2& rColorInput, const Ogre::Vector2& rDepthInput );
    void generateFaceModelMesh( );
    void destroyFaceModelMesh( );


    // Ogre::FrameListener
    virtual bool frameRenderingQueued(const Ogre::FrameEvent& evt);
 
    // OIS::KeyListener
    virtual bool keyPressed( const OIS::KeyEvent &arg );
    virtual bool keyReleased( const OIS::KeyEvent &arg );
    // OIS::MouseListener
    virtual bool mouseMoved( const OIS::MouseEvent &arg );
    virtual bool mousePressed( const OIS::MouseEvent &arg, OIS::MouseButtonID id );
    virtual bool mouseReleased( const OIS::MouseEvent &arg, OIS::MouseButtonID id );
 
    // Ogre::WindowEventListener
    virtual void windowResized(Ogre::RenderWindow* rw);
    virtual void windowClosed(Ogre::RenderWindow* rw);
    
    // Ogre::SdkTrayListener
    virtual void sliderMoved( OgreBites::Slider* slider);
    virtual void buttonHit( OgreBites::Button * button );
    virtual void itemSelected( OgreBites::SelectMenu * selectMenu );
    virtual void labelHit( OgreBites::Label * label );
    virtual void checkBoxToggled( OgreBites::CheckBox * checkbox );
    // dialogs ... not needed for now ... 

    // method for creating a sphere Mesh
    void createSphereMesh(const std::string& strName, const float r, const int nRings = 16, const int nSegments = 16);
    void updateGlassesPose( );
    void update( ); // ideally everything done each frame should be done here!
    void updateCaptureFacePosition();

public:
     // --- PUBLIC FUNCTIONS ---
    static const std::string KINECT_COLOR_TEXTURE_NAME, KINECT_DEPTH_TEXTURE_NAME; // = "KinectNameTexture";
    OgreRender(void);
    virtual ~OgreRender(void);
    bool go(void);
    bool init( Ogre::FrameListener *pFrameListener, OgreBites::SdkTrayListener* pSdkListener, OIS::KeyListener *pKeyListener,
        const Ogre::Vector2& rColorInput, const Ogre::Vector2& rDepthInput , const float fovY );

    void initGUI();
    void setFaceTrackerDetails( IFTResult * pResult, bool isConverged = false );

	//EyeTracking
	void setLeftEyePosition(float x, float y);
	void setRightEyePosition(float x, float y);
	void setZoomPanelPosition(float x, float y);

	void updatePDLabel(float pupillaryDistance, float zdistance);

    void updateFaceModelMesh( const FT_TRIANGLE * pTriangles, const unsigned int numTriangles,
        const FT_VECTOR3D * m_pVertices, const unsigned int m_numVertices );
    void updateSkeletonPoints( const bool * pIsTracked, const FT_VECTOR3D * pHeadPoints,
        const FT_VECTOR3D* pNeckPoints, const unsigned int numSkeletons );
    bool isMousePressed( void ){ return m_isMousePressed; }
    void updateViewableFaces( const Ogre::StringVector& faceNames );
    void setDisplayFace( Face* pFace );
    void setFacePlaceHolderMesh( std::string meshName );
    void addSpectacles( Spectacles * pSpectacle );
    Ogre::Vector2 calculateMovement( const OIS::MouseState& startState, const OIS::MouseState& endState, unsigned int numItems );
    void setSpectaclesPivotPoint( const Ogre::Vector3& specPivotPoint );
    void setCoordinateCross( const std::string& name );
};
 
#endif // #ifndef __MinimalOgre_h_