//------------------------------------------------------------------------------
// Modification of "FTHelper.h" from Microsoft's Toolkit
//------------------------------------------------------------------------------

// Windows Header Files:
#include <windows.h>
#include <iostream>
#include <fstream>
// C RunTime Header Files
#include <stdio.h>
#include <stdlib.h>
#include <malloc.h>
#include <memory.h>
#include <tchar.h>
#include <crtdbg.h>

#include "FaceTracker.h"

//#include "Visualize.h"

#ifdef SAMPLE_OPTIONS
#include "Options.h"
#else
#include <iosfwd>
PVOID _opt = NULL;
#endif

FaceTracker::FaceTracker()
    :m_pDepthCamConfig( 0 ),
    m_pColorCamConfig( 0 ),
    m_pFaceModel( 0 ),
    m_pTriangles( 0 ),
    m_pVertices( 0 ),
    m_numVertices( 0 ),
    m_numTriangles( 0 ),
    m_numPoints( 0 ),
    m_pPoints( 0 ),
    m_isTextureUpdated( false )
{
    m_pFaceTracker = 0;
    //m_hWnd = NULL;
    m_pFTResult = NULL;
    m_pColorImage = NULL;
    m_pDepthImage = NULL;
    m_LastTrackSucceeded = false;
    m_XCenterFace = 0;
    m_YCenterFace = 0;
    m_DrawMask = TRUE;
    m_depthType = NUI_IMAGE_TYPE_DEPTH_AND_PLAYER_INDEX;
    m_depthRes = NUI_IMAGE_RESOLUTION_640x480;
    m_bNearMode = true;
    m_bFallbackToDefault = true;
    m_colorType = NUI_IMAGE_TYPE_COLOR;
    m_colorRes = NUI_IMAGE_RESOLUTION_640x480;
    m_bSeatedSkeletonMode = true;


    // init stuff for Kinect, like finding the sensor ...
    m_pDepthCamConfig = new FT_CAMERA_CONFIG();
    m_pColorCamConfig = new FT_CAMERA_CONFIG();


    // Try to get the Kinect camera to work
    HRESULT hr = m_KinectSensor.Init(m_depthType, m_depthRes, m_bNearMode, m_bFallbackToDefault, m_colorType, m_colorRes, m_bSeatedSkeletonMode);
    if (SUCCEEDED(hr))
    {
        m_isKinectSensorPresent = TRUE;
        m_KinectSensor.GetVideoConfiguration( m_pColorCamConfig );
        m_KinectSensor.GetDepthConfiguration( m_pDepthCamConfig );
        m_hint3D[0] = m_hint3D[1] = FT_VECTOR3D(0, 0, 0);
    }
    else
    {
        m_isKinectSensorPresent = FALSE;
        CHAR errorText[MAX_PATH];
        //ZeroMemory(errorText, sizeof(WCHAR) * MAX_PATH);
        sprintf_s(errorText, "Could not initialize the Kinect sensor. hr=0x%x\n", hr);
        std::cerr << L"Face Tracker Initialization Error\n" << errorText << std::endl;
        //MessageBoxW(m_hWnd, errorText, L"Face Tracker Initialization Error\n", MB_OK);
    }

}

FaceTracker::~FaceTracker()
{
    // stop tracker if running
    stop();

    delete m_pDepthCamConfig;
    delete m_pColorCamConfig;
}

