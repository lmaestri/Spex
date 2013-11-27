#ifndef Face_h__
#define Face_h__
#include <string>
#include <OgreVector3.h>
#include <OgreVector2.h>
#include "OgreImage.h"
//predefines
struct IFTImage;

class Face
{
    friend class FaceManager;
public:
    struct Pose 
    {
        float location[3];
        float rotation[3];
        float    scaling;
    };

    static float s_facePoseYAngleStepWidth; //Degrees

private:
    std::string		m_colorImageFileName; //stores filename if color has been written to disk
    std::string		m_depthImageFileName; //stores filename if has been written to disk
    Pose			m_pose; // pose of the face
    Pose			m_spectaclesPose; // local pose of the spectacles
    Ogre::Image		m_colorImage;
    Ogre::Image		m_depthImage;
    bool			m_isFaceDetected;   // if face is detected
    bool			m_bContainsDepthImage; // shows if depth image is presents
    bool			m_areTexturesChanged; // checks if textures changed, important for better copy
	Ogre::Vector3	m_leftEye3D;
	Ogre::Vector3	m_rightEye3D;
	Ogre::Vector2	m_leftEye2D;
	Ogre::Vector2	m_rightEye2D;

public:
    Face( );
    Face( const Face & src );
    ~Face( );
    const Pose& getPose( void ) const;
    void setPose( Pose& rPose );
	void setLeftEye3D(float x, float y, float z);
	void setRightEye3D(float x, float y, float z);
	void setLeftEye2D(float x, float y);
	void setRightEye2D(float x, float y);
    void setPose( float pTrans[], float pRot[],  float scale = 1 );
    void setPose( float transX, float transY, float transZ, float rotX, float rotY, float rotZ, float scale = 1 );
    void setSpectaclesPose( Pose& rPose );
    void setSpectaclesPose( float pTrans[], float pRot[],  float scale = 1 );
    void setSpectaclesPose( float transX, float transY, float transZ, float rotX, float rotY, float rotZ, float scale = 1 );
    Ogre::Image & getColorImage( void );
    Ogre::Image & getDepthImage( void );
    void updateImages( IFTImage * pSrcColorImage, IFTImage * pSrcDepthImage, bool isFaceDetected );
    bool hasDepthImage();
    bool areTexturesChanged( );
    void setAreTexturesChanged( bool value );
    std::string getCurrentPoseName(); // used for capturing and writing the faces to file!
    const Face::Pose& getSpectaclesPose( void ) const;
	Ogre::Vector3 getLeftEye3DLocation( void );
	Ogre::Vector3 getRightEye3DLocation( void );
	Ogre::Vector2 getLeftEye2DLocation( void );
	Ogre::Vector2 getRightEye2DLocation( void );
	float getPupillaryDistance( void );
};

#endif // Face_h__
