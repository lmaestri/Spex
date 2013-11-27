#pragma once

#include "OgreFrameListener.h"
#include "OgreRender.h"
#include "FaceTracker.h"
#include "SpectaclesManager.h"
#include "FaceManager.h"
#include "Face.h"
#include "OISKeyboard.h"

class AppLogic : public Ogre::FrameListener, OgreBites::SdkTrayListener, OIS::KeyListener
{
private:
    OgreRender          m_ogreRenderer;
    FaceTracker         m_faceTracker;
    SpectaclesManager   m_spectaclesManager;
    FaceManager         m_faceManager;
    Face                m_currentFace;
	bool        m_isTiltSliderMoving;
    int m_currentSpectaclesIndex;
	std::string			m_lastCapturePoseName;
public:
    AppLogic(void);


    ~AppLogic(void);

    bool start( void );
    void update( void );

    void setCurrentSpectacles( int index );

    // Ogre::FrameListener
    virtual bool frameRenderingQueued(const Ogre::FrameEvent& evt);
    // Ogre::SdkTrayListener
    virtual void sliderMoved( OgreBites::Slider* slider);
    virtual void buttonHit( OgreBites::Button* button );
	virtual void itemSelected( OgreBites::SelectMenu* selectMenu );
	virtual void checkBoxToggled( OgreBites::CheckBox* checkBox );

    void capture();
    
    void setupSpectacles( const std::string & rFileName );

	//OIS
    virtual bool keyPressed( const OIS::KeyEvent &arg );
    virtual bool keyReleased( const OIS::KeyEvent &arg );

};

