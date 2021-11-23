/*=============================================================================
  Copyright (C) 2019 Allied Vision Technologies.  All Rights Reserved.

  Redistribution of this file, in original or modified form, without
  prior written consent of Allied Vision Technologies is prohibited.

-------------------------------------------------------------------------------

  File:        program.cpp

  Description: Based on ListFeatures example of VimbaCPP.

  Explanation:
	  1. Implement SciCamera.h interface, with 
		 open_camera
		 start_acquire
		 shot
		 stop_acquire
		 close_camera
	 
	  2. Trigger modes supported: 
		 Software trigger
		 Hardware trigger
		 Free Run trigger 

	  3. Usage:
	     as item 1 above and you can repeat shot() to capture photos forever.

  Target: 
      Single shot() photo acquiring API for three modes.
	  In Free Run mode, shot() can get same speed as Vimba Viewer.  

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

#include <iostream>
#include <iomanip> 
#include <chrono>
#include <queue>
#include <conio.h>


#include "SciCamera.h"

#include "VimbaCPP/Include/VimbaCPP.h" 
using namespace AVT::VmbAPI;


// Milliseconds of time since epoch
long long tickets()
{
	std::chrono::milliseconds ms = std::chrono::duration_cast<std::chrono::milliseconds>( std::chrono::system_clock::now().time_since_epoch() );
	return ms.count(); 
}


// Save the raw data of the image into file
// Use ImageJ https://imagej.nih.gov/ij/download.html to view
void SaveImageRawDataToFile( BYTE* pBitmapBits,  
					   LONG nRawDataSize,
                       WORD wBitsPerPixel, 
                       const unsigned long& padding_size,
                       LPCTSTR lpszFileName )  
{  
    // Some basic bitmap parameters  
       
    // Create the file in disk to write  
    HANDLE hFile = CreateFile( lpszFileName,  
                                GENERIC_WRITE,  
                                0,  
                                NULL,  
                                CREATE_ALWAYS,  
                                FILE_ATTRIBUTE_NORMAL,  
                                NULL );  
       
    // Return if error opening file  
    if( !hFile ) return;  
       
    DWORD dwWritten = 0;  
       
       
    // Write the raw 8bit image data
    WriteFile( hFile,  
                pBitmapBits,  
				nRawDataSize,
                &dwWritten,  
                NULL );  
       
    // Close the file handle  
    CloseHandle( hFile );  
}  



int main( int argc, char* argv[] )
{
    std::cout << "\n";
    std::cout << "//////////////////////////////////////////////////////////////\n";
    std::cout << "///                                                        ///\n";
    std::cout << "/// Vimba C++ API Example - 3 Trigger Modes                ///\n";
    std::cout << "///           For OPTMV AVT API                            ///\n";
    std::cout << "///                                                        ///\n";
    std::cout << "/// 2019-05-17 17:31:50 by Joe Ge                          ///\n";
    std::cout << "///      Intilization version.                             ///\n";
    std::cout << "///                                                        ///\n";
    std::cout << "/// 2019-05-20 13:34:56 by Joe Ge                          ///\n";
    std::cout << "///      FreeRun mode is as quicker as Vimba Viewer.       ///\n";
    std::cout << "///                                                        ///\n";
    std::cout << "/// 2019-05-20 15:03:37 by Joe Ge                          ///\n";
    std::cout << "///      Save photo into file with raw data format.        ///\n";
    std::cout << "///                                                        ///\n";
    std::cout << "/// 2021-06-16 18:30:30 by Joe Ge                          ///\n";
    std::cout << "///      Upgrade to support Vimba 4.2 / 4.3 for            ///\n";
    std::cout << "///                                      Alvium USB3       ///\n";
    std::cout << "///                                                        ///\n";
    std::cout << "///                                                        ///\n";
    std::cout << "//////////////////////////////////////////////////////////////\n\n";

	// FPS calculation
	std::queue<long long> queueTickets;

	// Single Vimba SDK Instance
    VimbaSystem& sys = VimbaSystem::GetInstance();           // Get a reference to the VimbaSystem singleton
	sys.Startup();

	SciCamera camera; 
	CameraPtrVector cameras;

	// Open a camera
	std::string strCameraID;
	sys.GetCameras(cameras);
	if (!cameras.empty())
	{
		cameras[0]->GetID(strCameraID);
		camera.OpenCamera(strCameraID.c_str());
	}

	std::string strOpenedCameraName;
	camera.GetCameraID(strOpenedCameraName);
	std::cout << "A camera opened: " << strOpenedCameraName.c_str() << std::endl;
	std::cout << "\nPress Enter key to stop ...\n\n\n";




	// Start image acquisition mode
	//camera.StartAcquire(1); //  1. Software trigger  
	camera.StartAcquire(2); //  2. Hardware trigger  
	//camera.StartAcquire(3); //  3. Free Run trigger 


	// Capture Images in one of three modes: SW/HW/Free
	int i = 0;
	while (i++ < 20000)
	{
		std::cout << "Fetching image i = " << i << ", ticket : " << tickets() << std::endl;
		std::vector<VmbUchar_t> imageRaw;

		// Take one photo !!! 
		camera.Shot(imageRaw, 10*1000);

		// Save raw data of image
		// Use ImageJ or similar tool to view
		if(i < 2 ) {
			unsigned long padding = 0;
			SaveImageRawDataToFile(&imageRaw[0], imageRaw.size(), 8, padding, "vimba-raw-image.raw");
		}


		// FPS calculation, print image sequence information
		long long tGap = 1; // 1ms
		queueTickets.push(tickets());
		while (queueTickets.size() > 11) {
			queueTickets.pop();
		}
		if (queueTickets.size() >= 2) {
			tGap = queueTickets.back() - queueTickets.front(); 
			double fGapTwoShots = (tGap * 1.0) / (queueTickets.size() - 1); 
			std::cout << std::fixed; 
			std::cout << "Fetching image i = " << i << std::setprecision(4) << ", gap    : " << fGapTwoShots << " ms,      FPS = " << 1000.0 / fGapTwoShots << std::endl;
		}

		if (_kbhit()) {
			getchar(); // consume the above pressed key
			std::cout << "\n\nUser keyboard stroke interrupted the loop ...";
			break;
		}


	}



	// -------------------------- stop camera and quit -------------------------------------- //
	// Following steps only to be applied when the system wants to exit.

	// Stop image acquisition
	camera.StopAcquire();

	// Close a camera
	camera.CloseCamera();

	// Clean Vimba SDK Resource before exiting
	sys.Shutdown();

	// Waiting for user's Enter key stroking ...
	std::cout << "\n\nPress Enter key to exit ..." << std::endl;
	getchar();

	
    std::cout <<"\n";
}