HRESULT FaceTracker::start()
{
    HRESULT hr;

    if( !m_isKinectSensorPresent )
    {
        std::cerr << L"Face Tracker Initialization Error\n Could not initialize the Kinect sensor." << std::endl;
        return 1;
    }

    // Try to start the face tracker.
    m_pFaceTracker = FTCreateFaceTracker(_opt);
    if (!m_pFaceTracker)
    {
        std::cerr << L"Could not create the face tracker.\n" << L"Face Tracker Initialization Error\n" << std::endl;
        //MessageBoxW(m_hWnd, L"Could not create the face tracker.\n", L"Face Tracker Initialization Error\n", MB_OK);
        return 2;
    }

    hr = m_pFaceTracker->Initialize( m_pColorCamConfig, m_pDepthCamConfig, NULL, NULL); 
    if (FAILED(hr))
    {
        CHAR path[512], buffer[1024];
        GetCurrentDirectory(ARRAYSIZE(path), path);
        sprintf_s(buffer, "Could not initialize face tracker (%s). hr=0x%x", path, hr);

        std::cerr << buffer << L"Face Tracker Initialization Error\n" << std::endl;
        //MessageBoxW(m_hWnd, /*L"Could not initialize the face tracker.\n"*/ buffer, L"Face Tracker Initialization Error\n", MB_OK);

        return 3;
    }
	
    hr = m_pFaceTracker->CreateFTResult(&m_pFTResult);
    if (FAILED(hr) || !m_pFTResult)
    {
        std::cerr << "Could not initialize the face tracker result.\n" << L"Face Tracker Initialization Error\n" << std::endl;
        // MessageBoxW(m_hWnd, L"Could not initialize the face tracker result.\n", L"Face Tracker Initialization Error\n", MB_OK);
        return 4;
    }

    // Initialize the RGB image.
    m_pColorImage = FTCreateImage();
    if (!m_pColorImage || FAILED(hr = m_pColorImage->Allocate(m_pColorCamConfig->Width, m_pColorCamConfig->Height, KinectSensor::VIDEO_BUFFER_FORMAT )))
    {
        return 5;
    }

    if( m_pDepthCamConfig )
    {
        m_pDepthImage = FTCreateImage();
        if (!m_pDepthImage || FAILED(hr = m_pDepthImage->Allocate(m_pDepthCamConfig->Width, m_pDepthCamConfig->Height, KinectSensor::DEPTH_BUFFER_FORMAT )))
        {
            return 6;
        }
    }

    m_pFaceTracker->SetShapeComputationState( TRUE );

    SetCenterOfImage(NULL);
    m_LastTrackSucceeded = false;
    return S_OK;
}

HRESULT FaceTracker::stop()
{
    if( m_isKinectSensorPresent )
    {
        m_pFaceTracker->Release();
        m_pFaceTracker = NULL;

        if(m_pColorImage)
        {
            m_pColorImage->Release();
            m_pColorImage = NULL;
        }

        if(m_pDepthImage) 
        {
            m_pDepthImage->Release();
            m_pDepthImage = NULL;
        }

        if(m_pFTResult)
        {
            m_pFTResult->Release();
            m_pFTResult = NULL;
        }
        m_KinectSensor.Release();
        return 0;
    }

    if( m_pFaceModel )
    {
        m_pFaceModel->Release();
        m_pFaceModel = NULL;
    }

    if( 0 != m_pVertices ) 
    {
        delete m_pVertices;
    }


    return S_FALSE;
}

BOOL FaceTracker::SubmitFraceTrackingResult(IFTResult* pResult)
{
    if (pResult != NULL && SUCCEEDED(pResult->GetStatus()))
    {

        if (m_DrawMask)
        {
            FLOAT* pSU = NULL;
            UINT numSU;
            BOOL suConverged;
            m_pFaceTracker->GetShapeUnits(NULL, &pSU, &numSU, &suConverged);
            POINT viewOffset = {0, 0};
            FT_CAMERA_CONFIG cameraConfig;
            if (m_isKinectSensorPresent)
            {
                m_KinectSensor.GetVideoConfiguration(&cameraConfig);
            }
            else
            {
                cameraConfig.Width = 640;
                cameraConfig.Height = 480;
                cameraConfig.FocalLength = 500.0f;
            }
            IFTModel* ftModel;
            HRESULT hr = m_pFaceTracker->GetFaceModel(&ftModel);
            if (SUCCEEDED(hr))
            {
                //hr = VisualizeFaceModel(m_colorImage, ftModel, &cameraConfig, pSU, 1.0, viewOffset, pResult, 0x00FFFF00);
                ftModel->Release();
            }
        }
    }
    return TRUE;
}

// We compute here the nominal "center of attention" that is used when zooming the presented image.
void FaceTracker::SetCenterOfImage(IFTResult* pResult)
{
    float centerX = ((float)m_pColorImage->GetWidth())/2.0f;
    float centerY = ((float)m_pColorImage->GetHeight())/2.0f;
    if (pResult)
    {
        if (SUCCEEDED(pResult->GetStatus()))
        {
            RECT faceRect;
            pResult->GetFaceRect(&faceRect);
            centerX = (faceRect.left+faceRect.right)/2.0f;
            centerY = (faceRect.top+faceRect.bottom)/2.0f;
        }
        m_XCenterFace += 0.02f*(centerX-m_XCenterFace);
        m_YCenterFace += 0.02f*(centerY-m_YCenterFace);
    }
    else
    {
        m_XCenterFace = centerX;
        m_YCenterFace = centerY;
    }
}

