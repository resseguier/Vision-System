#include <VISION/VISION.h>
#include <stdio.h>
#include <stdlib.h>

int main( int argc, char** argv ) {
    sdlGetStdout() ;
    visionVersion() ;

    //SDL_Surface *scr ;
    //SDL_Event event ;
    //char windowsname[25] ;
    //Image2D image ;
    int camWidth = 640 ;
    int camHeight = 480 ;
    FILE *plan_3d = NULL ;
    FILE *plan_3d_m = NULL ;
    float real_x, real_y, real_z ;
    int point_x, point_y ;
    float eyes_x = camWidth/2 ;
    float eyes_y = camHeight/2 ;
    float eyes_z = 1 ;
    float rotation = 8 ;
    int lines = 0 ;
    int done = 0 ;
/*
    sprintf(windowsname,"Pojnt cloud in 3D") ; //Save windows name

    //Load SDL window
    if( !sdlInit() ) return -5 ;
    if( !(scr = SDL_SetVideoMode( camWidth, camHeight, 32, SDL_HWSURFACE))) return -6 ; //Allocate scr (SDL buffer) and set param //SDL_HWSURFACE - SDL_FULLSCREEN - SDL_RESIZABLE - SDL_NOFRAME
    SDL_WM_SetCaption(windowsname , NULL) ; //Define windows name
    SDL_Flip(scr) ;

    //Alloc picture in memory
    visionImageAlloc(image, camWidth, camHeight) ;
*/
    plan_3d_m = fopen("2D_meshlab_m.txt", "w" ) ;
    plan_3d = fopen("3D_meshlab.txt", "r" ) ;
    if( plan_3d == NULL ) {
        printf("Error opening txt\n");
    }

    while(!feof(plan_3d))
        if(fgetc(plan_3d) == '\n')
            lines++;
    fseek(plan_3d, 0, SEEK_SET);
    printf("Nombre de points 3D : %d\n\n", lines) ;

/*
    for( int x = 0 ; x < camWidth ; x++)
        for( int y = 0 ; y < camHeight ; y++)
            image.img[x][y] = 255 ;
*/

    for( int i = 0 ; i < lines ; i++ ) {
        printf("\rCalcul en cours ... %d", i+1) ;
        fscanf(plan_3d, "%f", &real_x);
        fscanf(plan_3d, "%f", &real_y);
        fscanf(plan_3d, "%f", &real_z);

        eyes_x = real_x*cos(M_PI/rotation) + real_z*sin(M_PI/rotation) ;
        eyes_z = -real_x*sin(M_PI/rotation) + real_z*cos(M_PI/rotation) ;
        eyes_y = -real_y ;

        real_x = eyes_x ;
        real_y = eyes_y*cos(M_PI/-rotation) + eyes_z*sin(M_PI/-rotation) ;
        real_z = -eyes_y*sin(M_PI/-rotation) + eyes_z*cos(M_PI/-rotation) ;

        fprintf(plan_3d_m,"%f\t;\t%f\t;\t%f\n",real_x, real_y, real_z) ;

/*
        point_x = ((eyes_z*(real_x-eyes_x))/(eyes_z+real_z))+eyes_x*2 ;
        point_y = ((eyes_z*(real_y-eyes_y))/(eyes_z+real_z))+eyes_y*2 ;

        printf(" %d \t %d", point_x, point_y) ;

        if(  point_y >= 0 && point_y < camHeight && point_x >= 0 && point_x < camWidth ) {
            printf("  ...ok %f\n", eyes_z) ;
            image.img[point_x][point_y] = 0 ;
        }
*/
    }
/*
    while( done != 10 ) {
        //Update SDL window
        sdlImageToScr(image , scr, 0) ;
        SDL_UpdateRect(scr,0,0,0,0) ;
        SDL_Delay(5) ;

        //Listen to keyboard and mousse
        while(SDL_PollEvent(&event)) {
            if(event.type == SDL_QUIT || SDL_GetKeyState(NULL)[SDLK_ESCAPE])
                done = 10 ;
            else if(SDL_GetKeyState(NULL)[SDLK_RETURN] || SDL_GetKeyState(NULL)[SDLK_KP_ENTER])
                done = 10 ;

            if( event.type == SDL_MOUSEBUTTONUP ) {
                if(event.button.button == SDL_BUTTON_WHEELUP ) {
                    rotation++ ;
                } else if(event.button.button == SDL_BUTTON_WHEELDOWN ) {
                    rotation-- ;
                }
            }
        }
    }
*/
    /* Flush mem before quit */
/*
        SDL_FreeSurface(scr) ;
        sdlCleanup() ;
        visionImageFree(image) ;
*/
        fclose(plan_3d);
        fclose(plan_3d_m);
        printf("\n\nCache vide !\n") ;
	/* END */




    return 1 ;
}
