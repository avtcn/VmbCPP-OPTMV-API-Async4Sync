/*=============================================================================
  Copyright (C) 2012 - 2016 Allied Vision Technologies.  All Rights Reserved.

  Redistribution of this file, in original or modified form, without
  prior written consent of Allied Vision Technologies is prohibited.

-------------------------------------------------------------------------------

  File:        ListFeatures.h

  Description: The ListFeatures example will list all available features of a
               camera that are found by VimbaCPP.

-------------------------------------------------------------------------------

  THIS SOFTWARE IS PROVIDED BY THE AUTHOR "AS IS" AND ANY EXPRESS OR IMPLIED
  WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF TITLE,
  NON-INFRINGEMENT, MERCHANTABILITY AND FITNESS FOR A PARTICULAR  PURPOSE ARE
  DISCLAIMED.  IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR ANY DIRECT,
  INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
  (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
  LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED
  AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR
  TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
  OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.

=============================================================================*/

#ifndef AVT_VMBAPI_EXAMPLES_SCICAMERA
#define AVT_VMBAPI_EXAMPLES_SCICAMERA

#include <string>


#include "VimbaCPP/Include/VimbaCPP.h" 
#include "FrameObserver.h"

using namespace AVT::VmbAPI;

class SciCamera
{
public: // OPTMV API
	SciCamera();
	~SciCamera(); 

	VmbErrorType GetCameraList(CameraPtrVector &rCameras);

	VmbErrorType OpenCamera(const char * strCameraID);
	VmbErrorType CloseCamera();


	long StartAcquire(int mode = 1);
	long StopAcquire();

	long Shot(std::vector<VmbUchar_t> & photo, int msTimeout = 5000);

public: // Joe Ge added
	VmbErrorType GetCameraID(std::string &rStrID);

private:
	CameraPtr m_pCamera;
	int       m_nCameraMode;

	//IFrameObserver

	VimbaSystem& sys;

	//VmbErrorType        PrepareCamera();
    FrameObserver*      m_pFrameObserver;           // Every camera has its own frame observer

	bool SetInternalMode(int mode);

	bool TriggerSoftwareSignal(); 
	long WaitForNextImageReady(int ms);

	std::vector<VmbUchar_t> GetImage(); 
	std::vector<VmbUchar_t> GetImage(int & nBufferSize);


};






#endif
