#include <stdio.h>
//#include <fstream>
#include <string>
#include <sstream>

#include <LODEPNG/LODEPNG.h>
#include <ESCAPI/escapi.h>
#include <VISION/VISION.h>

//SDL version 1.2 (template pour codeblocks par Alexandre Laurent
#include <SDL/SDL.h>
#include <SDL/SDL_image.h>
#include <SDL/SDL_ttf.h>

#define VERSIONY 14
#define VERSIONM 10
#define VERSIOND 22

void printfstereo( unsigned int ***img1, unsigned int ***img2, unsigned int ***final1, unsigned int ***final2, float **feature1, float **feature2, unsigned int width, unsigned int height, int i ) {
    //On affiche le deplacement du point sur l'image 1
    ligne( final1, feature1[i][7], feature1[i][8], feature2[(int) feature1[i][5]][7], feature2[(int) feature1[i][5]][8]) ;
    final1[(int) feature2[(int) feature1[i][5]][7]][(int) feature2[(int) feature1[i][5]][8]][1] = 255 ;
    drawpoint( final1, width, height, feature1[i][7], feature1[i][8], 2, 5, 1) ;
    final1[(int) feature1[i][7]][(int) feature1[i][8]][1] = 255 ;

    //On affiche le deplacement du point sur l'image 2
    ligne( final2, feature1[i][7], feature1[i][8], feature2[(int) feature1[i][5]][7], feature2[(int) feature1[i][5]][8]) ;
    final2[(int) feature1[i][7]][(int) feature1[i][8]][1] = 255 ;
    drawpoint( final2, width, height, feature2[(int) feature1[i][5]][7], feature2[(int) feature1[i][5]][8], 2, 5, 1) ;
    final2[(int) feature2[(int) feature1[i][5]][7]][(int) feature2[(int) feature1[i][5]][8]][1] = 255 ;
}

void clearall ( unsigned int ***img, unsigned int ***final, unsigned char* image, unsigned int width, unsigned int height) {
	freeimg( img, width, height) ;
	freeimg( final, width, height) ;
	free( image ) ;
	std::cout << "Cache vide !" << std::endl ;
}

void cleanup() {
    TTF_Quit();
    IMG_Quit();
    SDL_Quit();
}


char const * convertint( int text) {
    std::stringstream convert;
    convert << text;
    return convert.str().c_str() ;
}

