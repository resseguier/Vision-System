#define _VISION_H

#include <LODEPNG/LODEPNG.h>
#include <ESCAPI/escapi.h>

//SDL version 1.2 (template pour codeblocks par Alexandre Laurent)
#include <SDL/SDL.h>
#include <SDL/SDL_image.h>
#include <SDL/SDL_ttf.h>

#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <iostream>
#include "windows.h"
//#include <fstream>
//#include <string>
//#include <sstream>

#ifdef __linux__
    #include <time.h>
    #define OS   1
    #define CLEAN_SCREEN "clear"
#elif _WIN32
    #include <time.h>
    #define OS   2
    #define CLEAN_SCREEN "CLS"
#else
    #define OS   0
#endif

#define VERSIONY   15
#define VERSIONM   1
#define VERSIOND   9

#define MIN(a,b)   (a<b?a:b)
#define MAX(a,b)   (a>b?a:b)
#define NO_HUE   -1

#define pi18   0.3927
#define pi38   1.1781
#define pi58   1.9635
#define pi78   2.7489

//Image data struct
typedef struct img2D {
    //Tableau pour stocker l'image
    int ** img ;
    //La largeur de l'image
    int width ;
    //La hauteur de l'image
    int height ;
} Image2D ;
//Image data struct
typedef struct img2DRGB {
    //Tableau pour stocker l'image
    int *** imgRGB ;
    //La largeur de l'image
    int width ;
    //La hauteur de l'image
    int height ;
} Image2DRGB ;

//Pixel feature struct
typedef struct pixfeature {
    int x ;
    int y ;
    float i ;
    float moy ;
    float ecart ;
    float etendue ;
    float det ;
    float amp ;
    float theta ;
    float histogram[9] ;
} PixelFeature ;

//Pixel feature struct
typedef struct pixfound {
    int x ;
    int y ;
    int nb ;
} PixelFound ;

//Escapi lib
//----------------------------------------------------
    int escapiInit() ; //Initialise ESCAPI. | return number of devices if no error
    int escapiInit( int an_int ) ; //Initialise  ESCAPI and print the name of all webcam it find | return number of devices if no error
    void escapiCaptureAlloc(struct SimpleCapParams &cap, int w, int h) ; //Allocate capture buffer
    int escapiCaptureInit(struct SimpleCapParams &cap, int cam) ; //Initialize capture - only one capture may be active per device
    int escapiComplete(const char *argcamera, struct SimpleCapParams &cap, Image2D &image, int width, int height) ;
    void escapiDeinitCapture(const char *argcamera) ;

    void escapiCaptureReq(int cam) ; //Request a capture
    void escapiCaptureReq2(const char *argcamera, struct SimpleCapParams cap, Image2D image) ;

    int escapiPixGetRed(struct SimpleCapParams cap, int x, int y) ; //Get the Red value of the Pix(x,y) | return : [0;255]
    int escapiPixGetGreen(struct SimpleCapParams cap, int x, int y) ; //Get the Green value of the Pix(x,y) | return : [0;255]
    int escapiPixGetBlue(struct SimpleCapParams cap, int x, int y) ; //Get the Blue value of the Pix(x,y) | return : [0;255]
    void escapiPixToRGB(struct SimpleCapParams &cap, int x, int y, int r, int g, int b) ; //Put the new RGB value to the Pix(x,y) | r,g,b : [0;255]
//----------------------------------------------------

//SCILAB
//----------------------------------------------------
    int scilabInit() ;
//----------------------------------------------------

//SDL lib
//----------------------------------------------------
    void sdlGetStdout() ; //By default SDL write all stdout in file, with this we get the stdout back in cmd
    void sdlFillStdout() ;
    void sdlCleanup() ; //Free all SDL cache
    int sdlInit() ; //Initialise SDL
    Uint32 * sdlBufpPos(SDL_Surface *scr, int x, int y) ; //Set Bufp (pixel) position in order to be writen
    void sdlImageToScr(Image2D &image, SDL_Surface *scr, int camnb) ; //Retrive the color of pix and return it in order to be writen in bufp
    void sdlImageToScrRGB(Image2DRGB &image, SDL_Surface *scr, int camnb) ;
    void sdlLigne(SDL_Surface *scr, int xi, int yi, int xf, int yf, int color) ;
    void sdlLigneLight(SDL_Surface *scr, int width, int height, int xi, int yi, int xf, int yf, int color) ;
    void sdlLigneDark(SDL_Surface *scr, int width, int height, int xi, int yi, int xf, int yf, int color) ;
