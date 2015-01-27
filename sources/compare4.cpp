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
    //system("start /i /min Affiche_3D.exe") ;
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
        if( !scilabInit() ) return -7 ;
        if( !sdlInit() ) return -5 ;
        if(type)
            if( !escapiInit(0) ) return -2 ;
	}

    double  time_begin, time_end ;
    SDL_Surface *scr ;
    SDL_Event event ;
    Image2D image[argc-2] ;
    Image2D final ;
    struct SimpleCapParams capture[argc-2] ;
    unsigned int camWidth = 640 ;
    unsigned int camHeight = 480 ;
    char text[100] ;
    int done = 1 ;
    PixelFeature feat[argc-2][2000] ;
    char calib_text[200] ;
    int nb_feat[argc-2] ;
    int threshold = 30 ;
    int threshold_x = 100 ;
    int threshold_y = 70 ;
    int max_fast = 11 ;
    int show_line = 1 ;
    int show_circle = 1 ;
    float dist_moy_real ;
    int total_point ;
    float best_score, actual_score ;
    int min_pos ;
    int best_score2, actual_score2, min_pos2 ;
    int min_score = 120 ;
    int false_score ;
    int pause_system = 0 ;

    /* Starting by loading picture or camera and SDL component */
        //Load picture or camera
        for( int i = 0 ; i < argc-2 ; i++ ) {
            if(type)
                escapiComplete(argv[i+2], capture[i], image[i], camWidth, camHeight) ;
            else
                if( visionPicture2Image( argv[i+2], image[i], camWidth, camHeight) ) return -4 ;
        }
        visionImageAlloc(final, camWidth, camHeight) ;
        printf("\nCamera/Picture size = %d*%d\n", camWidth, camHeight) ;

        //Load Scilab
        sdlFillStdout() ; SendScilabJob("exec ('Stereo_scilab.sci', -1)") ; sdlGetStdout() ;

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

    while( done < 5 ) { //Loop until escape is pressed
        //Print final picture to SDL
        for( int i = 0 ; i < argc-2 ; i++ ) {
            if( pause_system == 0 ) {
                if(type)
                    escapiCaptureReq2(argv[i+2], capture[i], image[i]) ;
                //visionMedian(image[i], final) ;
                //visionCloneImage(final, image[i]) ;
            }

            visionFast(image[i], final, threshold, max_fast) ;
            visionErosionAndfeature(image[i], final, 10, feat[i], nb_feat[i]) ;

            for( int j = 0 ; j < nb_feat[i] ; j++)
                if(show_circle)
                    drawpoint(final, feat[i][j].x, feat[i][j].y, 2, 5, 100) ;

            sdlImageToScr(final , scr, i) ;
        }

        dist_moy_real = 0 ;
        total_point = 0 ;
        if( argc-2 == 2 ) {
            SendScilabJob("fakeview = mopen('temp.txt','w') ;") ;
            for( int i = 0 ; i < nb_feat[0] ; i++ ) {
                best_score = 1000000 ;
                min_pos = nb_feat[1] + 100 ;

                //for( int u = 0 ; u < 9 ; u++ )
                    //printf("h[%d] = %0.2f\n", u, feat[0][i].histogram[u]) ;

                for( int j = 0 ; j < nb_feat[1] ; j++ ) {
                    //if( abs(feat[0][i].y-feat[1][j].y) < threshold_y ) {
                    if( abs(feat[0][i].y-feat[1][j].y) < threshold_y && abs(feat[0][i].x-feat[1][j].x) < threshold_x ) {
                    //if( abs(feat[0][i].y-feat[1][j].y) < threshold_y && abs(feat[0][i].x-feat[1][j].x) < 230 && abs(feat[0][i].x-feat[1][j].x) > 160 ) {
                        actual_score = visionCompareFeature4(feat[0][i], feat[1][j]) ;
                        actual_score = sqrt(actual_score) ;
                        if( actual_score < best_score ) {
                            best_score = actual_score ;
                            min_pos = j ;
                        }
                    }
                }

                if( min_pos < nb_feat[1] /*&& best_score < min_score*/ ) {
                    total_point++ ;
                    dist_moy_real += sqrt((feat[0][i].x-feat[1][min_pos].x)*(feat[0][i].x-feat[1][min_pos].x)+(feat[0][i].y-feat[1][min_pos].y)*(feat[0][i].y-feat[1][min_pos].y)) ;

                    sprintf(calib_text,"[x,y,z]=camera_2D_3D(Glob_g, Glob_d, %d, %d, %d, %d)", feat[0][i].x, feat[0][i].y, feat[1][min_pos].x, feat[1][min_pos].y) ;
                    SendScilabJob(calib_text) ;
                    SendScilabJob("mfprintf(fakeview,'%f\t%f\t%f\n', x, y, z) ;") ;

                    if( show_line == 1 )
                        sdlLigneLight(scr, camWidth*(argc-2), camHeight, feat[0][i].x, feat[0][i].y, feat[1][min_pos].x+camWidth, feat[1][min_pos].y, 100) ;
                    else if( show_line == 0 ) {
                        if( abs(event.motion.x-feat[0][i].x) < 10 && abs(event.motion.y-feat[0][i].y) < 10 ) {
                            printf("\nscore \t= %0.2f %d-%d\n", best_score, min_pos, i) ;
                            visionPrintFeature2(feat[0][i],feat[1][min_pos]) ;
                            //printf("\n\ndist = %0.2f\n",sqrt((feat[0][i].x-feat[1][min_pos].x)*(feat[0][i].x-feat[1][min_pos].x)+(feat[0][i].y-feat[1][min_pos].y)*(feat[0][i].y-feat[1][min_pos].y))) ;
                            sdlLigneLight(scr, camWidth*(argc-2), camHeight, feat[0][i].x, feat[0][i].y, feat[1][min_pos].x+camWidth, feat[1][min_pos].y, 100) ;
                            sdlLigneLight(scr, camWidth*(argc-2), camHeight, feat[0][i].x-threshold_x+camWidth, feat[0][i].y-threshold_y, feat[0][i].x+threshold_x+camWidth, feat[0][i].y-threshold_y, 50) ;
                            sdlLigneLight(scr, camWidth*(argc-2), camHeight, feat[0][i].x-threshold_x+camWidth, feat[0][i].y+threshold_y, feat[0][i].x+threshold_x+camWidth, feat[0][i].y+threshold_y, 50) ;
                            sdlLigneLight(scr, camWidth*(argc-2), camHeight, feat[0][i].x-threshold_x+camWidth, feat[0][i].y-threshold_y, feat[0][i].x-threshold_x+camWidth, feat[0][i].y+threshold_y, 50) ;
                            sdlLigneLight(scr, camWidth*(argc-2), camHeight, feat[0][i].x+threshold_x+camWidth, feat[0][i].y-threshold_y, feat[0][i].x+threshold_x+camWidth, feat[0][i].y+threshold_y, 50) ;
                        }
                    }
                }
            }

            SendScilabJob("mclose(fakeview) ;") ;
            SendScilabJob("clear fakeview ;") ;
            SDL_Delay(5) ;

            if( total_point > 0 ) {
                dist_moy_real /= total_point ;
                do_system("copy /Y temp.txt 2D_meshlab.txt") ;
            }
            SDL_Delay(5) ;

            sprintf(text,"Total points valides : %d", total_point) ;
            pFontSurface = TTF_RenderText_Solid(pFont,text,{255, 255, 255}) ;
            texteDestination.w = pFontSurface->w ;
            texteDestination.h = pFontSurface->h ;
            texteDestination.x = camWidth*2 - 15 - texteDestination.w ;
            texteDestination.y = camHeight - 15 - texteDestination.h ;
            SDL_BlitSurface(pFontSurface,NULL,scr,&texteDestination) ;

            sprintf(text,"Points gauche/droite : %d/%d", nb_feat[0], nb_feat[1]) ;
            pFontSurface = TTF_RenderText_Solid(pFont,text,{255, 255, 255}) ;
            texteDestination.w = pFontSurface->w ;
            texteDestination.h = pFontSurface->h ;
            texteDestination.x = camWidth*2 - 15 - texteDestination.w ;
            texteDestination.y = camHeight - 15*2 - texteDestination.h*2 ;
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
            else if(SDL_GetKeyState(NULL)[SDLK_UP])
                max_fast++ ;
            else if(SDL_GetKeyState(NULL)[SDLK_DOWN])
                max_fast-- ;
            else if(SDL_GetKeyState(NULL)[SDLK_KP7])
                threshold_y++ ;
            else if(SDL_GetKeyState(NULL)[SDLK_KP1])
                threshold_y-- ;
            else if(SDL_GetKeyState(NULL)[SDLK_KP8])
                threshold_x++ ;
            else if(SDL_GetKeyState(NULL)[SDLK_KP2])
                threshold_x-- ;
            else if(SDL_GetKeyState(NULL)[SDLK_KP9])
                min_score++ ;
            else if(SDL_GetKeyState(NULL)[SDLK_KP3])
                min_score-- ;
            else if(SDL_GetKeyState(NULL)[SDLK_s])
                pause_system = pause_system==1?0:1 ;
            else if(SDL_GetKeyState(NULL)[SDLK_p])
                show_circle = show_circle==1?0:1 ;
            else if(SDL_GetKeyState(NULL)[SDLK_l])
                show_line = show_line==1?0:1 ;

            if( event.type == SDL_MOUSEBUTTONUP ) {
                if(event.button.button == SDL_BUTTON_WHEELUP ) {
                    threshold++ ;
                } else if(event.button.button == SDL_BUTTON_WHEELDOWN ) {
                    threshold-- ;
                }
            }
        }
        printf("\r%d y = %d x = %d min = %d Threshold = %d Nmore = %d  ",pause_system, threshold_y, threshold_x, min_score, threshold, max_fast) ;
        time_end = get_time() ;
        show_diff_time( time_end, time_begin, 1) ;
        time_begin = get_time() ;
    }
    printf("\n\nEnding Vision System\n") ;

    /* Flush mem before quit */
        SDL_FreeSurface(scr) ;
        TTF_CloseFont(pFont);
        sdlCleanup() ;
        visionImageFree(final) ;
        for( int i = 0 ; i < argc-2 ; i++ ) {
            visionImageFree(image[i]) ;
            if(type)
                escapiDeinitCapture(argv[i+2]) ;
        }
        if( TerminateScilab(NULL) == FALSE ) return -10 ;
        printf("Cache vide !\n") ;
	/* END */

    return done ;
}