int showscreen( int &A, int &B, int &C, int &D, int &E, int &F, int &G, int &H, int &I, int &J, int &K) {
    unsigned int SDLwidth, SDLheight ;
	int error ;
    int done = 0 ;
    unsigned char* background ;
    unsigned char* logo ;
    unsigned char* circle ;
    unsigned char* button ;
    unsigned char* start ;
    unsigned char* resetn ;
    unsigned char* resetp ;

    //Chargement du background
	if( (error = lodepng_decode32_file(&background, &SDLwidth, &SDLheight, "./screen/bg2.png")) ) {
		std::cerr << "decoder error " << error << ": " << lodepng_error_text(error) << std::endl ;
		return 2 ;
	}
	std::cout << "L'image \"./screen/bg2.png\" a ete chargee ..." << std::endl ;

	unsigned int LOGOwidth, LOGOheight ;
    //Chargement du cercle blanc
	if( (error = lodepng_decode32_file(&logo, &LOGOwidth, &LOGOheight, "./screen/logo.png")) ) {
		std::cerr << "decoder error " << error << ": " << lodepng_error_text(error) << std::endl ;
		return 2 ;
	}
	std::cout << "L'image \"./screen/logo.png\" a ete chargee ..." << std::endl ;

	unsigned int CIRCLEwidth, CIRCLEheight ;
    //Chargement du cercle blanc
	if( (error = lodepng_decode32_file(&circle, &CIRCLEwidth, &CIRCLEheight, "./screen/circle.png")) ) {
		std::cerr << "decoder error " << error << ": " << lodepng_error_text(error) << std::endl ;
		return 2 ;
	}
	std::cout << "L'image \"./screen/circle.png\" a ete chargee ..." << std::endl ;

	unsigned int BUTTONwidth, BUTTONheight ;
    //Chargement du cercle blanc
	if( (error = lodepng_decode32_file(&button, &BUTTONwidth, &BUTTONheight, "./screen/button.png")) ) {
		std::cerr << "decoder error " << error << ": " << lodepng_error_text(error) << std::endl ;
		return 2 ;
	}
	std::cout << "L'image \"./screen/button.png\" a ete chargee ..." << std::endl ;

	unsigned int STARTwidth, STARTheight ;
    //Chargement du bouton start
	if( (error = lodepng_decode32_file(&start, &STARTwidth, &STARTheight, "./screen/start_p.png")) ) {
		std::cerr << "decoder error " << error << ": " << lodepng_error_text(error) << std::endl ;
		return 2 ;
	}
	std::cout << "L'image \"./screen/start_p.png\" a ete chargee ..." << std::endl ;

	unsigned int RESETwidth, RESETheight ;
    //Chargement du bouton reset_n
	if( (error = lodepng_decode32_file(&resetn, &RESETwidth, &RESETheight, "./screen/reset_n.png")) ) {
		std::cerr << "decoder error " << error << ": " << lodepng_error_text(error) << std::endl ;
		return 2 ;
	}
	std::cout << "L'image \"./screen/reset_n.png\" a ete chargee ..." << std::endl ;

    //Chargement du bouton reset_p
	if( (error = lodepng_decode32_file(&resetp, &RESETwidth, &RESETheight, "./screen/reset_p.png")) ) {
		std::cerr << "decoder error " << error << ": " << lodepng_error_text(error) << std::endl ;
		return 2 ;
	}
	std::cout << "L'image \"./screen/reset_p.png\" a ete chargee ..." << std::endl ;

    int imgFlags = IMG_INIT_TIF ; //Flags pour charger le support du TIF
    TTF_Font* pFont = NULL;
    const char* fontPath = "./screen/font.ttf" ;
    SDL_Color fontColor = {0, 0, 0} ;
    SDL_Surface* pFontSurface = NULL ;
    SDL_Rect texteDestination ;

    //init SDL
    if(SDL_Init(SDL_INIT_VIDEO) < 0) {
        std::cout << "error, SDL video init failed" << std::endl ;
        free( background ) ;
        cleanup() ;
        return -1 ;
    }

    if ( IMG_Init(imgFlags) != imgFlags ) {
        fprintf(stderr,"Erreur lors de l'initialisation de la SDL_image : '%s'\n",IMG_GetError()) ;
        free( background ) ;
        free( start ) ;
        cleanup() ;
        return -1 ;
    }

    SDL_Surface* scr = SDL_SetVideoMode(SDLwidth , SDLheight , 32, SDL_HWSURFACE) ;
    if(!scr) {
        std::cout << "error, no SDL screen" << std::endl ;
        free( background ) ;
        free( start ) ;
        cleanup() ;
        return -2;
    }

    if ( TTF_Init() == -1 ) {
        fprintf(stderr,"Erreur lors de l'initialisation de la SDL_TTF : '%s'\n",TTF_GetError()) ;
        free( background ) ;
        free( start ) ;
        cleanup() ;
        return -1 ;
    }

    SDL_WM_SetCaption("Vision System", NULL) ; //set window caption

    //on imprime le background
    for( unsigned x = 0 ; x < SDLwidth ; x ++ ) {
        for( unsigned y = 0 ; y < SDLheight ; y ++ ) {
            //give the color value to the pixel of the screenbuffer
            if (background[4*y*SDLwidth+4*x+3] != 0) { //alpha
                Uint32* bufp;
                bufp = (Uint32 *)scr->pixels + (y * scr->pitch / 4) + x ;
                *bufp = 65536 * background[4*y*SDLwidth+4*x+0] + 256 * background[4*y*SDLwidth+4*x+1] + background[4*y*SDLwidth+4*x+2] ;
            }
        }
    }

    //Imprime le logo blanc en haut à droite
    for( unsigned x = 0 ; x < LOGOwidth ; x ++ ) {
        for( unsigned y = 0 ; y < LOGOheight ; y ++ ) {
            //give the color value to the pixel of the screenbuffer
            Uint32* bufp;
            bufp = (Uint32 *)scr->pixels + (y * scr->pitch / 4) + (x+SDLwidth-LOGOwidth) ;
            float a = float(logo[4*y*LOGOwidth+4*x+3]) / 254  ;
            unsigned r = background[4*y*SDLwidth+4*(x+SDLwidth-LOGOwidth)+0] * ( 1 - a ) + logo[4*y*LOGOwidth+4*x+0] * a ;
            unsigned g = background[4*y*SDLwidth+4*(x+SDLwidth-LOGOwidth)+1] * ( 1 - a ) + logo[4*y*LOGOwidth+4*x+1] * a ;
            unsigned b = background[4*y*SDLwidth+4*(x+SDLwidth-LOGOwidth)+2] * ( 1 - a ) + logo[4*y*LOGOwidth+4*x+2] * a ;
            *bufp = 65536 * r + 256 * g + b ;
        }
    }

    //Imprime le cercle blanc en bas à droite
    for( unsigned x = 0 ; x < CIRCLEwidth ; x ++ ) {
        for( unsigned y = 0 ; y < CIRCLEheight ; y ++ ) {
            //give the color value to the pixel of the screenbuffer
            Uint32* bufp;
            bufp = (Uint32 *)scr->pixels + ((y+SDLheight-CIRCLEheight) * scr->pitch / 4) + (x+SDLwidth-CIRCLEwidth) ;
            float a = float(circle[4*y*CIRCLEwidth+4*x+3]) / 254  ;
            unsigned r = background[4*(y+SDLheight-CIRCLEheight)*SDLwidth+4*(x+SDLwidth-CIRCLEwidth)+0] * ( 1 - a ) + circle[4*y*CIRCLEwidth+4*x+0] * a ;
            unsigned g = background[4*(y+SDLheight-CIRCLEheight)*SDLwidth+4*(x+SDLwidth-CIRCLEwidth)+1] * ( 1 - a ) + circle[4*y*CIRCLEwidth+4*x+1] * a ;
            unsigned b = background[4*(y+SDLheight-CIRCLEheight)*SDLwidth+4*(x+SDLwidth-CIRCLEwidth)+2] * ( 1 - a ) + circle[4*y*CIRCLEwidth+4*x+2] * a ;
            *bufp = 65536 * r + 256 * g + b ;
        }
    }

    // Charge la police
    pFont = TTF_OpenFont(fontPath,16) ;
    if ( pFont == NULL ) {
        fprintf(stderr,"Erreur de chargement de la police %s : %s\n",fontPath,TTF_GetError()) ;
        free( background ) ;
        free( start ) ;
        cleanup() ;
        return -3 ;
    }

    int text ;
    for( text = 0 ; text < 8 ; text++ ) {
        if ( text == 0 ) // Genere la SDL_Surface a partir de la police
            pFontSurface = TTF_RenderText_Solid(pFont,"Taille du détecteur de points d'intérêt ?",fontColor) ;
        else if ( text == 1 )
            pFontSurface = TTF_RenderText_Solid(pFont,"Seuil pour le detecteur ?",fontColor) ;
        else if ( text == 2 )
            pFontSurface = TTF_RenderText_Solid(pFont,"Taille maximale de la fenêtre d'érosion ?",fontColor) ;
        else if ( text == 3 )
            pFontSurface = TTF_RenderText_Solid(pFont,"Taille de la fenêtre de correspondance ?",fontColor) ;
        else if ( text == 4 )
            pFontSurface = TTF_RenderText_Solid(pFont,"Nombre de cadrants pour l'angle ?",fontColor) ;
        else if ( text == 5 )
            pFontSurface = TTF_RenderText_Solid(pFont,"Verifier l'angle ? Vérifier l'amplitude?",fontColor) ;
        else if ( text == 6 )
            pFontSurface = TTF_RenderText_Solid(pFont,"Seuil pour l'angle ? Seuil pour l'amplitude ?",fontColor) ;
        else if ( text == 7 )
            pFontSurface = TTF_RenderText_Solid(pFont,"Afficher les points nuls ? Enregistrer les images ?",fontColor) ;

        // Nous avons notre surface pour le texte donc nous calculons la position relative
        texteDestination.w = pFontSurface->w ;
        texteDestination.h = pFontSurface->h ;
        texteDestination.x = 15 ; //43
        texteDestination.y = 15 + (texteDestination.h*2 + 35 ) * text ; //45

        //Arrière plan texte
        if ( text < 8 ) {
            int gapx2 = -27 ;
            int gapy2 = 0 ;
            int gapx = texteDestination.w - 20 ;
            int gapy = texteDestination.y - 44 + gapy2 ;

            //On imprime le début du bouton
            for( unsigned x = 0 ; x < (BUTTONwidth-1)/2 ; x ++ ) {
                for( unsigned y = 0 ; y < BUTTONheight ; y ++ ) {
                    if ( y+gapy >= 0 && y+gapy <= SDLheight && x+gapx2 >= 0 && x+gapx2 <= SDLwidth ) {
                        //give the color value to the pixel of the screenbuffer
                        Uint32* bufp;
                        bufp = (Uint32 *)scr->pixels + ((y+gapy) * scr->pitch / 4) + (x+gapx2) ;
                        float a = float(button[4*y*BUTTONwidth+4*x+3]) / 254  ;
                        background[4*(y+gapy)*SDLwidth+4*(x+gapx2)+0] = background[4*(y+gapy)*SDLwidth+4*(x+gapx2)+0] * ( 1 - a ) + button[4*y*BUTTONwidth+4*x+0] * a ;
                        unsigned r = background[4*(y+gapy)*SDLwidth+4*(x+gapx2)+0] ;
                        background[4*(y+gapy)*SDLwidth+4*(x+gapx2)+1] = background[4*(y+gapy)*SDLwidth+4*(x+gapx2)+1] * ( 1 - a ) + button[4*y*BUTTONwidth+4*x+1] * a ;
                        unsigned g = background[4*(y+gapy)*SDLwidth+4*(x+gapx2)+1] ;
                        background[4*(y+gapy)*SDLwidth+4*(x+gapx2)+2] = background[4*(y+gapy)*SDLwidth+4*(x+gapx2)+2] * ( 1 - a ) + button[4*y*BUTTONwidth+4*x+2] * a ;
                        unsigned b = background[4*(y+gapy)*SDLwidth+4*(x+gapx2)+2] ;
                        *bufp = 65536 * r + 256 * g + b ;
                    }
                }
            }

            //On imprime la fin du bouton
            for( unsigned x = (BUTTONwidth-1)/2 ; x < (BUTTONwidth-1)/2 + gapx + 1 ; x ++ ) {
                for( unsigned y = 0 ; y < BUTTONheight ; y ++ ) {
                    if ( y+gapy >= 0 && y+gapy <= SDLheight && x+gapx2 >= 0 && x+gapx2 <= SDLwidth ) {
                        //give the color value to the pixel of the screenbuffer
                        Uint32* bufp;
                        bufp = (Uint32 *)scr->pixels + ((y+gapy) * scr->pitch) / 4 + (x+gapx2) ;
                        float a = float(button[4*y*BUTTONwidth+4*(BUTTONwidth-1)/2+3]) / 254  ;
                        background[4*(y+gapy)*SDLwidth+4*(x+gapx2)+0] = background[4*(y+gapy)*SDLwidth+4*(x+gapx2)+0] * ( 1 - a ) + button[4*y*BUTTONwidth+4*(BUTTONwidth-1)/2+0] * a ;
                        unsigned r = background[4*(y+gapy)*SDLwidth+4*(x+gapx2)+0] ;
                        background[4*(y+gapy)*SDLwidth+4*(x+gapx2)+1] = background[4*(y+gapy)*SDLwidth+4*(x+gapx2)+1] * ( 1 - a ) + button[4*y*BUTTONwidth+4*(BUTTONwidth-1)/2+1] * a ;
                        unsigned g = background[4*(y+gapy)*SDLwidth+4*(x+gapx2)+1] ;
                        background[4*(y+gapy)*SDLwidth+4*(x+gapx2)+2] = background[4*(y+gapy)*SDLwidth+4*(x+gapx2)+2] * ( 1 - a ) + button[4*y*BUTTONwidth+4*(BUTTONwidth-1)/2+2] * a ;
                        unsigned b = background[4*(y+gapy)*SDLwidth+4*(x+gapx2)+2] ;
                        *bufp = 65536 * r + 256 * g + b ;
                    }
                }
            }

            //On imprime la fin du bouton
            for( unsigned x = 1+(BUTTONwidth-1)/2 ; x < BUTTONwidth ; x ++ ) {
                for( unsigned y = 0 ; y < BUTTONheight ; y ++ ) {
                    if ( y+gapy >= 0 && y+gapy <= SDLheight && x+gapx2+gapx >= 0 && x+gapx2+gapx <= SDLwidth ) {
                        //give the color value to the pixel of the screenbuffer
                        Uint32* bufp;
                        bufp = (Uint32 *)scr->pixels + ((y+gapy) * scr->pitch) / 4 + (x+gapx2+gapx) ;
                        float a = float(button[4*y*BUTTONwidth+4*x+3]) / 254  ;
                        background[4*(y+gapy)*SDLwidth+4*(x+gapx2+gapx)+0] = background[4*(y+gapy)*SDLwidth+4*(x+gapx2+gapx)+0] * ( 1 - a ) + button[4*y*BUTTONwidth+4*x+0] * a ;
                        unsigned r = background[4*(y+gapy)*SDLwidth+4*(x+gapx2+gapx)+0] ;
                        background[4*(y+gapy)*SDLwidth+4*(x+gapx2+gapx)+1] = background[4*(y+gapy)*SDLwidth+4*(x+gapx2+gapx)+1] * ( 1 - a ) + button[4*y*BUTTONwidth+4*x+1] * a ;
                        unsigned g = background[4*(y+gapy)*SDLwidth+4*(x+gapx2+gapx)+1] ;
                        background[4*(y+gapy)*SDLwidth+4*(x+gapx2+gapx)+2] = background[4*(y+gapy)*SDLwidth+4*(x+gapx2+gapx)+2] * ( 1 - a ) + button[4*y*BUTTONwidth+4*x+2] * a ;
                        unsigned b = background[4*(y+gapy)*SDLwidth+4*(x+gapx2+gapx)+2] ;
                        *bufp = 65536 * r + 256 * g + b ;
                    }
                }
            }
        }

        //Arrière plan reponse
        if ( text < 8 ) {
            int gapx2 = -23 ;
            int gapy2 = texteDestination.h + 18 ;
            int gapx = 0 ;
            if ( text == 0 || text == 4 )
                gapx = 0 ;
            else if ( text == 1 ||text == 2 || text == 3 )
                gapx = 10 ;
            else if ( text == 5 || text == 7 )
                gapx = 36 ;
            else if ( text == 6 )
                gapx = 52 ;
            int gapy = texteDestination.y - 44 + gapy2 ;

            //On imprime le début du bouton
            for( unsigned x = 0 ; x < (BUTTONwidth-1)/2 ; x ++ ) {
                for( unsigned y = 0 ; y < BUTTONheight ; y ++ ) {
                    if ( y+gapy >= 0 && y+gapy <= SDLheight && x+gapx2 >= 0 && x+gapx2 <= SDLwidth ) {
                        //give the color value to the pixel of the screenbuffer
                        Uint32* bufp;
                        bufp = (Uint32 *)scr->pixels + ((y+gapy) * scr->pitch / 4) + (x+gapx2) ;
                        float a = float(button[4*y*BUTTONwidth+4*x+3]) / 254  ;
                        background[4*(y+gapy)*SDLwidth+4*(x+gapx2)+0] = background[4*(y+gapy)*SDLwidth+4*(x+gapx2)+0] * ( 1 - a ) + button[4*y*BUTTONwidth+4*x+0] * a ;
                        unsigned r = background[4*(y+gapy)*SDLwidth+4*(x+gapx2)+0] ;
                        background[4*(y+gapy)*SDLwidth+4*(x+gapx2)+1] = background[4*(y+gapy)*SDLwidth+4*(x+gapx2)+1] * ( 1 - a ) + button[4*y*BUTTONwidth+4*x+1] * a ;
                        unsigned g = background[4*(y+gapy)*SDLwidth+4*(x+gapx2)+1] ;
                        background[4*(y+gapy)*SDLwidth+4*(x+gapx2)+2] = background[4*(y+gapy)*SDLwidth+4*(x+gapx2)+2] * ( 1 - a ) + button[4*y*BUTTONwidth+4*x+2] * a ;
                        unsigned b = background[4*(y+gapy)*SDLwidth+4*(x+gapx2)+2] ;
                        *bufp = 65536 * r + 256 * g + b ;
                    }
                }
            }

            //On imprime la fin du bouton
            for( unsigned x = (BUTTONwidth-1)/2 ; x < (BUTTONwidth-1)/2 + gapx + 1 ; x ++ ) {
                for( unsigned y = 0 ; y < BUTTONheight ; y ++ ) {
                    if ( y+gapy >= 0 && y+gapy <= SDLheight && x+gapx2 >= 0 && x+gapx2 <= SDLwidth ) {
                        //give the color value to the pixel of the screenbuffer
                        Uint32* bufp;
                        bufp = (Uint32 *)scr->pixels + ((y+gapy) * scr->pitch) / 4 + (x+gapx2) ;
                        float a = float(button[4*y*BUTTONwidth+4*(BUTTONwidth-1)/2+3]) / 254  ;
                        background[4*(y+gapy)*SDLwidth+4*(x+gapx2)+0] = background[4*(y+gapy)*SDLwidth+4*(x+gapx2)+0] * ( 1 - a ) + button[4*y*BUTTONwidth+4*(BUTTONwidth-1)/2+0] * a ;
                        unsigned r = background[4*(y+gapy)*SDLwidth+4*(x+gapx2)+0] ;
                        background[4*(y+gapy)*SDLwidth+4*(x+gapx2)+1] = background[4*(y+gapy)*SDLwidth+4*(x+gapx2)+1] * ( 1 - a ) + button[4*y*BUTTONwidth+4*(BUTTONwidth-1)/2+1] * a ;
                        unsigned g = background[4*(y+gapy)*SDLwidth+4*(x+gapx2)+1] ;
                        background[4*(y+gapy)*SDLwidth+4*(x+gapx2)+2] = background[4*(y+gapy)*SDLwidth+4*(x+gapx2)+2] * ( 1 - a ) + button[4*y*BUTTONwidth+4*(BUTTONwidth-1)/2+2] * a ;
                        unsigned b = background[4*(y+gapy)*SDLwidth+4*(x+gapx2)+2] ;
                        *bufp = 65536 * r + 256 * g + b ;
                    }
                }
            }

            //On imprime la fin du bouton
            for( unsigned x = 1+(BUTTONwidth-1)/2 ; x < BUTTONwidth ; x ++ ) {
                for( unsigned y = 0 ; y < BUTTONheight ; y ++ ) {
                    if ( y+gapy >= 0 && y+gapy <= SDLheight && x+gapx2+gapx >= 0 && x+gapx2+gapx <= SDLwidth ) {
                        //give the color value to the pixel of the screenbuffer
                        Uint32* bufp;
                        bufp = (Uint32 *)scr->pixels + ((y+gapy) * scr->pitch) / 4 + (x+gapx2+gapx) ;
                        float a = float(button[4*y*BUTTONwidth+4*x+3]) / 254  ;
                        background[4*(y+gapy)*SDLwidth+4*(x+gapx2+gapx)+0] = background[4*(y+gapy)*SDLwidth+4*(x+gapx2+gapx)+0] * ( 1 - a ) + button[4*y*BUTTONwidth+4*x+0] * a ;
                        unsigned r = background[4*(y+gapy)*SDLwidth+4*(x+gapx2+gapx)+0] ;
                        background[4*(y+gapy)*SDLwidth+4*(x+gapx2+gapx)+1] = background[4*(y+gapy)*SDLwidth+4*(x+gapx2+gapx)+1] * ( 1 - a ) + button[4*y*BUTTONwidth+4*x+1] * a ;
                        unsigned g = background[4*(y+gapy)*SDLwidth+4*(x+gapx2+gapx)+1] ;
                        background[4*(y+gapy)*SDLwidth+4*(x+gapx2+gapx)+2] = background[4*(y+gapy)*SDLwidth+4*(x+gapx2+gapx)+2] * ( 1 - a ) + button[4*y*BUTTONwidth+4*x+2] * a ;
                        unsigned b = background[4*(y+gapy)*SDLwidth+4*(x+gapx2+gapx)+2] ;
                        *bufp = 65536 * r + 256 * g + b ;
                    }
                }
            }
        }

        //Affiche le texte
        if ( SDL_BlitSurface(pFontSurface,NULL,scr,&texteDestination) != 0 )  {
            fprintf(stderr,"Erreur de copie du texte sur l'ecran\n") ;
            free( background ) ;
            free( start ) ;
            return -3 ;
        }
    }

    //pause until you press escape and meanwhile redraw screen
    SDL_Flip(scr) ;
    SDL_Event event ;
    int mousex, mousey ;
    text = 2 ;
    int susantype = A ;
    int susangap = B ;
    int erosionwindow = C ;
    int Rwindow = D ;
    int thresholdtheta = E ;
    int checktheta = F ;
    int gaptheta = G ;
    int checkamp = H ;
    int gapamp = I ;
    int printnull = J ;
    int printfinal = K ;
    char const *textbox ;

    while(done < 5) {
        while(SDL_PollEvent(&event)) {

            //Get the mouse offsets
            mousex = event.motion.x ;
            mousey = event.motion.y ;

            if ( mousex >= SDLwidth-CIRCLEwidth && mousex <= SDLwidth &&  mousey >= SDLheight-CIRCLEheight && mousey <= SDLheight ) {
                for( unsigned x = 0 ; x < STARTwidth ; x ++ ) {
                    for( unsigned y = 0 ; y < STARTheight ; y ++ ) {
                        //give the color value to the pixel of the screenbuffer
                        Uint32* bufp;
                        bufp = (Uint32 *)scr->pixels + ((y+SDLheight-CIRCLEheight/2) * scr->pitch / 4) + (x+SDLwidth-CIRCLEwidth/2-10) ;
                        float a = float(start[4*y*STARTwidth+4*x+3]) / 254  ;
                        unsigned r = background[4*(y+SDLheight-CIRCLEheight/2)*SDLwidth+4*(x+SDLwidth-CIRCLEwidth/2-10)+0] * ( 1 - a ) + start[4*y*STARTwidth+4*x+0] * a ;
                        unsigned g = background[4*(y+SDLheight-CIRCLEheight/2)*SDLwidth+4*(x+SDLwidth-CIRCLEwidth/2-10)+1] * ( 1 - a ) + start[4*y*STARTwidth+4*x+1] * a ;
                        unsigned b = background[4*(y+SDLheight-CIRCLEheight/2)*SDLwidth+4*(x+SDLwidth-CIRCLEwidth/2-10)+2] * ( 1 - a ) + start[4*y*STARTwidth+4*x+2] * a ;
                        *bufp = 65536 * r + 256 * g + b ;
                    }
                }
                if( event.type == SDL_MOUSEBUTTONUP )
                    if( event.button.button == SDL_BUTTON_LEFT )
                        done = 5 ;
            } else {
                for( unsigned x = 0 ; x < STARTwidth ; x ++ ) {
                    for( unsigned y = 0 ; y < STARTheight ; y ++ ) {
                        //give the color value to the pixel of the screenbuffer
                        Uint32* bufp;
                        bufp = (Uint32 *)scr->pixels + ((y+SDLheight-CIRCLEheight/2) * scr->pitch / 4) + (x+SDLwidth-CIRCLEwidth/2-10) ;
                        float a = float(start[4*y*STARTwidth+4*x+3]) / 254  ;
                        unsigned r = background[4*(y+SDLheight-CIRCLEheight/2)*SDLwidth+4*(x+SDLwidth-CIRCLEwidth/2-10)+0] ;
                        unsigned g = background[4*(y+SDLheight-CIRCLEheight/2)*SDLwidth+4*(x+SDLwidth-CIRCLEwidth/2-10)+1] ;
                        unsigned b = background[4*(y+SDLheight-CIRCLEheight/2)*SDLwidth+4*(x+SDLwidth-CIRCLEwidth/2-10)+2] ;
                        *bufp = 65536 * r + 256 * g + b ;
                    }
                }
            }


            if(event.type == SDL_QUIT)
                done = 10 ;
            else if(SDL_GetKeyState(NULL)[SDLK_ESCAPE])
                done = 10 ;
            else if(SDL_GetKeyState(NULL)[SDLK_RETURN] || SDL_GetKeyState(NULL)[SDLK_KP_ENTER]) //Touche entrer pour valider les modifications
                done = 5 ;

            if( event.type == SDL_MOUSEBUTTONUP ) {
                if ( mousex >= 15 && mousex <= texteDestination.h + 25 ) {
                    if ( mousey >= 30+texteDestination.h+(texteDestination.h*2+35)*0  && mousey <= 35+texteDestination.h*2+(texteDestination.h*2+35)*0 ) {
                        if( event.button.button == SDL_BUTTON_WHEELUP || event.button.button == SDL_BUTTON_LEFT) {
                            if ( susantype == 17 )
                                susantype = 21 ;
                            else if ( susantype == 9 )
                                susantype = 17 ;
                        }

                        if( event.button.button == SDL_BUTTON_WHEELDOWN || event.button.button == SDL_BUTTON_RIGHT) {
                            if ( susantype == 17 )
                                susantype = 9 ;
                            else if ( susantype == 21 )
                                susantype = 17 ;
                        }

                    }
                    if ( mousey >= 30+texteDestination.h+(texteDestination.h*2+35)*1  && mousey <= 35+texteDestination.h*2+(texteDestination.h*2+35)*1 ) {
                        if( event.button.button == SDL_BUTTON_WHEELUP || event.button.button == SDL_BUTTON_LEFT)
                            if ( susangap < 128 )
                                susangap ++ ;

                        if( event.button.button == SDL_BUTTON_WHEELDOWN || event.button.button == SDL_BUTTON_RIGHT)
                            if ( susangap > 0 )
                                susangap -- ;
                    }
                    if ( mousey >= 30+texteDestination.h+(texteDestination.h*2+35)*2  && mousey <= 35+texteDestination.h*2+(texteDestination.h*2+35)*2 ) {
                        if( event.button.button == SDL_BUTTON_WHEELUP || event.button.button == SDL_BUTTON_LEFT)
                            if ( erosionwindow < 200 )
                                erosionwindow ++ ;

                        if( event.button.button == SDL_BUTTON_WHEELDOWN || event.button.button == SDL_BUTTON_RIGHT)
                            if ( erosionwindow > 2 )
                                erosionwindow -- ;
                    }
                    if ( mousey >= 30+texteDestination.h+(texteDestination.h*2+35)*3  && mousey <= 35+texteDestination.h*2+(texteDestination.h*2+35)*3 ) {
                        if( event.button.button == SDL_BUTTON_WHEELUP || event.button.button == SDL_BUTTON_LEFT)
                            if ( Rwindow < 500 )
                                Rwindow ++ ;

                        if( event.button.button == SDL_BUTTON_WHEELDOWN || event.button.button == SDL_BUTTON_RIGHT)
                            if ( Rwindow > 5 )
                                Rwindow -- ;
                    }
                    if ( mousey >= 30+texteDestination.h+(texteDestination.h*2+35)*4  && mousey <= 35+texteDestination.h*2+(texteDestination.h*2+35)*4 ) {
                        if( event.button.button == SDL_BUTTON_WHEELUP || event.button.button == SDL_BUTTON_LEFT)
                            if ( thresholdtheta < 90 )
                                thresholdtheta ++ ;

                        if( event.button.button == SDL_BUTTON_WHEELDOWN || event.button.button == SDL_BUTTON_RIGHT)
                            if ( thresholdtheta > 1 )
                                thresholdtheta -- ;
                    }
                    if ( mousey >= 30+texteDestination.h+(texteDestination.h*2+35)*5  && mousey <= 35+texteDestination.h*2+(texteDestination.h*2+35)*5 ) {
                        if( event.button.button == SDL_BUTTON_WHEELUP || event.button.button == SDL_BUTTON_LEFT)
                                checktheta = 1 ;

                        if( event.button.button == SDL_BUTTON_WHEELDOWN || event.button.button == SDL_BUTTON_RIGHT)
                                checktheta = 0 ;
                    }
                    if ( mousey >= 30+texteDestination.h+(texteDestination.h*2+35)*6  && mousey <= 35+texteDestination.h*2+(texteDestination.h*2+35)*6 ) {
                        if( event.button.button == SDL_BUTTON_WHEELUP || event.button.button == SDL_BUTTON_LEFT)
                            if ( gaptheta < 90 )
                                gaptheta ++ ;

                        if( event.button.button == SDL_BUTTON_WHEELDOWN || event.button.button == SDL_BUTTON_RIGHT)
                            if ( gaptheta > 1 )
                                gaptheta -- ;
                    }
                    if ( mousey >= 30+texteDestination.h+(texteDestination.h*2+35)*7  && mousey <= 35+texteDestination.h*2+(texteDestination.h*2+35)*7 ) {
                        if( event.button.button == SDL_BUTTON_WHEELUP || event.button.button == SDL_BUTTON_LEFT)
                                printnull = 1 ;

                        if( event.button.button == SDL_BUTTON_WHEELDOWN || event.button.button == SDL_BUTTON_RIGHT)
                                printnull = 0 ;
                    }
                } else if ( mousex >= 20 + texteDestination.h*2 && mousex <= 30 + texteDestination.h*3 ) {
                    if ( mousey >= 30+texteDestination.h+(texteDestination.h*2+35)*5  && mousey <= 35+texteDestination.h*2+(texteDestination.h*2+35)*5 ) {
                        if( event.button.button == SDL_BUTTON_WHEELUP || event.button.button == SDL_BUTTON_LEFT)
                                checkamp = 1 ;

                        if( event.button.button == SDL_BUTTON_WHEELDOWN || event.button.button == SDL_BUTTON_RIGHT)
                                checkamp = 0 ;
                    }
                    if ( mousey >= 30+texteDestination.h+(texteDestination.h*2+35)*6  && mousey <= 35+texteDestination.h*2+(texteDestination.h*2+35)*6 ) {
                        if( event.button.button == SDL_BUTTON_WHEELUP || event.button.button == SDL_BUTTON_LEFT)
                            if ( gapamp < 400 )
                                gapamp ++ ;

                        if( event.button.button == SDL_BUTTON_WHEELDOWN || event.button.button == SDL_BUTTON_RIGHT)
                            if ( gapamp > 0 )
                                gapamp -- ;
                    }
                    if ( mousey >= 30+texteDestination.h+(texteDestination.h*2+35)*7  && mousey <= 35+texteDestination.h*2+(texteDestination.h*2+35)*7 ) {
                        if( event.button.button == SDL_BUTTON_WHEELUP || event.button.button == SDL_BUTTON_LEFT)
                                printfinal =1 ;

                        if( event.button.button == SDL_BUTTON_WHEELDOWN || event.button.button == SDL_BUTTON_RIGHT)
                                printfinal = 0 ;
                    }
                }
            }


            if ( mousex >= SDLwidth/2-RESETwidth/2 && mousex <= SDLwidth/2+RESETwidth/2 &&  mousey >= SDLheight-RESETheight && mousey <= SDLheight ) {
                for( unsigned x = 0 ; x < RESETwidth ; x ++ ) {
                    for( unsigned y = 0 ; y < RESETheight ; y ++ ) {
                        //give the color value to the pixel of the screenbuffer
                        Uint32* bufp;
                        bufp = (Uint32 *)scr->pixels + ((y+SDLheight-RESETheight) * scr->pitch / 4) + (x+SDLwidth/2-RESETwidth/2) ;
                        float a = float(resetp[4*y*RESETwidth+4*x+3]) / 254  ;
                        unsigned r = background[4*(y+SDLheight-RESETheight)*SDLwidth+4*(x+SDLwidth/2-RESETwidth/2)+0] * ( 1 - a ) + resetp[4*y*RESETwidth+4*x+0] * a ;
                        unsigned g = background[4*(y+SDLheight-RESETheight)*SDLwidth+4*(x+SDLwidth/2-RESETwidth/2)+1] * ( 1 - a ) + resetp[4*y*RESETwidth+4*x+1] * a ;
                        unsigned b = background[4*(y+SDLheight-RESETheight)*SDLwidth+4*(x+SDLwidth/2-RESETwidth/2)+2] * ( 1 - a ) + resetp[4*y*RESETwidth+4*x+2] * a ;
                        *bufp = 65536 * r + 256 * g + b ;
                    }
                }
                if( event.type == SDL_MOUSEBUTTONUP ) {
                    if( event.button.button == SDL_BUTTON_LEFT ) {
                        //On remet les valeurs par defaut
                        susantype = A ;
                        susangap = B ;
                        erosionwindow = C ;
                        Rwindow = D ;
                        thresholdtheta = E ;
                        checktheta = F ;
                        gaptheta = G ;
                        checkamp = H ;
                        gapamp = I ;
                        printnull = J ;
                        printfinal = K ;
                    }
                }
            } else {
                for( unsigned x = 0 ; x < RESETwidth ; x ++ ) {
                    for( unsigned y = 0 ; y < RESETheight ; y ++ ) {
                        //give the color value to the pixel of the screenbuffer
                        Uint32* bufp;
                        bufp = (Uint32 *)scr->pixels + ((y+SDLheight-RESETheight) * scr->pitch / 4) + (x+SDLwidth/2-RESETwidth/2) ;
                        float a = float(resetn[4*y*RESETwidth+4*x+3]) / 254  ;
                        unsigned r = background[4*(y+SDLheight-RESETheight)*SDLwidth+4*(x+SDLwidth/2-RESETwidth/2)+0] * ( 1 - a ) + resetn[4*y*RESETwidth+4*x+0] * a ;
                        unsigned g = background[4*(y+SDLheight-RESETheight)*SDLwidth+4*(x+SDLwidth/2-RESETwidth/2)+1] * ( 1 - a ) + resetn[4*y*RESETwidth+4*x+1] * a ;
                        unsigned b = background[4*(y+SDLheight-RESETheight)*SDLwidth+4*(x+SDLwidth/2-RESETwidth/2)+2] * ( 1 - a ) + resetn[4*y*RESETwidth+4*x+2] * a ;
                        *bufp = 65536 * r + 256 * g + b ;
                    }
                }
            }

            //On affiche les valeurs dans les cadres correspondants
            for( text = 0 ; text < 11 ; text++ ) {
                if ( text == 0 )
                    textbox = convertint( susantype ) ;
                else if ( text == 1 )
                    textbox = convertint( susangap ) ;
                else if ( text == 2 )
                    textbox = convertint( erosionwindow ) ;
                else if ( text == 3 )
                    textbox = convertint( Rwindow ) ;
                else if ( text == 4 )
                    textbox = convertint( thresholdtheta ) ;
                else if ( text == 5 )
                    textbox = convertint( checktheta ) ;
                else if ( text == 6 )
                    textbox = convertint( gaptheta ) ;
                else if ( text == 7 )
                    textbox = convertint( printnull ) ;

                else if ( text == 8 )
                    textbox = convertint( checkamp ) ;
                else if ( text == 9 )
                    textbox = convertint( gapamp ) ;
                else if ( text == 10 )
                    textbox = convertint( printfinal ) ;

                fontColor = {0, 0, 0} ;
                pFontSurface = TTF_RenderText_Solid(pFont,textbox,fontColor) ;
                texteDestination.w = pFontSurface->w ;
                texteDestination.h = pFontSurface->h ;
                texteDestination.x = 20 ;
                texteDestination.y = 34 + (texteDestination.h*2 + 35 ) * text + texteDestination.h ;

                 //Bouton - rep
                 if ( text < 8 ) {
                    for( unsigned x = texteDestination.x-5 ; x < texteDestination.x+texteDestination.h+15 ; x ++) {
                        for( unsigned y = texteDestination.y+texteDestination.h-23 ; y < texteDestination.y+texteDestination.h*2-17 ; y ++ ) {
                            //give the color value to the pixel of the screenbuffer
                            Uint32* bufp;
                            bufp = (Uint32 *)scr->pixels + (y * scr->pitch / 4) + x  ;
                            *bufp = 65536 * background[4*y*SDLwidth+4*x+0] + 256 * background[4*y*SDLwidth+4*x+1] + background[4*y*SDLwidth+4*x+2] ;
                        }
                    }
                    if ( text == 5 || text == 6 || text == 7 ) {
                        //Bouton - rep decale
                        for( unsigned x = texteDestination.x+texteDestination.h*2 ; x < texteDestination.x+texteDestination.h*3+20 ; x ++) {
                            for( unsigned y = texteDestination.y+texteDestination.h-23 ; y < texteDestination.y+texteDestination.h*2-17 ; y ++ ) {
                                //give the color value to the pixel of the screenbuffer
                                Uint32* bufp;
                                bufp = (Uint32 *)scr->pixels + (y * scr->pitch / 4) + x  ;
                                *bufp = 65536 * background[4*y*SDLwidth+4*x+0] + 256 * background[4*y*SDLwidth+4*x+1] + background[4*y*SDLwidth+4*x+2] ;
                            }
                        }
                    }
                }

                if ( text == 8 || text == 9 || text == 10 ) {
                    texteDestination.x += 5 + texteDestination.h * 2 ;
                    texteDestination.y = 34 + (texteDestination.h*2 + 35 ) * (text-3) + texteDestination.h ;
                }

                //Affiche le texte
                if ( SDL_BlitSurface(pFontSurface,NULL,scr,&texteDestination) != 0 )  {
                    fprintf(stderr,"Erreur de copie du texte sur l'ecran\n") ;
                    free( background ) ;
                    free( start ) ;
                    return -3 ;
                }
            }
        }
        SDL_UpdateRect(scr, 0, 0, 0, 0); //redraw screen
        SDL_Delay(5); //pause 5 ms so it consumes less processing power
    }

    SDL_FreeSurface(scr) ;
    TTF_CloseFont(pFont) ;
    SDL_Quit();
    cleanup() ;
    free( background ) ;
    free( circle ) ;
    free( button ) ;
    free( start ) ;
    free( resetn ) ;
    free( resetp ) ;
	std::cout << "Cache vide !" << std::endl << std::endl ;

    if ( done == 5 ) {
        A = susantype ;
        B = susangap ;
        C = erosionwindow ;
        D = Rwindow ;
        E = thresholdtheta ;
        F = checktheta ;
        G = gaptheta ;
        H = checkamp ;
        I = gapamp ;
        J = printnull ;
        K = printfinal ;

        return 0 ;
    }

    return done ;
}

