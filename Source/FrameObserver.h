
#ifndef AVT_VMBAPI_EXAMPLES_FRAMEOBSERVER
#define AVT_VMBAPI_EXAMPLES_FRAMEOBSERVER

#include <queue>
#include "VimbaCPP/Include/VimbaCPP.h"
//#include "ProgramConfig.h"
#ifdef WIN32
#include <Windows.h>
#endif //WIN32

using namespace AVT::VmbAPI;

class FrameObserver : virtual public IFrameObserver
{
public:
    //
    // We pass the camera that will deliver the frames to the constructor
    //
    // Parameters:
    //  [in]    pCamera             The camera the frame was queued at
    //  [in]    eFrameInfos         Indicates how the frame will be displayed
    //  [in]    eColorProcessing    Indicates how color processing is applied
    //
    //FrameObserver( CameraPtr pCamera, FrameInfos eFrameInfos, ColorProcessing eColorProcessing, bool bRGBValue ); 
	FrameObserver(CameraPtr pCamera, bool bRGBValue, int mode);
	~FrameObserver();
    
    //
    // This is our callback routine that will be executed on every received frame.
    // Triggered by the API.
    //
    // Parameters:
    //  [in]    pFrame          The frame returned from the API
    //
    virtual void FrameReceived( const FramePtr pFrame );

	long WaitForImageReady(int timeout_milliseconds);

	std::vector<VmbUchar_t> GetImage(); 
	std::vector<VmbUchar_t> GetImage(int & nBufferSize); 

private:
    //void ShowFrameInfos( const FramePtr & );

    double GetTime();
    template <typename T>
    class ValueWithState
    {
    private:
        T m_Value;
        bool m_State;
    public:
        ValueWithState()
            : m_State( false )
        {}
        ValueWithState( T &value )
            : m_Value ( value )
            , m_State( true )
        {}
        const T& operator()() const
        {
            return m_Value;
        }
        void operator()( const T &value )
        {
            m_Value = value;
            m_State = true;
        }
        bool IsValid() const
        {
            return m_State;
        }
        void Invalidate()
        {
            m_State = false;
        }
    };
    const bool                  m_bRGB;
    ValueWithState<double>      m_FrameTime;
    ValueWithState<VmbUint64_t> m_FrameID;

	Mutex                                          m_mutex; 
	std::queue<std::vector<VmbUchar_t> >           m_framesQueue; 
	int                                            m_modeTrigger; 
	// semaphore to count the amount of the buffered images
	HANDLE                                         hSemaphore;

};


#endif
