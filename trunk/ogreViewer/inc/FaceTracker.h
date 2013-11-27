//------------------------------------------------------------------------------
// Modification of "FTHelper.h" from Microsoft's Toolkit
// Kinect stuff: http://msdn.microsoft.com/en-us/library/jj130970
//------------------------------------------------------------------------------

#pragma once
#include <FaceTrackLib.h>
#include "KinectSensor.h"

typedef void (*FTHelperCallBack)(PVOID lpParam);

class FaceTracker
{


private:
    KinectSensor                m_KinectSensor;
    BOOL                        m_isKinectSensorPresent;
    IFTFaceTracker*             m_pFaceTracker;
    //HWND                        m_hWnd; // i think this is not needed anymore!
    IFTResult*                  m_pFTResult;
    IFTImage*                   m_pColorImage;
    IFTImage*                   m_pDepthImage;
    FT_VECTOR3D                 m_hint3D[2];
    bool                        m_LastTrackSucceeded;
    float                       m_XCenterFace;
    float                       m_YCenterFace;
    HANDLE                      m_hFaceTrackingThread;
    BOOL                        m_DrawMask;
    NUI_IMAGE_TYPE              m_depthType;
    NUI_IMAGE_RESOLUTION        m_depthRes;
    BOOL                        m_bNearMode;
    BOOL                        m_bFallbackToDefault;
    BOOL                        m_bSeatedSkeletonMode;
    NUI_IMAGE_TYPE              m_colorType;
    NUI_IMAGE_RESOLUTION        m_colorRes;
    FT_CAMERA_CONFIG*           m_pDepthCamConfig;
    FT_CAMERA_CONFIG*           m_pColorCamConfig;

    IFTModel*                   m_pFaceModel;
public:
    FT_TRIANGLE *               m_pTriangles;
    FT_VECTOR3D *               m_pVertices;
    FT_VECTOR2D                *m_pPoints; // 2D points for GetProjectedShape
    unsigned int                m_numVertices,
                                m_numTriangles,
                                m_numPoints;
    bool                        m_isFaceModelConverged;
    bool                        m_isTextureUpdated; // indicates if a new image is ready to copy  into a render texture.

    BOOL SubmitFraceTrackingResult(IFTResult* pResult);
    void SetCenterOfImage(IFTResult* pResult);
    void CheckCameraInput();

public:
    FaceTracker();
    ~FaceTracker();

    HRESULT start( );
    HRESULT stop();
    IFTResult* GetResult()      { return(m_pFTResult);}
    BOOL IsKinectPresent()      { return(m_isKinectSensorPresent);}
    IFTImage* GetColorImage()   { return(m_pColorImage); }
    IFTImage* GetDepthImage()   { return(m_pDepthImage); }
    float GetXCenterFace()      { return(m_XCenterFace);}
    float GetYCenterFace()      { return(m_YCenterFace);}
    void SetDrawMask(BOOL drawMask) { m_DrawMask = drawMask;}
    BOOL GetDrawMask()          { return(m_DrawMask);}
    IFTFaceTracker* GetTracker() { return(m_pFaceTracker);}
    FT_CAMERA_CONFIG* GetDepthCameraConfig( void );
    FT_CAMERA_CONFIG* GetColorCameraConfig( void );
    void update();
    IFTModel* getFaceModel();
    void update3DFaceModel( IFTResult* pResult, bool isTransformed = true, bool isAnimated = true, bool isShapedTransformed = true  );
    void update2DFaceModel( IFTResult* pResult, bool isTransformed = true, bool isAnimated = true, bool isShapeTransformed = true  );
    bool IsFaceDetected( ){ return m_LastTrackSucceeded; }
    KinectSensor * getKinectSensor( void ) { return &m_KinectSensor; }  

    // some debugging stuff
    void write3DFaceModelObjToFile( const std::string& rName, bool useCurrent = true, IFTResult* pTrackingResult = 0, bool isTransformed = true, bool isAnimated = true, bool isShapeTransformed = true );

    static void writeObjFile( const std::string &rName, unsigned int numVertices,
        FT_VECTOR3D * pVertices, unsigned int numTriangles, FT_TRIANGLE * pTriangles );
    void writeTransform3DFaceModelToFile( const std::string& rFileName );
    void write2DFaceModelToImage( IFTImage* pDstImage );

};