int showscreen2( int &A, int &B, int &C, int &D, int &E, int &F, int &G, int &H, int &I, int &J, int &K) {
    unsigned int SDLwidth, SDLheight ;
	int error ;
    int done = 0 ;
    unsigned char* background ;
    unsigned char* logo ;
    unsigned char* circle ;
    unsigned char* matrice ;
    unsigned char* button ;
    unsigned char* start ;
    unsigned char* resetn ;
    unsigned char* resetp ;


    // Initialize ESCAPI
	int devices = setupESCAPI();
    int zoom = 1 ;
    int xr, yr ;

	if (devices == 0) {
		printf("ESCAPI initialization failure or no devices found.\n");
		return 1 ;
	}

	for (int i = 0; i < devices; i++) {
		char temp[256];
		getCaptureDeviceName(i, temp, 256);
		printf("Device %d: \"%s\"\n", i, temp);
	}

	struct SimpleCapParams capture;
	//300/225/3
	//400/300/2
	//440/330/2
	//460/345/2
	//560/420/2
	//640/480/2
	//800/600/1
	capture.mWidth = 440 ;
	capture.mHeight = 330 ;
	unsigned int CAMwidth = capture.mWidth ;
	unsigned int CAMheight = capture.mHeight ;
	capture.mTargetBuf = new int[CAMwidth * CAMheight];

	//Initialize capture - only one capture may be active per device,
    //but several devices may be captured at the same time.
    //0 is the first device.
	if (initCapture(1, &capture) == 0) {
		printf("Capture failed - device may already be in use.\n");
		return 2 ;
	}

    unsigned int*** image = newimg( CAMwidth, CAMheight) ;
    unsigned int*** final = newimg( CAMwidth, CAMheight) ;

    //Chargement du background
	if( (error = lodepng_decode32_file(&background, &SDLwidth, &SDLheight, "./screen/bg3.png")) ) {
		std::cerr << "decoder error " << error << ": " << lodepng_error_text(error) << std::endl ;
		return 2 ;
	}
	std::cout << "L'image \"./screen/bg3.png\" a ete chargee ..." << std::endl ;

	unsigned int LOGOwidth, LOGOheight ;
    //Chargement du cercle blanc
	if( (error = lodepng_decode32_file(&logo, &LOGOwidth, &LOGOheight, "./screen/logo.png")) ) {
		std::cerr << "decoder error " << error << ": " << lodepng_error_text(error) << std::endl ;
		return 2 ;
	}
	std::cout << "L'image \"./screen/logo.png\" a ete chargee ..." << std::endl ;

	unsigned int CIRCLEwidth, CIRCLEheight ;
    //Chargement du cercle blanc
	if( (error = lodepng_decode32_file(&circle, &CIRCLEwidth, &CIRCLEheight, "./screen/circle.png")) ) {
		std::cerr << "decoder error " << error << ": " << lodepng_error_text(error) << std::endl ;
		return 2 ;
	}
	std::cout << "L'image \"./screen/circle.png\" a ete chargee ..." << std::endl ;

	unsigned int MATRICEwidth, MATRICEheight ;
    //Chargement du fond de la matrice
	if( (error = lodepng_decode32_file(&matrice, &MATRICEwidth, &MATRICEheight, "./screen/matrice.png")) ) {
		std::cerr << "decoder error " << error << ": " << lodepng_error_text(error) << std::endl ;
		return 2 ;
	}
	std::cout << "L'image \"./screen/matrice.png\" a ete chargee ..." << std::endl ;

	unsigned int BUTTONwidth, BUTTONheight ;
    //Chargement du cercle blanc
	if( (error = lodepng_decode32_file(&button, &BUTTONwidth, &BUTTONheight, "./screen/button.png")) ) {
		std::cerr << "decoder error " << error << ": " << lodepng_error_text(error) << std::endl ;
		return 2 ;
	}
	std::cout << "L'image \"./screen/button.png\" a ete chargee ..." << std::endl ;

	unsigned int STARTwidth, STARTheight ;
    //Chargement du bouton start
	if( (error = lodepng_decode32_file(&start, &STARTwidth, &STARTheight, "./screen/start_p.png")) ) {
		std::cerr << "decoder error " << error << ": " << lodepng_error_text(error) << std::endl ;
		return 2 ;
	}
	std::cout << "L'image \"./screen/start_p.png\" a ete chargee ..." << std::endl ;

	unsigned int RESETwidth, RESETheight ;
    //Chargement du bouton reset_n
	if( (error = lodepng_decode32_file(&resetn, &RESETwidth, &RESETheight, "./screen/reset_n.png")) ) {
		std::cerr << "decoder error " << error << ": " << lodepng_error_text(error) << std::endl ;
		return 2 ;
	}
	std::cout << "L'image \"./screen/reset_n.png\" a ete chargee ..." << std::endl ;

    //Chargement du bouton reset_p
	if( (error = lodepng_decode32_file(&resetp, &RESETwidth, &RESETheight, "./screen/reset_p.png")) ) {
		std::cerr << "decoder error " << error << ": " << lodepng_error_text(error) << std::endl ;
		return 2 ;
	}
	std::cout << "L'image \"./screen/reset_p.png\" a ete chargee ..." << std::endl ;

    int imgFlags = IMG_INIT_TIF ; //Flags pour charger le support du TIF
    TTF_Font* pFont = NULL;
    const char* fontPath = "./screen/font.ttf" ;
    SDL_Color fontColor = {0, 0, 0} ;
    SDL_Surface* pFontSurface = NULL ;
    SDL_Rect texteDestination ;

    //init SDL
    if(SDL_Init(SDL_INIT_VIDEO) < 0) {
        std::cout << "error, SDL video init failed" << std::endl ;
        free( background ) ;
        cleanup() ;
        return -1 ;
    }

    if ( IMG_Init(imgFlags) != imgFlags ) {
        fprintf(stderr,"Erreur lors de l'initialisation de la SDL_image : '%s'\n",IMG_GetError()) ;
        free( background ) ;
        free( start ) ;
        cleanup() ;
        return -1 ;
    }

    SDL_Surface* scr = SDL_SetVideoMode(SDLwidth , SDLheight , 32, SDL_HWSURFACE) ;
    if(!scr) {
        std::cout << "error, no SDL screen" << std::endl ;
        free( background ) ;
        free( start ) ;
        cleanup() ;
        return -2;
    }

    if ( TTF_Init() == -1 ) {
        fprintf(stderr,"Erreur lors de l'initialisation de la SDL_TTF : '%s'\n",TTF_GetError()) ;
        free( background ) ;
        free( start ) ;
        cleanup() ;
        return -1 ;
    }

    SDL_WM_SetCaption("Vision System", NULL) ; //set window caption

    //on imprime le background
    for( unsigned x = 0 ; x < SDLwidth ; x ++ ) {
        for( unsigned y = 0 ; y < SDLheight ; y ++ ) {
            //give the color value to the pixel of the screenbuffer
            if (background[4*y*SDLwidth+4*x+3] != 0) { //alpha
                Uint32* bufp;
                bufp = (Uint32 *)scr->pixels + (y * scr->pitch / 4) + x ;
                *bufp = 65536 * background[4*y*SDLwidth+4*x+0] + 256 * background[4*y*SDLwidth+4*x+1] + background[4*y*SDLwidth+4*x+2] ;
            }
        }
    }

    //Imprime le logo blanc en haut à droite
    for( unsigned x = 0 ; x < LOGOwidth ; x ++ ) {
        for( unsigned y = 0 ; y < LOGOheight ; y ++ ) {
            //give the color value to the pixel of the screenbuffer
            Uint32* bufp;
            bufp = (Uint32 *)scr->pixels + (y * scr->pitch / 4) + (x+SDLwidth-LOGOwidth) ;
            float a = float(logo[4*y*LOGOwidth+4*x+3]) / 254  ;
            background[4*y*SDLwidth+4*(x+SDLwidth-LOGOwidth)+0] = background[4*y*SDLwidth+4*(x+SDLwidth-LOGOwidth)+0] * ( 1 - a ) + logo[4*y*LOGOwidth+4*x+0] * a ;
            background[4*y*SDLwidth+4*(x+SDLwidth-LOGOwidth)+1] = background[4*y*SDLwidth+4*(x+SDLwidth-LOGOwidth)+1] * ( 1 - a ) + logo[4*y*LOGOwidth+4*x+1] * a ;
            background[4*y*SDLwidth+4*(x+SDLwidth-LOGOwidth)+2] = background[4*y*SDLwidth+4*(x+SDLwidth-LOGOwidth)+2] * ( 1 - a ) + logo[4*y*LOGOwidth+4*x+2] * a ;
            *bufp = 65536 * background[4*y*SDLwidth+4*(x+SDLwidth-LOGOwidth)+0] + 256 * background[4*y*SDLwidth+4*(x+SDLwidth-LOGOwidth)+1] + background[4*y*SDLwidth+4*(x+SDLwidth-LOGOwidth)+2] ;
        }
    }

    //Imprime le cercle blanc en bas à droite
    for( unsigned x = 0 ; x < CIRCLEwidth ; x ++ ) {
        for( unsigned y = 0 ; y < CIRCLEheight ; y ++ ) {
            //give the color value to the pixel of the screenbuffer
            Uint32* bufp;
            bufp = (Uint32 *)scr->pixels + ((y+SDLheight-CIRCLEheight) * scr->pitch / 4) + (x+SDLwidth-CIRCLEwidth) ;
            float a = float(circle[4*y*CIRCLEwidth+4*x+3]) / 254  ;
            background[4*(y+SDLheight-CIRCLEheight)*SDLwidth+4*(x+SDLwidth-CIRCLEwidth)+0] = background[4*(y+SDLheight-CIRCLEheight)*SDLwidth+4*(x+SDLwidth-CIRCLEwidth)+0] * ( 1 - a ) + circle[4*y*CIRCLEwidth+4*x+0] * a ;
            background[4*(y+SDLheight-CIRCLEheight)*SDLwidth+4*(x+SDLwidth-CIRCLEwidth)+1] = background[4*(y+SDLheight-CIRCLEheight)*SDLwidth+4*(x+SDLwidth-CIRCLEwidth)+1] * ( 1 - a ) + circle[4*y*CIRCLEwidth+4*x+1] * a ;
            background[4*(y+SDLheight-CIRCLEheight)*SDLwidth+4*(x+SDLwidth-CIRCLEwidth)+2] = background[4*(y+SDLheight-CIRCLEheight)*SDLwidth+4*(x+SDLwidth-CIRCLEwidth)+2] * ( 1 - a ) + circle[4*y*CIRCLEwidth+4*x+2] * a ;
            *bufp = 65536 * background[4*(y+SDLheight-CIRCLEheight)*SDLwidth+4*(x+SDLwidth-CIRCLEwidth)+0] + 256 * background[4*(y+SDLheight-CIRCLEheight)*SDLwidth+4*(x+SDLwidth-CIRCLEwidth)+1] + background[4*(y+SDLheight-CIRCLEheight)*SDLwidth+4*(x+SDLwidth-CIRCLEwidth)+2] ;
        }
    }

    // Charge la police
    pFont = TTF_OpenFont(fontPath,16) ;
    if ( pFont == NULL ) {
        fprintf(stderr,"Erreur de chargement de la police %s : %s\n",fontPath,TTF_GetError()) ;
        free( background ) ;
        free( start ) ;
        cleanup() ;
        return -3 ;
    }

    int text ;
    for( text = 0 ; text < 8 ; text++ ) {
        if ( text == 0 ) // Genere la SDL_Surface a partir de la police
            pFontSurface = TTF_RenderText_Solid(pFont,"Taille du détecteur de points d'intérêt ?",fontColor) ;
        else if ( text == 1 )
            pFontSurface = TTF_RenderText_Solid(pFont,"Seuil pour le detecteur ?",fontColor) ;
        else if ( text == 2 )
            pFontSurface = TTF_RenderText_Solid(pFont,"Taille maximale de la fenêtre d'érosion ?",fontColor) ;
        else if ( text == 3 )
            pFontSurface = TTF_RenderText_Solid(pFont,"Taille de la fenêtre de correspondance ?",fontColor) ;
        else if ( text == 4 )
            pFontSurface = TTF_RenderText_Solid(pFont,"Nombre de cadrants pour l'angle ?",fontColor) ;
        else if ( text == 5 )
            pFontSurface = TTF_RenderText_Solid(pFont,"Verifier l'angle ? Vérifier l'amplitude?",fontColor) ;
        else if ( text == 6 )
            pFontSurface = TTF_RenderText_Solid(pFont,"Seuil pour l'angle ? Seuil pour l'amplitude ?",fontColor) ;
        else if ( text == 7 )
            pFontSurface = TTF_RenderText_Solid(pFont,"Afficher les points nuls ? Enregistrer les images ?",fontColor) ;

        // Nous avons notre surface pour le texte donc nous calculons la position relative
        texteDestination.w = pFontSurface->w ;
        texteDestination.h = pFontSurface->h ;
        texteDestination.x = 15 ; //43
        texteDestination.y = 15 + (texteDestination.h*2 + 35 ) * text ; //45

        //Arrière plan texte
        if ( text < 8 ) {
            int gapx2 = -27 ;
            int gapy2 = 0 ;
            int gapx = texteDestination.w - 20 ;
            int gapy = texteDestination.y - 44 + gapy2 ;

            //On imprime le début du bouton
            for( unsigned x = 0 ; x < (BUTTONwidth-1)/2 ; x ++ ) {
                for( unsigned y = 0 ; y < BUTTONheight ; y ++ ) {
                    if ( y+gapy >= 0 && y+gapy <= SDLheight && x+gapx2 >= 0 && x+gapx2 <= SDLwidth ) {
                        //give the color value to the pixel of the screenbuffer
                        Uint32* bufp;
                        bufp = (Uint32 *)scr->pixels + ((y+gapy) * scr->pitch / 4) + (x+gapx2) ;
                        float a = float(button[4*y*BUTTONwidth+4*x+3]) / 254  ;
                        background[4*(y+gapy)*SDLwidth+4*(x+gapx2)+0] = background[4*(y+gapy)*SDLwidth+4*(x+gapx2)+0] * ( 1 - a ) + button[4*y*BUTTONwidth+4*x+0] * a ;
                        unsigned r = background[4*(y+gapy)*SDLwidth+4*(x+gapx2)+0] ;
                        background[4*(y+gapy)*SDLwidth+4*(x+gapx2)+1] = background[4*(y+gapy)*SDLwidth+4*(x+gapx2)+1] * ( 1 - a ) + button[4*y*BUTTONwidth+4*x+1] * a ;
                        unsigned g = background[4*(y+gapy)*SDLwidth+4*(x+gapx2)+1] ;
                        background[4*(y+gapy)*SDLwidth+4*(x+gapx2)+2] = background[4*(y+gapy)*SDLwidth+4*(x+gapx2)+2] * ( 1 - a ) + button[4*y*BUTTONwidth+4*x+2] * a ;
                        unsigned b = background[4*(y+gapy)*SDLwidth+4*(x+gapx2)+2] ;
                        *bufp = 65536 * r + 256 * g + b ;
                    }
                }
            }

            //On imprime la fin du bouton
            for( unsigned x = (BUTTONwidth-1)/2 ; x < (BUTTONwidth-1)/2 + gapx + 1 ; x ++ ) {
                for( unsigned y = 0 ; y < BUTTONheight ; y ++ ) {
                    if ( y+gapy >= 0 && y+gapy <= SDLheight && x+gapx2 >= 0 && x+gapx2 <= SDLwidth ) {
                        //give the color value to the pixel of the screenbuffer
                        Uint32* bufp;
                        bufp = (Uint32 *)scr->pixels + ((y+gapy) * scr->pitch) / 4 + (x+gapx2) ;
                        float a = float(button[4*y*BUTTONwidth+4*(BUTTONwidth-1)/2+3]) / 254  ;
                        background[4*(y+gapy)*SDLwidth+4*(x+gapx2)+0] = background[4*(y+gapy)*SDLwidth+4*(x+gapx2)+0] * ( 1 - a ) + button[4*y*BUTTONwidth+4*(BUTTONwidth-1)/2+0] * a ;
                        unsigned r = background[4*(y+gapy)*SDLwidth+4*(x+gapx2)+0] ;
                        background[4*(y+gapy)*SDLwidth+4*(x+gapx2)+1] = background[4*(y+gapy)*SDLwidth+4*(x+gapx2)+1] * ( 1 - a ) + button[4*y*BUTTONwidth+4*(BUTTONwidth-1)/2+1] * a ;
                        unsigned g = background[4*(y+gapy)*SDLwidth+4*(x+gapx2)+1] ;
                        background[4*(y+gapy)*SDLwidth+4*(x+gapx2)+2] = background[4*(y+gapy)*SDLwidth+4*(x+gapx2)+2] * ( 1 - a ) + button[4*y*BUTTONwidth+4*(BUTTONwidth-1)/2+2] * a ;
                        unsigned b = background[4*(y+gapy)*SDLwidth+4*(x+gapx2)+2] ;
                        *bufp = 65536 * r + 256 * g + b ;
                    }
                }
            }

            //On imprime la fin du bouton
            for( unsigned x = 1+(BUTTONwidth-1)/2 ; x < BUTTONwidth ; x ++ ) {
                for( unsigned y = 0 ; y < BUTTONheight ; y ++ ) {
                    if ( y+gapy >= 0 && y+gapy <= SDLheight && x+gapx2+gapx >= 0 && x+gapx2+gapx <= SDLwidth ) {
                        //give the color value to the pixel of the screenbuffer
                        Uint32* bufp;
                        bufp = (Uint32 *)scr->pixels + ((y+gapy) * scr->pitch) / 4 + (x+gapx2+gapx) ;
                        float a = float(button[4*y*BUTTONwidth+4*x+3]) / 254  ;
                        background[4*(y+gapy)*SDLwidth+4*(x+gapx2+gapx)+0] = background[4*(y+gapy)*SDLwidth+4*(x+gapx2+gapx)+0] * ( 1 - a ) + button[4*y*BUTTONwidth+4*x+0] * a ;
                        unsigned r = background[4*(y+gapy)*SDLwidth+4*(x+gapx2+gapx)+0] ;
                        background[4*(y+gapy)*SDLwidth+4*(x+gapx2+gapx)+1] = background[4*(y+gapy)*SDLwidth+4*(x+gapx2+gapx)+1] * ( 1 - a ) + button[4*y*BUTTONwidth+4*x+1] * a ;
                        unsigned g = background[4*(y+gapy)*SDLwidth+4*(x+gapx2+gapx)+1] ;
                        background[4*(y+gapy)*SDLwidth+4*(x+gapx2+gapx)+2] = background[4*(y+gapy)*SDLwidth+4*(x+gapx2+gapx)+2] * ( 1 - a ) + button[4*y*BUTTONwidth+4*x+2] * a ;
                        unsigned b = background[4*(y+gapy)*SDLwidth+4*(x+gapx2+gapx)+2] ;
                        *bufp = 65536 * r + 256 * g + b ;
                    }
                }
            }
        }

        //Arrière plan reponse
        if ( text < 8 ) {
            int gapx2 = -23 ;
            int gapy2 = texteDestination.h + 18 ;
            int gapx = 0 ;
            if ( text == 0 || text == 4 )
                gapx = 0 ;
            else if ( text == 1 ||text == 2 || text == 3 )
                gapx = 10 ;
            else if ( text == 5 || text == 7 )
                gapx = 36 ;
            else if ( text == 6 )
                gapx = 52 ;
            int gapy = texteDestination.y - 44 + gapy2 ;

            //On imprime le début du bouton
            for( unsigned x = 0 ; x < (BUTTONwidth-1)/2 ; x ++ ) {
                for( unsigned y = 0 ; y < BUTTONheight ; y ++ ) {
                    if ( y+gapy >= 0 && y+gapy <= SDLheight && x+gapx2 >= 0 && x+gapx2 <= SDLwidth ) {
                        //give the color value to the pixel of the screenbuffer
                        Uint32* bufp;
                        bufp = (Uint32 *)scr->pixels + ((y+gapy) * scr->pitch / 4) + (x+gapx2) ;
                        float a = float(button[4*y*BUTTONwidth+4*x+3]) / 254  ;
                        background[4*(y+gapy)*SDLwidth+4*(x+gapx2)+0] = background[4*(y+gapy)*SDLwidth+4*(x+gapx2)+0] * ( 1 - a ) + button[4*y*BUTTONwidth+4*x+0] * a ;
                        unsigned r = background[4*(y+gapy)*SDLwidth+4*(x+gapx2)+0] ;
                        background[4*(y+gapy)*SDLwidth+4*(x+gapx2)+1] = background[4*(y+gapy)*SDLwidth+4*(x+gapx2)+1] * ( 1 - a ) + button[4*y*BUTTONwidth+4*x+1] * a ;
                        unsigned g = background[4*(y+gapy)*SDLwidth+4*(x+gapx2)+1] ;
                        background[4*(y+gapy)*SDLwidth+4*(x+gapx2)+2] = background[4*(y+gapy)*SDLwidth+4*(x+gapx2)+2] * ( 1 - a ) + button[4*y*BUTTONwidth+4*x+2] * a ;
                        unsigned b = background[4*(y+gapy)*SDLwidth+4*(x+gapx2)+2] ;
                        *bufp = 65536 * r + 256 * g + b ;
                    }
                }
            }

            //On imprime la fin du bouton
            for( unsigned x = (BUTTONwidth-1)/2 ; x < (BUTTONwidth-1)/2 + gapx + 1 ; x ++ ) {
                for( unsigned y = 0 ; y < BUTTONheight ; y ++ ) {
                    if ( y+gapy >= 0 && y+gapy <= SDLheight && x+gapx2 >= 0 && x+gapx2 <= SDLwidth ) {
                        //give the color value to the pixel of the screenbuffer
                        Uint32* bufp;
                        bufp = (Uint32 *)scr->pixels + ((y+gapy) * scr->pitch) / 4 + (x+gapx2) ;
                        float a = float(button[4*y*BUTTONwidth+4*(BUTTONwidth-1)/2+3]) / 254  ;
                        background[4*(y+gapy)*SDLwidth+4*(x+gapx2)+0] = background[4*(y+gapy)*SDLwidth+4*(x+gapx2)+0] * ( 1 - a ) + button[4*y*BUTTONwidth+4*(BUTTONwidth-1)/2+0] * a ;
                        unsigned r = background[4*(y+gapy)*SDLwidth+4*(x+gapx2)+0] ;
                        background[4*(y+gapy)*SDLwidth+4*(x+gapx2)+1] = background[4*(y+gapy)*SDLwidth+4*(x+gapx2)+1] * ( 1 - a ) + button[4*y*BUTTONwidth+4*(BUTTONwidth-1)/2+1] * a ;
                        unsigned g = background[4*(y+gapy)*SDLwidth+4*(x+gapx2)+1] ;
                        background[4*(y+gapy)*SDLwidth+4*(x+gapx2)+2] = background[4*(y+gapy)*SDLwidth+4*(x+gapx2)+2] * ( 1 - a ) + button[4*y*BUTTONwidth+4*(BUTTONwidth-1)/2+2] * a ;
                        unsigned b = background[4*(y+gapy)*SDLwidth+4*(x+gapx2)+2] ;
                        *bufp = 65536 * r + 256 * g + b ;
                    }
                }
            }

            //On imprime la fin du bouton
            for( unsigned x = 1+(BUTTONwidth-1)/2 ; x < BUTTONwidth ; x ++ ) {
                for( unsigned y = 0 ; y < BUTTONheight ; y ++ ) {
                    if ( y+gapy >= 0 && y+gapy <= SDLheight && x+gapx2+gapx >= 0 && x+gapx2+gapx <= SDLwidth ) {
                        //give the color value to the pixel of the screenbuffer
                        Uint32* bufp;
                        bufp = (Uint32 *)scr->pixels + ((y+gapy) * scr->pitch) / 4 + (x+gapx2+gapx) ;
                        float a = float(button[4*y*BUTTONwidth+4*x+3]) / 254  ;
                        background[4*(y+gapy)*SDLwidth+4*(x+gapx2+gapx)+0] = background[4*(y+gapy)*SDLwidth+4*(x+gapx2+gapx)+0] * ( 1 - a ) + button[4*y*BUTTONwidth+4*x+0] * a ;
                        unsigned r = background[4*(y+gapy)*SDLwidth+4*(x+gapx2+gapx)+0] ;
                        background[4*(y+gapy)*SDLwidth+4*(x+gapx2+gapx)+1] = background[4*(y+gapy)*SDLwidth+4*(x+gapx2+gapx)+1] * ( 1 - a ) + button[4*y*BUTTONwidth+4*x+1] * a ;
                        unsigned g = background[4*(y+gapy)*SDLwidth+4*(x+gapx2+gapx)+1] ;
                        background[4*(y+gapy)*SDLwidth+4*(x+gapx2+gapx)+2] = background[4*(y+gapy)*SDLwidth+4*(x+gapx2+gapx)+2] * ( 1 - a ) + button[4*y*BUTTONwidth+4*x+2] * a ;
                        unsigned b = background[4*(y+gapy)*SDLwidth+4*(x+gapx2+gapx)+2] ;
                        *bufp = 65536 * r + 256 * g + b ;
                    }
                }
            }
        }

        //Affiche le texte
        if ( SDL_BlitSurface(pFontSurface,NULL,scr,&texteDestination) != 0 )  {
            fprintf(stderr,"Erreur de copie du texte sur l'ecran\n") ;
            free( background ) ;
            free( start ) ;
            return -3 ;
        }
    }

    //Arriere plan matrice debut
    int gapy = 7 ;
    for( unsigned x = 0 ; x < MATRICEwidth ; x ++ ) {
        for( unsigned y = 0 ; y < MATRICEheight ; y ++ ) {
            if ( (y+gapy) <= MATRICEheight ) {
                //give the color value to the pixel of the screenbuffer
                Uint32* bufp;
                bufp = (Uint32 *)scr->pixels + (y * scr->pitch / 4) + (x+SDLwidth/2-MATRICEwidth/2) ;
                float a = float(matrice[4*(y+gapy)*MATRICEwidth+4*x+3]) / 254  ;
                background[4*y*SDLwidth+4*(x+SDLwidth/2-MATRICEwidth/2)+0] = background[4*y*SDLwidth+4*(x+SDLwidth/2-MATRICEwidth/2)+0] * ( 1 - a ) + matrice[4*(y+gapy)*MATRICEwidth+4*x+0] * a ;
                background[4*y*SDLwidth+4*(x+SDLwidth/2-MATRICEwidth/2)+1] = background[4*y*SDLwidth+4*(x+SDLwidth/2-MATRICEwidth/2)+1] * ( 1 - a ) + matrice[4*(y+gapy)*MATRICEwidth+4*x+1] * a ;
                background[4*y*SDLwidth+4*(x+SDLwidth/2-MATRICEwidth/2)+2] = background[4*y*SDLwidth+4*(x+SDLwidth/2-MATRICEwidth/2)+2] * ( 1 - a ) + matrice[4*(y+gapy)*MATRICEwidth+4*x+2] * a ;
                *bufp = 65536 * background[4*y*SDLwidth+4*(x+SDLwidth/2-MATRICEwidth/2)+0] + 256 * background[4*y*SDLwidth+4*(x+SDLwidth/2-MATRICEwidth/2)+1] + background[4*y*SDLwidth+4*(x+SDLwidth/2-MATRICEwidth/2)+2] ;
            }
        }
    }
    //Arriere plan matrice debut

    //pause until you press escape and meanwhile redraw screen
    SDL_Flip(scr) ;
    SDL_Event event ;
    int mousex, mousey ;
    int CAMmousex = 0 ;
    int CAMmousey = 0 ;
    text = 2 ;
    int susantype = A ;
    int susangap = B ;
    int erosionwindow = C ;
    int Rwindow = D ;
    int thresholdtheta = E ;
    int checktheta = F ;
    int gaptheta = G ;
    int checkamp = H ;
    int gapamp = I ;
    int printnull = J ;
    int printfinal = K ;

    int MA = 0 ;
    int MB = 0 ;
    int MC = 0 ;
    int MD = 0 ;
    int ME = 1 ;
    int MF = 0 ;
    int MG = 0 ;
    int MH = 0 ;
    int MI = 0 ;

    unsigned int hue = 20 ;
    int gap_h = 20 ;
    unsigned int sat = 40 ;
    int gap_s = 20 ;
    unsigned int val = 50 ;
    int gap_v = 35 ;

    char const *textbox ;

    while( done < 5 ) {
        while(SDL_PollEvent(&event)) {
            //Get the mouse offsets
            mousex = event.motion.x ;
            mousey = event.motion.y ;

            if ( mousex >= SDLwidth-CIRCLEwidth && mousex <= SDLwidth &&  mousey >= SDLheight-CIRCLEheight && mousey <= SDLheight ) {
                for( unsigned x = 0 ; x < STARTwidth ; x ++ ) {
                    for( unsigned y = 0 ; y < STARTheight ; y ++ ) {
                        //give the color value to the pixel of the screenbuffer
                        Uint32* bufp;
                        bufp = (Uint32 *)scr->pixels + ((y+SDLheight-CIRCLEheight/2) * scr->pitch / 4) + (x+SDLwidth-CIRCLEwidth/2-10) ;
                        float a = float(start[4*y*STARTwidth+4*x+3]) / 254  ;
                        unsigned r = background[4*(y+SDLheight-CIRCLEheight/2)*SDLwidth+4*(x+SDLwidth-CIRCLEwidth/2-10)+0] * ( 1 - a ) + start[4*y*STARTwidth+4*x+0] * a ;
                        unsigned g = background[4*(y+SDLheight-CIRCLEheight/2)*SDLwidth+4*(x+SDLwidth-CIRCLEwidth/2-10)+1] * ( 1 - a ) + start[4*y*STARTwidth+4*x+1] * a ;
                        unsigned b = background[4*(y+SDLheight-CIRCLEheight/2)*SDLwidth+4*(x+SDLwidth-CIRCLEwidth/2-10)+2] * ( 1 - a ) + start[4*y*STARTwidth+4*x+2] * a ;
                        *bufp = 65536 * r + 256 * g + b ;
                    }
                }
                if( event.type == SDL_MOUSEBUTTONUP )
                    if( event.button.button == SDL_BUTTON_LEFT )
                        done = 5 ;
            } else {
                for( unsigned x = 0 ; x < STARTwidth ; x ++ ) {
                    for( unsigned y = 0 ; y < STARTheight ; y ++ ) {
                        //give the color value to the pixel of the screenbuffer
                        Uint32* bufp;
                        bufp = (Uint32 *)scr->pixels + ((y+SDLheight-CIRCLEheight/2) * scr->pitch / 4) + (x+SDLwidth-CIRCLEwidth/2-10) ;
                        float a = float(start[4*y*STARTwidth+4*x+3]) / 254  ;
                        unsigned r = background[4*(y+SDLheight-CIRCLEheight/2)*SDLwidth+4*(x+SDLwidth-CIRCLEwidth/2-10)+0] ;
                        unsigned g = background[4*(y+SDLheight-CIRCLEheight/2)*SDLwidth+4*(x+SDLwidth-CIRCLEwidth/2-10)+1] ;
                        unsigned b = background[4*(y+SDLheight-CIRCLEheight/2)*SDLwidth+4*(x+SDLwidth-CIRCLEwidth/2-10)+2] ;
                        *bufp = 65536 * r + 256 * g + b ;
                    }
                }
            }


            if(event.type == SDL_QUIT)
                done = 10 ;
            else if(SDL_GetKeyState(NULL)[SDLK_ESCAPE])
                done = 10 ;
            else if(SDL_GetKeyState(NULL)[SDLK_RETURN] || SDL_GetKeyState(NULL)[SDLK_KP_ENTER]) //Touche entrer pour valider les modifications
                done = 5 ;

            printf("x = %d - y = %d\n",CAMmousex, CAMmousey) ;
            printf("hue = %d - sat = %d - val = %d\n",hue, sat, val) ;
            if( event.type == SDL_MOUSEBUTTONUP ) {
                if ( mousex >= (SDLwidth/2-(CAMwidth/2)*zoom) && mousex <= (SDLwidth/2+(CAMwidth/2)*zoom) ) {
                    if ( mousey >= (SDLheight/2-(CAMheight/2)*zoom) && mousey <= (SDLheight/2+(CAMheight/2)*zoom) ) {
                        if( event.button.button == SDL_BUTTON_RIGHT ) {
                            CAMmousex = (mousex - (SDLwidth/2-(CAMwidth/2)*zoom))/zoom ;
                            CAMmousey = (mousey - (SDLheight/2-(CAMheight/2)*zoom))/zoom ;
                            rgb2hsv( image[CAMmousex][CAMmousey][0], image[CAMmousex][CAMmousey][1], image[CAMmousex][CAMmousey][2], hue, sat, val) ;
                        }
                    }
                }

                if ( mousex >= SDLwidth/2-(CAMwidth/2)*zoom && mousex <= SDLwidth/2+(CAMwidth/2)*zoom && mousey >= SDLheight/2-(CAMheight/2)*zoom && mousey <= SDLheight/2+(CAMheight/2)*zoom ) {
                    //Si on bouge la molette vers le haut
                    if( event.button.button == SDL_BUTTON_WHEELUP ) {
                        if ( susangap < 128 )
                            susangap ++ ;
                    }
                    //Si on bouge la molette vers le bas
                    if( event.button.button == SDL_BUTTON_WHEELDOWN ) {
                        if ( susangap > 0 )
                            susangap -- ;
                    }

                    if( event.button.button == SDL_BUTTON_LEFT )
                        zoom = 2 ;

                    if( event.button.button == SDL_BUTTON_RIGHT ) {
                        zoom = 1 ;

                        for( unsigned x = 0 ; x < SDLwidth ; x ++ ) {
                            for( unsigned y = 0 ; y < SDLheight ; y ++ ) {
                                //give the color value to the pixel of the screenbuffer
                                Uint32* bufp;
                                bufp = (Uint32 *)scr->pixels + (y * scr->pitch / 4) + x ;
                                unsigned r = background[4*y*SDLwidth+4*x+0] ;
                                unsigned g = background[4*y*SDLwidth+4*x+1] ;
                                unsigned b = background[4*y*SDLwidth+4*x+2] ;
                                *bufp = 65536 * r + 256 * g + b ;
                            }
                        }

                        for( text = 0 ; text < 8 ; text++ ) {
                            if ( text == 0 ) // Genere la SDL_Surface a partir de la police
                                pFontSurface = TTF_RenderText_Solid(pFont,"Taille du détecteur de points d'intérêt ?",fontColor) ;
                            else if ( text == 1 )
                                pFontSurface = TTF_RenderText_Solid(pFont,"Seuil pour le detecteur ?",fontColor) ;
                            else if ( text == 2 )
                                pFontSurface = TTF_RenderText_Solid(pFont,"Taille maximale de la fenêtre d'érosion ?",fontColor) ;
                            else if ( text == 3 )
                                pFontSurface = TTF_RenderText_Solid(pFont,"Taille de la fenêtre de correspondance ?",fontColor) ;
                            else if ( text == 4 )
                                pFontSurface = TTF_RenderText_Solid(pFont,"Nombre de cadrants pour l'angle ?",fontColor) ;
                            else if ( text == 5 )
                                pFontSurface = TTF_RenderText_Solid(pFont,"Verifier l'angle ? Vérifier l'amplitude?",fontColor) ;
                            else if ( text == 6 )
                                pFontSurface = TTF_RenderText_Solid(pFont,"Seuil pour l'angle ? Seuil pour l'amplitude ?",fontColor) ;
                            else if ( text == 7 )
                                pFontSurface = TTF_RenderText_Solid(pFont,"Afficher les points nuls ? Enregistrer les images ?",fontColor) ;

                            // Nous avons notre surface pour le texte donc nous calculons la position relative
                            texteDestination.w = pFontSurface->w ;
                            texteDestination.h = pFontSurface->h ;
                            texteDestination.x = 15 ; //43
                            texteDestination.y = 15 + (texteDestination.h*2 + 35 ) * text ; //45

                            //Affiche le texte
                            if ( SDL_BlitSurface(pFontSurface,NULL,scr,&texteDestination) != 0 )  {
                                fprintf(stderr,"Erreur de copie du texte sur l'ecran\n") ;
                                free( background ) ;
                                free( start ) ;
                                return -3 ;
                            }
                        }
                    }
                }



                if ( mousey >= 32 && mousey <= 72 && mousex >= 472 && mousex <= 530 ) {
                    if( event.button.button == SDL_BUTTON_WHEELUP || event.button.button == SDL_BUTTON_LEFT )
                        MA ++ ;

                    if( event.button.button == SDL_BUTTON_WHEELDOWN || event.button.button == SDL_BUTTON_RIGHT )
                        MA -- ;
                }
                if ( mousey >= 32 && mousey <= 72 && mousex >= 530 && mousex <= 588 ) {
                    if( event.button.button == SDL_BUTTON_WHEELUP || event.button.button == SDL_BUTTON_LEFT )
                        MB ++ ;

                    if( event.button.button == SDL_BUTTON_WHEELDOWN || event.button.button == SDL_BUTTON_RIGHT )
                        MB -- ;
                }
                if ( mousey >= 32 && mousey <= 72 && mousex >= 588 && mousex <= 646 ) {
                    if( event.button.button == SDL_BUTTON_WHEELUP || event.button.button == SDL_BUTTON_LEFT )
                        MC ++ ;

                    if( event.button.button == SDL_BUTTON_WHEELDOWN || event.button.button == SDL_BUTTON_RIGHT )
                        MC -- ;
                }
                if ( mousey >= 72 && mousey <= 112 && mousex >= 472 && mousex <= 530 ) {
                    if( event.button.button == SDL_BUTTON_WHEELUP || event.button.button == SDL_BUTTON_LEFT )
                        MD ++ ;

                    if( event.button.button == SDL_BUTTON_WHEELDOWN || event.button.button == SDL_BUTTON_RIGHT )
                        MD -- ;
                }
                if ( mousey >= 72 && mousey <= 112 && mousex >= 530 && mousex <= 588 ) {
                    if( event.button.button == SDL_BUTTON_WHEELUP || event.button.button == SDL_BUTTON_LEFT )
                        ME ++ ;

                    if( event.button.button == SDL_BUTTON_WHEELDOWN || event.button.button == SDL_BUTTON_RIGHT )
                        ME -- ;
                }
                if ( mousey >= 72 && mousey <= 112 && mousex >= 588 && mousex <= 646 ) {
                    if( event.button.button == SDL_BUTTON_WHEELUP || event.button.button == SDL_BUTTON_LEFT )
                        MF ++ ;

                    if( event.button.button == SDL_BUTTON_WHEELDOWN || event.button.button == SDL_BUTTON_RIGHT )
                        MF -- ;
                }
                if ( mousey >= 112 && mousey <= 152 && mousex >= 472 && mousex <= 530 ) {
                    if( event.button.button == SDL_BUTTON_WHEELUP || event.button.button == SDL_BUTTON_LEFT )
                        MG ++ ;

                    if( event.button.button == SDL_BUTTON_WHEELDOWN || event.button.button == SDL_BUTTON_RIGHT )
                        MG -- ;
                }
                if ( mousey >= 112 && mousey <= 152 && mousex >= 530 && mousex <= 588 ) {
                    if( event.button.button == SDL_BUTTON_WHEELUP || event.button.button == SDL_BUTTON_LEFT )
                        MH ++ ;

                    if( event.button.button == SDL_BUTTON_WHEELDOWN || event.button.button == SDL_BUTTON_RIGHT )
                        MH -- ;
                }
                if ( mousey >= 112 && mousey <= 152 && mousex >= 588 && mousex <= 646 ) {
                    if( event.button.button == SDL_BUTTON_WHEELUP || event.button.button == SDL_BUTTON_LEFT )
                        MI ++ ;

                    if( event.button.button == SDL_BUTTON_WHEELDOWN || event.button.button == SDL_BUTTON_RIGHT )
                        MI -- ;
                }


                if ( mousex >= 15 && mousex <= texteDestination.h + 25 ) {
                    if ( mousey >= 30+texteDestination.h+(texteDestination.h*2+35)*0  && mousey <= 35+texteDestination.h*2+(texteDestination.h*2+35)*0 ) {
                        if( event.button.button == SDL_BUTTON_WHEELUP || event.button.button == SDL_BUTTON_LEFT) {
                            if ( susantype == 17 )
                                susantype = 21 ;
                            else if ( susantype == 9 )
                                susantype = 17 ;
                        }

                        if( event.button.button == SDL_BUTTON_WHEELDOWN || event.button.button == SDL_BUTTON_RIGHT) {
                            if ( susantype == 17 )
                                susantype = 9 ;
                            else if ( susantype == 21 )
                                susantype = 17 ;
                        }

                    }
                    if ( mousey >= 30+texteDestination.h+(texteDestination.h*2+35)*1  && mousey <= 35+texteDestination.h*2+(texteDestination.h*2+35)*1 ) {
                        if( event.button.button == SDL_BUTTON_WHEELUP || event.button.button == SDL_BUTTON_LEFT)
                            if ( susangap < 128 )
                                susangap ++ ;

                        if( event.button.button == SDL_BUTTON_WHEELDOWN || event.button.button == SDL_BUTTON_RIGHT)
                            if ( susangap > 0 )
                                susangap -- ;
                    }
                    if ( mousey >= 30+texteDestination.h+(texteDestination.h*2+35)*2  && mousey <= 35+texteDestination.h*2+(texteDestination.h*2+35)*2 ) {
                        if( event.button.button == SDL_BUTTON_WHEELUP || event.button.button == SDL_BUTTON_LEFT)
                            if ( erosionwindow < 200 )
                                erosionwindow ++ ;

                        if( event.button.button == SDL_BUTTON_WHEELDOWN || event.button.button == SDL_BUTTON_RIGHT)
                            if ( erosionwindow > 2 )
                                erosionwindow -- ;
                    }
                    if ( mousey >= 30+texteDestination.h+(texteDestination.h*2+35)*3  && mousey <= 35+texteDestination.h*2+(texteDestination.h*2+35)*3 ) {
                        if( event.button.button == SDL_BUTTON_WHEELUP || event.button.button == SDL_BUTTON_LEFT)
                            if ( Rwindow < 500 )
                                Rwindow ++ ;

                        if( event.button.button == SDL_BUTTON_WHEELDOWN || event.button.button == SDL_BUTTON_RIGHT)
                            if ( Rwindow > 5 )
                                Rwindow -- ;
                    }
                    if ( mousey >= 30+texteDestination.h+(texteDestination.h*2+35)*4  && mousey <= 35+texteDestination.h*2+(texteDestination.h*2+35)*4 ) {
                        if( event.button.button == SDL_BUTTON_WHEELUP || event.button.button == SDL_BUTTON_LEFT)
                            if ( thresholdtheta < 90 )
                                thresholdtheta ++ ;

                        if( event.button.button == SDL_BUTTON_WHEELDOWN || event.button.button == SDL_BUTTON_RIGHT)
                            if ( thresholdtheta > 1 )
                                thresholdtheta -- ;
                    }
                    if ( mousey >= 30+texteDestination.h+(texteDestination.h*2+35)*5  && mousey <= 35+texteDestination.h*2+(texteDestination.h*2+35)*5 ) {
                        if( event.button.button == SDL_BUTTON_WHEELUP || event.button.button == SDL_BUTTON_LEFT)
                                checktheta = 1 ;

                        if( event.button.button == SDL_BUTTON_WHEELDOWN || event.button.button == SDL_BUTTON_RIGHT)
                                checktheta = 0 ;
                    }
                    if ( mousey >= 30+texteDestination.h+(texteDestination.h*2+35)*6  && mousey <= 35+texteDestination.h*2+(texteDestination.h*2+35)*6 ) {
                        if( event.button.button == SDL_BUTTON_WHEELUP || event.button.button == SDL_BUTTON_LEFT)
                            if ( gaptheta < 90 )
                                gaptheta ++ ;

                        if( event.button.button == SDL_BUTTON_WHEELDOWN || event.button.button == SDL_BUTTON_RIGHT)
                            if ( gaptheta > 1 )
                                gaptheta -- ;
                    }
                    if ( mousey >= 30+texteDestination.h+(texteDestination.h*2+35)*7  && mousey <= 35+texteDestination.h*2+(texteDestination.h*2+35)*7 ) {
                        if( event.button.button == SDL_BUTTON_WHEELUP || event.button.button == SDL_BUTTON_LEFT)
                                printnull = 1 ;

                        if( event.button.button == SDL_BUTTON_WHEELDOWN || event.button.button == SDL_BUTTON_RIGHT)
                                printnull = 0 ;
                    }
                } else if ( mousex >= 20 + texteDestination.h*2 && mousex <= 30 + texteDestination.h*3 ) {
                    if ( mousey >= 30+texteDestination.h+(texteDestination.h*2+35)*5  && mousey <= 35+texteDestination.h*2+(texteDestination.h*2+35)*5 ) {
                        if( event.button.button == SDL_BUTTON_WHEELUP || event.button.button == SDL_BUTTON_LEFT)
                                checkamp = 1 ;

                        if( event.button.button == SDL_BUTTON_WHEELDOWN || event.button.button == SDL_BUTTON_RIGHT)
                                checkamp = 0 ;
                    }
                    if ( mousey >= 30+texteDestination.h+(texteDestination.h*2+35)*6  && mousey <= 35+texteDestination.h*2+(texteDestination.h*2+35)*6 ) {
                        if( event.button.button == SDL_BUTTON_WHEELUP || event.button.button == SDL_BUTTON_LEFT)
                            if ( gapamp < 400 )
                                gapamp ++ ;

                        if( event.button.button == SDL_BUTTON_WHEELDOWN || event.button.button == SDL_BUTTON_RIGHT)
                            if ( gapamp > 0 )
                                gapamp -- ;
                    }
                    if ( mousey >= 30+texteDestination.h+(texteDestination.h*2+35)*7  && mousey <= 35+texteDestination.h*2+(texteDestination.h*2+35)*7 ) {
                        if( event.button.button == SDL_BUTTON_WHEELUP || event.button.button == SDL_BUTTON_LEFT)
                                printfinal =1 ;

                        if( event.button.button == SDL_BUTTON_WHEELDOWN || event.button.button == SDL_BUTTON_RIGHT)
                                printfinal = 0 ;
                    }
                }
            }


            if ( mousex >= SDLwidth/2-RESETwidth/2 && mousex <= SDLwidth/2+RESETwidth/2 &&  mousey >= SDLheight-RESETheight && mousey <= SDLheight ) {
                for( unsigned x = 0 ; x < RESETwidth ; x ++ ) {
                    for( unsigned y = 0 ; y < RESETheight ; y ++ ) {
                        //give the color value to the pixel of the screenbuffer
                        Uint32* bufp;
                        bufp = (Uint32 *)scr->pixels + ((y+SDLheight-RESETheight) * scr->pitch / 4) + (x+SDLwidth/2-RESETwidth/2) ;
                        float a = float(resetp[4*y*RESETwidth+4*x+3]) / 254  ;
                        unsigned r = background[4*(y+SDLheight-RESETheight)*SDLwidth+4*(x+SDLwidth/2-RESETwidth/2)+0] * ( 1 - a ) + resetp[4*y*RESETwidth+4*x+0] * a ;
                        unsigned g = background[4*(y+SDLheight-RESETheight)*SDLwidth+4*(x+SDLwidth/2-RESETwidth/2)+1] * ( 1 - a ) + resetp[4*y*RESETwidth+4*x+1] * a ;
                        unsigned b = background[4*(y+SDLheight-RESETheight)*SDLwidth+4*(x+SDLwidth/2-RESETwidth/2)+2] * ( 1 - a ) + resetp[4*y*RESETwidth+4*x+2] * a ;
                        *bufp = 65536 * r + 256 * g + b ;
                    }
                }
                if( event.type == SDL_MOUSEBUTTONUP ) {
                    if( event.button.button == SDL_BUTTON_LEFT ) {
                        //On remet les valeurs par defaut
                        susantype = A ;
                        susangap = B ;
                        erosionwindow = C ;
                        Rwindow = D ;
                        thresholdtheta = E ;
                        checktheta = F ;
                        gaptheta = G ;
                        checkamp = H ;
                        gapamp = I ;
                        printnull = J ;
                        printfinal = K ;

                        MA = -1 ;
                        MB = -1 ;
                        MC = -1 ;
                        MD = -1 ;
                        ME = 9 ;
                        MF = -1 ;
                        MG = -1 ;
                        MH = -1 ;
                        MI = -1 ;
                    }
                }
            } else {
                for( unsigned x = 0 ; x < RESETwidth ; x ++ ) {
                    for( unsigned y = 0 ; y < RESETheight ; y ++ ) {
                        //give the color value to the pixel of the screenbuffer
                        Uint32* bufp;
                        bufp = (Uint32 *)scr->pixels + ((y+SDLheight-RESETheight) * scr->pitch / 4) + (x+SDLwidth/2-RESETwidth/2) ;
                        float a = float(resetn[4*y*RESETwidth+4*x+3]) / 254  ;
                        unsigned r = background[4*(y+SDLheight-RESETheight)*SDLwidth+4*(x+SDLwidth/2-RESETwidth/2)+0] * ( 1 - a ) + resetn[4*y*RESETwidth+4*x+0] * a ;
                        unsigned g = background[4*(y+SDLheight-RESETheight)*SDLwidth+4*(x+SDLwidth/2-RESETwidth/2)+1] * ( 1 - a ) + resetn[4*y*RESETwidth+4*x+1] * a ;
                        unsigned b = background[4*(y+SDLheight-RESETheight)*SDLwidth+4*(x+SDLwidth/2-RESETwidth/2)+2] * ( 1 - a ) + resetn[4*y*RESETwidth+4*x+2] * a ;
                        *bufp = 65536 * r + 256 * g + b ;
                    }
                }
            }

            //On affiche les valeurs dans les cadres correspondants
            for( text = 0 ; text < 11 ; text++ ) {
                if ( text == 0 )
                    textbox = convertint( susantype ) ;
                else if ( text == 1 )
                    textbox = convertint( susangap ) ;
                else if ( text == 2 )
                    textbox = convertint( erosionwindow ) ;
                else if ( text == 3 )
                    textbox = convertint( Rwindow ) ;
                else if ( text == 4 )
                    textbox = convertint( thresholdtheta ) ;
                else if ( text == 5 )
                    textbox = convertint( checktheta ) ;
                else if ( text == 6 )
                    textbox = convertint( gaptheta ) ;
                else if ( text == 7 )
                    textbox = convertint( printnull ) ;

                else if ( text == 8 )
                    textbox = convertint( checkamp ) ;
                else if ( text == 9 )
                    textbox = convertint( gapamp ) ;
                else if ( text == 10 )
                    textbox = convertint( printfinal ) ;

                fontColor = {0, 0, 0} ;
                pFontSurface = TTF_RenderText_Solid(pFont,textbox,fontColor) ;
                texteDestination.w = pFontSurface->w ;
                texteDestination.h = pFontSurface->h ;
                texteDestination.x = 20 ;
                texteDestination.y = 34 + (texteDestination.h*2 + 35 ) * text + texteDestination.h ;

                 //Bouton - rep
                 if ( text < 8 ) {
                    for( unsigned x = texteDestination.x-5 ; x < texteDestination.x+texteDestination.h+15 ; x ++) {
                        for( unsigned y = texteDestination.y+texteDestination.h-23 ; y < texteDestination.y+texteDestination.h*2-17 ; y ++ ) {
                            //give the color value to the pixel of the screenbuffer
                            Uint32* bufp;
                            bufp = (Uint32 *)scr->pixels + (y * scr->pitch / 4) + x  ;
                            *bufp = 65536 * background[4*y*SDLwidth+4*x+0] + 256 * background[4*y*SDLwidth+4*x+1] + background[4*y*SDLwidth+4*x+2] ;
                        }
                    }
                    if ( text == 5 || text == 6 || text == 7 ) {
                        //Bouton - rep decale
                        for( unsigned x = texteDestination.x+texteDestination.h*2 ; x < texteDestination.x+texteDestination.h*3+20 ; x ++) {
                            for( unsigned y = texteDestination.y+texteDestination.h-23 ; y < texteDestination.y+texteDestination.h*2-17 ; y ++ ) {
                                //give the color value to the pixel of the screenbuffer
                                Uint32* bufp;
                                bufp = (Uint32 *)scr->pixels + (y * scr->pitch / 4) + x  ;
                                *bufp = 65536 * background[4*y*SDLwidth+4*x+0] + 256 * background[4*y*SDLwidth+4*x+1] + background[4*y*SDLwidth+4*x+2] ;
                            }
                        }
                    }
                }

                if ( text == 8 || text == 9 || text == 10 ) {
                    texteDestination.x += 5 + texteDestination.h * 2 ;
                    texteDestination.y = 34 + (texteDestination.h*2 + 35 ) * (text-3) + texteDestination.h ;
                }

                //Affiche le texte
                if ( SDL_BlitSurface(pFontSurface,NULL,scr,&texteDestination) != 0 )  {
                    fprintf(stderr,"Erreur de copie du texte sur l'ecran\n") ;
                    free( background ) ;
                    free( start ) ;
                    return -3 ;
                }
            }

            //On affiche les valeurs de la matrice
            for (int x = 0 ; x < 3 ; x++ ) {
                for (int y = 0 ; y < 3 ; y++ ) {
                    if ( x == 0 && y == 0 )
                        textbox = convertint( MA ) ;
                    else if ( x == 1 && y == 0 )
                        textbox = convertint( MB ) ;
                    else if ( x == 2 && y == 0 )
                        textbox = convertint( MC ) ;
                    else if ( x == 0 && y == 1 )
                        textbox = convertint( MD ) ;
                    else if ( x == 1 && y == 1 )
                        textbox = convertint( ME ) ;
                    else if ( x == 2 && y == 1 )
                        textbox = convertint( MF ) ;
                    else if ( x == 0 && y == 2 )
                        textbox = convertint( MG ) ;
                    else if ( x == 1 && y == 2 )
                        textbox = convertint( MH ) ;
                    else if ( x == 2 && y == 2 )
                        textbox = convertint( MI ) ;

                    fontColor = {0, 0, 0} ;
                    pFontSurface = TTF_RenderText_Solid(pFont,textbox,fontColor) ;
                    texteDestination.w = pFontSurface->w ;
                    texteDestination.h = pFontSurface->h ;
                    if ( x == 0)
                        texteDestination.x = SDLwidth/2 - texteDestination.w/2 - 60;
                    else if ( x == 1 )
                        texteDestination.x = SDLwidth/2 - texteDestination.w/2 ;
                    else if ( x == 2 )
                        texteDestination.x = SDLwidth/2 - texteDestination.w/2 + 60 ;
                    texteDestination.y = 45 + y*2*texteDestination.h;

                     //Bouton - rep
                    for( unsigned x = texteDestination.x-5 ; x < texteDestination.x+texteDestination.h+15 ; x ++) {
                        for( unsigned y = texteDestination.y+texteDestination.h-23 ; y < texteDestination.y+texteDestination.h*2-17 ; y ++ ) {
                            //give the color value to the pixel of the screenbuffer
                            Uint32* bufp;
                            bufp = (Uint32 *)scr->pixels + (y * scr->pitch / 4) + x  ;
                            *bufp = 65536 * background[4*y*SDLwidth+4*x+0] + 256 * background[4*y*SDLwidth+4*x+1] + background[4*y*SDLwidth+4*x+2] ;
                        }
                    }

                    //Affiche le texte
                    if ( SDL_BlitSurface(pFontSurface,NULL,scr,&texteDestination) != 0 )  {
                        fprintf(stderr,"Erreur de copie du texte sur l'ecran\n") ;
                        free( background ) ;
                        free( start ) ;
                        return -3 ;
                    }
                }
            }
        }


        //Traitement d'image debut

        //center_color( image, final, CAMwidth, CAMheight, hue, gap_h, sat, gap_s, val, gap_v ) ;
        //dog2( image, final, CAMwidth, CAMheight) ;
        //dog3( image, final, CAMwidth, CAMheight, MA,  MB,  MC, MD,  ME,  MF, MG,  MH,  MI ) ;


/*
        for(unsigned int x = 2 ; x < CAMwidth-2 ; x++) {
            for(unsigned int y = 2 ; y < CAMheight-2 ; y++) {
                if ( image[x][y][0] < 80 && image[x][y][1] < 80 && image[x][y][2] < 80 ) {
                    final[x][y][0] = 255 ;
                    final[x][y][1] = image[x][y][1] ;
                    final[x][y][2] = image[x][y][2] ;
                } else {
                    final[x][y][0] = image[x][y][0] ;
                    final[x][y][1] = image[x][y][1] ;
                    final[x][y][2] = image[x][y][2] ;
                }
            }
        }

        int p1, p2 ;
        int compteur ;
        int posx, posy ;
        unsigned int nbpoint = 0 ;
        int R = 150 ;

        for( unsigned int x = R ; x < CAMwidth-R ; x++) {
            for( unsigned int y = R ; y < CAMheight-R ; y++) {
                if( final[x][y][0] == 255 ) {
                    if(x-R >= 0 && x+R < CAMwidth && y-R >= 0 && y+R < CAMheight ) {
                        p1 = p2 = 0 ;

                        for( int i = -R ; i <= R ; i++ ) {
                            for( int j = -R ; j <= R ; j++ ) {
                                if( abs(i) == R || abs(j) == R ){
                                    if ( final[x + i][y + j][0] != 255 ) {
                                        p1++ ;
                                    }
                                    p2 ++ ;
                                }
                            }
                        }

                        printf ("p1 = %d - p2 = %d\n", p1, p2) ;

                        if ( p1 == p2 && p1 > 0 ) {
                            compteur = posx = posy = 0 ;

                            for( int i = -R ; i <= R ; i++) {
                                for( int j = -R ; j <= R ; j++) {
                                    if ( final[x + i][y + j][0] == 255 ) {
                                        compteur++  ;
                                        posx += (x+i) ;
                                        posy += (y+j) ;

                                        final[x + i][y + j][0] = final[x + i][y + j][2] = 0 ;
                                    }
                                }
                            }

                            if ( compteur > 0 ) {
                                posx /= compteur ;
                                posy /= compteur ;

                                final[posx][posy][0] = final[posx][posy][1] = final[posx][posy][2] = 0 ;
                                nbpoint++ ;
                                R = erosionwindow+1 ;
                            }
                        }
                    }
                }
            }
        }
        std::cout << "Il reste : " << nbpoint << " pixels." << std::endl ;
*/



        for( unsigned int x = 2 ; x < CAMwidth-2 ; x++)
            for( unsigned int y = 2 ; y < CAMheight-2 ; y++)
                final[x][y][0] = final[x][y][1] = final[x][y][2] = 0 ;

        if ( susantype == 9 )
            susan9( image, final, CAMwidth, CAMheight, susangap) ;
        if ( susantype == 17 )
            susan17( image, final, CAMwidth, CAMheight, susangap) ;
        if ( susantype == 21 )
            susan21( image, final, CAMwidth, CAMheight, susangap) ;

        unsigned int nbpoint = 0 ;
        erosion( final, CAMwidth, CAMheight, nbpoint, erosionwindow) ;


        for( unsigned int x = 2 ; x < CAMwidth-2 ; x++) {
            for( unsigned int y = 2 ; y < CAMheight-2 ; y++) {
                if ( final[x][y][2] > 0 ) {
                    drawpoint( final, CAMwidth, CAMheight, x, y, 2, 5, 1) ;
                    final[x][y][1] = 255 ;
                }
                if ( final[x][y][0] == 0 || printnull == 0 )
                    final[x][y][0] = (capture.mTargetBuf[y*CAMwidth+x] >> 16) & 0xFF ; //On divise par deux pour avoir une image plus sombre et voir les points

                final[x][y][2] = (capture.mTargetBuf[y*CAMwidth+x]) & 0xFF ;

                if ( final[x][y][1] == 0 )
                    final[x][y][1] = (capture.mTargetBuf[y*CAMwidth+x] >> 8) & 0xFF ;
            }
        }

        //Traitement d'image fin


        doCapture(1);
        while (isCaptureDone(1) == 0) {
        }

        int xr = 0 ;
        int yr = 0 ;
        //plot the pixels of the PNG file
        for( unsigned x = 0 ; x < CAMwidth ; x ++ ) {
            for( unsigned y = 0 ; y < CAMheight ; y ++ ) {
                for ( unsigned l = 0 ; l < zoom ; l++ ) {
                    for ( unsigned k = 0 ; k < zoom ; k++ ) {
                        if ( xr + k >= 0 && xr + k <= CAMwidth*zoom ) {
                            if ( yr + l >= 0 && yr + l <= CAMheight*zoom ) {
                                //give the color value to the pixel of the screenbuffer
                                Uint32* bufp ;
                                bufp = (Uint32 *)scr->pixels + ((yr+k+SDLheight/2-(CAMheight/2)*zoom) * scr->pitch / 4) + (xr+l+SDLwidth/2-(CAMwidth/2)*zoom) ;
                                *bufp = 65536 * final[x][y][0] + 256 * final[x][y][1] + final[x][y][2] ;
                                //*bufp = 65536 * image[x][y][0] + 256 * image[x][y][1] + image[x][y][2] ;
                            }
                        }
                    }
                }
                if ( x <= CAMwidth && y <= CAMheight) {
                    image[x][y][0] = (capture.mTargetBuf[y*CAMwidth+x] >> 16) & 0xFF ;
                    image[x][y][1] = (capture.mTargetBuf[y*CAMwidth+x] >> 8) & 0xFF ;
                    image[x][y][2] = (capture.mTargetBuf[y*CAMwidth+x]) & 0xFF ;
                }
                //On passe au pixels reel suivants
                yr += zoom ;
            }
            yr = 0 ;
            xr += zoom ;
        }

        SDL_UpdateRect(scr, 0, 0, 0, 0); //redraw screen
        SDL_Delay(5); //pause 5 ms so it consumes less processing power
    }

	//Efface les allocations en memoire des images
    freeimg( image, CAMwidth, CAMheight) ;
    freeimg( final, CAMwidth, CAMheight) ;

    //Quite et Efface SDL
    SDL_FreeSurface(scr) ;
    TTF_CloseFont(pFont) ;
    SDL_Quit();
    cleanup() ;

    //Efface les images pour affichage graphique de la memoire
    free( background ) ;
    free( logo ) ;
    free( circle ) ;
    free( matrice ) ;
    free( button ) ;
    free( start ) ;
    free( resetn ) ;
    free( resetp ) ;
	std::cout << "Cache vide !" << std::endl << std::endl ;

    if ( done == 5 ) {
        A = susantype ;
        B = susangap ;
        C = erosionwindow ;
        D = Rwindow ;
        E = thresholdtheta ;
        F = checktheta ;
        G = gaptheta ;
        H = checkamp ;
        I = gapamp ;
        J = printnull ;
        K = printfinal ;

        return 0 ;
    }

    return done ;
}

