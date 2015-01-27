/***********************************************************
*  --- OpenSURF ---                                       *
*  This library is distributed under the GNU GPL. Please   *
*  use the contact form at http://www.chrisevansdev.com    *
*  for more information.                                   *
*                                                          *
*  C. Evans, Research Into Robust Visual Features,         *
*  MSc University of Bristol, 2008.                        *
*                                                          *
************************************************************/

#include <SURF/surflib.h>
#include <SURF/surflib.h>
//SCILAB vers 5.5.1 32bits for windows
#include <SCILAB/api_scilab.h>
#include <SCILAB/stack-c.h> /* Provide functions to access to the memory of Scilab */
#include <SCILAB/call_scilab.h> /* Provide functions to call Scilab engine */
#include <ctime>
#include <iostream>
#include <stdio.h>
#include <stdlib.h>

#define PROCEDURE 5

//-------------------------------------------------------

int scilabInit2() {
    printf("|-Loading Scilab...\n") ;
    #ifdef _MSC_VER
    if( StartScilab(NULL,NULL,NULL) == FALSE )
    #else
    if( StartScilab(getenv("SCI"),NULL,NULL) == FALSE )
    #endif
    {
        fprintf(stderr,"| |-Error while calling StartScilab\n") ;
        return 0 ;
    }
    return 1 ;
}

//-------------------------------------------------------

int mainImage(void)
{
  // Declare Ipoints and other stuff
  IpVec ipts;
  IplImage *img=cvLoadImage("imgs/sf.jpg");

  // Detect and describe interest points in the image
  clock_t start = clock();
  surfDetDes(img, ipts, false, 5, 4, 2, 0.0004f);
  clock_t end = clock();

  std::cout<< "OpenSURF found: " << ipts.size() << " interest points" << std::endl;
  std::cout<< "OpenSURF took: " << float(end - start) / CLOCKS_PER_SEC  << " seconds" << std::endl;

  // Draw the detected points
  drawIpoints(img, ipts);

  // Display the result
  showImage(img);

  return 0;
}

int mainImage2(void)
{
  // Declare Ipoints and other stuff
  IpVec ipts;
  IplImage *img=cvLoadImage("ballleft.png");

  // Detect and describe interest points in the image
  surfDetDes(img, ipts, false, 5, 4, 2, 0.0004f);

  std::cout<< "OpenSURF found: " << ipts.size() << " interest points" << std::endl;

  // Draw the detected points
  drawIpoints(img, ipts);

  // Display the result
  showImage(img);

  return 0;
}
//-------------------------------------------------------

int mainVideo(void)
{
  // Initialise capture device
  CvCapture* capture = cvCaptureFromCAM( 1 );
  if(!capture) error("No Capture");

  // Initialise video writer
  //cv::VideoWriter vw("c:\\out.avi", CV_FOURCC('D','I','V','X'),10,cvSize(320,240),1);
  //vw << img;

  // Create a window
  cvNamedWindow("OpenSURF", CV_WINDOW_AUTOSIZE );

  // Declare Ipoints and other stuff
  IpVec ipts;
  IplImage *img=NULL;

  // Main capture loop
  while( 1 )
  {
    // Grab frame from the capture source
    img = cvQueryFrame(capture);

    // Extract surf points
    surfDetDes(img, ipts, false, 4, 4, 2, 0.004f);

    // Draw the detected points
    drawIpoints(img, ipts);

    // Draw the FPS figure
    drawFPS(img);

    // Display the result
    cvShowImage("OpenSURF", img);

    // If ESC key pressed exit loop
    if( (cvWaitKey(10) & 255) == 27 ) break;
  }

  cvReleaseCapture( &capture );
  cvDestroyWindow( "OpenSURF" );
  return 0;
}

//-------------------------------------------------------

int mainStaticMatch()
{
    // Initialise Scilab
    if ( !scilabInit2() ) return -7 ;
    // Load Scilab
    SendScilabJob("exec ('Stereo_scilab.sci', -1)") ;
    char calib_text[200] ;
    IplImage *img1, *img2;
    img1 = cvLoadImage("stereo_g.png");
    img2 = cvLoadImage("stereo_d.png");

    IpVec ipts1, ipts2;
    surfDetDes(img1,ipts1,false,4,4,2,0.00001f);
    surfDetDes(img2,ipts2,false,4,4,2,0.00001f);

    IpPairVec matches;
    getMatches(ipts1,ipts2,matches);

    SendScilabJob("fakeview = mopen('temp.txt','w') ;") ;
    for ( unsigned int i = 0 ; i < matches.size() ; ++i ) {
        drawPoint(img1,matches[i].first);
        drawPoint(img2,matches[i].second);

        const int & w = img1->width;
        cvLine(img1,cvPoint(matches[i].first.x,matches[i].first.y),cvPoint(matches[i].second.x+w,matches[i].second.y), cvScalar(255,255,255),1);
        cvLine(img2,cvPoint(matches[i].first.x-w,matches[i].first.y),cvPoint(matches[i].second.x,matches[i].second.y), cvScalar(255,255,255),1);
        sprintf(calib_text,"[x,y,z]=camera_2D_3D(Glob_g, Glob_d, %d, %d, %d, %d)", matches[i].first.x, matches[i].first.y, matches[i].second.x+w, matches[i].second.y) ;
        SendScilabJob(calib_text) ;
        SendScilabJob("mfprintf(fakeview,'%f\t%f\t%f\n', x, y, z) ;") ;
    }

    std::cout<< "Matches: " << matches.size();

    system("copy /Y temp.txt 2D_meshlab.txt") ;
    SendScilabJob("mclose(fakeview) ;") ;
    SendScilabJob("clear fakeview ;") ;

    cvNamedWindow("1", CV_WINDOW_AUTOSIZE );
    cvNamedWindow("2", CV_WINDOW_AUTOSIZE );
    cvShowImage("1", img1);
    cvShowImage("2",img2);
    if ( TerminateScilab(NULL) == FALSE ) return -10 ;
    cvWaitKey(0);

    return 0;
}

