// MediaLAN 02/2013
// CRtspSession
// - Main and streamer master listener socket
// - Client session thread

#include <iostream>

#include "CJpegSample.h"
#include "CWebcamCvSample.h"
#include "CRtspMaster.h"

int main()  
{    
    CRtspMaster master;
    //CJpegSample jpegSample(0);
    CWebcamCvSample webcamSample;

#if defined(_WIN64)
    /* Microsoft Windows (64-bit). ------------------------------ */
    std::cout << "WIN64\n";
#elif defined(_WIN32)
    /* Microsoft Windows (32-bit). ------------------------------ */
    std::cout << "WIN32\n";
#endif

    master.bindModel(&webcamSample);
    master.startRtsp();

    webcamSample.setImageHeight(webcamSample.getImageHeight() / 32);
    webcamSample.setImageWidth(webcamSample.getImageWidth() / 32);

    webcamSample.readFromWebcam(true);

    master.quitRtsp();
    master.waitRtsp();

    return 0;  
} 