int showSDL(const std::string& filename1, unsigned int ***img1, const std::string& filename2, unsigned int ***img2, int nbimg, unsigned width, unsigned height) {
    //init SDL
    if(SDL_Init(SDL_INIT_VIDEO) < 0) {
        std::cerr << "error, SDL video init failed" << std::endl ;
        return 0 ;
    }

    SDL_Event event ;
    int done = 0 ;
    int zoom = 1 ;
    int xr, yr, xp, yp ;
    int gapx = 0 ;
    int gapy = 0 ;
    int gapx2 = 0 ;
    int gapy2 = 0 ;
    int mol = 0 ;
    int molxin = 0 ;
    int molyin = 0 ;
    int molx = 0 ;
    int moly = 0 ;

    //pause until you press escape and meanwhile redraw screen
    while(done != -10) {
        SDL_Surface* scr = SDL_SetVideoMode(width, height, 32, SDL_HWSURFACE) ;
        if(!scr) {
            std::cerr << "error, no SDL screen" << std::endl ;
            return 0 ;
        }

        //std::cerr << "zoom = " <<  zoom << std::endl ;
        if (done == 0)
            SDL_WM_SetCaption(filename1.c_str(), NULL) ; //set window filename
        else if (done == 1)
            SDL_WM_SetCaption(filename2.c_str(), NULL) ; //set window filename

        //plot the pixels of the PNG file
        xr = (width - width/zoom)/2 ; //position X reel dans l'image
        yr = (height - height/zoom)/2 ; //position Y reel dans l'image

        for( unsigned x = 0 ; x < width ; x += zoom ) {
            for( unsigned y = 0 ; y < height ; y += zoom ) {
                //give the color value to the pixel of the screenbuffer
                //On reste dans le cadre du zoom en X
                if ( xr > width - (width - width/zoom)/2 )
                    xr = width - (width - width/zoom)/2 ;

                //On reste dans le cadre du zoom en Y
                if ( yr > height - (height - height/zoom)/2 )
                    yr = height - (height - height/zoom)/2 ;

                xp = xr+(zoom == 1 ? 0 : gapx) ;
                yp = yr+(zoom == 1 ? 0 : gapy) ;

                if ( xp < 0 )
                    xp = 0 ;

                if ( xp >= width )
                    xp = width-1 ;

                if ( yp < 0 )
                    yp = 0 ;

                if ( yp >= height )
                    yp = height-1 ;


                for ( unsigned l = 0 ; l < zoom ; l++ ) {
                    for ( unsigned k = 0 ; k < zoom ; k++ ) {
                        //give the color value to the pixel of the screenbuffer
                        Uint32* bufp ;
                        bufp = (Uint32 *)scr->pixels + ((y+k) * scr->pitch / 4) + (x+l) ;

                        if (done == 0)
                            *bufp = 65536 * img1[xp][yp][0] + 256 * img1[xp][yp][1] + img1[xp][yp][2] ;
                        else if (done == 1)
                            *bufp = 65536 * img2[xp][yp][0] + 256 * img2[xp][yp][1] + img2[xp][yp][2] ;
                    }
                }
                //On passe au pixels reel suivants
                yr ++ ;
            }
            //On passe au pixels reel suivants
            yr = (height - height/zoom)/2 ; //position Y reel dans l'image
            xr ++ ;
        }

        while(SDL_PollEvent(&event)) {
            if(event.type == SDL_QUIT)
                done = -10 ;
            else if(SDL_GetKeyState(NULL)[SDLK_ESCAPE])
                done = -10 ;
            else if(SDL_GetKeyState(NULL)[SDLK_RETURN] || SDL_GetKeyState(NULL)[SDLK_KP_ENTER])
                done = -10 ;
            else if(SDL_GetKeyState(NULL)[SDLK_SPACE]) { //On passe à l'image suivante
                done ++ ;
                if ( done > nbimg )
                    done = 0 ;
            }

            if( event.type == SDL_MOUSEBUTTONUP ) {
                //Si on clique sur la molette
                if( event.button.button == SDL_BUTTON_MIDDLE ) { //On affiche la deuxieme image tant que l'on  appuie sur la molette
                    done ++ ;
                    if ( done > nbimg )
                        done = 0 ;
                }

                //Si on bouge la molette vers le haut
                if( event.button.button == SDL_BUTTON_WHEELUP ) {
                    if ( zoom != 8 )
                        zoom = zoom * 2 ;
                }

                //Si on bouge la molette vers le bas
                if( event.button.button == SDL_BUTTON_WHEELDOWN ) {
                    if ( zoom != 1 )
                        zoom = zoom / 2 ;

                    if ( zoom == 1 )
                        gapx = gapy = gapx2 = gapy2 = 0 ;
                }

                //Si on appuis sur  la molette
                if( event.button.button == SDL_BUTTON_LEFT  && zoom != 1 ) {
                    //Get the mouse offsets
                    molxin = event.button.x ;
                    molyin = event.button.y ;

                    if (mol == 0)
                        mol = 1 ;
                    else {
                        gapx2 = gapx ;
                        gapy2 = gapy ;
                        mol = 0 ;
                    }
                }
            }

            if( event.type == SDL_MOUSEMOTION && mol == 1) {
                //Get the mouse offsets
                molx = event.motion.x ;
                moly = event.motion.y ;
                gapx = (molxin - molx) / zoom + gapx2 ;
                gapy = (molyin - moly) / zoom + gapy2 ;
            }

        }

        SDL_UpdateRect(scr, 0, 0, 0, 0) ; //redraw screen
        SDL_Delay(5) ; //pause 5 ms so it consumes less processing power
    }

    //std::cerr << "done = " << done << std::endl ;
    SDL_Quit() ;
    return done == -10 ? 0 : done ;
}

