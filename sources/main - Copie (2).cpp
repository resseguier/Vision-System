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
//0 : all done without trouble
/* END */

int main(int argc, char *argv[]) {
    sdlGetStdout() ;

    if( OS == 1 )
        printf("  Working on LINUX OS\n\n\n") ;
    else if( OS == 2 )
        printf(" Working on WINDOWS OS\n\n\n") ;
    else {
        printf("   OS not supported !\n  Aborting program...\n\n\n") ;
        return -1 ;
    }

    struct SimpleCapParams capture[camList] ;
    unsigned char* picture;
    Image2D image, final ;
    SDL_Surface *scr ;
    SDL_Event event ;
    int camList = 0 ; //0 if you want to work on a picture, else point out the number of camera to work with
    double  time_begin, time_end ;
    unsigned int camWidth = 640 ;
    unsigned int camHeight = 480 ;
    char windowsname[25] ;
    int camnb ;
    int done = 1 ;
    int threshold = 80 ;
    int divider = 10 ;
    int do_canvas = 0 ;
    int stop_canvas = 0 ;

    /* Starting by loading picture or camera and SDL component */
    visionVersion() ;
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
        sprintf(windowsname,"Vision System %d.%d.%d", VERSIOND, VERSIONM, VERSIONY) ; //Save windows name
    } else {
        if( lodepng_decode32_file(&picture, &camWidth, &camHeight, "data/canvas_calibration5.png") ) return -4 ;
        std::cout << "L'image \"" << "data/canvas_calibration5.png" << "\" a ete chargee ..." << std::endl ;
        sprintf(windowsname,"Vision System %d.%d.%d", VERSIOND, VERSIONM, VERSIONY) ; //Save windows name
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
                if( OS == 2 ) escapiCaptureReq(camnb) ;
                visionCapture2Image(capture[camnb], image) ;

                PixelFound canvas_blue[5] ;
                PixelFound canvas_black[100] ;

                int user_valid = 0 ;
                int pos_x, pos_y ;
                canvas_blue[0].nb == 0 ;
                canvas_black[0].nb == 0 ;

                if( stop_canvas ) {
                    while( stop_canvas != 0 ) {
                        SDL_Event event_out ;
                        while(SDL_PollEvent(&event_out)) {
                            if(SDL_GetKeyState(NULL)[SDLK_LEFT])
                                stop_canvas = 0 ;
                            else if(SDL_GetKeyState(NULL)[SDLK_KP8])
                                divider++ ;
                            else if(SDL_GetKeyState(NULL)[SDLK_KP2])
                                divider-- ;
                            else if(SDL_GetKeyState(NULL)[SDLK_UP]) {
                                do_canvas = 1 ;
                                stop_canvas = 0 ;
                            }

                            if( event_out.type == SDL_MOUSEBUTTONUP ) {
                                if(event_out.button.button == SDL_BUTTON_WHEELUP ) {
                                    threshold++ ;
                                } else if(event_out.button.button == SDL_BUTTON_WHEELDOWN ) {
                                    threshold-- ;
                                }
                            }
                        }

                        visionFindCanvas(image, final, canvas_black, threshold, divider) ;

                        for( int i = 0 ; i < canvas_black[0].nb ; i++ )
                            drawpoint( final, canvas_black[i].x, canvas_black[i].y, 2, 20, 100) ;


                        sdlImageToScr(final , scr, camnb) ; //On affiche l'image finale
                        SDL_UpdateRect(scr,0,0,0,0) ;
                        SDL_Delay(5) ;

                        printf("\n\n\nthreshold : %d\n", threshold) ;
                        printf("divider : %d\n", divider) ;
                    }
                } else {
                    visionFindCanvas(image, final, canvas_black, threshold, divider) ;

                    for( int i = 0 ; i < canvas_black[0].nb ; i++ )
                        drawpoint( final, canvas_black[i].x, canvas_black[i].y, 2, 20, 100) ;

                    sdlImageToScr(final , scr, camnb) ; //On affiche l'image finale
                    SDL_UpdateRect(scr,0,0,0,0) ;
                    SDL_Delay(5) ;
                }

                if( do_canvas ) {
                    if( canvas_black[0].nb > 11 ) {
                        SDL_Event event_in ;
                        while( user_valid < 4 ) {
                            while(SDL_PollEvent(&event_in)) {
                                if(SDL_GetKeyState(NULL)[SDLK_DOWN]) {
                                    user_valid = 5 ;
                                }
                                else {
                                    if( event_in.type == SDL_MOUSEMOTION ) {
                                        pos_x = event_in.motion.x;
                                        pos_y = event_in.motion.y;
                                    }
                                    if( event_in.type == SDL_MOUSEBUTTONUP ) {
                                        if(event_in.button.button == SDL_BUTTON_LEFT ) {
                                            //printf("gauche : %d - %d\n",pos_x,pos_y) ;

                                            for( int i = 0 ; i < canvas_black[0].nb ; i++ ) {
                                                if( abs(pos_x-canvas_black[i].x) < 15 && abs(pos_y-canvas_black[i].y) < 15 ) {
                                                    canvas_blue[user_valid].x = canvas_black[i].x ;
                                                    canvas_blue[user_valid].y = canvas_black[i].y ;
                                                    user_valid++ ;
                                                    canvas_blue[0].nb = user_valid ;

                                                    drawpoint( final, canvas_black[i].x, canvas_black[i].y, 0, 10, 100) ;

                                                    for( int j = i ; j < canvas_black[0].nb ; j++ ) {
                                                        canvas_black[j].x = canvas_black[j+1].x ;
                                                        canvas_black[j].y = canvas_black[j+1].y ;
                                                    }

                                                    canvas_black[0].nb-- ;

                                                    sdlImageToScr(final , scr, camnb) ; //On affiche l'image finale
                                                    SDL_UpdateRect(scr,0,0,0,0) ;
                                                    SDL_Delay(5) ;

                                                    i = 1000 ;
                                                }
                                            }
                                        } else if(event_in.button.button == SDL_BUTTON_RIGHT ) {
                                            //printf("droite : %d - %d\n",pos_x,pos_y) ;

                                            for( int i = 0 ; i < canvas_black[0].nb ; i++ ) {
                                                if( abs(pos_x-canvas_black[i].x) < 15 && abs(pos_y-canvas_black[i].y) < 15 ) {
                                                    drawpoint( final, canvas_black[i].x, canvas_black[i].y, 1, 10, 100) ;

                                                    for( int j = i ; j < canvas_black[0].nb ; j++ ) {
                                                        canvas_black[j].x = canvas_black[j+1].x ;
                                                        canvas_black[j].y = canvas_black[j+1].y ;
                                                    }

                                                    canvas_black[0].nb-- ;

                                                    sdlImageToScr(final , scr, camnb) ; //On affiche l'image finale
                                                    SDL_UpdateRect(scr,0,0,0,0) ;
                                                    SDL_Delay(5) ;

                                                    i = 1000 ;
                                                }
                                            }
                                        }
                                    }
                                }
                            }
                        }

                        if( user_valid == 4 ) {
                            //printf("ok !\n") ;
                            //for( int i = 0 ; i < canvas_blue[0].nb ; i++ )
                                //printf("%d = %d - %d\n",i,canvas_blue[i].x,canvas_blue[i].y) ;

                            for( int x = 0 ; x < final.width ; x++) {
                                for( int y = 0 ; y < final.height ; y++) {
                                    final.img[x][y] = 255 ;
                                    if( image.img[x][y] < threshold )
                                        final.img[x][y] = 150 ;
                                }
                            }


                            for( int i = 0 ; i < canvas_black[0].nb ; i++ )
                                drawpoint( final, canvas_black[i].x, canvas_black[i].y, 2, 20, 100) ;


                            for( int i = 0 ; i < canvas_blue[0].nb ; i++ )
                                drawpoint( final, canvas_blue[i].x, canvas_blue[i].y, 0, 20, 100) ;


                            sdlImageToScr(final , scr, camnb) ; //On affiche l'image finale
                            SDL_UpdateRect(scr,0,0,0,0) ;
                            SDL_Delay(5) ;

                            Sleep(500) ;
                        }
                    }

                    if( canvas_blue[0].nb == 4 && canvas_black[0].nb == 8 ) {
                        PixelFound canvas_blue_corrected[5] ;
                        PixelFound canvas_black_corrected[9] ;
                        PixelFound canvas_final[12] ;
                        PixelFound center_blue ;
                        PixelFound center_black ;
                        int l, r, b, t ;

                        canvas_final[0].nb == 0 ;

                        center_blue.x = center_blue.y = 0 ;
                        center_black.x = center_black.y = 0 ;

                        for( int i = 0 ; i < canvas_blue[0].nb ; i++ ) {
                            center_blue.x += canvas_blue[i].x ;
                            center_blue.y += canvas_blue[i].y ;
                        }
                        center_blue.x /= canvas_blue[0].nb ;
                        center_blue.y /= canvas_blue[0].nb ;

                        for( int i = 0 ; i < canvas_black[0].nb ; i++ ) {
                            center_black.x += canvas_black[i].x ;
                            center_black.y += canvas_black[i].y ;
                        }
                        center_black.x /= canvas_black[0].nb ;
                        center_black.y /= canvas_black[0].nb ;

                        //Rotate all point for later compute
                        visionRotateCanvas(center_black, center_blue, canvas_blue, canvas_blue_corrected) ;
                        visionRotateCanvas(center_black, center_blue, canvas_black, canvas_black_corrected) ;

                        //Get real 3D coordinate of all blue points
                        l = r = b = t = 0 ;
                        for( int i = 0 ; i < canvas_blue_corrected[0].nb ; i++ ) {
                                if( canvas_blue_corrected[i].x < canvas_blue_corrected[l].x )
                                    l = i ;

                                if( canvas_blue_corrected[i].x > canvas_blue_corrected[r].x )
                                    r = i ;

                                if( canvas_blue_corrected[i].y > canvas_blue_corrected[b].y )
                                    b = i ;

                                if( canvas_blue_corrected[i].y < canvas_blue_corrected[t].y )
                                    t = i ;
                        }
                        canvas_final[0].x = canvas_blue[l].x ;
                        canvas_final[0].y = canvas_blue[l].y ;
                        canvas_final[1].x = canvas_blue[r].x ;
                        canvas_final[1].y = canvas_blue[r].y ;
                        canvas_final[2].x = canvas_blue[b].x ;
                        canvas_final[2].y = canvas_blue[b].y ;
                        canvas_final[3].x = canvas_blue[t].x ;
                        canvas_final[3].y = canvas_blue[t].y ;

                        //Get real 3D coordinate of all left black points
                        for( int i = 0 ; i < canvas_black_corrected[0].nb ; i++ )
                            if( canvas_black_corrected[i].x < center_black.x )
                                l = r = b = t = i ;

                        for( int i = 0 ; i < canvas_black_corrected[0].nb ; i++ ) {
                            if( canvas_black_corrected[i].x < center_black.x ) {
                                if( canvas_black_corrected[i].x < canvas_black_corrected[l].x )
                                    l = i ;

                                if( canvas_black_corrected[i].x > canvas_black_corrected[r].x )
                                    r = i ;

                                if( canvas_black_corrected[i].y > canvas_black_corrected[b].y )
                                    b = i ;

                                if( canvas_black_corrected[i].y < canvas_black_corrected[t].y )
                                    t = i ;
                            }
                        }
                        canvas_final[4].x = canvas_black[l].x ;
                        canvas_final[4].y = canvas_black[l].y ;
                        canvas_final[5].x = canvas_black[r].x ;
                        canvas_final[5].y = canvas_black[r].y ;
                        canvas_final[6].x = canvas_black[b].x ;
                        canvas_final[6].y = canvas_black[b].y ;
                        canvas_final[7].x = canvas_black[t].x ;
                        canvas_final[7].y = canvas_black[t].y ;

                        //Get real 3D coordinate of all right black points
                        for( int i = 0 ; i < canvas_black_corrected[0].nb ; i++ )
                            if( canvas_black_corrected[i].x > center_black.x )
                                l = r = b = t = i ;

                        for( int i = 0 ; i < canvas_black_corrected[0].nb ; i++ ) {
                            if( canvas_black_corrected[i].x > center_black.x ) {
                                if( canvas_black_corrected[i].x < canvas_black_corrected[l].x )
                                    l = i ;

                                if( canvas_black_corrected[i].x > canvas_black_corrected[r].x )
                                    r = i ;

                                if( canvas_black_corrected[i].y > canvas_black_corrected[b].y )
                                    b = i ;

                                if( canvas_black_corrected[i].y < canvas_black_corrected[t].y )
                                    t = i ;
                            }
                        }
                        canvas_final[8].x = canvas_black[l].x ;
                        canvas_final[8].y = canvas_black[l].y ;
                        canvas_final[9].x = canvas_black[r].x ;
                        canvas_final[9].y = canvas_black[r].y ;
                        canvas_final[10].x = canvas_black[b].x ;
                        canvas_final[10].y = canvas_black[b].y ;
                        canvas_final[11].x = canvas_black[t].x ;
                        canvas_final[11].y = canvas_black[t].y ;


                        FILE *camera_d = NULL ;
                        camera_d = fopen("2D_droite.txt", "w" ) ;

                        for( int i = 0 ; i < 12 ; i++ ) {
                            printf("%d = %d-%d\n", i, canvas_final[i].x, canvas_final[i].y) ;
                            fprintf(camera_d,"%d\t%d\n", canvas_final[i].x, canvas_final[i].y) ;
                        }

                        fclose(camera_d) ;


                        drawpoint( final, center_blue.x, center_blue.y, 3, 15, 100) ;
                        drawpoint( final, center_black.x, center_black.y, 3, 15, 100) ;

                        sdlImageToScr(final , scr, camnb) ; //On affiche l'image finale
                        SDL_UpdateRect(scr,0,0,0,0) ;
                        SDL_Delay(5) ;
                        Sleep(1000) ;

                        for( int x = 0 ; x < final.width ; x++ )
                            for( int y = 0 ; y < final.height ; y++ )
                                final.img[x][y] = 255 ;

                        for( int i = 0 ; i < canvas_blue_corrected[0].nb ; i++ )
                            drawpoint( final, canvas_blue_corrected[i].x, canvas_blue_corrected[i].y, 1, 20, 100) ;

                        for( int i = 0 ; i < canvas_black_corrected[0].nb ; i++ )
                            drawpoint( final, canvas_black_corrected[i].x, canvas_black_corrected[i].y, 3, 20, 100) ;

                        sdlImageToScr(final , scr, camnb) ; //On affiche l'image finale
                        SDL_UpdateRect(scr,0,0,0,0) ;
                        SDL_Delay(5) ;
                        Sleep(1000) ;
                    }

                }

                sdlImageToScr(final , scr, camnb) ; //On affiche l'image finale
                SDL_UpdateRect(scr,0,0,0,0) ;
                SDL_Delay(5) ;
            } else {
                visionPicture2Image(picture, image) ;

                PixelFound canvas_blue[5] ;
                PixelFound canvas_black[100] ;

                int user_valid = 0 ;
                int pos_x, pos_y ;
                canvas_blue[0].nb == 0 ;
                canvas_black[0].nb == 0 ;

                if( stop_canvas ) {
                    while( stop_canvas != 0 ) {
                        SDL_Event event_out ;
                        while(SDL_PollEvent(&event_out)) {
                            if(SDL_GetKeyState(NULL)[SDLK_LEFT])
                                stop_canvas = 0 ;
                            else if(SDL_GetKeyState(NULL)[SDLK_KP8])
                                divider++ ;
                            else if(SDL_GetKeyState(NULL)[SDLK_KP2])
                                divider-- ;
                            else if(SDL_GetKeyState(NULL)[SDLK_UP]) {
                                do_canvas = 1 ;
                                stop_canvas = 0 ;
                            }

                            if( event_out.type == SDL_MOUSEBUTTONUP ) {
                                if(event_out.button.button == SDL_BUTTON_WHEELUP ) {
                                    threshold++ ;
                                } else if(event_out.button.button == SDL_BUTTON_WHEELDOWN ) {
                                    threshold-- ;
                                }
                            }
                        }

                        visionFindCanvas(image, final, canvas_black, threshold, divider) ;

                        for( int i = 0 ; i < canvas_black[0].nb ; i++ )
                            drawpoint( final, canvas_black[i].x, canvas_black[i].y, 2, 20, 100) ;


                        sdlImageToScr(final , scr, camnb) ; //On affiche l'image finale
                        SDL_UpdateRect(scr,0,0,0,0) ;
                        SDL_Delay(5) ;

                        printf("\n\n\nthreshold : %d\n", threshold) ;
                        printf("divider : %d\n", divider) ;
                    }
                } else {
                    visionFindCanvas(image, final, canvas_black, threshold, divider) ;

                    for( int i = 0 ; i < canvas_black[0].nb ; i++ )
                        drawpoint( final, canvas_black[i].x, canvas_black[i].y, 2, 20, 100) ;

                    sdlImageToScr(final , scr, camnb) ; //On affiche l'image finale
                    SDL_UpdateRect(scr,0,0,0,0) ;
                    SDL_Delay(5) ;
                }

                if( do_canvas ) {
                    if( canvas_black[0].nb > 11 ) {
                        SDL_Event event_in ;
                        while( user_valid < 4 ) {
                            while(SDL_PollEvent(&event_in)) {
                                if(SDL_GetKeyState(NULL)[SDLK_DOWN]) {
                                    user_valid = 5 ;
                                }
                                else {
                                    if( event_in.type == SDL_MOUSEMOTION ) {
                                        pos_x = event_in.motion.x;
                                        pos_y = event_in.motion.y;
                                    }
                                    if( event_in.type == SDL_MOUSEBUTTONUP ) {
                                        if(event_in.button.button == SDL_BUTTON_LEFT ) {
                                            //printf("gauche : %d - %d\n",pos_x,pos_y) ;

                                            for( int i = 0 ; i < canvas_black[0].nb ; i++ ) {
                                                if( abs(pos_x-canvas_black[i].x) < 15 && abs(pos_y-canvas_black[i].y) < 15 ) {
                                                    canvas_blue[user_valid].x = canvas_black[i].x ;
                                                    canvas_blue[user_valid].y = canvas_black[i].y ;
                                                    user_valid++ ;
                                                    canvas_blue[0].nb = user_valid ;

                                                    drawpoint( final, canvas_black[i].x, canvas_black[i].y, 0, 10, 100) ;

                                                    for( int j = i ; j < canvas_black[0].nb ; j++ ) {
                                                        canvas_black[j].x = canvas_black[j+1].x ;
                                                        canvas_black[j].y = canvas_black[j+1].y ;
                                                    }

                                                    canvas_black[0].nb-- ;

                                                    sdlImageToScr(final , scr, camnb) ; //On affiche l'image finale
                                                    SDL_UpdateRect(scr,0,0,0,0) ;
                                                    SDL_Delay(5) ;

                                                    i = 1000 ;
                                                }
                                            }
                                        } else if(event_in.button.button == SDL_BUTTON_RIGHT ) {
                                            //printf("droite : %d - %d\n",pos_x,pos_y) ;

                                            for( int i = 0 ; i < canvas_black[0].nb ; i++ ) {
                                                if( abs(pos_x-canvas_black[i].x) < 15 && abs(pos_y-canvas_black[i].y) < 15 ) {
                                                    drawpoint( final, canvas_black[i].x, canvas_black[i].y, 1, 10, 100) ;

                                                    for( int j = i ; j < canvas_black[0].nb ; j++ ) {
                                                        canvas_black[j].x = canvas_black[j+1].x ;
                                                        canvas_black[j].y = canvas_black[j+1].y ;
                                                    }

                                                    canvas_black[0].nb-- ;

                                                    sdlImageToScr(final , scr, camnb) ; //On affiche l'image finale
                                                    SDL_UpdateRect(scr,0,0,0,0) ;
                                                    SDL_Delay(5) ;

                                                    i = 1000 ;
                                                }
                                            }
                                        }
                                    }
                                }
                            }
                        }

                        if( user_valid == 4 ) {
                            //printf("ok !\n") ;
                            //for( int i = 0 ; i < canvas_blue[0].nb ; i++ )
                                //printf("%d = %d - %d\n",i,canvas_blue[i].x,canvas_blue[i].y) ;

                            for( int x = 0 ; x < final.width ; x++) {
                                for( int y = 0 ; y < final.height ; y++) {
                                    final.img[x][y] = 255 ;
                                    if( image.img[x][y] < threshold )
                                        final.img[x][y] = 150 ;
                                }
                            }


                            for( int i = 0 ; i < canvas_black[0].nb ; i++ )
                                drawpoint( final, canvas_black[i].x, canvas_black[i].y, 2, 20, 100) ;


                            for( int i = 0 ; i < canvas_blue[0].nb ; i++ )
                                drawpoint( final, canvas_blue[i].x, canvas_blue[i].y, 0, 20, 100) ;


                            sdlImageToScr(final , scr, camnb) ; //On affiche l'image finale
                            SDL_UpdateRect(scr,0,0,0,0) ;
                            SDL_Delay(5) ;

                            Sleep(500) ;
                        }
                    }

                    if( canvas_blue[0].nb == 4 && canvas_black[0].nb == 8 ) {
                        PixelFound canvas_blue_corrected[5] ;
                        PixelFound canvas_black_corrected[9] ;
                        PixelFound canvas_final[12] ;
                        PixelFound center_blue ;
                        PixelFound center_black ;
                        int l, r, b, t ;

                        canvas_final[0].nb == 0 ;

                        center_blue.x = center_blue.y = 0 ;
                        center_black.x = center_black.y = 0 ;

                        for( int i = 0 ; i < canvas_blue[0].nb ; i++ ) {
                            center_blue.x += canvas_blue[i].x ;
                            center_blue.y += canvas_blue[i].y ;
                        }
                        center_blue.x /= canvas_blue[0].nb ;
                        center_blue.y /= canvas_blue[0].nb ;

                        for( int i = 0 ; i < canvas_black[0].nb ; i++ ) {
                            center_black.x += canvas_black[i].x ;
                            center_black.y += canvas_black[i].y ;
                        }
                        center_black.x /= canvas_black[0].nb ;
                        center_black.y /= canvas_black[0].nb ;

                        //Rotate all point for later compute
                        visionRotateCanvas(center_black, center_blue, canvas_blue, canvas_blue_corrected) ;
                        visionRotateCanvas(center_black, center_blue, canvas_black, canvas_black_corrected) ;

                        //Get real 3D coordinate of all blue points
                        l = r = b = t = 0 ;
                        for( int i = 0 ; i < canvas_blue_corrected[0].nb ; i++ ) {
                                if( canvas_blue_corrected[i].x < canvas_blue_corrected[l].x )
                                    l = i ;

                                if( canvas_blue_corrected[i].x > canvas_blue_corrected[r].x )
                                    r = i ;

                                if( canvas_blue_corrected[i].y > canvas_blue_corrected[b].y )
                                    b = i ;

                                if( canvas_blue_corrected[i].y < canvas_blue_corrected[t].y )
                                    t = i ;
                        }
                        canvas_final[0].x = canvas_blue[l].x ;
                        canvas_final[0].y = canvas_blue[l].y ;
                        canvas_final[1].x = canvas_blue[r].x ;
                        canvas_final[1].y = canvas_blue[r].y ;
                        canvas_final[2].x = canvas_blue[b].x ;
                        canvas_final[2].y = canvas_blue[b].y ;
                        canvas_final[3].x = canvas_blue[t].x ;
                        canvas_final[3].y = canvas_blue[t].y ;

                        //Get real 3D coordinate of all left black points
                        for( int i = 0 ; i < canvas_black_corrected[0].nb ; i++ )
                            if( canvas_black_corrected[i].x < center_black.x )
                                l = r = b = t = i ;

                        for( int i = 0 ; i < canvas_black_corrected[0].nb ; i++ ) {
                            if( canvas_black_corrected[i].x < center_black.x ) {
                                if( canvas_black_corrected[i].x < canvas_black_corrected[l].x )
                                    l = i ;

                                if( canvas_black_corrected[i].x > canvas_black_corrected[r].x )
                                    r = i ;

                                if( canvas_black_corrected[i].y > canvas_black_corrected[b].y )
                                    b = i ;

                                if( canvas_black_corrected[i].y < canvas_black_corrected[t].y )
                                    t = i ;
                            }
                        }
                        canvas_final[4].x = canvas_black[l].x ;
                        canvas_final[4].y = canvas_black[l].y ;
                        canvas_final[5].x = canvas_black[r].x ;
                        canvas_final[5].y = canvas_black[r].y ;
                        canvas_final[6].x = canvas_black[b].x ;
                        canvas_final[6].y = canvas_black[b].y ;
                        canvas_final[7].x = canvas_black[t].x ;
                        canvas_final[7].y = canvas_black[t].y ;

                        //Get real 3D coordinate of all right black points
                        for( int i = 0 ; i < canvas_black_corrected[0].nb ; i++ )
                            if( canvas_black_corrected[i].x > center_black.x )
                                l = r = b = t = i ;

                        for( int i = 0 ; i < canvas_black_corrected[0].nb ; i++ ) {
                            if( canvas_black_corrected[i].x > center_black.x ) {
                                if( canvas_black_corrected[i].x < canvas_black_corrected[l].x )
                                    l = i ;

                                if( canvas_black_corrected[i].x > canvas_black_corrected[r].x )
                                    r = i ;

                                if( canvas_black_corrected[i].y > canvas_black_corrected[b].y )
                                    b = i ;

                                if( canvas_black_corrected[i].y < canvas_black_corrected[t].y )
                                    t = i ;
                            }
                        }
                        canvas_final[8].x = canvas_black[l].x ;
                        canvas_final[8].y = canvas_black[l].y ;
                        canvas_final[9].x = canvas_black[r].x ;
                        canvas_final[9].y = canvas_black[r].y ;
                        canvas_final[10].x = canvas_black[b].x ;
                        canvas_final[10].y = canvas_black[b].y ;
                        canvas_final[11].x = canvas_black[t].x ;
                        canvas_final[11].y = canvas_black[t].y ;


                        FILE *camera_d = NULL ;
                        camera_d = fopen("2D_droite.txt", "w" ) ;

                        for( int i = 0 ; i < 12 ; i++ ) {
                            printf("%d = %d-%d\n", i, canvas_final[i].x, canvas_final[i].y) ;
                            fprintf(camera_d,"%d\t%d\n", canvas_final[i].x, canvas_final[i].y) ;
                        }

                        fclose(camera_d) ;


                        drawpoint( final, center_blue.x, center_blue.y, 3, 15, 100) ;
                        drawpoint( final, center_black.x, center_black.y, 3, 15, 100) ;

                        sdlImageToScr(final , scr, camnb) ; //On affiche l'image finale
                        SDL_UpdateRect(scr,0,0,0,0) ;
                        SDL_Delay(5) ;
                        Sleep(1000) ;

                        for( int x = 0 ; x < final.width ; x++ )
                            for( int y = 0 ; y < final.height ; y++ )
                                final.img[x][y] = 255 ;

                        for( int i = 0 ; i < canvas_blue_corrected[0].nb ; i++ )
                            drawpoint( final, canvas_blue_corrected[i].x, canvas_blue_corrected[i].y, 1, 20, 100) ;

                        for( int i = 0 ; i < canvas_black_corrected[0].nb ; i++ )
                            drawpoint( final, canvas_black_corrected[i].x, canvas_black_corrected[i].y, 3, 20, 100) ;

                        sdlImageToScr(final , scr, camnb) ; //On affiche l'image finale
                        SDL_UpdateRect(scr,0,0,0,0) ;
                        SDL_Delay(5) ;
                        Sleep(1000) ;
                    }

                }

                sdlImageToScr(final , scr, camnb) ; //On affiche l'image finale
                SDL_UpdateRect(scr,0,0,0,0) ;
                SDL_Delay(5) ;
            }
        }

        do_canvas = 0 ;

        //Listen to keyboard and mousse
        while(SDL_PollEvent(&event)) {
            if(event.type == SDL_QUIT || SDL_GetKeyState(NULL)[SDLK_ESCAPE])
                done = 10 ;
            else if(SDL_GetKeyState(NULL)[SDLK_RETURN] || SDL_GetKeyState(NULL)[SDLK_KP_ENTER])
                done = 10 ;
            else if(SDL_GetKeyState(NULL)[SDLK_UP])
                do_canvas = 1 ;
            else if(SDL_GetKeyState(NULL)[SDLK_DOWN])
                do_canvas = 0 ;
            else if(SDL_GetKeyState(NULL)[SDLK_RIGHT])
                stop_canvas = 1 ;


            if( event.type == SDL_MOUSEBUTTONUP ) {
                if(event.button.button == SDL_BUTTON_WHEELUP ) {
                    threshold++ ;
                } else if(event.button.button == SDL_BUTTON_WHEELDOWN ) {
                    threshold-- ;
                }
            }
        }

        printf("\rthreshold : %d\t", threshold) ;

        //Show time between each picture in sec. or FPS
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
