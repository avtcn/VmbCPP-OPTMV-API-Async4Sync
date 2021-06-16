/*=============================================================================
  Copyright (C) 2012 - 2016 Allied Vision Technologies.  All Rights Reserved.

  Redistribution of this file, in original or modified form, without
  prior written consent of Allied Vision Technologies is prohibited.

-------------------------------------------------------------------------------

  File:        ListFeatures.cpp

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

#include <sstream>
#include <iostream>
#include <vector>
#include <algorithm>

#include <SciCamera.h>

#include "VimbaCPP/Include/VimbaCPP.h"
#include "Common/StreamSystemInfo.h"
#include "Common/ErrorCodeToMessage.h"

#define NUM_FRAMES 5



SciCamera::SciCamera()
	:sys(VimbaSystem::GetInstance())	// Get a reference to the VimbaSystem singleton
{
}

SciCamera::~SciCamera()
{
}


VmbErrorType SciCamera::GetCameraList(CameraPtrVector &rCameras)
{
	return sys.GetCameras(rCameras);
}


VmbErrorType SciCamera::OpenCamera(const char * strCameraID)
{
	return sys.OpenCameraByID( strCameraID, VmbAccessModeFull, m_pCamera); // Get and open the camera, store it in m_pCamera

}

VmbErrorType SciCamera::CloseCamera()
{
	return m_pCamera->Close();
}

VmbErrorType SciCamera::GetCameraID(std::string &rStrID)
{
	return m_pCamera->GetID(rStrID);
}


long SciCamera::StartAcquire(int mode)
{
	// Set the GeV packet size to the highest possible value
	// (In this example we do not test whether this cam actually is a GigE cam)
	FeaturePtr pCommandFeature;
	if (VmbErrorSuccess == m_pCamera->GetFeatureByName("GVSPAdjustPacketSize", pCommandFeature))
	{
		if (VmbErrorSuccess == pCommandFeature->RunCommand())
		{
			bool bIsCommandDone = false;
			do
			{
				if (VmbErrorSuccess != pCommandFeature->IsCommandDone(bIsCommandDone))
				{
					break;
				}
			} while (false == bIsCommandDone);
		}
	}

	// set camera so that transform algorithms will never fail
	//VmbErrorType res = PrepareCamera();
	VmbErrorType res = VmbErrorSuccess;
	if (VmbErrorSuccess == res)
	{

		// TODO: set one of proper camera modes: 
		//
		//                  1. Software trigger
		//                  2. Hardware trigger
		//                  3. Free Run trigger 
		m_nCameraMode = mode; // trigger mode
		SetInternalMode(m_nCameraMode);

		// Create a frame observer for this camera (This will be wrapped in a shared_ptr so we don't delete it)
		m_pFrameObserver = new FrameObserver(m_pCamera, false, m_nCameraMode);
		// Start streaming
		res = m_pCamera->StartContinuousImageAcquisition(NUM_FRAMES, IFrameObserverPtr(m_pFrameObserver)); 

	}


	return 1;
}


long SciCamera::StopAcquire()
{
	// Stop streaming
    m_pCamera->StopContinuousImageAcquisition(); 

	return 1;
}


long SciCamera::Shot(std::vector<VmbUchar_t> & photo, int msTimeout)
{
	if (msTimeout < 1000)
		msTimeout = 1000;

	if (m_nCameraMode == 1)
	{
		// Software Trigger mode !!!!
		//std::cout << "Software trigger i = " << 0 << " ...\n";
		TriggerSoftwareSignal();

		// Wait unitl sw triggered image is ready or time out happen 
		if (WaitForNextImageReady(msTimeout) == 0)
		{
			std::cout << "Image is Ready !!! \n";

			// TODO: try to get the image triggered by above Software trigger
			int nBufferSize = -1;
			std::vector<VmbUchar_t> frame = GetImage(nBufferSize);
			photo = frame;

			//std::cout << "DATA of Image: " << (int)frame[0] << ", " << frame[1] << ", " << frame[2] << ". \n";
			char old_fill_char = std::cout.fill('0');
			std::cout << std::hex << "[SW] Captured Image Data: 1st = 0x" << (int)frame[0] << " "
				<< "2nd = 0x" << (int)frame[1] << " "
				<< "3rd = 0x" << (int)frame[2] << std::dec << "; nBufferSize = " << nBufferSize 
				<< ", ImageSize = " << frame.size()
				<< " \n";
			std::cout.fill(old_fill_char);
			std::cout << std::dec;

		}
		else
		{
			std::cout << "Failed to wait for nexe image !!!\n";
		}
	}
	else 
	{
		int nBufferSize = -1;
		// Free RUN mode
		if (m_nCameraMode == 3 && WaitForNextImageReady(msTimeout) == 0)
		{
			// try to get the image triggered by above Free Run trigger mode !!
			std::vector<VmbUchar_t> frame = GetImage(nBufferSize); 
			photo = frame;

			//std::cout << "Captured Image Data: " << frame[0] << ", " << frame[1] << ", " << frame[2] << ". \n"; 
			char old_fill_char = std::cout.fill('0');
			std::cout << std::hex << "[Free Run] Captured Image Data: 1st = 0x" << (int)frame[0] << " "
				<< "2nd = 0x" << (int)frame[1] << " "
				<< "3rd = 0x" << (int)frame[2] << std::dec << "; nBufferSize = " << nBufferSize 
				<< ", ImageSize = " << frame.size()
				<< " \n";
			std::cout.fill(old_fill_char);
			std::cout << std::dec;

		}
		// HW Trigger, waiting forever until the hardware trigger signal happens
		//  If dwMilliseconds is INFINITE, the function will return only when the object is signaled.
		// For HW trigger, we will wait until the HW trigger happen.
		else if (m_nCameraMode == 2 && WaitForNextImageReady(INFINITE) == 0)
		{
			// try to get the image triggered by above Hardware trigger mode !!!
			std::vector<VmbUchar_t> frame = GetImage(nBufferSize);
			photo = frame;

			//std::cout << "Captured Image Data: " << frame[0] << ", " << frame[1] << ", " << frame[2] << ". \n"; 
			char old_fill_char = std::cout.fill('0');
			std::cout << std::hex << "[HW] Captured Image Data: 1st = 0x" << (int)frame[0] << " "
				<< "2nd = 0x" << (int)frame[1] << " "
				<< "3rd = 0x" << (int)frame[2] << std::dec << "; nBufferSize = " << nBufferSize 
				<< ", ImageSize = " << frame.size()
				<< " \n";
			std::cout.fill(old_fill_char);
			std::cout << std::dec;
		}
		else
		{
			std::cout << "Failed to wait for nexe image !!!\n";
		}
	}


	return 1;
}



// Set camera with one of following trigger modes:
//                  1. Software trigger
//                  2. Hardware trigger
//                  3. Free Run trigger
bool SciCamera::SetInternalMode(int mode) 
{
	if (SP_ISNULL(m_pCamera))
    {
		return false;
    }

	FeaturePtr feature;
	std::string lineMode;

	m_nCameraMode = mode; 
	switch(mode)
	{

	case 2:
		/// setting trigger type to ExposureStart
		SP_ACCESS(m_pCamera)->GetFeatureByName("TriggerSelector", feature);
		feature->SetValue("FrameStart");
		/// Select Line1
		SP_ACCESS(m_pCamera)->GetFeatureByName("TriggerSource", feature);
		feature->SetValue("Line1");
		/// configuring rising edge trigger
		SP_ACCESS(m_pCamera)->GetFeatureByName("TriggerActivation", feature);
		feature->SetValue("RisingEdge");
		/// setting TriggerMode to On
		SP_ACCESS(m_pCamera)->GetFeatureByName("TriggerMode", feature);
		feature->SetValue("On");
		break;

	case 1:
		/// setting trigger type to ExposureStart
		SP_ACCESS(m_pCamera)->GetFeatureByName("TriggerSelector", feature);
		feature->SetValue("FrameStart");
		/// Select Line1
		SP_ACCESS(m_pCamera)->GetFeatureByName("TriggerSource", feature);
		feature->SetValue("Software");
		/// configuring rising edge trigger
		SP_ACCESS(m_pCamera)->GetFeatureByName("TriggerActivation", feature);
		feature->SetValue("RisingEdge");
		/// setting TriggerMode to On
		SP_ACCESS(m_pCamera)->GetFeatureByName("TriggerMode", feature);
		feature->SetValue("On");
		break;

	case 3:
	default:
		/// setting trigger type to ExposureStart
		SP_ACCESS(m_pCamera)->GetFeatureByName("TriggerSelector", feature);
		feature->SetValue("FrameStart");
		/// Select Line1
		SP_ACCESS(m_pCamera)->GetFeatureByName("TriggerSource", feature);
		feature->SetValue("Software");
		/// configuring rising edge trigger
		SP_ACCESS(m_pCamera)->GetFeatureByName("TriggerActivation", feature);
		feature->SetValue("RisingEdge");
		/// setting TriggerMode to On
		SP_ACCESS(m_pCamera)->GetFeatureByName("TriggerMode", feature);
		feature->SetValue("Off"); 
		break;
	}



	return true;
}


//                  1. Software trigger
//                  2. Hardware trigger
//                  3. Free Run trigger


bool SciCamera::TriggerSoftwareSignal()
{
	if (SP_ISNULL(m_pCamera))
    {
		return false;
    }

	if (m_nCameraMode != 1) {
		return false;
	}


	// Trigger software signal
	FeaturePtr pCommandFeature;
	if( VmbErrorSuccess == m_pCamera->GetFeatureByName( "TriggerSoftware", pCommandFeature ) )
	{
		if( VmbErrorSuccess == pCommandFeature->RunCommand() )
		{
			bool bIsCommandDone = false;
			do
			{
				if( VmbErrorSuccess != pCommandFeature->IsCommandDone( bIsCommandDone ) )
				{
					break;
				}
			} while( false == bIsCommandDone );
		}
	}

	return true; 
}


long SciCamera::WaitForNextImageReady(int ms)
{
	if (m_pFrameObserver) {
		return m_pFrameObserver->WaitForImageReady(ms); 
	} 

	return 0;
}


std::vector<VmbUchar_t> SciCamera::GetImage()
{
	return m_pFrameObserver->GetImage(); 
}

std::vector<VmbUchar_t> SciCamera::GetImage(int & nBufferSize)
{
	return m_pFrameObserver->GetImage(nBufferSize); 
}


