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
    char windowsname[25] ;
    int camnb ;
    int done = 1 ;
    int show_circle = 1 ;
    int show_line = 1 ;
    int show_line_one = 0 ;
    int max_fast = 11 ;
    int threshold = 50 ;
    PixelFeature feat[2][1000] ;
    int nb_feat[camList] ;
    int min_score, actual_score, min_pos ;
    FILE *camera_d = NULL ;
    FILE *camera_g = NULL ;

    /* Starting by loading picture or camera and SDL component */
        //Load camera or picture as asked by user
        if(usecam) {
            if( OS == 2 ) {
                if( !escapiInit(0) ) return -2 ;
                for( camnb = 0 ; camnb < camList ; camnb++ ) {
                    escapiCaptureAlloc(capture[camnb], camWidth, camHeight) ;
                    if( !escapiCaptureInit(capture[camnb], camnb) ) return -3 ;
                    printf("Initialisation camera %d\n", camnb+1) ;
                }
            }
            sprintf(windowsname,"Camera") ; //Save windows name
        } else {
            for( camnb = 0 ; camnb < camList ; camnb++ ) {
                if( lodepng_decode32_file(&picture[camnb], &camWidth, &camHeight, argv[2+camnb]) ) return -4 ;
                printf("L'image \"%s\" a ete chargee ...\n", argv[2+camnb]) ;
            }

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
            if(usecam) {
                //Get the actual capture
                if( OS == 2 ) escapiCaptureReq(camnb) ;
                visionCapture2Image(capture[camnb], image) ;

                visionFast(image, final, threshold, max_fast) ;
                visionErosion(final, 10) ;

                nb_feat[camnb] = 0 ;
                for( int x = 0 ; x < image.width ; x++) {
                    for( int y = 0 ; y < image.height ; y++) {
                        if( final.img[x][y] == 255 ) {
                            if(show_circle)
                                drawpoint( final, x, y, 2, 5, 100) ;
                            if( camnb == 0 )
                                visionGetFeature(image, x, y, feat[camnb], nb_feat[camnb]) ;
                            else if( camnb == 1 )
                                visionGetFeature(image, x, y, feat[camnb], nb_feat[camnb]) ;
                            nb_feat[camnb]++ ;
                        }
                    }
                }

                //Print final picture to SDL
                sdlImageToScr(final , scr, camnb) ;
            } else {
                //Get the actual picture
                visionPicture2Image(picture[camnb], image) ;

                visionFast(image, final, threshold, max_fast) ;
                visionErosion(final, 10) ;

                nb_feat[camnb] = 0 ;
                for( int x = 0 ; x < image.width ; x++) {
                    for( int y = 0 ; y < image.height ; y++) {
                        if( final.img[x][y] == 255 ) {
                            if(show_circle)
                                drawpoint( final, x, y, 2, 5, 100) ;
                            if( camnb == 0 )
                                visionGetFeature(image, x, y, feat[camnb], nb_feat[camnb]) ;
                            else if( camnb == 1 )
                                visionGetFeature(image, x, y, feat[camnb], nb_feat[camnb]) ;
                            nb_feat[camnb]++ ;
                        }
                    }
                }

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
            else if(SDL_GetKeyState(NULL)[SDLK_p])
                show_circle = show_circle==1?0:1 ;
            else if(SDL_GetKeyState(NULL)[SDLK_l]) {
                show_line++ ;
                show_line = show_line>2?0:show_line ;
            }

            if( event.type == SDL_MOUSEBUTTONUP ) {
                if(event.button.button == SDL_BUTTON_WHEELUP ) {
                    threshold++ ;
                } else if(event.button.button == SDL_BUTTON_WHEELDOWN ) {
                    threshold-- ;
                }
            }
        }

        if( camList == 2 ) {
            camera_d = fopen("2D_droite_A.txt", "w" ) ;
            camera_g = fopen("2D_gauche_A.txt", "w" ) ;

            for( int i = 0 ; i < nb_feat[0] ; i++ ) {
                min_score = 1000000 ;
                min_pos = nb_feat[1] + 100 ;
                for( int j = 0 ; j < nb_feat[1] ; j++ ) {
                    if( abs(feat[0][i].y-feat[1][j].y) < 10 && abs(feat[0][i].x-feat[1][j].x) < 300 ) {
                        actual_score = visionCompareFeature(feat[0][i], feat[1][j]) ;
                        if( actual_score < min_score ) {
                            min_score = actual_score ;
                            min_pos = j ;
                        }
                    }
                }

                //printf("Meilleur score pour %d : %d\n", i, min_pos) ;
                if( min_pos < nb_feat[1] && min_score < 120 ) {
                    fprintf(camera_d,"%d\t%d\n", feat[1][min_pos].x, feat[1][min_pos].y) ;
                    fprintf(camera_g,"%d\t%d\n", feat[0][i].x, feat[0][i].y) ;
                    if( show_line == 1 )
                        sdlLigne(scr, feat[0][i].x, feat[0][i].y, feat[1][min_pos].x+camWidth, feat[1][min_pos].y, 100) ;
                    else if( show_line == 2 ) {
                        if( abs(event.motion.x-feat[0][i].x) < 10 && abs(event.motion.y-feat[0][i].y) < 10 ) {
                            sdlLigne(scr, feat[0][i].x, feat[0][i].y, feat[1][min_pos].x+camWidth, feat[1][min_pos].y, 100) ;
                        }
                    }
                }
            }
        }

        //Update SDL window
        SDL_UpdateRect(scr,0,0,0,0) ;
        SDL_Delay(5) ;

        //Show time between each picture in sec. or FPS
        printf("\r%d Threshold = %d\tNmore = %d\t",show_line, threshold, max_fast) ;
        time_end = get_time() ;
        show_diff_time( time_end, time_begin, 1) ;
        time_begin = get_time() ;
    }


    /* Flush mem before quit */
        fclose(camera_d) ;
        fclose(camera_g) ;

        SDL_FreeSurface(scr) ;
        sdlCleanup() ;

        visionImageFree(image) ;
        visionImageFree(final) ;
        for(camnb = 0 ; camnb < camList ; camnb++ ) {
            free(picture[camnb]) ;
            //deinitCapture(camnb) ;
        }

        printf("\n\nCache vide !\n") ;
	/* END */

    return done ;
}