// Get a video image and process it.
void FaceTracker::update()
{
    static int s_lastTotalFrames = -1; 

    HRESULT hrFT = E_FAIL;

    if( !m_LastTrackSucceeded )
    {
        m_pFTResult->Reset();
    }

    // check if sensor is present and we can get the video buffer
    // additionally don't get the buffer if no new frame is present (s_lastTotalFrames)
    if (m_isKinectSensorPresent && m_KinectSensor.GetVideoBuffer() ) // && s_lastTotalFrames!= m_KinectSensor.getTotalFrames() )
    {
        s_lastTotalFrames = m_KinectSensor.getTotalFrames();
        m_isTextureUpdated = true;

        HRESULT hrCopy = m_KinectSensor.GetVideoBuffer()->CopyTo(m_pColorImage, NULL, 0, 0);
        if (SUCCEEDED(hrCopy) && m_KinectSensor.GetDepthBuffer())
        {
            hrCopy = m_KinectSensor.GetDepthBuffer()->CopyTo(m_pDepthImage, NULL, 0, 0);
        }
        // Do face tracking
        if (SUCCEEDED(hrCopy))
        {
            FT_SENSOR_DATA sensorData(m_pColorImage, m_pDepthImage, m_KinectSensor.GetZoomFactor(), m_KinectSensor.GetViewOffSet());
            
            FT_VECTOR3D* hint = NULL;
            if (SUCCEEDED(m_KinectSensor.GetClosestHint(m_hint3D)))
            {
                hint = m_hint3D;
            } 

            if (m_LastTrackSucceeded)
            {
                hrFT = m_pFaceTracker->ContinueTracking(&sensorData, hint, m_pFTResult);
            }
            else
            {
                m_pFTResult->Reset();
                hrFT = m_pFaceTracker->StartTracking(&sensorData, NULL, hint, m_pFTResult);
            }
        }
    }

    m_LastTrackSucceeded = SUCCEEDED(hrFT) && SUCCEEDED(m_pFTResult->GetStatus());


   
    // if tracking succeeded we can do something with the data. :)
    if( m_LastTrackSucceeded )
    {
        update3DFaceModel( m_pFTResult, true, true, true );
        update2DFaceModel( m_pFTResult, true, true, true );
        //write2DFaceModelToImage( m_pColorImage );
    }
     
}

FT_CAMERA_CONFIG* FaceTracker::GetDepthCameraConfig( void )
{
    return m_pDepthCamConfig;
}

FT_CAMERA_CONFIG* FaceTracker::GetColorCameraConfig( void )
{
    return m_pColorCamConfig;
}

IFTModel* FaceTracker::getFaceModel()
{
    // lazy initialization of the face-model which is only fetched once from 
    // the facetracker
    if( 0 == m_pFaceModel && 0 != m_pFaceTracker )
    {
        m_pFaceTracker->GetFaceModel( &m_pFaceModel );
    }

    return m_pFaceModel;
}

void FaceTracker::update3DFaceModel( IFTResult* pTrackingResult, bool isTransformed, bool isAnimated, bool isShapeTransformed ) 
{
    float tmpSUCoefs[] =  { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 };
    FLOAT* pSUCoefs = tmpSUCoefs;
    UINT numSU = 11;
    BOOL suConverged;
    if( isShapeTransformed )
    {
        m_pFaceTracker->GetShapeUnits(NULL, &pSUCoefs, &numSU, &suConverged );
        m_isFaceModelConverged = suConverged == TRUE;
    }

    IFTModel* pFTModel;
    HRESULT hr = m_pFaceTracker->GetFaceModel( &pFTModel );
    if (SUCCEEDED(hr))
    {
        
        unsigned int numAUCoefs = 6;
        float tmpAUCoefs[] =  { 0, 0, 0, 0, 0, 0 };
        float *pAUCoefs = tmpAUCoefs;
        if( pTrackingResult && isAnimated )
        {
            pTrackingResult->GetAUCoefficients( &pAUCoefs, &numAUCoefs );
        }

        float scale = 1, pRot[3] = { 0,0,0 }, pTrans[3] = { 0,0,0 };
        if( pTrackingResult &&  isTransformed )
        {
            pTrackingResult->Get3DPose( &scale, pRot, pTrans );
        }

        if( pFTModel->GetVertexCount() != m_numVertices )
        {
            if( 0 != m_pVertices ) delete m_pVertices;
            m_numVertices = pFTModel->GetVertexCount();
            m_pVertices = new FT_VECTOR3D[ m_numVertices ];
        }
        
        // get the vertexpositions
        unsigned int numSUCoefs = pFTModel->GetSUCount();
        HRESULT hr = pFTModel->Get3DShape( pSUCoefs, numSUCoefs, pAUCoefs, numAUCoefs, scale, pRot, pTrans, m_pVertices, m_numVertices );
        if( !SUCCEEDED(hr) )
        {
            printf( "problem while getting the 3DShape in FaceTracker.cpp" );
        }
        
        // get the indices of the triangles
        // pointer doesn't change but get it anyway
        pFTModel->GetTriangles( &m_pTriangles, &m_numTriangles );

        pFTModel->Release();

    }
}