//----------------------------------------------------

//Vision lib
//----------------------------------------------------
    //Image allocation, destroy, copy and clone
        void visionImageAlloc(Image2D &image, int width, int height) ;
        void visionImageFree(Image2D image) ;
        void visionCapture2Image(struct SimpleCapParams capture, Image2D image) ;
        int visionPicture2Image(const char *argpicture, Image2D &image, unsigned int &width, unsigned int &height) ;
        void visionPicture3Image(unsigned char* picture, Image2D image) ;

        int visionBMPproperties(const char *argbmp, unsigned int &width, unsigned int &height) ;
        int visionBMP2Image(const char *argbmp, Image2D image) ;
        void visionCloneImage(Image2D img_in, Image2D img_out) ;

        void visionImageAllocRGB(Image2DRGB &image, int width, int height) ;
        void visionImageFreeRGB(Image2DRGB image) ;
        void visionCapture2ImageRGB(struct SimpleCapParams capture, Image2DRGB image) ;
        void visionPicture2ImageRGB(unsigned char* picture, Image2DRGB image) ;
        void visionCloneImageRGB(Image2DRGB img_in, Image2DRGB img_out) ;

        void visionImageRGB2Image(Image2DRGB img_in, Image2D img_out) ;
        void visionImage2ImageRGB(Image2D img_in, Image2DRGB img_out) ;
    //Corner detector
        void visionSusan9(Image2D img_in, Image2D img_out, int gap) ;
        void visionSusan17(Image2D img_in, Image2D img_out, int gap) ;
        void visionSusan21(Image2D img_in, Image2D img_out, int gap) ;
        void visionSusan21Binary(Image2D img_in, Image2D img_out) ;
    //Modification RGB / HSV / BW
        void rgb2hsv(float r, float g, float b, int &h, int &s , int &v) ;
        void hsv2rgb(float h, float s, float v, int &r, int &g, int &b) ;
        float rgb2h(float r, float g, float b) ;
    //Draw line and point
        void ligne( Image2D img, int xi, int yi, int xf, int yf, int color) ;
        void drawpoint( Image2D img_out, int x, int y, int type, int R, int color) ; //Type 0 : empty square - Type 1 : plain square - Type 2 : empty circle - Type 3 : plain circle
    //Simple edition
        void visionBin(Image2D img_in, Image2D img_out, int threshold) ;
        void visionBinFindH(Image2DRGB img_in, Image2D img_out, int H, int threshold) ;
        void visionExcludeH(Image2DRGB img_in, Image2D img_out, int H, int threshold) ;
    //Blur effect
        void visionBlurPix(Image2D img_in, Image2D img_out, int x, int y, int R) ;
        void visionBlurImage(Image2D img_in, Image2D img_out, int type, int R) ; //Type define complexity of blur : 0 means simple pass, 1 (or more) complex with multiple pass (take a lot more process depending on value of R)

        void visionDog2(Image2D img_in, Image2D img_out) ;
        void visionGaussian3(Image2D img_in, Image2D img_out) ;
        int visionGaussian3pix(Image2D img_in, int x, int y) ;
        void visionGaussian5(Image2D img_in, Image2D img_out) ;
        void visionMatrice(Image2D img_in, Image2D img_out, int divider, float A, float B, float C, float D, float E, float F, float G, float H, float I) ;
        void visionMatriceConv(Image2D img_in, Image2D img_out, int divider, int divider2, float A, float B, float C, float D, float E, float F, float G, float H, float I, float A2, float B2, float C2, float D2, float E2, float F2, float G2, float H2, float I2) ;
        void visionCanny(Image2D img_in, Image2D img_out, int gap_min, int gap_max) ;
        void visionMedian(Image2D img_in, Image2D img_out) ;
        void visionSupprMinMax(Image2D image, int gap_min, int gap_max, int bin) ;

        void visionCannyNew(Image2D img_in, Image2D img_out, int min_threshold, int max_threshold) ;
        void vision2Susan17(Image2D img_in, Image2D img_out, int gap) ;
        void visionFast(Image2D img_in, Image2D img_out, int threshold, int max_fast) ;
        void corner21(Image2DRGB img_in, Image2D img_out, int tri_suite, int gap, int gap_hmin, int gap_hmax) ;
        void visionKeypointDetector(Image2D img_in, Image2D img_out, int threshold, int N) ;

        void visionFindCanvas(Image2D img_in, Image2D img_out,PixelFound *canvas, int threshold, int Nmore) ;
        void visionFindCanvas2(Image2D img_in, Image2D img_out,PixelFound *canvas, int threshold, int Nmore, int Emax) ;
        void visionDrawCanvas(Image2D img_in, Image2D img_out, int threshold, int Nmore, int Emax) ;
        void visionRotateImage(Image2D img_in, Image2D img_out, int cx, int cy, float angle) ;
        void visionRotateAround(Image2D img_in, Image2D img_out, int cx, int cy, float angle, int R) ;
        void visionRotateCanvas(PixelFound center, PixelFound direction, PixelFound *canvas, PixelFound *canvasnew) ;
        int visionGetCanvas(Image2D img_in, Image2D img_out, PixelFound *canvas_blue, PixelFound *canvas_black, int &action, int &Nmore, int &threshold) ;
        int visionGetCanvas2(Image2D img_in, Image2D img_out, int campos, PixelFound *canvas_blue, PixelFound *canvas_black, int &action, int &Nmore, int Emax, int &threshold, int &finished) ;


        void visionGetFeature(Image2D img_in, int x, int y, PixelFeature *feat, int pos) ;
        void visionGetFeature2(Image2D img_in, int x, int y, PixelFeature *feat, int pos) ;
        void visionGetFeature3(Image2D img_in, int x, int y, PixelFeature *feat, int pos) ;
        void visionGetFeature4(Image2D img_in, Image2D img_out, int cx, int cy, PixelFeature *feat, int pos) ;
        void visionPrintFeature(PixelFeature feat) ;
        void visionPrintFeature2(PixelFeature feat1, PixelFeature feat2) ;
        void visionPrintFeature3(PixelFeature feat1, PixelFeature feat2) ;
        float visionCompareFeature(PixelFeature feat_1, PixelFeature feat_2) ;
        float visionCompareFeature3(PixelFeature feat_1, PixelFeature feat_2) ;
        float visionCompareFeature4(PixelFeature feat_1, PixelFeature feat_2) ;
        void visionErosion( Image2D img_in, int Nmax) ;
        void visionErosionAndfeature(Image2D img_ori, Image2D img_in, int Nmax,PixelFeature *feat, int &nb_feat) ;
