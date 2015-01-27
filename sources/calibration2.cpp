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
//SCILAB vers 5.5.1 32bits for windows
#include <SCILAB/api_scilab.h>
#include <SCILAB/stack-c.h> /* Provide functions to access to the memory of Scilab */
#include <SCILAB/call_scilab.h> /* Provide functions to call Scilab engine */
#include <omp.h>
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
    visionVersion() ;

	//Check and show OS
    if( OS == 1 )
        printf("  Working on LINUX OS\n\n\n") ;
    else if( OS == 2 )
        printf(" Working on WINDOWS OS\n\n\n") ;
    else {
        printf("Error\n   OS not supported !\n  Aborting program...\n\n\n") ;
        return -1 ;
    }

    int type ;
    sscanf(argv[1], "%d", &type) ;
    if( argc < 3 && (type < 0 || type > 1) ) {
        printf("Error\n") ;
		printf("usage :\n\t\t ./main type inpufile1 inpufile2 etc\n") ;
        printf("\t\t type : 0 if you want to work on a picture (PNG and BMP only)\n") ;
        printf("\t\t        1 if you want to work on a camera\n") ;
        printf("\t\t        then point out the number of camera(s) or name of the picture(s) to work with\n\n") ;
		return -10 ;
	} else {
        printf("Will work with %d %s...\n", argc-2, type==0?"picture(s)":"camera(s)") ;
        if( !sdlInit() ) return -5 ;
        if(type)
            if( !escapiInit(0) ) return -2 ;
	}

    double  time_begin, time_end ;
    SDL_Surface *scr ;
    SDL_Event event ;
    Image2D image[argc-2] ;
    Image2D final[argc-2] ;
    struct SimpleCapParams capture[argc-2] ;
    unsigned int camWidth = 640 ;
    unsigned int camHeight = 480 ;
    char text[100] ;
    int done = 1 ;
    int action = 0 ;
    int Nmore = 0 ;
    int Emax = 12 ;
    int threshold = 80 ;
    PixelFound canvas_blue[argc-2][5] ;
    PixelFound canvas_black[argc-2][100] ;
    int finished[argc-2] ;

    /* Starting by loading picture or camera and SDL component */
        //Load picture or camera
        for( int i = 0 ; i < argc-2 ; i++ ) {
            if(type)
                escapiComplete(argv[i+2], capture[i], image[i], camWidth, camHeight) ;
            else
                if( visionPicture2Image( argv[i+2], image[i], camWidth, camHeight) ) return -4 ;
            visionImageAlloc(final[i], camWidth, camHeight) ;
        }
        printf("\nCamera/Picture size = %d*%d\n", camWidth, camHeight) ;

        //Load SDL window
        if( !(scr = SDL_SetVideoMode(camWidth*(argc-2), camHeight, 32, SDL_HWSURFACE))) return -6 ; //Allocate scr (SDL buffer) and set param //SDL_HWSURFACE - SDL_FULLSCREEN - SDL_RESIZABLE - SDL_NOFRAME
        SDL_WM_SetCaption("Vision System", NULL) ; //Define windows name

        //Load SDL font
        TTF_Init() ;
        SDL_Rect texteDestination ;
        SDL_Surface* pFontSurface = NULL ;
        TTF_Font* pFont = TTF_OpenFont("./screen/font.ttf",16) ;
    /* END */

    printf("Beginning Vision System\n\n") ;
    int camnb = 0 ;
    while( done < 5 ) { //Loop until escape is pressed
        while( action > 0 ) {
            while(SDL_PollEvent(&event)) {
                if(event.motion.x <= camWidth) {
                    sprintf(text,"2D_gauche.txt") ;
                    camnb = 0 ;
                } else if(event.motion.x > camWidth ) {
                    sprintf(text,"2D_droite.txt") ;
                    camnb = 1 ;
                }
            }

            if( finished[camnb] == 0 )
                if(visionGetCanvas2(image[camnb], final[camnb], camnb, canvas_blue[camnb], canvas_black[camnb], action, Nmore, Emax, threshold, finished[camnb]) == -1 ) action = -1 ;
            sdlImageToScr(final[camnb] , scr, camnb) ;

            //Print text to SDL
            sprintf(text,"Points face haute : %d", canvas_blue[camnb][0].nb) ;
            pFontSurface = TTF_RenderText_Solid(pFont,text,{0, 0, 0}) ;
            texteDestination.w = pFontSurface->w ;
            texteDestination.h = pFontSurface->h ;
            texteDestination.x = camWidth*(1+camnb) - 15 - texteDestination.w ;
            texteDestination.y = camHeight - 15*2 - texteDestination.h*2 ;
            SDL_BlitSurface(pFontSurface,NULL,scr,&texteDestination) ;

            sprintf(text,"Points restants : %d", canvas_black[camnb][0].nb) ;
            pFontSurface = TTF_RenderText_Solid(pFont,text,{0, 0, 0}) ;
            texteDestination.w = pFontSurface->w ;
            texteDestination.h = pFontSurface->h ;
            texteDestination.x = camWidth*(1+camnb) - 15 - texteDestination.w ;
            texteDestination.y = camHeight - 15 - texteDestination.h ;
            SDL_BlitSurface(pFontSurface,NULL,scr,&texteDestination) ;

            if( !action && argc-2 > 1 )
                for( int i = 0 ; i < 12 ; i++ )
                    sdlLigneDark(scr, camWidth*(argc-2), camHeight, canvas_black[0][i].x, canvas_black[0][i].y, canvas_black[1][i].x+camWidth, canvas_black[1][i].y, 100) ;

            //Update SDL window
            SDL_UpdateRect(scr,0,0,0,0) ;
            SDL_Delay(5) ;
            if(!action)
                SDL_Delay(500) ;
        }

        action = 0 ;

        for( int i = 0 ; i < argc-2 ; i++ ) {
            if(type) escapiCaptureReq2(argv[i+2], capture[i], image[i]) ;
            canvas_blue[i][0].nb = 0 ;
            canvas_black[i][0].nb = 0 ;
            finished[i] = 0 ;
            visionFindCanvas2(image[i], final[i], canvas_black[i], threshold, Nmore, Emax) ;
            sdlImageToScr(final[i] , scr, i) ;

            //Print text to SDL
            sprintf(text,"Total points : %d", canvas_black[i][0].nb) ;
            pFontSurface = TTF_RenderText_Solid(pFont,text,{0, 0, 0}) ;
            texteDestination.w = pFontSurface->w ;
            texteDestination.h = pFontSurface->h ;
            texteDestination.x = camWidth*(1+i) - 15 - texteDestination.w ;
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
                action-- ;
            else if(SDL_GetKeyState(NULL)[SDLK_UP])
                action = argc-2 ;
            else if(SDL_GetKeyState(NULL)[SDLK_KP8] && action != 1 )
                Nmore++ ;
            else if(SDL_GetKeyState(NULL)[SDLK_KP2] && action != 1 )
                Nmore-- ;
            else if(SDL_GetKeyState(NULL)[SDLK_KP9] && action != 1 )
                Emax++ ;
            else if(SDL_GetKeyState(NULL)[SDLK_KP3] && action != 1 )
                Emax-- ;

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
            printf("\r%d threshold : %d\tNmore : %d  Emax : %d\t",action, threshold, Nmore, Emax) ;
            time_end = get_time() ;
            show_diff_time( time_end, time_begin, 1) ;
            time_begin = get_time() ;
        } else
            printf("\rPlease use left button to select top side points, right to delete.") ;
    }
    printf("\n\nEnding Vision System\n") ;

    /* Flush mem before quit */
        SDL_FreeSurface(scr) ;
        TTF_CloseFont(pFont);
        sdlCleanup() ;
        for( int i = 0 ; i < argc-2 ; i++ ) {
            visionImageFree(final[i]) ;
            visionImageFree(image[i]) ;
            if(type)
                escapiDeinitCapture(argv[i+2]) ;
        }
        printf("Cache vide !\n") ;
	/* END */

    return done ;
}
