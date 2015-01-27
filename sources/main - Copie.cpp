#include <VISION/VISION.h>

int main(int argc, char *argv[]) {
    sdlGetStdout() ;
    visionVersion() ;

    SDL_Surface *scr ;
    SDL_Event event ;
    int camList = 1 ;
	struct SimpleCapParams capture[camList] ;
    unsigned int camWidth = 640 ;
    unsigned int camHeight = 480 ;
    char windowsname[25] ;
    int camnb ;
    int done = 1 ;
    SYSTEMTIME  time_start, time_stop ;
    SYSTEMTIME  time_begin, time_end ;

    GetSystemTime(&time_start) ;

    Image2D image, final, final2 ;
    visionImageAlloc(image, camWidth, camHeight) ;
    visionImageAlloc(final, camWidth, camHeight) ;
    visionImageAlloc(final2, camWidth, camHeight) ;

    if ( !escapiInit(0) ) return 1 ;
    for(camnb = 0 ; camnb < camList ; camnb++ ) {
        escapiCaptureAlloc(capture[camnb], camWidth, camHeight) ;
        if ( !escapiCaptureInit(capture[camnb], camnb) ) return 2 ;
    }

    if ( !sdlInit() ) return 3 ;
    if( ! (scr = SDL_SetVideoMode( camWidth*camList, camHeight, 32, SDL_HWSURFACE))) return 4 ; //Allocate scr (SDL buffer) and set param //SDL_HWSURFACE - SDL_FULLSCREEN - SDL_RESIZABLE - SDL_NOFRAME
    sprintf(windowsname,"Vision System %d.%d.%d", VERSIONY, VERSIONM, VERSIOND) ; //Save windows name
    SDL_WM_SetCaption(windowsname , NULL) ; //Define windows name

    SDL_Flip(scr) ;

    while( done < 5 ) { //Loop until escape is pressed
        GetSystemTime(&time_begin) ;
        for(camnb = 0 ; camnb < camList ; camnb++ ) {
            escapiCaptureReq(camnb) ;

            visionCaptureToImage(capture[camnb], image) ;

            //visionBlurImage(image, final, 0, 1) ;
            //visionDog2(image, final) ;
            //visionMedian(image, final) ;
/*
            visionMatrice(image, final, 16, 1, 2, 1,
                                           2, 4, 2,
                                           1, 2, 1) ;
*/
/*
            visionMatriceConv(image, final, 1, 1, 1, 1, 1,
                                                  0, 0, 0,
                                                  -1, -1, -1,

                                                  -1, 0, 1,
                                                  -1, 0, 1,
                                                  -1, 0, 1) ;
*/
            visionGaussian5(image, final) ;
            visionCanny(final, image, 30, 90) ;

            vision2Susan17(final, image, 15) ;

            //visionSusan21(final, image, 5) ;
            //visionSupprMinMax(final, 60, 150, 0) ;

            sdlImageToScr(image, scr, camnb) ; //On affiche l'image finale
        }

        SDL_UpdateRect(scr,0,0,0,0) ;
        //SDL_Delay(5) ;
        GetSystemTime(&time_end) ;
        show_time( time_begin, time_end, 1) ;

        while(SDL_PollEvent(&event)) { //Listen to keyboard and mousse
            if(event.type == SDL_QUIT || SDL_GetKeyState(NULL)[SDLK_ESCAPE])
                done = 10 ;
            else if(SDL_GetKeyState(NULL)[SDLK_RETURN] || SDL_GetKeyState(NULL)[SDLK_KP_ENTER])
                done = 10 ;
        }
    }

    //Flush mem before quit
    for(camnb = 0 ; camnb < camList ; camnb++ )
        deinitCapture(camnb) ;

    visionImageFree(image) ;
    visionImageFree(final) ;
    visionImageFree(final2) ;

	SDL_FreeSurface(scr) ;
    sdlCleanup() ;
	printf("Cache vide !\n\n") ;

    GetSystemTime(&time_stop) ;
    show_time( time_start, time_stop, 0) ;

    return done ;
}