void FaceTracker::update2DFaceModel( IFTResult* pTrackingResult, bool isTransformed, bool isAnimated, bool isShapeTransformed ) 
{
    float tmpSUCoefs[] =  { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 };
    FLOAT* pSUCoefs = tmpSUCoefs;
    UINT numSU = 11;
    BOOL suConverged;
    if( isShapeTransformed )
    {
        m_pFaceTracker->GetShapeUnits(NULL, &pSUCoefs, &numSU, &suConverged );
        m_isFaceModelConverged = suConverged == TRUE;
    }       

    POINT viewOffset = {0, 0};
    IFTModel* pFTModel;
    HRESULT hr = m_pFaceTracker->GetFaceModel( &pFTModel );
    if (SUCCEEDED(hr))
    {
        
        unsigned int numAUCoefs = 6;
        float tmpAUCoefs[] =  { 0, 0, 0, 0, 0, 0 };
        float *pAUCoefs = tmpAUCoefs;
        if( pTrackingResult && isAnimated )
        {
            pTrackingResult->GetAUCoefficients( &pAUCoefs, &numAUCoefs );
        }

        float scale = 1, pRot[3] = { 0,0,0 }, pTrans[3] = { 0,0,0 };
        if( pTrackingResult &&  isTransformed )
        {
            pTrackingResult->Get3DPose( &scale, pRot, pTrans );
        }

        if( pFTModel->GetVertexCount() != m_numPoints )
        {
            if( 0 != m_pPoints ) delete m_pPoints;
            m_numPoints = pFTModel->GetVertexCount();
            m_pPoints = new FT_VECTOR2D[ m_numPoints ];
        }
        // get the vertexpositions
        unsigned int numSUCoefs = pFTModel->GetSUCount();
        HRESULT hr = pFTModel->GetProjectedShape( m_pColorCamConfig, 1.0, viewOffset, pSUCoefs, numSUCoefs, pAUCoefs, numAUCoefs, scale, pRot, pTrans, m_pPoints, m_numPoints );
        if( !SUCCEEDED(hr) )
        {
            printf( "problem while getting the 3DShape in FaceTracker.cpp" );
        }
        
        // get the indices of the triangles
        // pointer doesn't change but get it anyway
        pFTModel->GetTriangles( &m_pTriangles, &m_numTriangles );

        pFTModel->Release();

    }
}

// debugging functions
// -----------------------------------------------------------------------------
void FaceTracker::write3DFaceModelObjToFile( const std::string& rName, bool useCurrent, 
    IFTResult * pTrackingResult, bool isTransformed, bool isAnimated, bool isShapeTransformed )
{
    if( !useCurrent )
    {
        update3DFaceModel( pTrackingResult, isTransformed, isAnimated, isShapeTransformed );
    }

     writeObjFile( rName, m_numVertices, m_pVertices, m_numTriangles, m_pTriangles);
}

void FaceTracker::writeObjFile( const std::string &rName, unsigned int numVertices, FT_VECTOR3D * pVertices, unsigned int numTriangles, FT_TRIANGLE * pTriangles )
{
    std::ofstream file;
    file.open( rName.c_str() );
    file << "# OBJ export of FaceTracker mesh " << std::endl;

    // vertices
    file << "# vertices (" << numVertices << ")" << std::endl;
    file << "o Face" << std::endl;
    for( unsigned int i = 0; i < numVertices; ++ i )
    {
        file << "v " << pVertices[i].x << " " << pVertices[i].y << " " << pVertices[i].z << std::endl;
    }
    // indices
    //file << "# indices, faces (" << numTriangles << ") " << std::endl;
    file << "s off" << std::endl;
    for( unsigned int i = 0; i < numTriangles; ++ i )
    {
        // keep kji order so meshes will be shown (backface culling)
        file << "f " <<  pTriangles[i].i+1  << " " <<
            pTriangles[i].j+1 << " " << pTriangles[i].k+1 << std::endl; 
    }
    file.close();
}