int showCAM() {
	// Initialize ESCAPI
	int devices = setupESCAPI();

	if (devices == 0) {
		printf("ESCAPI initialization failure or no devices found.\n");
		return 1 ;
	}

	for (int i = 0; i < devices; i++) {
		char temp[256];
		getCaptureDeviceName(i, temp, 256);
		printf("Device %d: \"%s\"\n", i, temp);
	}

	struct SimpleCapParams capture;
	//300/225/3
	//400/300/2
	//440/330/2
	//460/345/2
	//560/420/2
	//640/480/2
	//800/600/1
	capture.mWidth = 640 ;
	capture.mHeight = 480 ;
	int SDLwidth = capture.mWidth ;
	int SDLheight = capture.mHeight ;
	capture.mTargetBuf = new int[SDLwidth * SDLheight];

	//Initialize capture - only one capture may be active per device,
    //but several devices may be captured at the same time.
    //0 is the first device.
	if (initCapture(0, &capture) == 0) {
		printf("Capture failed - device may already be in use.\n");
		return 2 ;
	}

    //init SDL
    if(SDL_Init(SDL_INIT_VIDEO) < 0) {
        std::cout << "error, SDL video init failed" << std::endl ;
        cleanup() ;
        return -1 ;
    }

    SDL_Surface* scr = SDL_SetVideoMode(SDLwidth , SDLheight , 32, SDL_HWSURFACE) ;
    if(!scr) {
        std::cout << "error, no SDL screen" << std::endl ;
        cleanup() ;
        return -2;
    }

    //On defini le titre de la fenetre
    SDL_WM_SetCaption("Vision System", NULL) ;

    unsigned int r, g, b ;
    unsigned int x, y ;
    int compteur, posx, posy, maxy ;
    int color = 240 ;
    int gap = 30 ;
    int done = 1 ;

    SDL_Flip(scr) ;
    SDL_Event event ;

    //pause until you press escape and meanwhile redraw screen
    while(done < 5) {
        while(SDL_PollEvent(&event)) {
            if(event.type == SDL_QUIT)
                done = 15 ;
            else if(SDL_GetKeyState(NULL)[SDLK_ESCAPE])
                done = 10 ;
            else if(SDL_GetKeyState(NULL)[SDLK_RETURN] || SDL_GetKeyState(NULL)[SDLK_KP_ENTER]) //Touche entrer pour valider les modifications
                done = 5 ;
        }
        SDL_UpdateRect(scr, 0, 0, 0, 0); //redraw screen
        SDL_Delay(5); //pause 5 ms so it consumes less processing power

        //request a capture
        doCapture(0);
        while (isCaptureDone(0) == 0) {
        }

        //Compte le nombre pixels ayant une intensité supperieure a 240
        compteur = posx = posy = maxy = 0 ;
        for( x = 0 ; x < SDLwidth ; x++) {
            for(y = 0 ; y < SDLheight ; y++) {
                if ( ((capture.mTargetBuf[y*SDLwidth+x] >> 16) & 0xFF) > color && ((capture.mTargetBuf[y*SDLwidth+x] >> 8) & 0xFF) > color && ((capture.mTargetBuf[y*SDLwidth+x]) & 0xFF) > color ) {
                    posx += x ;
                    posy += y ;
                    if ( y > maxy )
                        maxy = y ;
                    compteur ++ ;
                }
            }
        }

        //Calcule le centre de gravite de la masse de pixel correspondants
        if ( compteur > 50 ) {
            posx = posx / compteur ;
            posy = posy / compteur ;

            if ( posx > SDLwidth || posx < 0 )
                posx = SDLwidth ;
            if ( posy > SDLheight || posy < 0 )
                posy = SDLheight ;

            //dessine un cercle vide
            //Algorithme de trace de cercle d'Andres
            int R = maxy - posy ;
            int i = 0 ;
            int j = R ;
            int d = R-1 ;

            while ( j >= i ) {
                if( posx + i >= 0 && posx + i < SDLwidth && posy + j >= 0 && posy + j < SDLheight ) {
                    capture.mTargetBuf[(posy+j)*SDLwidth+(posx+i)] = 0 ;
                }

                if( posx + j >= 0 && posx + j < SDLwidth && posy + i >= 0 && posy + i < SDLheight ) {
                    capture.mTargetBuf[(posy+i)*SDLwidth+(posx+j)] = 0 ;
                }

                if( posx - i >= 0 && posx - i < SDLwidth && posy + j >= 0 && posy + j < SDLheight ) {
                    capture.mTargetBuf[(posy+j)*SDLwidth+(posx-i)] = 0 ;
                }

                if( posx - j >= 0 && posx - j < SDLwidth && posy + i >= 0 && posy + i < SDLheight ) {
                    capture.mTargetBuf[(posy+i)*SDLwidth+(posx-j)] = 0 ;
                }

                if( posx + i >= 0 && posx + i < SDLwidth && posy - j >= 0 && posy - j < SDLheight ) {
                    capture.mTargetBuf[(posy-j)*SDLwidth+(posx+i)] = 0 ;
                }

                if( posx + j >= 0 && posx + j < SDLwidth && posy - i >= 0 && posy - i < SDLheight ) {
                    capture.mTargetBuf[(posy-i)*SDLwidth+(posx+j)] = 0 ;
                }

                if( posx - i >= 0 && posx - i < SDLwidth && posy - j >= 0 && posy - j < SDLheight ) {
                    capture.mTargetBuf[(posy-j)*SDLwidth+(posx-i)] = 0 ;
                }

                if( posx - j >= 0 && posx - j < SDLwidth && posy - i >= 0 && posy - i < SDLheight ) {
                    capture.mTargetBuf[(posy-i)*SDLwidth+(posx-j)] = ((0)<<16)+ ((0)<<8)+ ((0)) | 0xff000000;
                }

                if ( d >= 2*i ) {
                    d = d - 2*i - 1 ;
                    i = i + 1 ;
                } else if ( d < 2*(R-j) ) {
                    d = d + 2*j - 1 ;
                    j = j -1 ;
                } else {
                    d = d + 2*(j - i - 1) ;
                    j = j - 1 ;
                    i = i + 1 ;
                }
            }

            printf ("r = %d / g = %d / b = %d\n", ((capture.mTargetBuf[posy*SDLwidth+posx] >> 16) & 0xFF), ((capture.mTargetBuf[posy*SDLwidth+posx] >> 8) & 0xFF), ((capture.mTargetBuf[posy*SDLwidth+posx]) & 0xFF)) ;
            printf ("   posx = %d - posy = %d - compteur = %d\n", SDLwidth - posx, posy, compteur) ;
            printf ("   maxy = %d - dif = %d \n", maxy, maxy - posy) ;
        } else {
            compteur = posx = posy = 0 ;//Sinon c'est qu'il n'y a pas de masse et donc pas de centre de gravite
        }

        //On affiche l'image finale
        for( x = 0 ; x < SDLwidth ; x ++ ) {
            for( y = 0 ; y < SDLheight ; y ++ ) {
                //give the color value to the pixel of the screenbuffer
                Uint32* bufp ;
                bufp = (Uint32 *)scr->pixels + (y * scr->pitch / 4) + x ;
                if ( x == posx && y == posy || x == posx-1 && y == posy || x == posx+1 && y == posy || x == posx && y == posy-1 || x == posx && y == posy+1 || x == posx-2 && y == posy || x == posx+2 && y == posy || x == posx && y == posy-2 || x == posx && y == posy+2 ) {
                    *bufp = 0 ; //Affiche le centre de gravite
                } else {
                    *bufp = 65536 * ((capture.mTargetBuf[y*SDLwidth+x] >> 16) & 0xFF) + 256 * ((capture.mTargetBuf[y*SDLwidth+x] >> 8) & 0xFF) + ((capture.mTargetBuf[y*SDLwidth+x]) & 0xFF) ;
                }
            }
        }

    }

    //On vide la memoire avant de terminer
	deinitCapture(0);
    SDL_Quit();
    cleanup() ;
	std::cout << "Cache vide !" << std::endl << std::endl ;

    return done ;
}

