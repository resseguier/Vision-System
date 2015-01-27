#define _VISION_H

#include <LODEPNG/LODEPNG.h>
#include <ESCAPI/escapi.h>

//SDL version 1.2 (template pour codeblocks par Alexandre Laurent
#include <SDL/SDL.h>
#include <SDL/SDL_image.h>
#include <SDL/SDL_ttf.h>

#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <iostream>
//#include <fstream>
//#include <string>
//#include <sstream>

#define VERSIONY 14
#define VERSIONM 11
#define VERSIOND 15

#ifdef __linux__
    #include <time.h>
#elif _WIN32
    #include <time.h>
#else

#endif

//Escapi lib
//----------------------------------------------------
    int escapiInit() ; //Initialise ESCAPI. | return number of devices if no error
    int escapiInit( int an_int ) ; //Initialise  ESCAPI and print the name of all webcam it find | return number of devices if no error
    void escapiCaptureAlloc(struct SimpleCapParams &cap, int w, int h) ; //Allocate capture buffer
    int escapiCaptureInit(struct SimpleCapParams &cap, int cam) ; //Initialize capture - only one capture may be active per device

    void escapiCaptureReq(int cam) ; //Request a capture

    int escapiPixGetRed(struct SimpleCapParams cap, int x,int y) ; //Get the Red value of the Pix(x,y) | return : [0;255]
    int escapiPixGetGreen(struct SimpleCapParams cap, int x,int y) ; //Get the Green value of the Pix(x,y) | return : [0;255]
    int escapiPixGetBlue(struct SimpleCapParams cap, int x,int y) ; //Get the Blue value of the Pix(x,y) | return : [0;255]
    void escapiPixToRGB(struct SimpleCapParams &cap, int x,int y, int r, int g, int b) ; //Put the new RGB value to the Pix(x,y) | r,g,b : [0;255]
//----------------------------------------------------

//SDL lib
//----------------------------------------------------
    void sdlGetStdout() ; //By default SDL write all stdout in file, with this we get the stdout back in cmd
    void sdlCleanup() ; //Free all SDL cache
    int sdlInit() ; //Initialise
    Uint32 * sdlBufpPos(SDL_Surface *scr, int x, int y) ; //Set Bufp (pixel) position in order to be writen
//----------------------------------------------------

//Vision lib
//----------------------------------------------------
//----------------------------------------------------

//Other
//----------------------------------------------------
    void visionVersion() ; //Print VISION_SYSTEM actual version
    void visionHelp() ; //Print VISION_SYSTEM help/guide
//----------------------------------------------------












//Creation, chargement et suppression d'image en memoire
unsigned int*** newimg( unsigned int &width, unsigned int &height) ;
unsigned int** newmask( unsigned int &width, unsigned int &height) ;
float** newfeature( unsigned int &nbpoint) ;
void freeimg( unsigned int*** img, unsigned int &width, unsigned int &height) ;
void freemask( unsigned int** mask, unsigned int &width) ;
void freefeature( float** feature, unsigned int &nbpoint) ;
unsigned int*** loadimg( unsigned char* image, unsigned int &width, unsigned int &height) ;
void cloneimg( unsigned int*** imgin, unsigned int*** imgout, unsigned int &width, unsigned int &height) ;
void saveimg( unsigned int*** img, unsigned char* image, unsigned int &width, unsigned int &height) ;

//Modification RGB, HSV et Black&White
void rgb2hsv( float red, float green, float blue, unsigned int &hue, unsigned int &saturation , unsigned int &value) ;
void rgb2v( unsigned int red, unsigned int green, unsigned int blue, unsigned int &value) ;
void hsv2rgb( float h, float s, float v, unsigned int &r, unsigned int &g, unsigned int &b) ;
void rgb2bw( unsigned int &r, unsigned int &g, unsigned int &b) ;

//Blur effect
void blurpix( unsigned int*** imgin, unsigned int*** imgout, unsigned int width, unsigned int height, unsigned int x, unsigned int y, int R) ;
void blurimg( unsigned int*** imgin, unsigned int*** imgout, unsigned int width, unsigned int height, int R) ;

//Detection de bord
void dog( unsigned int*** imgin, unsigned int*** imgout, unsigned int width, unsigned int height, int R) ;
void dog2( unsigned int*** imgin, unsigned int*** imgout, unsigned int width, unsigned int height) ;
void dog3( unsigned int*** imgin, unsigned int*** imgout, unsigned int width, unsigned int height, float A, float B, float C, float D, float E, float F, float G, float H, float I) ;

//dessine un cercle ou carre autour des points interessants
void drawpoint( unsigned int*** imgout, unsigned int width, unsigned int height, unsigned int x, unsigned int y, int type, int R, int color) ;

//dessine le segment entre deux points
void ligne( unsigned int*** img, int xi, int yi, int xf, int yf) ;

//Erosion / barrycentre
void erosion( unsigned int*** img, unsigned int width, unsigned int height, unsigned int &total_pix_all, unsigned int Rmax) ;
void center_color( unsigned int*** img, unsigned int*** out, unsigned int width, unsigned int height, int color, int gap ) ;
void center_color( unsigned int*** img, unsigned int*** out, unsigned int width, unsigned int height, int hue, int gap_h, int val, int gap_v, int sat, int gap_s ) ;

//susan
void susan9( unsigned int*** imgin, unsigned int*** imgout, unsigned int width, unsigned int height, int gap) ;
void susan17( unsigned int*** imgin, unsigned int*** imgout, unsigned int width, unsigned int height, int gap) ;
void susan21( unsigned int*** imgin, unsigned int*** imgout, unsigned int width, unsigned int height, int gap) ;
