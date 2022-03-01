
#include <iostream>
#include <iomanip>

#include "FrameObserver.h"

//
// We pass the camera that will deliver the frames to the constructor
//
// Parameters:
//  [in]    pCamera             The camera the frame was queued at
//
FrameObserver::FrameObserver(CameraPtr pCamera, bool bRGBValue, int mode)
    :   IFrameObserver( pCamera )
    ,   m_bRGB( bRGBValue )
	,   m_modeTrigger(mode)
{ 
	hSemaphore = CreateSemaphore(NULL, 0, 1, "Buffered_Images_Amount"); 
}


FrameObserver::~FrameObserver()
{
	if (hSemaphore)
	{
		CloseHandle(hSemaphore);
		hSemaphore = NULL;
	}
}

//
// Gets the current timestamp for interval measurement
//
double FrameObserver::GetTime()
{
    double dTime = 0.0;


    return dTime;
}

//
// Prints out frame parameters such as 
// - width
// - height
// - pixel format
//
// Parameters:
//  [in]    pFrame          The frame to work on
//
void PrintFrameInfo( const FramePtr &pFrame )
{
    std::cout<<" Size:";
    VmbUint32_t     nWidth = 0;
    VmbErrorType    res;
    res = pFrame->GetWidth(nWidth);
    if( VmbErrorSuccess == res )
    {
        std::cout<<nWidth;
    }
    else
    {
        std::cout<<"?";
    }

    std::cout<<"x";
    VmbUint32_t nHeight = 0;
    res = pFrame->GetHeight(nHeight);
    if( VmbErrorSuccess == res )
    {
        std::cout<< nHeight;
    }
    else
    {
        std::cout<<"?";
    }

    std::cout<<" Format:";
    VmbPixelFormatType ePixelFormat = VmbPixelFormatMono8;
    res = pFrame->GetPixelFormat( ePixelFormat );
    if( VmbErrorSuccess == res )
    {
        std::cout<<"0x"<<std::hex<<ePixelFormat<<std::dec;
    }
    else
    {
        std::cout<<"?";
    }
}

//
// Prints out frame status codes as readable status messages
//
// Parameters:
//  [in]    eFrameStatus    The error code to be converted and printed out
//
void PrintFrameStatus( VmbFrameStatusType eFrameStatus )
{
    switch( eFrameStatus )
    {
    case VmbFrameStatusComplete:
        std::cout<<"Complete";
        break;

    case VmbFrameStatusIncomplete:
        std::cout<<"Incomplete";
        break;

    case VmbFrameStatusTooSmall:
        std::cout<<"Too small";
        break;

    case VmbFrameStatusInvalid:
        std::cout<<"Invalid";
        break;

    default:
        std::cout<<"unknown frame status";
        break;
    }
}


//
// This is our callback routine that will be executed on every received frame.
// Triggered by the API.
//
// Parameters:
//  [in]    pFrame          The frame returned from the API
//
void FrameObserver::FrameReceived( const FramePtr pFrame )
{
    if(! SP_ISNULL( pFrame ) )
    {

        VmbFrameStatusType status;
        VmbErrorType Result;
        Result = SP_ACCESS( pFrame)->GetReceiveStatus( status);
        if( VmbErrorSuccess == Result && VmbFrameStatusComplete == status)
		{
			VmbUint32_t nImgSize = 0;
			pFrame->GetImageSize(nImgSize);
            std::vector<VmbUchar_t> TransformedData(nImgSize);

			VmbUchar_t *DataBegin = NULL; 
            if ( m_bRGB )
            {
				// TODO: color image processing 
            }
			else
			{
				// For mono8 image type 
				//Result = TransformImage(pFrame, TransformedData, "MONO8"); 
				pFrame->GetImage(DataBegin);
				memcpy(&TransformedData[0], DataBegin, nImgSize); 
			}

			m_mutex.Lock();

			// Deleted the oldest buffers to avoid memory flood
			while (m_framesQueue.size() > 20) {
				std::cout << "WARNING: buffered frames flood size = " << m_framesQueue.size() << "\n";
                m_framesQueue.pop();
                m_framesQueue.empty();
			}

			m_framesQueue.push(TransformedData); 

			m_mutex.Unlock();

			// notify the main thread that the frame buffer is ready
			ReleaseSemaphore(hSemaphore, 1, NULL);

			if (VmbErrorSuccess == Result && TransformedData.size() >= 3)
			{
				/*
				std::cout << "Mono image Transform to buffer and print first three bytes : \n";
				char old_fill_char = std::cout.fill('0');
				std::cout<<std::hex <<"1st = 0x"<<std::setw(2)<<(int)TransformedData[0]<<" "
									<<"2nd = 0x"<<std::setw(2)<<(int)TransformedData[1]<<" "
									<<"3rd = 0x"<<std::setw(2)<<(int)TransformedData[2]<<std::dec<<"\n";
				std::cout.fill( old_fill_char );

				std::cout << std::dec << "image size = " << TransformedData.size() << ", Queue Size = " << m_framesQueue.size() << "\n";
				std::cout << std::dec;
				*/
			}
			else
			{
				std::cout << "Transformation failed.\n";
			}

        }
        else
        {
            std::cout<<"frame incomplete\n";
        }
    }
    else
    {
        std::cout <<" frame pointer NULL\n";
    }

    m_pCamera->QueueFrame( pFrame );
}


long FrameObserver::WaitForImageReady(int timeout_milliseconds) 
{ 
	return WaitForSingleObject(hSemaphore, timeout_milliseconds); 
}



std::vector<VmbUchar_t> FrameObserver::GetImage()
{
	std::vector<VmbUchar_t> frameReturn;

	m_mutex.Lock();

	if(m_framesQueue.size() > 0) {
		frameReturn = m_framesQueue.front(); // Get first element of queue
		m_framesQueue.pop(); // Delete first element of the queue
	} 

	m_mutex.Unlock(); 

	return frameReturn;
}


std::vector<VmbUchar_t> FrameObserver::GetImage(int & nBufferSize)
{
	std::vector<VmbUchar_t> frameReturn;

	m_mutex.Lock();

	nBufferSize = m_framesQueue.size();

	if(m_framesQueue.size() > 0) {
		frameReturn = m_framesQueue.front(); // Get first element of queue
		m_framesQueue.pop(); // Delete first element of the queue
	} 

	m_mutex.Unlock(); 

	return frameReturn;
}



