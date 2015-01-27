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
    struct SimpleCapParams capture[argc-2] ;
    unsigned int camWidth = 640 ;
    unsigned int camHeight = 480 ;
    char text[100] ;
    int done = 1 ;
    int error ;



    /* Starting by loading picture or camera and SDL component */
        //Load picture or camera
        for( int i = 0 ; i < argc-2 ; i++ ) {
            if(type)
                escapiComplete(argv[i+2], capture[i], image[i], camWidth, camHeight) ;
            else
                if( visionPicture2Image( argv[i+2], image[i], camWidth, camHeight) ) return -4 ;

        }
        printf("\nCamera/Picture size = %d*%d\n", camWidth, camHeight) ;

        //Load SDL window
        if( !(scr = SDL_SetVideoMode(camWidth*(argc-2), camHeight, 32, SDL_HWSURFACE))) return -6 ; //Allocate scr (SDL buffer) and set param //SDL_HWSURFACE - SDL_FULLSCREEN - SDL_RESIZABLE - SDL_NOFRAME
        SDL_WM_SetCaption("Vision System", NULL) ; //Define windows name
    /* END */

    std::vector<unsigned char> image1;
    image1.resize(camWidth * camHeight * 4);

    printf("Beginning Vision System\n\n") ;

    while( done < 5 ) { //Loop until escape is pressed
        //Print final picture to SDL
        for( int i = 0 ; i < argc-2 ; i++ ) {
            if(type) escapiCaptureReq2(argv[i+2], capture[i], image[i]) ;
            sdlImageToScr(image[i] , scr, i) ;
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
            else if(SDL_GetKeyState(NULL)[SDLK_UP]) {
                for ( int x = 0 ; x < camWidth ; x++) {
                    for ( int y = 0 ; y < camHeight ; y++) {
                        image1[y*camWidth*4+x*4+0] = image[0].img[x][y] ;
                        image1[y*camWidth*4+x*4+1] = image[0].img[x][y] ;
                        image1[y*camWidth*4+x*4+2] = image[0].img[x][y] ;
                        image1[y*camWidth*4+x*4+3] = 255 ;
                    }
                }

                if( (error = lodepng::encode("cam_g.png", image1, camWidth, camHeight)) ) {
                    std::cerr << "encoder error " << error << ": " << lodepng_error_text(error) << std::endl ;
                    return 3 ;
                }

                for ( int x = 0 ; x < camWidth ; x++) {
                    for ( int y = 0 ; y < camHeight ; y++) {
                        image1[y*camWidth*4+x*4+0] = image[1].img[x][y] ;
                        image1[y*camWidth*4+x*4+1] = image[1].img[x][y] ;
                        image1[y*camWidth*4+x*4+2] = image[1].img[x][y] ;
                        image1[y*camWidth*4+x*4+3] = 255 ;
                    }
                }

                if( (error = lodepng::encode("cam_d.png", image1, camWidth, camHeight)) ) {
                    std::cerr << "encoder error " << error << ": " << lodepng_error_text(error) << std::endl ;
                    return 3 ;
                }
            }
        }

        //printf("\r") ;
        time_end = get_time() ;
        show_diff_time( time_end, time_begin, 1) ;
        time_begin = get_time() ;
    }
    printf("\n\nEnding Vision System\n") ;

    /* Flush mem before quit */
        SDL_FreeSurface(scr) ;
        sdlCleanup() ;
        for( int i = 0 ; i < argc-2 ; i++ ) {
            visionImageFree(image[i]) ;
            if(type)
                escapiDeinitCapture(argv[i+2]) ;
        }
        printf("Cache vide !\n") ;
	/* END */

    return done ;
}
