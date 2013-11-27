#include "Face.h"
#include "FaceTrackLib.h"
#include "OgrePixelFormat.h"
#include "OgreStringConverter.h"


float Face::s_facePoseYAngleStepWidth = 5.0f; // Degrees

Face::Face( )
    : m_colorImageFileName( "" ),
    m_depthImageFileName( "" ),
    m_isFaceDetected( false ),
    m_bContainsDepthImage( false )
{
    setPose( 0,0,0, 0,0,0, 1 );
    setSpectaclesPose( 0,0,0, 0,0,0, 1 );
	setLeftEye3D(0,0,0);
	setRightEye3D(0,0,0);
	setLeftEye2D(0,0);
	setRightEye2D(0,0);
}

// copy constructor
Face::Face( const Face & src )
    : m_colorImageFileName( src.m_colorImageFileName ),
    m_depthImage( src.m_depthImage ),
    m_isFaceDetected( src.m_isFaceDetected ),
    m_pose( src.m_pose ),
    m_spectaclesPose( src.m_spectaclesPose ),
    m_bContainsDepthImage( src.m_bContainsDepthImage ),
	m_leftEye3D( src.m_leftEye3D ),
	m_rightEye3D( src.m_rightEye3D ),
	m_leftEye2D( src.m_leftEye2D ),
	m_rightEye2D( src.m_rightEye2D )

{
    // further things have to be done to copy the Images
    unsigned int size = src.m_colorImage.getNumFaces() * Ogre::PixelUtil::getMemorySize( src.m_colorImage.getWidth(), src.m_colorImage.getHeight(), src.m_colorImage.getDepth(), src.m_colorImage.getFormat() );
    Ogre::uchar * colorData = OGRE_ALLOC_T( Ogre::uchar,  size, Ogre::MEMCATEGORY_GENERAL );
    memcpy( colorData, src.m_colorImage.getData(), size );
    m_colorImage.loadDynamicImage( colorData, src.m_colorImage.getWidth(), src.m_colorImage.getHeight(), src.m_colorImage.getDepth(), src.m_colorImage.getFormat(), true );

    if( src.m_bContainsDepthImage )
    {
        size = src.m_depthImage.getNumFaces() * Ogre::PixelUtil::getMemorySize( src.m_depthImage.getWidth(), src.m_depthImage.getHeight(), src.m_depthImage.getDepth(), src.m_depthImage.getFormat() );
        Ogre::uchar * depthData = OGRE_ALLOC_T( Ogre::uchar,  size, Ogre::MEMCATEGORY_GENERAL );
        memcpy( depthData, src.m_depthImage.getData(), size );
        m_depthImage.loadDynamicImage( depthData, src.m_depthImage.getWidth(), src.m_depthImage.getHeight(), src.m_depthImage.getDepth(), src.m_depthImage.getFormat(), true );
    }
}


Face::~Face( )
{
}


const Face::Pose& Face::getPose( void ) const
{
    return m_pose;
}

void Face::setPose( Pose& rPose )
{
    m_pose = rPose;
}

void Face::setPose( float pTrans[], float pRot[],  float scale )
{
    m_pose.scaling = scale;
    m_pose.rotation[0] = pRot[0];
    m_pose.rotation[1] = pRot[1];
    m_pose.rotation[2] = pRot[2];
    m_pose.location[0] = pTrans[0];
    m_pose.location[1] = pTrans[1];
    m_pose.location[2] = pTrans[2];
}

void Face::setPose( float transX, float transY, float transZ, float rotX, float rotY, float rotZ, float scale )
{
    m_pose.scaling = scale;
    m_pose.rotation[0] = rotX;
    m_pose.rotation[1] = rotY;
    m_pose.rotation[2] = rotZ;
    m_pose.location[0] = transX;
    m_pose.location[1] = transY;
    m_pose.location[2] = transZ;
}

const Face::Pose& Face::getSpectaclesPose( void ) const
{
    return m_spectaclesPose;
}
       
void Face::setLeftEye3D(float x, float y, float z){
	m_leftEye3D.x = x;
	m_leftEye3D.y = y;
	m_leftEye3D.z = z;
}

void Face::setRightEye3D(float x, float y, float z){
	m_rightEye3D.x = x;
	m_rightEye3D.y = y;
	m_rightEye3D.z = z;
}

void Face::setLeftEye2D( float x, float y){
	m_leftEye2D.x = x;
	m_leftEye2D.y = y;
}

void Face::setRightEye2D( float x, float y){
	m_rightEye2D.x = x;
	m_rightEye2D.y = y;
}

void Face::setSpectaclesPose( Pose& rPose )
{
    m_spectaclesPose = rPose;
}

void Face::setSpectaclesPose( float pTrans[], float pRot[], float scale /*= 1 */ )
{
    m_spectaclesPose.scaling = scale;
    m_spectaclesPose.rotation[0] = pRot[0];
    m_spectaclesPose.rotation[1] = pRot[1];
    m_spectaclesPose.rotation[2] = pRot[2];
    m_spectaclesPose.location[0] = pTrans[0];
    m_spectaclesPose.location[1] = pTrans[1];
    m_spectaclesPose.location[2] = pTrans[2];
}

