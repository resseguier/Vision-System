#include <VISION/VISION.h>
#include <SCILAB/api_scilab.h>
#include <SCILAB/stack-c.h> /* Provide functions to access to the memory of Scilab */
#include <SCILAB/call_scilab.h> /* Provide functions to call Scilab engine */
#include <stdio.h>
#include <stdlib.h>

int main( int argc, char** argv ) {
    sdlGetStdout() ;
    visionVersion() ;

    SDL_Surface *scr ;
    SDL_Event event ;
    char windowsname[25] ;
    Image2D image ;
    int camWidth = 640 ;
    int camHeight = 480 ;
    FILE *plan_2d = NULL ;
    double  time_begin, time_end ;
    float real_x, real_y, real_z ;
    float eyes_x, eyes_y, eyes_z ;
    float rotation = 1 ;
    int offset_x = 0 ;
    int offset_y = 0 ;
    int offset_x2 = 0 ;
    int offset_y2 = 0 ;
    char text[100] ;
    int lines = 0 ;
    int done = 0 ;
    int gapx = 0 ;
    int gapy = 0 ;
    int gapx2 = 0 ;
    int gapy2 = 0 ;
    int mo = 0 ;
    int mo2 = 0 ;
    int moxin = 0 ;
    int moyin = 0 ;
    int mox = 0 ;
    int moy = 0 ;
    int reset = 0 ;
    float best_score ;
    float minimal_score = 0.03 ;
    float actual_score ;
    int min_pos[5] ;
    int show_point = 1 ;
    int show_mesh = 1 ;

    //Save windows name
    sprintf(windowsname,"Pojnt in 3D") ;

    //Load SDL window
    if( !sdlInit() ) return -5 ;
    if( !(scr = SDL_SetVideoMode( camWidth, camHeight, 32, SDL_HWSURFACE))) return -6 ; //Allocate scr (SDL buffer) and set param //SDL_HWSURFACE - SDL_FULLSCREEN - SDL_RESIZABLE - SDL_NOFRAME
    SDL_WM_SetCaption(windowsname , NULL) ; //Define windows name
    SDL_Flip(scr) ;

    //Alloc picture in memory
    visionImageAlloc(image, camWidth, camHeight) ;

    //Load SDL font
    TTF_Init() ;
    SDL_Rect texteDestination ;
    SDL_Surface* pFontSurface = NULL ;
    TTF_Font* pFont = TTF_OpenFont("./screen/font.ttf",16) ;

    while( done != 10 ) {
        do_system("copy /Y 2D_meshlab.txt 2D_meshlab_m.txt") ;

        SDL_Delay(5) ;

        plan_2d = fopen("2D_meshlab_m.txt", "r" ) ;
        if( plan_2d != NULL ) {
            lines = 0 ;
            while(!feof(plan_2d))
                if(fgetc(plan_2d) == '\n')
                    lines++;
            fseek(plan_2d, 0, SEEK_SET);

            for( int x = 0 ; x < camWidth ; x++)
                for( int y = 0 ; y < camHeight ; y++)
                    image.img[x][y] = 0 ;

            float **tab3D ;
            tab3D = new float*[lines] ;
            for(int i = 0 ; i < lines ; i++)
                tab3D[i] = new float[6] ;


            for( int i = 0 ; i < lines ; i++ ) {
                fscanf(plan_2d, "%f", &tab3D[i][0]);
                fscanf(plan_2d, "%f", &tab3D[i][1]);
                fscanf(plan_2d, "%f", &tab3D[i][2]);
            }

            fseek(plan_2d, 0, SEEK_SET);

            if( lines != 0 ) {
                for( int i = 0 ; i < lines ; i++ ) {
                    //Correction of the original orientation to give a fake view in 3D
                    eyes_x = tab3D[i][0]*cos(-M_PI/4) + tab3D[i][1]*sin(-M_PI/4) ;
                    eyes_y = -tab3D[i][0]*sin(-M_PI/4) + tab3D[i][1]*cos(-M_PI/4) /*+ rotation*/ ;
                    eyes_z = -tab3D[i][2] ;

                    //rotation sur y prime
                    real_x = eyes_x*cos(gapx*M_PI/180) + eyes_y*sin(gapx*M_PI/180) ;
                    real_y = -eyes_x*sin(gapx*M_PI/180) + eyes_y*cos(gapx*M_PI/180) ;
                    real_z = eyes_z ;

                    //rotation sur x prime
                    eyes_y = real_y*cos(gapy*M_PI/180) + real_z*sin(gapy*M_PI/180) ;
                    eyes_z = -real_y*sin(gapy*M_PI/180) + real_z*cos(gapy*M_PI/180) ;
                    eyes_x = real_x ;


                    //tab3D[i][3] = 2500*eyes_x+camWidth/2 + offset_x ;
                    //tab3D[i][4] = 2500*eyes_z+camHeight/2 + offset_y ;
                    tab3D[i][3] = rotation*(2500*eyes_x) + camWidth/2 + offset_x ;
                    tab3D[i][4] = rotation*(2500*eyes_z) + camHeight/2 + offset_y;

                    if( eyes_y >= 0 )
                        tab3D[i][5] = 3/(1+30*eyes_y) ;
                    else
                        tab3D[i][5] = 3 ;

                    if( tab3D[i][5] > 3 )
                        tab3D[i][5] = 3 ;
                    else if ( tab3D[i][5] < 0.5 )
                        tab3D[i][5] = 0.5 ;
                }

                //Print final cloud point to SDL
                if(show_mesh) {
                    for( int i = 0 ; i < lines ; i++ ) {
                        //Search closest point
                        best_score = 100000000 ;
                        for( int j = 0 ; j < 5 ; j++ )
                            min_pos[j] = -1 ;
                        for( int j = 0 ; j < lines ; j++ ) {
                            if( j != i) {
                                actual_score = sqrt((tab3D[i][0]-tab3D[j][0])*(tab3D[i][0]-tab3D[j][0])+(tab3D[i][1]-tab3D[j][1])*(tab3D[i][1]-tab3D[j][1])+(tab3D[i][2]-tab3D[j][2])*(tab3D[i][2]-tab3D[j][2])) ;
                                if( actual_score < best_score && actual_score < minimal_score ) {
                                    min_pos[0] = min_pos[1] ;
                                    min_pos[1] = min_pos[2] ;
                                    min_pos[2] = min_pos[3] ;
                                    min_pos[3] = min_pos[4] ;
                                    best_score = actual_score ;
                                    min_pos[4] = j ;
                                }
                            }
                        }

                        //Then draw line between them
                        for( int j = 0 ; j < 5 ; j++ )
                            if( min_pos[j] != -1 )
                                ligne(image, tab3D[i][3], tab3D[i][4], tab3D[min_pos[j]][3], tab3D[min_pos[j]][4], 50) ;
                    }
                }

                for( int i = 0 ; i < lines ; i++ ) {
                    //Print point
                    if(show_point) {
                        for( int u = -tab3D[i][5] ; u < tab3D[i][5] ; u++ )
                            for( int v = -tab3D[i][5] ; v < tab3D[i][5] ; v++ )
                                if( tab3D[i][3]+u >= 0 && tab3D[i][3]+u < camWidth && tab3D[i][4]+v >= 0 && tab3D[i][4]+v < camHeight )
                                    image.img[(int) tab3D[i][3]+u][(int) tab3D[i][4]+v] = 255*tab3D[i][5]/3 ;
                    } else {
                        if( tab3D[i][3] >= 0 && tab3D[i][3] < camWidth && tab3D[i][4] >= 0 && tab3D[i][4] < camHeight )
                            image.img[(int) tab3D[i][3]][(int) tab3D[i][4]] = 255*tab3D[i][5]/3 ;
                    }
                }

            }

            //Efface les allocations en memoire de l'image
            for(int i = 0 ; i < lines ; i++)
                free( tab3D[i] ) ;
            free( tab3D ) ;

            //Correction si sur impression du pixel
            for( int x = 0 ; x < camWidth ; x++)
                for( int y = 0 ; y < camHeight ; y++)
                    if( image.img[x][y] > 255 )
                        image.img[x][y] = 255 ;

            //Print picture to SDL window
            sdlImageToScr(image , scr, 0) ;

            //Print text to SDL window
            sprintf(text,"off : %d/%d  Rotation x/y/zoom: %d/%d/%0.4f", offset_x, offset_y, gapx, gapy, rotation) ;
            pFontSurface = TTF_RenderText_Solid(pFont,text,{255, 255, 255}) ;
            texteDestination.w = pFontSurface->w ;
            texteDestination.h = pFontSurface->h ;
            texteDestination.x = camWidth - 15 - texteDestination.w ;
            texteDestination.y = camHeight - 15 - texteDestination.h ;
            SDL_BlitSurface(pFontSurface,NULL,scr,&texteDestination) ;

            //Update SDL window
            SDL_UpdateRect(scr,0,0,0,0) ;
            SDL_Delay(5) ;

            fclose(plan_2d);
        }

        SDL_Delay(5) ;

        done = 0 ;
        if( reset ) {
            if( gapx == 0 )
                gapx = gapx ;
            else if( gapx <= 180 )
                gapx -= 1+gapx*0.008 ;
            else if( gapx > 180 )
                gapx += 1+gapx*0.008 ;

            if( gapy == 0 )
                gapy = gapy ;
            else if( gapy <= 180 )
                gapy -= 1+gapy*0.008 ;
            else if( gapy > 180 )
                gapy += 1+gapy*0.008 ;

            if( gapx > 360 )
                gapx -= 360 ;

            if( gapy > 360 )
                gapy -= 360 ;

            if( rotation < 0.975 )
                rotation += 0.025 + rotation*0.03 ;
            else if( rotation > 1.025 )
                rotation -= 0.025 + rotation*0.03 ;

            if( offset_x > 0 )
                offset_x -= 1+offset_x*0.02  ;
            else if( offset_x < 0 )
                offset_x += 1-offset_x*0.02 ;

            if( offset_y  > 0 )
                offset_y -= 1+offset_y*0.02 ;
            else if( offset_y < 0 )
                offset_y += 1-offset_y*0.02 ;


            if( gapx+gapy+offset_x+offset_y < 0.017 && rotation > 0.975 && rotation < 1.025 ) {
                gapx = gapy = offset_x = offset_y = 0 ;
                rotation = 1 ;
                reset = 0 ;
            }
        }
        //Listen to keyboard and mousse
        while(SDL_PollEvent(&event)) {
            if(event.type == SDL_QUIT || SDL_GetKeyState(NULL)[SDLK_ESCAPE])
                done = 10 ;
            else if(SDL_GetKeyState(NULL)[SDLK_RETURN] || SDL_GetKeyState(NULL)[SDLK_KP_ENTER])
                done = 10 ;

            if( !reset ) {
                if(SDL_GetKeyState(NULL)[SDLK_r]) {
                    mo = mox = moy = moxin = moyin = gapx2 = gapy2 = offset_x2 = offset_y2 = 0 ;
                    reset = 1 ;
                } else if(SDL_GetKeyState(NULL)[SDLK_UP])
                    minimal_score += 0.005 ;
                else if(SDL_GetKeyState(NULL)[SDLK_DOWN])
                    minimal_score -= 0.005 ;
                else if(event.button.button == SDL_BUTTON_WHEELUP )
                    rotation += 0.05 ;
                else if(event.button.button == SDL_BUTTON_WHEELDOWN )
                    rotation -= 0.05 ;
                else if(SDL_GetKeyState(NULL)[SDLK_p])
                    show_point = show_point==1?0:1 ;
                else if(SDL_GetKeyState(NULL)[SDLK_l])
                    show_mesh = show_mesh==1?0:1 ;

                if( event.type == SDL_MOUSEBUTTONDOWN ) {
                    if( event.button.button == SDL_BUTTON_LEFT ) {
                        //Get the mouse offsets
                        moxin = event.button.x ;
                        moyin = event.button.y ;

                        mo = 1 ;
                    }
                    if( event.button.button == SDL_BUTTON_MIDDLE ) {
                        //Get the mouse offsets
                        moxin = event.button.x ;
                        moyin = event.button.y ;

                        mo2 = 1 ;
                    }
                }

                if(mo) {
                    //Get the mouse offsets
                    mox = event.motion.x ;
                    moy = event.motion.y ;
                    gapx = moxin - mox + gapx2 ;
                    gapy = -(moyin - moy + gapy2) ;

                    if( gapx > 360 )
                        gapx -= 360 ;
                    else if( gapx < 0 )
                        gapx += 360 ;

                    if( gapy > 360 )
                        gapy -= 360 ;
                    else if( gapy < 0 )
                        gapy += 360 ;
                }

                if(mo2) {
                    //Get the mouse offsets
                    mox = event.motion.x ;
                    moy = event.motion.y ;
                    offset_x = -(moxin - mox + offset_x2) ;
                    offset_y = -(moyin - moy + offset_y2) ;
                }

                if( event.type == SDL_MOUSEBUTTONUP ) {
                    if( event.button.button == SDL_BUTTON_LEFT ) {
                        //Get the mouse offsets
                        moxin = event.button.x ;
                        moyin = event.button.y ;

                        gapx2 = gapx ;
                        gapy2 = -gapy ;
                        mo = 0 ;
                    }
                    if( event.button.button == SDL_BUTTON_MIDDLE ) {
                        //Get the mouse offsets
                        moxin = event.button.x ;
                        moyin = event.button.y ;

                        offset_x2 = -offset_x ;
                        offset_y2 = -offset_y ;
                        mo2 = 0 ;
                    }
                }
            }
        }


        printf("\r") ;
        time_end = get_time() ;
        show_diff_time( time_end, time_begin, 1) ;
        time_begin = get_time() ;
    }

    /* Flush mem before quit */
        SDL_FreeSurface(scr) ;
        TTF_CloseFont(pFont);
        sdlCleanup() ;
        visionImageFree(image) ;

        printf("\n\nCache vide !\n") ;
	/* END */

    return 1 ;
}