//----------------------------------------------------

//Other
//----------------------------------------------------
    void visionVersion() ; //Print VISION_SYSTEM actual version
    double get_time() ;
    void show_diff_time( double time_start, double time_stop, int type) ; //Print back time duration (type = 0) or fps (type = 1)
    void do_system(const char *command) ;
    void HelpvisionGetCanvas() ; //Print the help/guide for the fonction of same name
    void Helpvision() ; //Print VISION_SYSTEM help/guide
//----------------------------------------------------


unsigned int*** newimg( unsigned int &width, unsigned int &height) ;
void freeimg( unsigned int*** img, unsigned int &width, unsigned int &height) ;
unsigned int** newmask( unsigned int &width, unsigned int &height) ;
void freemask( unsigned int** mask, unsigned int &width) ;

/*
//Creation, chargement et suppression d'image en memoire
float** newfeature( unsigned int &nbpoint) ;
void freefeature( float** feature, unsigned int &nbpoint) ;

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
*/

/*
	//check if user gave a filename
	if ( argc != 5 ) {
        printf("Error\n") ;
		printf("usage :\n\t\t ./main inpufile1 inpufile2 outpufile outpufile2\") ;
		return 1 ;
	}

    FILE *fichier = NULL ;
    fichier = fopen("text.txt", "w" ) ;
    printf("Salut a vous tous 1\n") ;
    fprintf(fichier, "Salut a vous tous 2\n") ;
    fclose( fichier ) ;
*/