void FaceTracker::writeTransform3DFaceModelToFile( const std::string& rFileName )
{
    float scale = 1, pRot[3] = { 0,0,0 }, pTrans[3] = { 0,0,0 };
    if( m_pFTResult )
    {
        m_pFTResult->Get3DPose( &scale, pRot, pTrans );

        std::ofstream file;
        file.open( rFileName.c_str() );
        file << "# transformations of the face mesh" << std::endl;

        file << "scale: " << scale << std::endl;
        file << "rotation: " << pRot[0] <<", "<< pRot[1] << ", " << pRot[2] << std::endl;
        file << "translate: " << pTrans[0] <<", "<< pTrans[1] << ", " << pTrans[2] << std::endl;

        file.close();
    }
}


void FaceTracker::write2DFaceModelToImage( IFTImage* pDstImage )
{
    POINT* p3DMdl   = reinterpret_cast<POINT*>(_malloca(sizeof(POINT) * m_numVertices ));
    if (p3DMdl)
    {
        for (UINT i = 0; i < m_numVertices; ++i)
        {
            p3DMdl[i].x = LONG(m_pPoints[i].x + 0.5f);
            p3DMdl[i].y = LONG(m_pPoints[i].y + 0.5f);
        }

        {
            struct EdgeHashTable
            {
                UINT32* pEdges;
                UINT edgesAlloc;

                void Insert(int a, int b) 
                {
                    UINT32 v = (min(a, b) << 16) | max(a, b);
                    UINT32 index = (v + (v << 8)) * 49157, i;
                    for (i = 0; i < edgesAlloc - 1 && pEdges[(index + i) & (edgesAlloc - 1)] && v != pEdges[(index + i) & (edgesAlloc - 1)]; ++i)
                    {
                    }
                    pEdges[(index + i) & (edgesAlloc - 1)] = v;
                }
            } eht;

            eht.edgesAlloc = 1 << UINT(log(2.f * (1 + m_numVertices + m_numTriangles )) / log(2.f));
            eht.pEdges = reinterpret_cast<UINT32*>(_malloca(sizeof(UINT32) * eht.edgesAlloc));
            if (eht.pEdges)
            {
                ZeroMemory(eht.pEdges, sizeof(UINT32) * eht.edgesAlloc);
                for (UINT i = 0; i < m_numTriangles ; ++i)
                { 
                    eht.Insert(m_pTriangles[i].i, m_pTriangles[i].j);
                    eht.Insert(m_pTriangles[i].j, m_pTriangles[i].k);
                    eht.Insert(m_pTriangles[i].k, m_pTriangles[i].i);
                }
                for (UINT i = 0; i < eht.edgesAlloc; ++i)
                {
                    if(eht.pEdges[i] != 0)
                    {
                        pDstImage->DrawLine(p3DMdl[eht.pEdges[i] >> 16], p3DMdl[eht.pEdges[i] & 0xFFFF], 0xffffffff, 1);
                    }
                }
                _freea(eht.pEdges);
            }

            // Render the face rect in magenta
//            RECT rectFace;
//            hr = pAAMRlt->GetFaceRect(&rectFace);
//            if (SUCCEEDED(hr))
//            {
//                POINT leftTop = {rectFace.left, rectFace.top};
//                POINT rightTop = {rectFace.right - 1, rectFace.top};
//                POINT leftBottom = {rectFace.left, rectFace.bottom - 1};
//                POINT rightBottom = {rectFace.right - 1, rectFace.bottom - 1};
//                UINT32 nColor = 0xff00ff;
//                SUCCEEDED(hr = pColorImg->DrawLine(leftTop, rightTop, nColor, 1)) &&
//                    SUCCEEDED(hr = pColorImg->DrawLine(rightTop, rightBottom, nColor, 1)) &&
//                    SUCCEEDED(hr = pColorImg->DrawLine(rightBottom, leftBottom, nColor, 1)) &&
//                    SUCCEEDED(hr = pColorImg->DrawLine(leftBottom, leftTop, nColor, 1));
//            }
        }
    }

        _freea(p3DMdl); 
}