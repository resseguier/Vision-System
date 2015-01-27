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
    char text[100] ;
    int camnb ;
    int done = 1 ;
    int action = 0 ;
    int Nmore = 0 ;
    int threshold = 80 ;
    PixelFound canvas_blue[5] ;
    PixelFound canvas_black[100] ;

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
            if( lodepng_decode32_file(&picture, &camWidth, &camHeight, "data/testt.png") ) return -4 ;
            std::cout << "L'image \"" << "data/canvas_calibration5.png" << "\" a ete chargee ..." << std::endl ;
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

        //Load SDL font
        TTF_Init() ;
        SDL_Rect texteDestination ;
        SDL_Surface* pFontSurface = NULL ;
        TTF_Font* pFont = TTF_OpenFont("./screen/font.ttf",16) ;
    /* END */

    while( done < 5 ) { //Loop until escape is pressed
        for(camnb = 0 ; camnb < (camList>0?camList:1) ; camnb++ ) {
            if( camList ) {
                //Get the actual capture
                if( action == 0 ) {
                    if( OS == 2 ) escapiCaptureReq(camnb) ;
                    visionCapture2Image(capture[camnb], image) ;
                }

                visionGetCanvas(image, final, canvas_blue, canvas_black, action, Nmore, threshold) ;

                //Print final picture to SDL
                sdlImageToScr(final , scr, camnb) ;
            } else {
                //Get the actual picture
                if(!action)
                    visionPicture2Image(picture, image) ;

                visionGetCanvas(image, final, canvas_blue, canvas_black, action, Nmore, threshold) ;

                //Print final picture to SDL
                sdlImageToScr(final , scr, camnb) ;
            }
        }

        //Print text to SDL
        if(!action) {
            sprintf(text,"Total points : %d", canvas_black[0].nb) ;
            pFontSurface = TTF_RenderText_Solid(pFont,text,{0, 0, 0}) ;
            texteDestination.w = pFontSurface->w ;
            texteDestination.h = pFontSurface->h ;
            texteDestination.x = camWidth - 15 - texteDestination.w ;
            texteDestination.y = camHeight - 15 - texteDestination.h ;
            SDL_BlitSurface(pFontSurface,NULL,scr,&texteDestination) ;
        } else {
            sprintf(text,"Points face haute : %d", canvas_blue[0].nb) ;
            pFontSurface = TTF_RenderText_Solid(pFont,text,{0, 0, 0}) ;
            texteDestination.w = pFontSurface->w ;
            texteDestination.h = pFontSurface->h ;
            texteDestination.x = camWidth - 15 - texteDestination.w ;
            texteDestination.y = camHeight - 15*2 - texteDestination.h*2 ;
            SDL_BlitSurface(pFontSurface,NULL,scr,&texteDestination) ;

            sprintf(text,"Points restants : %d", canvas_black[0].nb) ;
            pFontSurface = TTF_RenderText_Solid(pFont,text,{0, 0, 0}) ;
            texteDestination.w = pFontSurface->w ;
            texteDestination.h = pFontSurface->h ;
            texteDestination.x = camWidth - 15 - texteDestination.w ;
            texteDestination.y = camHeight - 15 - texteDestination.h ;
            SDL_BlitSurface(pFontSurface,NULL,scr,&texteDestination) ;
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
        if(!action) {
            printf("\r%d threshold : %d\tNmore : %d\t",action, threshold, Nmore) ;
            time_end = get_time() ;
            show_diff_time( time_end, time_begin, 1) ;
            time_begin = get_time() ;
        } else
            printf("\rPlease use left button to select top side points, right to delete.") ;
    }

    /* Flush mem before quit */
        SDL_FreeSurface(scr) ;
        TTF_CloseFont(pFont);
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