void Face::setSpectaclesPose( float transX, float transY, float transZ, float rotX, float rotY, float rotZ, float scale /*= 1 */ )
{
    m_spectaclesPose.scaling = scale;
    m_spectaclesPose.rotation[0] = rotX;
    m_spectaclesPose.rotation[1] = rotY;
    m_spectaclesPose.rotation[2] = rotZ;
    m_spectaclesPose.location[0] = transX;
    m_spectaclesPose.location[1] = transY;
    m_spectaclesPose.location[2] = transZ;
}

void Face::updateImages( IFTImage * pSrcColorImage, IFTImage * pSrcDepthImage, bool isFaceDetected )
{
    m_isFaceDetected = isFaceDetected;

    if( pSrcColorImage )
    {
        //FTIMAGEFORMAT format = pSrcColorImage->GetFormat();
        // if the FTIMAGEFORMAT is FTIMAGEFORMAT_UINT8_B8G8R8X8 then use Ogre::PF_X8R8G8B8!
        Ogre::PixelBox srcColorPix = Ogre::PixelBox( pSrcColorImage->GetWidth(), pSrcColorImage->GetHeight(), 1, Ogre::PF_X8R8G8B8, pSrcColorImage->GetBuffer() );
        m_colorImage.loadDynamicImage( (Ogre::uchar *)srcColorPix.data, srcColorPix.getWidth(), srcColorPix.getHeight(), 1, srcColorPix.format );
        //m_colorImage.save( "test_colorImage.png" ); <--- works!
        //m_colorTexturePtr->getBuffer()->blitFromMemory( srcColorPix );
    }

    if( pSrcDepthImage )
    {
        //FTIMAGEFORMAT format = pSrcDepthImage->GetFormat();
        Ogre::PixelBox srcDepthPix = Ogre::PixelBox( pSrcDepthImage->GetWidth(), pSrcDepthImage->GetHeight(), 1, Ogre::PF_L16, pSrcDepthImage->GetBuffer() );
        m_depthImage.loadDynamicImage( (Ogre::uchar *)srcDepthPix.data, srcDepthPix.getWidth(), srcDepthPix.getHeight(), 1, srcDepthPix.format );
        m_bContainsDepthImage = true;

        //m_depthImage.save( "depthImage.pfm" );
        //m_depthImage.save( "depthImage.bmp" );
    }
    else
    {
        m_bContainsDepthImage = false;
    }

    m_areTexturesChanged = true;
}

Ogre::Image & Face::getColorImage( void )
{
    return m_colorImage;
}


Ogre::Image & Face::getDepthImage( void )
{
    return m_depthImage;
}

bool Face::hasDepthImage()
{
    return m_bContainsDepthImage;
}

bool Face::areTexturesChanged( )
{
    return m_areTexturesChanged;
}

void Face::setAreTexturesChanged( bool value )
{
    m_areTexturesChanged = value;
}

std::string Face::getCurrentPoseName()
{
   float halfAngle = s_facePoseYAngleStepWidth / 2.0f;
   float yRotation = m_pose.rotation[1];

   float direction = yRotation / std::abs( yRotation ); // -1, 0 or 1

   int i = 0;
   while( i*s_facePoseYAngleStepWidth + halfAngle < std::abs( yRotation ) )
   {
       ++ i;
   }

    // construct name based on the above
    if( i <= 0 )
    {
        return "front";
    }
    else
    {
        std::string name = (direction > 0) ? "left" : "right";
        name.append( Ogre::StringConverter::toString( i*s_facePoseYAngleStepWidth, 0, 3, '_' ) );
        return name;
    }
}

Ogre::Vector3 Face::getRightEye3DLocation( void ){
	Ogre::Real x = m_rightEye3D[0];
	Ogre::Real y = m_rightEye3D[1];
	Ogre::Real z = m_rightEye3D[2];
	return Ogre::Vector3(x,y,z);
}

Ogre::Vector3 Face::getLeftEye3DLocation( void ){
	Ogre::Real x = m_leftEye3D[0];
	Ogre::Real y = m_leftEye3D[1];
	Ogre::Real z = m_leftEye3D[2];
	return Ogre::Vector3(x,y,z);
}

Ogre::Vector2 Face::getLeftEye2DLocation( void ){
	Ogre::Real x = m_leftEye2D[0];
	Ogre::Real y = m_leftEye2D[1];
	return Ogre::Vector2(x,y);
}

Ogre::Vector2 Face::getRightEye2DLocation( void ){
	Ogre::Real x = m_rightEye2D[0];
	Ogre::Real y = m_rightEye2D[1];
	return Ogre::Vector2(x,y);
}

float Face::getPupillaryDistance( void ){
	float dx = pow((m_leftEye3D[0] - m_rightEye3D[0]), 2);
    float dy = pow((m_leftEye3D[1] - m_rightEye3D[1]), 2);
    float dz = pow((m_leftEye3D[2] - m_rightEye3D[2]), 2);
	float pupillarydistance = sqrt(dx + dy + dz) * 1000;
	//float zdistance = 0.5f * (left_zs + right_zs);
	return pupillarydistance;
}