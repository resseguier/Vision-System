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
#include <SCILAB/api_scilab.h>
#include <SCILAB/stack-c.h> /* Provide functions to access to the memory of Scilab */
#include <SCILAB/call_scilab.h> /* Provide functions to call Scilab engine */
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
    system("start /i /min Affiche_3D.exe") ;
    sdlGetStdout() ;
    visionVersion() ;

	//Check and show OS
    if( OS == 1 )
        printf("  Working on LINUX OS\n\n\n") ;
    else if( OS == 2 )
        printf(" Working on WINDOWS OS\n\n\n") ;
    else {
        printf("   OS not supported !\n  Aborting program...\n\n\n") ;
        return -1 ;
    }

    int camList ;
    int usecam ;
	//Check if user gave a filename
    sscanf(argv[1], "%d", &camList) ;
	if( camList != 0 && camList < 3 ) {
        usecam = 1 ;
	} else if ( camList == 0 && (argc == 3 || argc == 4) ) {
        camList = argc-2 ;
        usecam = 0 ;
	} else {
        printf("Error\n") ;
		printf("usage :\n\t\t ./main type inpufile1 inpufile2") ;
        printf("\n\t\t type : 0 if you want to work on a picture, else point out the number of camera to work with") ;
		return -10 ;
	}

    SDL_Surface *scr ;
    SDL_Event event ;
    struct SimpleCapParams capture[camList] ;
    unsigned char* picture[camList] ;
    Image2D image, final ;
    double  time_begin, time_end ;
    unsigned int camWidth = 640 ;
    unsigned int camHeight = 480 ;
    char calib_text[200] ;
    char text[100] ;
    int camnb ;
    int camname ;
    int done = 1 ;
    int show_circle = 1 ;
    int show_line = 1 ;
    int max_fast = 11 ;
    int threshold = 15 ;
    float dist_moy_real ;
    int threshold_x = 50 ;
    int threshold_y = 10 ;
    int total_point ;
    int min_score = 120 ;
    int false_score ;
    int best_score, actual_score, min_pos ;
    int best_score2, actual_score2, min_pos2 ;
    PixelFeature feat[2][2000] ;
    int nb_feat[camList] ;
    int point_x, point_y ;

    /* Starting by loading picture or camera and SDL component */
        //Load camera or picture as asked by user
        if(usecam) {
            if( OS == 2 ) {
                if( !escapiInit(0) ) return -2 ;
                for( camnb = 0 ; camnb < camList ; camnb++ ) {
                    sscanf(argv[2+camnb], "%d", &camname) ;
                    printf("Initialisation camera %d\n", camname) ;
                    escapiCaptureAlloc(capture[camnb], camWidth, camHeight) ;
                    if( !escapiCaptureInit(capture[camnb], camname) ) return -3 ;
                }
            }
        } else {
            for( camnb = 0 ; camnb < camList ; camnb++ ) {
                if( lodepng_decode32_file(&picture[camnb], &camWidth, &camHeight, argv[2+camnb]) ) return -4 ;
                printf("L'image \"%s\" a ete chargee ...\n", argv[2+camnb]) ;
            }
        }

        visionImageAlloc(image, camWidth, camHeight) ;
        visionImageAlloc(final, camWidth, camHeight) ;
        printf("Largeur de l'image = %d\nHauteur de l'image = %d\n\n", camWidth, camHeight) ;

        //Initialization of Scilab
        #ifdef _MSC_VER
        if( StartScilab(NULL,NULL,NULL) == FALSE )
        #else
        if( StartScilab(getenv("SCI"),NULL,NULL) == FALSE )
        #endif
        {
            fprintf(stderr,"Error while calling StartScilab\n") ;
            return -1 ;
        }

        //Compute calibration parameters
        SendScilabJob("exec ('TP_stereovision_complet.sci', -1)") ;

        //Load SDL window
        if( !sdlInit() ) return -5 ;
        if( !(scr = SDL_SetVideoMode( camWidth*(camList>0?camList:1), camHeight, 32, SDL_HWSURFACE))) return -6 ; //Allocate scr (SDL buffer) and set param //SDL_HWSURFACE - SDL_FULLSCREEN - SDL_RESIZABLE - SDL_NOFRAME
        SDL_WM_SetCaption("Vision System", NULL) ; //Define windows name
        SDL_Flip(scr) ;

        //Load SDL font
        TTF_Init() ;
        SDL_Rect texteDestination ;
        SDL_Surface* pFontSurface = NULL ;
        TTF_Font* pFont = TTF_OpenFont("./screen/font.ttf",16) ;
    /* END */

    while( done < 5 ) { //Loop until escape is pressed
        for(camnb = 0 ; camnb < (camList>0?camList:1) ; camnb++ ) {
            if(usecam) {
                //Get the actual capture
                sscanf(argv[2+camnb], "%d", &camname) ;
                if( OS == 2 ) escapiCaptureReq(camname) ;
                visionCapture2Image(capture[camnb], image) ;

                visionFast(image, final, threshold, max_fast) ;
                visionErosionAndfeature(image, final, 10, feat[camnb], nb_feat[camnb]) ;

                for( int i = 0 ; i < nb_feat[camnb] ; i++)
                    if(show_circle)
                        drawpoint(final, feat[camnb][i].x, feat[camnb][i].y, 2, 5, 100) ;

                //Print final picture to SDL
                sdlImageToScr(image , scr, camnb) ;
            } else {
                //Get the actual picture
                visionPicture3Image(picture[camnb], image) ;

                visionFast(image, final, threshold, max_fast) ;
                visionErosionAndfeature(image, final, 10, feat[camnb], nb_feat[camnb]) ;

                for( int i = 0 ; i < nb_feat[camnb] ; i++)
                    if(show_circle)
                        drawpoint(final, feat[camnb][i].x, feat[camnb][i].y, 2, 5, 100) ;

                //Print final picture to SDL
                sdlImageToScr(final , scr, camnb) ;
            }
        }

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
            else if(SDL_GetKeyState(NULL)[SDLK_KP9])
                threshold_x++ ;
            else if(SDL_GetKeyState(NULL)[SDLK_KP3])
                threshold_x-- ;
            else if(SDL_GetKeyState(NULL)[SDLK_KP8])
                min_score++ ;
            else if(SDL_GetKeyState(NULL)[SDLK_KP2])
                min_score-- ;
            else if(SDL_GetKeyState(NULL)[SDLK_p])
                show_circle = show_circle==1?0:1 ;
            else if(SDL_GetKeyState(NULL)[SDLK_l]) {
                show_line = show_line==1?0:1 ;
            }

            if( event.type == SDL_MOUSEBUTTONUP ) {
                if(event.button.button == SDL_BUTTON_WHEELUP ) {
                    threshold++ ;
                } else if(event.button.button == SDL_BUTTON_WHEELDOWN ) {
                    threshold-- ;
                }
            }
        }

        dist_moy_real = 0 ;
        total_point = 0 ;
        if( camList == 2 ) {
            SendScilabJob("fakeview = mopen('temp.txt','w') ;") ;
            for( int i = 0 ; i < nb_feat[0] ; i++ ) {
                best_score = 1000000 ;
                min_pos = nb_feat[1] + 100 ;
                for( int j = 0 ; j < nb_feat[1] ; j++ ) {
                    //if( abs(feat[0][i].y-feat[1][j].y) < threshold_y && abs(feat[0][i].x-feat[1][j].x) < threshold_x ) {
                    if( abs(feat[0][i].y-feat[1][j].y) < threshold_y && abs(feat[0][i].x-feat[1][j].x) < 230 && abs(feat[0][i].x-feat[1][j].x) > 160 ) {
                        actual_score = visionCompareFeature(feat[0][i], feat[1][j]) ;
                        if( actual_score < best_score ) {
                            best_score = actual_score ;
                            min_pos = j ;
                        }
                    }
                }

                if( min_pos < nb_feat[1] && best_score < min_score ) {
                    best_score2 = 1000000 ;
                    min_pos2 = nb_feat[0] + 100 ;
                    false_score = 0 ;
                    for( int j = 0 ; j < nb_feat[0] ; j++ ) {
                        if( abs(feat[0][i].y-feat[0][j].y) < threshold_y && abs(feat[0][i].x-feat[0][j].x) < threshold_x ) {
                            actual_score2 = visionCompareFeature(feat[1][min_pos], feat[0][j]) ;
                            if( actual_score2 < best_score2 ) {
                                best_score2 = actual_score2 ;
                                min_pos2 = j ;
                            }
                        }
                    }

                    //if( i != min_pos2 )
                    //    false_score = 1 ;

                    if( show_line == 0 )
                        if( abs(event.motion.x-feat[0][i].x) < 10 && abs(event.motion.y-feat[0][i].y) < 10 )
                            printf("\nscore \t= %d %d-%d(%d)\n", best_score, min_pos, min_pos2, i) ;

                    if( false_score == 0 ) {
                        total_point++ ;
                        dist_moy_real += sqrt((feat[0][i].x-feat[1][min_pos].x)*(feat[0][i].x-feat[1][min_pos].x)+(feat[0][i].y-feat[1][min_pos].y)*(feat[0][i].y-feat[1][min_pos].y)) ;

                        sprintf(calib_text,"[x,y,z]=camera_2D_3D(Glob_g, Glob_d, %d, %d, %d, %d)", feat[0][i].x, feat[0][i].y, feat[1][min_pos].x, feat[1][min_pos].y) ;
                        SendScilabJob(calib_text) ;
                        SendScilabJob("mfprintf(fakeview,'%f\t%f\t%f\n', x, y, z) ;") ;

                        if( show_line == 1 )
                            sdlLigneLight(scr, feat[0][i].x, feat[0][i].y, feat[1][min_pos].x+camWidth, feat[1][min_pos].y, 100) ;
                        else if( show_line == 0 ) {
                            if( abs(event.motion.x-feat[0][i].x) < 10 && abs(event.motion.y-feat[0][i].y) < 10 ) {
                                visionPrintFeature2(feat[0][i],feat[1][min_pos]) ;
                                //printf("\n\ndist = %0.2f\n",sqrt((feat[0][i].x-feat[1][min_pos].x)*(feat[0][i].x-feat[1][min_pos].x)+(feat[0][i].y-feat[1][min_pos].y)*(feat[0][i].y-feat[1][min_pos].y))) ;
                                sdlLigneLight(scr, feat[0][i].x, feat[0][i].y, feat[1][min_pos].x+camWidth, feat[1][min_pos].y, 100) ;
                                sdlLigneLight(scr, feat[0][i].x-threshold_x+camWidth, feat[0][i].y-threshold_y, feat[0][i].x+threshold_x+camWidth, feat[0][i].y-threshold_y, 50) ;
                                sdlLigneLight(scr, feat[0][i].x-threshold_x+camWidth, feat[0][i].y+threshold_y, feat[0][i].x+threshold_x+camWidth, feat[0][i].y+threshold_y, 50) ;
                                sdlLigneLight(scr, feat[0][i].x-threshold_x+camWidth, feat[0][i].y-threshold_y, feat[0][i].x-threshold_x+camWidth, feat[0][i].y+threshold_y, 50) ;
                                sdlLigneLight(scr, feat[0][i].x+threshold_x+camWidth, feat[0][i].y-threshold_y, feat[0][i].x+threshold_x+camWidth, feat[0][i].y+threshold_y, 50) ;
                            }
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


        //Show time between each picture in sec. or FPS
        printf("\r%0.2f %d-%d-%d Threshold = %d\tNmore = %d\t", dist_moy_real, threshold_y, threshold_x, min_score, threshold, max_fast) ;
        time_end = get_time() ;
        show_diff_time( time_end, time_begin, 1) ;
        time_begin = get_time() ;
    }


    if( camList == 2 ) {
        SendScilabJob("meshlab = mopen('3D_meshlab.txt','w') ;") ;
        for( int i = 0 ; i < nb_feat[0] ; i++ ) {
            best_score = 1000000 ;
            min_pos = nb_feat[1] + 100 ;
            for( int j = 0 ; j < nb_feat[1] ; j++ ) {
                //if( abs(feat[0][i].y-feat[1][j].y) < threshold_y && abs(feat[0][i].x-feat[1][j].x) < threshold_x ) {
                if( abs(feat[0][i].y-feat[1][j].y) < threshold_y && abs(feat[0][i].x-feat[1][j].x) < 230 && abs(feat[0][i].x-feat[1][j].x) > 160 ) {
                    actual_score = visionCompareFeature(feat[0][i], feat[1][j]) ;
                    if( actual_score < best_score ) {
                        best_score = actual_score ;
                        min_pos = j ;
                    }
                }
            }

            if( min_pos < nb_feat[1] && best_score < min_score ) {
                sprintf(calib_text,"[x,y,z]=camera_2D_3D(Glob_g, Glob_d, %d, %d, %d, %d)", feat[0][i].x, feat[0][i].y, feat[1][min_pos].x, feat[1][min_pos].y) ;
                SendScilabJob(calib_text) ;
                SendScilabJob("mfprintf(meshlab,'%f\t;\t%f\t;\t%f\n', x, y, z) ;") ;
            }
        }
    }

    /* Flush mem before quit */
        SDL_FreeSurface(scr) ;
        sdlCleanup() ;

        visionImageFree(image) ;
        visionImageFree(final) ;
        for(camnb = 0 ; camnb < camList ; camnb++ ) {
            if(usecam)
                deinitCapture(camnb) ;
            else
                free(picture[camnb]) ;
        }

        SendScilabJob("mclose(meshlab) ;") ;
        SendScilabJob("clear meshlab ;") ;
        if( TerminateScilab(NULL) == FALSE ) {
            fprintf(stderr,"Error while calling TerminateScilab\n") ;
            return -2 ;
        }

        printf("\n\nCache vide !\n") ;
	/* END */

    return done ;
}