int main(int argc, char *argv[]) {
    //On veut avoir le std dans le cmd donc :
    freopen( "CON", "w", stdout );
    freopen( "CON", "w", stderr );

	//check if user gave a filename
	if ( argc != 5 ) {
        printf("Er\n") ;
		std::cerr << "usage :\n\t\t ./main inpufile1 inpufile2 outpufile outpufile2" << std::endl ;
		return -10 ;
	}

	std::cout << "\n\n---- VISION SYSTEM ----" << std::endl ;
	std::cout << "       " << VERSIONY << "." << VERSIONM << "." << VERSIOND << std::endl << std::endl ;


    FILE *fichier = NULL ;

    fichier = fopen("text.txt", "w" ) ;

    printf("Salut a vous tous 1\n") ;
    fprintf(fichier, "Salut a vous tous 2\n") ;

    fclose( fichier ) ;

    //showCAM() ;


    int susantype = 21 ;
    int susangap = 30 ;
    int erosionwindow = 10 ;
    int Rwindow = 10 ;
    int thresholdtheta = 1 ;
    int checktheta = 1 ;
    int gaptheta = 5 ;
    int checkamp = 1 ;
    int gapamp = 40 ;
    int printnull = 1 ;
    int printfinal = 1 ;
    int error ;
    if ( (error =  showscreen2( susantype, susangap, erosionwindow, Rwindow, thresholdtheta, checktheta, gaptheta, checkamp, gapamp, printnull, printfinal )) ) {
		std::cerr << "SDL showscreen error : " << error << std::endl ;
		return error ;
	}

/*
    unsigned int width, height ;
    unsigned char* image1;
    unsigned char* image2;

	if( (error = lodepng_decode32_file(&image1, &width, &height, argv[1])) ) {
		std::cerr << "decoder error " << error << ": " << lodepng_error_text(error) << std::endl ;
		return 2 ;
	}
	std::cout << "L'image \"" << argv[1] << "\" a ete chargee ..." << std::endl ;

	if( (error = lodepng_decode32_file(&image2, &width, &height, argv[2])) ) {
		std::cerr << "decoder error " << error << ": " << lodepng_error_text(error) << std::endl ;
		return 2 ;
	}
	std::cout << "L'image \"" << argv[2] << "\" a ete chargee ..." << std::endl ;

    unsigned int ***img1 = loadimg( image1, width, height ) ;
    unsigned int ***img2 = loadimg( image2, width, height ) ;
	unsigned int ***final1 = newimg( width, height ) ;
	unsigned int ***final2 = newimg( width, height ) ;

	//--susan detector debut--
    susan21( img1, final1, width, height, susangap) ;
    susan21( img2, final2, width, height, susangap) ;
	//--susan detector fin--

	//--erosion debut--
	unsigned int nbp1 = 0 ;
	unsigned int nbp2 = 0 ;
	unsigned int nbpoint1 = 0 ;
	unsigned int nbpoint2 = 0 ;
    erosion( final1, width, height, nbpoint1, erosionwindow) ;
	if( nbpoint1 == 0 ) {
		std::cerr << "Etosion error, no point found : change window treshold." << std::endl ;
        clearall ( img1, final1, image1, width, height) ;
        clearall ( img2, final2, image2, width, height) ;
		return 4 ;
	}
    erosion( final2, width, height, nbpoint2, 10) ;
	if( nbpoint2 == 0 ) {
		std::cerr << "Susan error no point found : change detector treshold." << std::endl ;
        clearall ( img1, final1, image1, width, height) ;
        clearall ( img2, final2, image2, width, height) ;
		return 4 ;
	}
	//--erosion fin--


	//--supper-position avec l'image originale debut--
	float **feature1 = newfeature( nbpoint1 ) ;
	float **feature2 = newfeature( nbpoint2 ) ;
	float carre[9] ;

    for( unsigned int x = 2 ; x < width-2 ; x++) {
        for( unsigned int y = 2 ; y < height-2 ; y++) {
            if ( final1[x][y][2] > 0 ) {
                carre[0] = img1[x][y][0] ;
                carre[1] = img1[x+1][y][0] ;
                carre[2] = img1[x+1][y-1][0] ;
                carre[3] = img1[x][y-1][0] ;
                carre[4] = img1[x-1][y-1][0] ;
                carre[5] = img1[x-1][y][0] ;
                carre[6] = img1[x-1][y+1][0] ;
                carre[7] = img1[x][y+1][0] ;
                carre[8] = img1[x+1][y+1][0] ;

                //Calcul des carracteristique du point
                feature1[nbp1][0] = carre[3]*carre[7] - carre[1]*carre[5] ; //determiant +
                feature1[nbp1][1] = carre[4]*carre[8] - carre[2]*carre[6] ; //determiant x
                feature1[nbp1][2] = 4*(carre[0]*carre[8] - carre[1]*carre[7]) - 3*(carre[5]*carre[8] - carre[1]*carre[6]) + 2*(carre[5]*carre[7] - carre[0]*carre[6]) ; //determiant 3x3
                if( feature1[nbp1][2] == 0 )
                    feature1[nbp1][2] = 0.0001 ;
                feature1[nbp1][3] = (carre[1] - carre[5])*(carre[1] - carre[5]) + (carre[7] - carre[3])*(carre[7] - carre[3]) ; //Amplitute (pas de racine)
                feature1[nbp1][4] = (carre[0] + carre[1] + carre[2] + carre[3] + carre[4] + carre[5] + carre[6] + carre[7] + carre[8]) / 9 ; //Moyenne 3x3
                feature1[nbp1][5] = -10 ; //On initialise les valeurs pour eviter les erreurs
                feature1[nbp1][6] = -10 ; //On initialise les valeurs pour eviter les erreurs

                feature1[nbp1][7] = x ; //Coordonnee X du point actuel
                feature1[nbp1][8] = y ; //Coordonnee Y du point actuel

                if ( printnull ) {
                    drawpoint( final1, width, height, x, y, 2, 5, 0) ;
                    final1[x][y][0] = 255 ;
                }
                nbp1 ++ ;
            }

            if ( final1[x][y][0] == 0 || printnull == 0 )
                final1[x][y][0] = img1[x][y][0]/2 ; //On divise par deux pour avoir une image plus sombre et voir les points

            final1[x][y][2] = img1[x][y][0]/2 ;

            if ( final1[x][y][1] == 0 )
                final1[x][y][1] = img1[x][y][0]/2 ;



            if ( final2[x][y][2] > 0 ) {
                carre[0] = img2[x][y][0] ;
                carre[1] = img2[x+1][y][0] ;
                carre[2] = img2[x+1][y-1][0] ;
                carre[3] = img2[x][y-1][0] ;
                carre[4] = img2[x-1][y-1][0] ;
                carre[5] = img2[x-1][y][0] ;
                carre[6] = img2[x-1][y+1][0] ;
                carre[7] = img2[x][y+1][0] ;
                carre[8] = img2[x+1][y+1][0] ;

                //Calcul des carracteristique du point
                feature2[nbp2][0] = carre[3]*carre[7] - carre[1]*carre[5] ; //determiant +
                feature2[nbp2][1] = carre[4]*carre[8] - carre[2]*carre[6] ; //determiant x
                feature2[nbp2][2] = 4*(carre[0]*carre[8] - carre[1]*carre[7]) - 3*(carre[5]*carre[8] - carre[1]*carre[6]) + 2*(carre[5]*carre[7] - carre[0]*carre[6]) ; //determiant 3x3
                if( feature2[nbp2][2] == 0 )
                    feature2[nbp2][2] = 0.0001 ;
                feature2[nbp2][3] = (carre[1] - carre[5])*(carre[1] - carre[5]) + (carre[7] - carre[3])*(carre[7] - carre[3]) ; //Amplitute (pas de racine)
                feature2[nbp2][4] = (carre[0] + carre[1] + carre[2] + carre[3] + carre[4] + carre[5] + carre[6] + carre[7] + carre[8]) / 9 ; //Moyenne 3x3
                feature2[nbp2][5] = -10 ; //On initialise les valeurs pour eviter les erreurs
                feature2[nbp2][6] = -10 ; //On initialise les valeurs pour eviter les erreurs

                feature2[nbp2][7] = x ; //Coordonnee X du point actuel
                feature2[nbp2][8] = y ; //Coordonnee Y du point actuel

                if ( printnull ) {
                    drawpoint( final2, width, height, x, y, 2, 5, 0) ;
                    final2[x][y][0] = 255 ;
                }
                nbp2 ++ ;
            }

            if ( final2[x][y][0] == 0 || printnull == 0 )
                final2[x][y][0] = img2[x][y][0]/2 ; //On divise par deux pour avoir une image plus sombre et voir les points
            final2[x][y][2] = img2[x][y][0]/2 ;

            if ( final2[x][y][1] == 0 )
                final2[x][y][1] = img2[x][y][0]/2 ;
        }
    }

    float total ;
    float maxx ;
    int pos ;
    int x1, x2, y1, y2 ;
    int hystogram[360 / thresholdtheta][2] ;

    for( unsigned int i = 0 ; i < 360 / thresholdtheta ; i++) {
        hystogram[i][0] = 0 ;
        hystogram[i][1] = 0 ;
    }

    for( unsigned int i = 0 ; i < nbpoint1 ; i++) {
        maxx = 10000000 ;
        pos = 10000000 ;
        x1 = feature1[i][7] ;
        y1 = feature1[i][8] ;

        for( unsigned int j = 0 ; j < nbpoint2 ; j++) {
            //Fenetre permettant de limiter la zone de recherche
            if(feature2[j][7] >= feature1[i][7] -Rwindow && feature2[j][7] <= feature1[i][7] +Rwindow && feature2[j][8] >= feature1[i][8] -Rwindow && feature2[j][8] <= feature1[i][8] +Rwindow){
                total = abs(((feature2[j][0] * 100 ) / feature1[i][0]) - 100 ) ;
                total += abs(((feature2[j][1] * 100 ) / feature1[i][1]) - 100 ) ;
                total += abs(((feature2[j][2] * 100 ) / feature1[i][2]) - 100 ) ;
                total += abs(((feature2[j][3] * 100 ) / feature1[i][3]) - 100 ) ;
                total += abs(((feature2[j][4] * 100 ) / feature1[i][4]) - 100 ) ;

                x2 = feature2[j][7] ;
                y2 = feature2[j][8] ;

                //On fait aussi un "Sum of Squarred Differences" pour plus de precision
                for (int k = -1 ; k <= 1 ; k++ ) {
                    for (int l = -1 ; l <= 1 ; l++ ) {
                        total += (img1[x1][y1][0] - img2[x2][y2][0]) * (img1[x1][y1][0] - img2[x2][y2][0]) ;
                    }
                }

                if ( total < maxx ) {
                    maxx = total ;
                    pos = j ;
                }
            }
        }

        if ( pos != 10000000 ) {
            x2 = feature2[pos][7] ;
            y2 = feature2[pos][8] ;

            //On enregistre le numero du point correspondant au point i
            feature1[i][5] = pos ;

            //On calcule l'amplitude du "glissement"
            total = sqrt((x2 - x1)*(x2 - x1) + (y2 - y1)*(y2 - y1)) ;
            feature1[i][9] = total ;

            //On calcule la direction dans laquelle on "glisse"
            if ( total != 0 ) {
                feature1[i][6] = acos((x2 - x1) / total ) * (180/M_PI) ;
                if ( y2 > y1 )
                    feature1[i][6] = 360 - feature1[i][6] ; //Correction pour le 3eme et 4eme cadran
            } else {
                //Si jamais l'amplitude est nulle c'est qu'il n'y a donc aucun deplacement (permet d'eviter les divisions par 0)
                feature1[i][6] = 0 ;
            }

            //On incremente l'hystogramme a 10 degres pres
            hystogram[(int) feature1[i][6] / thresholdtheta][0] ++ ; //angle
            hystogram[(int) feature1[i][6] / thresholdtheta][1] += total ; //amplitude


            //std::cout << "IMG1 point N'" << i << "\tx = " << x1 << "\ty = " << y1 << std::endl ;
            //if (pos != 10000000 )
            //    std::cout << "\tIMG2 point N'" << feature1[i][5] << "\tx = " << x1 << "\ty = " << y1 << "\tTheta = " << feature1[i][6] << "\tAmp = " << total << std::endl ;

        }
    }

    //Une fois l'hystogramme complet et les meilleurs points possible correspondants,
    //On regarde quel est l'angle/palier pour le quel on trouve le plus de points correspondants
    //Et donc on connait l'angle de la translation de l'image.
    //Au passage on calcule aussi l'intensite du glissement
    int theta = 0 ;
    float amp ;
    for( int i = 0 ; i < 360 / thresholdtheta ; i++) {
        if ( hystogram[i][0] > hystogram[theta][0] )
            theta = i ;
    }

    //On verifie que l'amplitude en theta soit bien plus grand que 0 sinon erreur !
    if ( hystogram[theta][1] == 0 ) {
		std::cerr << "ERROR :\nLa fenetre de recherche de correspondant est soit trop petite soir trop grande !" << std::endl ;
        clearall ( img1, final1, image1, width, height) ;
        clearall ( img2, final2, image2, width, height) ;
        return 10 ;
    }

    amp = hystogram[theta][1] / hystogram[theta][0] ; //Calcule l'amplitude moyenne correspondant a l'angle theta le plus important

    std::cout << "\nGlissement de l'image = " << theta*thresholdtheta << " degres"<< std::endl ;
    std::cout << "Amplitude du glissement = " << amp << " pixel(s)" << std::endl ;

    int zerotheta = theta*thresholdtheta - gaptheta < 0 ? 1 : 0 + theta*thresholdtheta + gaptheta > 360 ? 1 : 0 ; //Checkteta permet de faire attention si le gaptheta se trouve à + et - de 0 degres (si c'est le cas alors il est egal a 1 sinon 0)

    int check = checktheta + checkamp ;
    std::cout << "Recherche de correspondant en fonction du glisement : " << std::endl ;
    if ( check > 0 )
        std::cout << "OUI" << std::endl ;
    else
        std::cout << "NON" << std::endl ;

    unsigned int thetamin = theta*thresholdtheta - gaptheta < 0 ? theta*thresholdtheta - gaptheta + 360 : theta*thresholdtheta - gaptheta ;
    unsigned int thetamax = theta*thresholdtheta + gaptheta > 360 ? theta*thresholdtheta + gaptheta - 360 : theta*thresholdtheta + gaptheta ;

    total = 0 ; //Ici total vas compter le nombre de pixels de l'image 1 reconnus comme étants correspondants à ceux de l'image 2
    for( unsigned int i = 0 ; i < nbpoint1 ; i++) {
        //A la demande de l'utulisateur
        //On affiche/valide les correspondances dont la translation ne depasse pas la tolerance + ou - gaptheta (en degres)
        if ( feature1[i][6] != -10 ) {
            if ( checktheta > 0 ) {
                if ( zerotheta ) { //Si on fait un gap autour de 0 degres
                    if ( feature1[i][6] >= thetamin || feature1[i][6] <= thetamax ) {
                        if ( checkamp ) {
                            if (feature1[i][9] >= amp - gapamp && feature1[i][9] <= amp + gapamp ) {
                                //On affiche le deplacement du point sur les images
                                printfstereo( img1, img2, final1, final2, feature1, feature2, width, height, i ) ;
                                total ++ ;
                            }
                        } else {
                            //On affiche le deplacement du point sur les images
                            printfstereo( img1, img2, final1, final2, feature1, feature2, width, height, i ) ;
                            total ++ ;
                        }
                    }
                } else { //Si le gap n'est pas autour de zero
                    if ( feature1[i][6] >= thetamin && feature1[i][6] <= thetamax ) {
                        if ( checkamp ) {
                            if (feature1[i][9] >= amp - gapamp && feature1[i][9] <= amp + gapamp ) {
                                //On affiche le deplacement du point sur les images
                                printfstereo( img1, img2, final1, final2, feature1, feature2, width, height, i ) ;
                                total ++ ;
                            }
                        } else {
                            //On affiche le deplacement du point sur les images
                            printfstereo( img1, img2, final1, final2, feature1, feature2, width, height, i ) ;
                            total ++ ;
                        }
                    }
                }
            } else {
                if ( checkamp ) {
                    if (feature1[i][9] >= amp - gapamp && feature1[i][9] <= amp + gapamp ) {
                        //On affiche le deplacement du point sur les images
                        printfstereo( img1, img2, final1, final2, feature1, feature2, width, height, i ) ;
                        total ++ ;
                    }
                } else {
                    //On affiche le deplacement du point sur les images
                    printfstereo( img1, img2, final1, final2, feature1, feature2, width, height, i ) ;
                    total ++ ;
                }
            }
        }
    }

    //On vide les features
	freefeature( feature1, nbpoint1) ;
	freefeature( feature2, nbpoint2) ;

    std::cout << "\nNombre de points finaux avec correspondants = " << total << std::endl << std::endl ;
	//--supper-position avec l'image originale fin--

    //On affiche l'image finale avec sdl
	if( (error = showSDL( argv[3], final1, argv[4], final2, 1, width, height)) ) {
		std::cerr << "SDL error " << error << ": malfunction in SDL" << std::endl ;
        clearall ( img1, final1, image1, width, height) ;
        clearall ( img2, final2, image2, width, height) ;
        return error ;
	}

	std::cout << printfinal << std::endl ;
	std::cout << printnull << std::endl ;

    if ( printfinal ) {
        //On enregistre l'image finale
        saveimg( final1, image1, width, height) ;
        if( (error = lodepng::encode(argv[3], image1, width, height)) ) {
            std::cerr << "encoder error " << error << ": " << lodepng_error_text(error) << std::endl ;
            clearall ( img1, final1, image1, width, height) ;
            clearall ( img2, final2, image2, width, height) ;
            return 3 ;
        }
        std::cout << "L'image \"" << argv[3] << "\" a ete enregistree ..." << std::endl ;

        //On enregistre l'image finale
        saveimg( final2, image2, width, height) ;
        if( (error = lodepng::encode(argv[4], image2, width, height)) ) {
            std::cerr << "encoder error " << error << ": " << lodepng_error_text(error) << std::endl ;
            clearall ( img1, final1, image1, width, height) ;
            clearall ( img2, final2, image2, width, height) ;
            return 3 ;
        }
        std::cout << "L'image \"" << argv[4] << "\" a ete enregistree ..." << std::endl ;
    }

    //On vide la memoire puis on quite le programme
    clearall ( img1, final1, image1, width, height) ;
    clearall ( img2, final2, image2, width, height) ;
*/
    return 0 ;
}
