/* "simplest", example of simply enumerating the available devices with ESCAPI */
#include <stdio.h>
#include <stdlib.h>
#include <iostream>
//#include <fstream>
#include <string>
#include <sstream>

#include <ESCAPI/escapi.h>

#include <LODEPNG/LODEPNG.h>

#include <VISION/VISION.h>

#define VERSIONY 14
#define VERSIONM 10
#define VERSIOND 20

int main() {

    FILE *fichier = NULL ;

    fichier = fopen("text.txt", "w" ) ;

    printf("Salut a vous tous 1\n") ;
    fprintf(fichier, "Salut a vous tous 2\n") ;

    fclose( fichier ) ;

	int x, y;
	int i ;

	// Initialize ESCAPI

	int devices = setupESCAPI();

	if (devices == 0) {
		printf("ESCAPI initialization failure or no devices found.\n");
		return 1 ;
	}

	for (i = 0; i < devices; i++)
	{
		char temp[256];
		getCaptureDeviceName(i, temp, 256);
		printf("Device %d: \"%s\"\n", i, temp);
	}

	struct SimpleCapParams capture;
	capture.mWidth = 24;
	capture.mHeight = 18;
	capture.mTargetBuf = new int[24 * 18];

	//Initialize capture - only one capture may be active per device,
    //but several devices may be captured at the same time.
    //0 is the first device.


	if (initCapture(0, &capture) == 0) {
		printf("Capture failed - device may already be in use.\n");
		return 2 ;
	}

	//Go through 10 capture loops so that the camera has
	//had time to adjust to the lighting conditions and
	//should give us a sane image..
	for (i = 0; i < 10; i++) {
		// request a capture
		doCapture(0);

		while (isCaptureDone(0) == 0) {
			//Wait until capture is done.
			//Warning: if capture init failed, or if the capture
			//simply fails (i.e, user unplugs the web camera), this
			//will be an infinite loop.
		}
	}

	//now we have the data.. what shall we do with it? let's
	//render it in ASCII.. (using 3 top bits of green as the value)
	char light[] = " .,-o+O0@";
	for (y = 0; y < 18; y++) {
		for (x = 0; x < 24; x++) {
			printf("%c", light[(capture.mTargetBuf[y*24+x] >> 13) & 7]);

            //B = (capture.mTargetBuf[y*24+x] >> 16) & $FF ;
            //G = (capture.mTargetBuf[y*24+x] >> 8) & $FF ;
            //R = capture.mTargetBuf[y*24+x] & $FF ;

		}
		printf("\n");
	}

	deinitCapture(0);



	return 0 ;





}