//-------------------------------------------------------

int mainVideoMatch(void) {
    // Initialise Scilab
    if ( !scilabInit2() ) return -7 ;
    // Load Scilab
    SendScilabJob("exec ('Stereo_scilab.sci', -1)") ;
    char calib_text[200] ;

    // Initialise capture device
    CvCapture* capture = cvCaptureFromCAM( 1 );
    cvSetCaptureProperty(capture, CV_CAP_PROP_FRAME_WIDTH,640);
    cvSetCaptureProperty(capture, CV_CAP_PROP_FRAME_HEIGHT,480);
    if(!capture) error("No Capture");
    CvCapture* capture2 = cvCaptureFromCAM( 2 );
    if(!capture2) error("No Capture2");
    cvSetCaptureProperty(capture2, CV_CAP_PROP_FRAME_WIDTH,640);
    cvSetCaptureProperty(capture2, CV_CAP_PROP_FRAME_HEIGHT,480);

    // Create a window
    cvNamedWindow("OpenSURF", CV_WINDOW_AUTOSIZE );
    cvNamedWindow("OpenSURF2", CV_WINDOW_AUTOSIZE );

    // Declare Ipoints and other stuff
    IpVec ipts;
    IplImage *img=NULL;
    IpVec ipts2;
    IplImage *img2=NULL;

    // Main capture loop
    while( 1 ) {
        SendScilabJob("fakeview = mopen('temp.txt','w') ;") ;
        // Grab frame from the capture source
        img = cvQueryFrame(capture);
        img2 = cvQueryFrame(capture2);

        // Extract surf points
        surfDetDes(img, ipts, false, 4, 4, 2, 0.001f);
        surfDetDes(img2, ipts2, false, 4, 4, 2, 0.001f);

        // Draw the detected points
        drawIpoints(img, ipts);
        drawIpoints(img2, ipts2);

        IpPairVec matches;
        getMatches(ipts,ipts2,matches);

        for (unsigned int i = 0; i < matches.size(); ++i) {
            drawPoint(img,matches[i].first);
            drawPoint(img2,matches[i].second);

            const int & w = img->width;
            cvLine(img,cvPoint(matches[i].first.x,matches[i].first.y),cvPoint(matches[i].second.x+w,matches[i].second.y), cvScalar(255,255,255),1);
            cvLine(img2,cvPoint(matches[i].first.x-w,matches[i].first.y),cvPoint(matches[i].second.x,matches[i].second.y), cvScalar(255,255,255),1);
            sprintf(calib_text,"[x,y,z]=camera_2D_3D(Glob_g, Glob_d, %d, %d, %d, %d)", matches[i].first.x, matches[i].first.y, matches[i].second.x, matches[i].second.y) ;
            SendScilabJob(calib_text) ;
            SendScilabJob("mfprintf(fakeview,'%f\t%f\t%f\n', x, y, z) ;") ;

        }

        // Draw the FPS figure
        drawFPS(img);
        drawFPS(img2);

        system("copy /Y temp.txt 2D_meshlab.txt") ;

        // Display the result
        cvShowImage("OpenSURF", img);
        cvShowImage("OpenSURF2", img2);

        SendScilabJob("mclose(fakeview) ;") ;
        SendScilabJob("clear fakeview ;") ;

        // If ESC key pressed exit loop
        if( (cvWaitKey(10) & 255) == 27 ) break;
    }

    cvReleaseCapture( &capture );
    cvReleaseCapture( &capture2 );
    cvDestroyWindow( "OpenSURF" );
    cvDestroyWindow( "OpenSURF2" );
    if ( TerminateScilab(NULL) == FALSE ) return -10 ;
    return 0;
}

int main(void)
{
  if (PROCEDURE == 1) return mainImage();
  if (PROCEDURE == 2) return mainImage2();
  if (PROCEDURE == 3) return mainVideo();
  if (PROCEDURE == 5) return mainStaticMatch();
  if (PROCEDURE == 7) return mainVideoMatch();
}
