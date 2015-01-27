/*********************************************/
/**              VISION SYSTEM              **/
/**                vers. 2.1                **/
/**                                         **/
/**                 Made by                 **/
/**            RESSEGUIER THOMAS            **/
/**                                         **/
/**                2014-2015                **/
/*********************************************/

#include <VISION/VISION.h>
#include <omp.h>
#include <stdlib.h>
//#pragma omp parallel

/* Information about what main will return */
    //-5 / -6 : Error when init SDL
    //-4 : Error when init LODEPNG
    //-2 / -3 : Error when init ESCAPI
    //-1 : OS not supported
    //10 : all done without trouble
/* END */

int main(int argc, char *argv[]) {
    sdlGetStdout() ;
    HelpvisionGetCanvas() ;
    visionVersion() ;

    if( OS == 1 )
        printf("  Working on LINUX OS\n\n\n") ;
    else if( OS == 2 )
        printf(" Working on WINDOWS OS\n\n\n") ;
    else {
        printf("   OS not supported !\n  Aborting program...\n\n\n") ;
        return -1 ;
    }

    unsigned char* picture;
    Image2D image, final ;
    SDL_Surface *scr ;
    SDL_Event event ;
    int camList = 0 ; //0 if you want to work on a picture, else point out the number of camera to work with
    struct SimpleCapParams capture[camList] ;
    double  time_begin, time_end ;
    unsigned int camWidth = 640 ;
    unsigned int camHeight = 480 ;
    char windowsname[25] ;
    int camnb ;
    int done = 1 ;
    int action = 0 ;
    int Nmore = 0 ;
    int threshold = 80 ;
    PixelFeature feat[5] ;

    /* Starting by loading picture or camera and SDL component */
        //Load camera or picture as asked by user
        if( camList ) {
            if( OS == 2 ) {
                printf("Initialisation camera\n") ;

                if( !escapiInit(0) ) return -2 ;
                for(camnb = 0 ; camnb < camList ; camnb++ ) {
                    escapiCaptureAlloc(capture[camnb], camWidth, camHeight) ;
                    if( !escapiCaptureInit(capture[camnb], camnb) ) return -3 ;
                }
            }
            sprintf(windowsname,"Camera %d", camnb) ; //Save windows name
        } else {
            if( lodepng_decode32_file(&picture, &camWidth, &camHeight, "data/tsukubaleft.png") ) return -4 ;
            std::cout << "L'image \"" << "data/tsukubaleft.png" << "\" a ete chargee ..." << std::endl ;
            sprintf(windowsname,"Picture") ; //Save windows name
        }

        visionImageAlloc(image, camWidth, camHeight) ;
        visionImageAlloc(final, camWidth, camHeight) ;
        printf("Largeur de l'image = %d\nHauteur de l'image = %d\n\n", camWidth, camHeight) ;

        //Load SDL window
        if( !sdlInit() ) return -5 ;
        if( !(scr = SDL_SetVideoMode( camWidth*(camList>0?camList:1), camHeight, 32, SDL_HWSURFACE))) return -6 ; //Allocate scr (SDL buffer) and set param //SDL_HWSURFACE - SDL_FULLSCREEN - SDL_RESIZABLE - SDL_NOFRAME
        SDL_WM_SetCaption(windowsname , NULL) ; //Define windows name
        SDL_Flip(scr) ;
    /* END */

    while( done < 5 ) { //Loop until escape is pressed
        for(camnb = 0 ; camnb < (camList>0?camList:1) ; camnb++ ) {
            if( camList ) {
                //Get the actual capture
                if( OS == 2 ) escapiCaptureReq(camnb) ;
                visionCapture2Image(capture[camnb], image) ;

                //Print final picture to SDL
                sdlImageToScr(final , scr, camnb) ;
            } else {
                //Get the actual picture
                visionPicture2Image(picture, image) ;

                visionGetFeature(image, 253, 192, feat, 0) ;
                visionGetFeature(image, 306, 117, feat, 1) ;
                visionGetFeature(image, 210, 221, feat, 2) ;
                visionGetFeature(image, 146, 231, feat, 3) ;
                visionGetFeature(image, 55, 156, feat, 4) ;
                //visionPrintFeature(feat, 0) ;
                //visionPrintFeature(feat, 1) ;
                printf("\nresult 1 = %f\n",visionCompareFeature(feat[0], feat[1])) ;
                printf("result 2 = %f\n",visionCompareFeature(feat[0], feat[2])) ;
                printf("result 3 = %f\n",visionCompareFeature(feat[0], feat[3])) ;
                printf("result 4 = %f\n",visionCompareFeature(feat[0], feat[4])) ;

                //Print final picture to SDL
                sdlImageToScr(image , scr, camnb) ;
            }
        }

        //Update SDL window
        SDL_UpdateRect(scr,0,0,0,0) ;
        SDL_Delay(5) ;

        //Listen to keyboard and mousse
        while(SDL_PollEvent(&event)) {
            if(event.type == SDL_QUIT || SDL_GetKeyState(NULL)[SDLK_ESCAPE])
                done = 10 ;
            else if(SDL_GetKeyState(NULL)[SDLK_RETURN] || SDL_GetKeyState(NULL)[SDLK_KP_ENTER])
                done = 10 ;
            else if(SDL_GetKeyState(NULL)[SDLK_DOWN])
                action = 0 ;
            else if(SDL_GetKeyState(NULL)[SDLK_UP])
                action = 1 ;
            else if(SDL_GetKeyState(NULL)[SDLK_KP8] && action != 1 )
                Nmore++ ;
            else if(SDL_GetKeyState(NULL)[SDLK_KP2] && action != 1 )
                Nmore-- ;

            if( event.type == SDL_MOUSEBUTTONUP && action != 1 ) {
                if(event.button.button == SDL_BUTTON_WHEELUP ) {
                    threshold++ ;
                } else if(event.button.button == SDL_BUTTON_WHEELDOWN ) {
                    threshold-- ;
                }
            }
        }

        //Show time between each picture in sec. or FPS
        printf("\n") ;
        time_end = get_time() ;
        show_diff_time( time_end, time_begin, 1) ;
        time_begin = get_time() ;
    }

    /* Flush mem before quit */
        SDL_FreeSurface(scr) ;
        sdlCleanup() ;
        free(picture) ;
        visionImageFree(image) ;
        visionImageFree(final) ;
        for(camnb = 0 ; camnb < camList ; camnb++ )
            deinitCapture(camnb) ;

        printf("\n\nCache vide !\n") ;
	/* END */

    return done ;
}
