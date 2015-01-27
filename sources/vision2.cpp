#include <VISION/VISION.h>

#define MIN(a,b) (a<b?a:b)
#define MAX(a,b) (a>b?a:b)
#define NO_HUE   -1

//Escapi lib
//----------------------------------------------------
    int escapiInit() {
        // Initialize ESCAPI
        int devices = setupESCAPI();

        if (devices == 0) {
            printf("ESCAPI initialization failure or no devices found.\n") ;
            return 0 ;
        }

        return devices ;
    }

    int escapiInit( int an_int ) {
        // Initialize ESCAPI
        int devices = setupESCAPI();

        if (devices == 0) {
            printf("ESCAPI initialization failure or no devices found.\n") ;
            return 0 ;
        }

        for (int i = 0; i < devices; i++) {
            char temp[256] ;
            getCaptureDeviceName(i, temp, 256) ;
            printf("Device %d: \"%s\"\n", i, temp) ;
        }

        return devices ;
    }

    void escapiCaptureAlloc(struct SimpleCapParams &cap, int w, int h) {
        cap.mWidth = w ;
        cap.mHeight = h ;
        cap.mTargetBuf = new int[w * h] ;
    }

    int escapiCaptureInit(struct SimpleCapParams &cap, int cam) {
        if (initCapture(cam, &cap) == 0) {
            printf("Capture failed, device [%d] may already be in use.\n", cam);
            return 0 ;
        }

        return 1 ;
    }

    void escapiCaptureReq(int cam) {
        doCapture(cam) ;
        while ( !isCaptureDone(cam) ) {}
    }

    int escapiPixGetRed(struct SimpleCapParams cap, int x,int y) {
        return ((cap.mTargetBuf[y*cap.mWidth+x] >> 16) & 0xFF) ;
    }

    int escapiPixGetGreen(struct SimpleCapParams cap, int x,int y) {
        return ((cap.mTargetBuf[y*cap.mWidth+x] >> 8) & 0xFF) ;
    }

    int escapiPixGetBlue(struct SimpleCapParams cap, int x,int y) {
        return (cap.mTargetBuf[y*cap.mWidth+x] & 0xFF) ;
    }

    void escapiPixToRGB(struct SimpleCapParams &cap, int x,int y, int r, int g, int b) {
        cap.mTargetBuf[y*cap.mWidth+x] = ((r<<16) + (g<<8) + b) | 0xff000000;
    }
//----------------------------------------------------


//SDL lib
//----------------------------------------------------
    void sdlGetStdout() {
        freopen( "CON", "w", stdout );
        freopen( "CON", "w", stderr );
    }

    void sdlCleanup() {
        TTF_Quit();
        IMG_Quit();
        SDL_Quit();
    }

    //init SDL
    int sdlInit() {
        if( SDL_Init(SDL_INIT_VIDEO) < 0 ) {
            printf("error, SDL video init failed") ;
            sdlCleanup() ;
            return 1 ;
        }

        return 0 ;
    }

    //init SDL
    Uint32 * sdlBufpPos(SDL_Surface *scr, int x, int y) {
        return ((Uint32 *)scr->pixels + (y * scr->pitch / 4) + x) ;
    }
//----------------------------------------------------

//Vision lib
//----------------------------------------------------
//----------------------------------------------------

//Other
//----------------------------------------------------
    void visionVersion() {
        printf("\n\n---- VISION SYSTEM ----\n") ;
        printf("       %d.%d.%d\n\n", VERSIONY, VERSIONM, VERSIOND) ;
    }

    void visionHelp() {
        printf("\n\n\nHi there !\n\n") ;
        printf("This soft have been created as a standalone and it work on every platform (Windows, Linux ,[WIP OSX and LATER Android).\n") ;
        printf("All you need is at least a terminal with gcc/g++ and at best Code::Blocks\n") ;
        printf("THis soft wil retrieve all your webcam and can do some image processing (POI, stereo, HOG, etc)\n\n") ;
        printf("Webcam output is 4/3 format, here are some exemple of resolution you MUST use :\n") ;
        printf("-300 / 225\n") ;
        printf("-400 / 300\n") ;
        printf("-440 / 330\n") ;
        printf("-460 / 345\n") ;
        printf("-560 / 420\n") ;
        printf("-640 / 480\n") ;
        printf("-800 / 600\n") ;
        printf("\nThank for using this software\n") ;
        printf("\nAbout me :\n") ;
        printf("RESSEGUIER Thomas\n") ;
        printf("Student in Industrial Vision System\n") ;
        printf("Mail : resseguier.thomas@gmail.com\n\n\n") ;
    }
//----------------------------------------------------





















unsigned int*** newimg( unsigned int &width, unsigned int &height) {
	//Allocation de l'image en memoire
	unsigned int ***img = new unsigned int**[width] ;
	for(unsigned int x = 0 ; x < width ; x++) {
		img[x] = new unsigned int*[height] ;
		for(unsigned int y = 0 ; y < height ; y++) {
			img[x][y] = new unsigned int[3] ;
			img[x][y][0] = img[x][y][1] = img[x][y][2] = 0 ; //Au passage on initialise l'image a 0 ;
		}
	}

	return img ;
}

unsigned int** newmask( unsigned int &width, unsigned int &height) {
	//Allocation du masque en memoire
	unsigned int **mask = new unsigned int*[width] ;
	for(unsigned int x = 0 ; x < width ; x++) {
		mask[x] = new unsigned int[height] ;
        for(unsigned int y = 0 ; y < height ; y++)
			mask[x][y] = 0 ; //Au passage on initialise le masque a 0 ;
	}

	return mask ;
}

float** newfeature( unsigned int &nbpoint) {
	//Allocation du masque de feature en memoire
	float **feature = new float*[nbpoint] ;
	for(unsigned int x = 0 ; x < nbpoint ; x++) {
		feature[x] = new float[10] ;
        for(unsigned int y = 0 ; y < 10 ; y++)
			feature[x][y] = 0 ; //Au passage on initialise le masque de feature a 0 ;
	}

	return feature ;
}

void freeimg( unsigned int*** img, unsigned int &width, unsigned int &height) {
	//Efface les allocations en memoire de l'image
	for(unsigned int x = 0 ; x < width ; x++) {
		for(unsigned int y = 0 ; y < height ; y++) {
			free( img[x][y] );
		}
		free( img[x] );
	}

	free( img );
}

void freemask( unsigned int** mask, unsigned int &width) {
	//Efface les allocations en memoire du masque
	for(unsigned int x = 0 ; x < width ; x++) {
		free( mask[x] );
	}

	free( mask );
}

void freefeature( float** feature, unsigned int &nbpoint) {
	//Efface les allocations en memoire du masque de feature
	for(unsigned int x = 0 ; x < nbpoint ; x++) {
		free( feature[x] );
	}

	free( feature );
}

unsigned int*** loadimg( unsigned char* image, unsigned int &width, unsigned int &height) {
    unsigned int x, y ;

	//Allocation de l'image en memoire
	unsigned int ***img = new unsigned int**[width];
	for(x = 0 ; x < width ; x++) {
		img[x] = new unsigned int*[height];
		for(y = 0 ; y < height ; y++) {
			img[x][y] = new unsigned int[3];
		}
	}

    for ( x = 0 ; x < width ; x++) {
		for ( y = 0 ; y < height ; y++) {
			img[x][y][0] = image[y*width*4+x*4+0] ;
			img[x][y][1] = image[y*width*4+x*4+1] ;
			img[x][y][2] = image[y*width*4+x*4+2] ;
		}
	}

	return img ;
}

void cloneimg( unsigned int*** imgin, unsigned int*** imgout, unsigned int &width, unsigned int &height) {
	for( unsigned int x = 0 ; x < width ; x++) {
		for( unsigned int y = 0 ; y < height ; y++) {
			imgout[x][y][0] = imgin[x][y][0] ;
			imgout[x][y][1] = imgin[x][y][1] ;
			imgout[x][y][2] = imgin[x][y][2] ;
		}
	}
}

void saveimg( unsigned int*** img, unsigned char* image, unsigned int &width, unsigned int &height) {
    unsigned int x, y ;

    for ( x = 0 ; x < width ; x++) {
		for ( y = 0 ; y < height ; y++) {
            image[y*width*4+x*4+0] = img[x][y][0] ;
            image[y*width*4+x*4+1] = img[x][y][1] ;
            image[y*width*4+x*4+2] = img[x][y][2] ;
		}
	}
}

//Original rgb2hsv code :
//http://lolengine.net/blog/2013/01/13/fast-rgb-to-hsv
void rgb2hsv( float red, float green, float blue, unsigned int &hue, unsigned int &saturation , unsigned int &value) {
	float K = 0.f;

	if (green < blue)
	{
		std::swap(green, blue);
		K = -1.f;
	}

	if (red < green)
	{
		std::swap(red, green);
		K = -2.f / 6.f - K;
	}

	float chroma = red - std::min(green, blue);
	hue = (fabs(K + (green - blue) / (6.f * chroma + 1e-20f)))*360;
	saturation = (chroma / (red + 1e-20f))*100;
	value = (MAX (red, MAX (green, blue)))*100/255;
}

void rgb2v( unsigned int red, unsigned int green, unsigned int blue, unsigned int &value) {
	value = (MAX (red, MAX (green, blue)))*100/255;
}

//Original hsv2rgb code :
//http://www.algorytm.org/modele-barw/model-hsv/hsv-c.html
void hsv2rgb( float h, float s, float v, unsigned int &r, unsigned int &g, unsigned int &b) {
	int i ;
	float f, p, q, t ;

	s /= 100 ;
	v /= 100 ;

	if(v == 0) {
		r = 0 ;
		g = 0 ;
		v = 0 ;
	}
	else {
		h /= 60 ;
		i = floor(h) ;
		f = h-i ;
		p = v*(1-s) ;
		q = v*(1-(s*f)) ;
		t = v*(1-(s*(1-f))) ;

		if (i == 0) {
			r = v*255 ;
			g = t*255 ;
			b = p*255 ;
		}
		else if (i == 1) {
			r = q*255 ;
			g = v*255 ;
			b = p*255 ;
		}
		else if (i == 2) {
			r = p*255 ;
			g = v*255 ;
			b = t*255 ;
		}
		else if (i == 3) {
			r = p*255 ;
			g = q*255 ;
			b = v*255 ;
		}
		else if (i == 4) {
			r = t*255 ;
			g = p*255 ;
			b = v*255 ;
		}
		else if (i == 5) {
			r = v*255 ;
			g = p*255 ;
			b = q*255 ;
		}
	}
}

void rgb2bw( unsigned int &r, unsigned int &g, unsigned int &b) {
	unsigned int h, s, v ;

	rgb2hsv( r, g, b, h, s, v ) ;
	hsv2rgb( h, 0, v, r, g, b ) ;
}

void blurpix( unsigned int*** imgin, unsigned int*** imgout, unsigned int width, unsigned int height, unsigned int x, unsigned int y, int R) {
	int  p, r, g, b ;
	int i, j ;
	p = r = g = b = 0 ;

	for( i = -R ; i <= R ; i++){
		for( j = -R ; j <= R ; j++){
			if(x + i >= 0 && x + i < width && y + j >= 0 && y + j < height){
				r += imgin[x + i][y + j][0] ;
				g += imgin[x + i][y + j][1] ;
				b += imgin[x + i][y + j][2] ;
				p++ ;
			}
		}
	}

	imgout[x][y][0] = r / p ;
	imgout[x][y][1] = g / p ;
	imgout[x][y][2] = b / p ;
}

void blurimg( unsigned int*** imgin, unsigned int*** imgout, unsigned int width, unsigned int height, int R) {
	unsigned int x, y ;
	int n = 0 ;

	unsigned int ***inter = newimg( width, height ) ;

    cloneimg( imgin, inter, width, height) ;

	while ( n < R) {
		n++ ;

		if ( n & 1) {
			for( x = 0 ; x < width ; x++){
				for( y = 0 ; y < height ; y++){
					blurpix( inter, imgout, width, height, x, y, n) ;
				}
			}
		}
		else {
			for( x = 0 ; x < width ; x++){
				for( y = 0 ; y < height ; y++){
					blurpix( imgout, inter, width, height, x, y, n) ;
				}
			}
		}
	}

	if ( (n-1) & 1) {
		for( x = 0 ; x < width ; x++) {
			for( y = 0 ; y < height ; y++) {
				imgout[x][y][0] = inter[x][y][0] ;
				imgout[x][y][1] = inter[x][y][1] ;
				imgout[x][y][2] = inter[x][y][2] ;
			}
		}
	}

	freeimg( inter, width, height) ;
}

//Difference of Gaussian
//Detection de bord
void dog( unsigned int*** imgin, unsigned int*** imgout, unsigned int width, unsigned int height, int R) {
	unsigned int x, y ,p , max ;

	max = 0 ;

	unsigned int ***inter = newimg( width, height ) ;
    cloneimg( imgin, inter, width, height) ;

	for( x = 0 ; x < width ; x++) {
		for( y = 0 ; y < height ; y++) {
			blurpix( inter, imgout, width, height, x, y, R) ;
			rgb2bw( inter[x][y][0],  inter[x][y][1],  inter[x][y][2]) ;
			rgb2bw( imgout[x][y][0],  imgout[x][y][1],  imgout[x][y][2]) ;
			p = abs((int) imgout[x][y][0] - (int) inter[x][y][0]) ;
			imgout[x][y][0] = p ;
			imgout[x][y][1] = p ;
			imgout[x][y][2] = p ;

			if( p > max ) {
				max = p ;
			}
		}
	}

	for( x = 0 ; x < width ; x++) {
		for( y = 0 ; y < height ; y++) {
			imgout[x][y][0] = (imgout[x][y][0] * 255 ) / max ;
			imgout[x][y][1] = (imgout[x][y][1] * 255 ) / max ;
			imgout[x][y][2] = (imgout[x][y][2] * 255 ) / max ;
		}
	}

	for( x = 0 ; x < width ; x++){
		for( y = 0 ; y < height ; y++){
			int bw ;
			p = bw = 0 ;

			for(int i = -R ; i <= R ; i++){
				for(int j = -R ; j <= R ; j++){
					if(x + i >= 0 && x + i < width && y + j >= 0 && y + j < height){
						bw += imgout[x + i][y + j][0] ;
						p++ ;
					}
				}
			}

			bw /= p ;

			if( bw < 20 ) {
				imgout[x][y][0] = 0 ;
				imgout[x][y][1] = 0 ;
				imgout[x][y][2] = 0 ;
			}
		}
	}

	freeimg( inter, width, height) ;
}

void dog2( unsigned int*** imgin, unsigned int*** imgout, unsigned int width, unsigned int height) {
	unsigned int x, y ;
	int i, j ;
	int total, p, R, max ;
	float D1, D2, z ;

	max = R = 1 ;

	for( x = 0 ; x < width ; x++) {
		for( y = 0 ; y < height ; y++) {
            rgb2bw( imgin[x][y][0],  imgin[x][y][1],  imgin[x][y][2]) ;
		}
    }

	for( x = 0 ; x < width ; x++) {
		for( y = 0 ; y < height ; y++) {
			total = p = 0 ;
			for( i = -R ; i <= R ; i++){
				for( j = -R ; j <= R ; j++){
					if(x + i >= 0 && x + i < width && y + j >= 0 && y + j < height){
						z = abs( (int) imgin[x][y][0] - (int) imgin[x + i][y + j][0] ) ;
						D1 = i * i + j * j ;
						D2 = D1 * D1 + z * z ;

						if(D1 > 0)
							total += D2 / (2*D1)  ;
						else
							total += D2 ;

						p++ ;
					}
				}
			}

			total /= p ;

			if( total > max ) {
				max = total ;
			}

			imgout[x][y][0] = imgout[x][y][1] = imgout[x][y][2] = total ;
		}
	}

	//Augmente le contraste du masque pour plus de lisibilite
	for( x = 0 ; x < width ; x++) {
		for( y = 0 ; y < height ; y++) {
			//On fait un petit tri pour supprimer les points "faibles"
			p = ((imgout[x][y][0] * 255 ) / max) * 3 ;
			if( p > 255 ) {
				imgout[x][y][0] = imgout[x][y][1] = imgout[x][y][2] = 255 ;
			} else if( p > 0 ) {
				imgout[x][y][0] = imgout[x][y][1] = imgout[x][y][2] = p ;
			} else {
				imgout[x][y][0] = imgout[x][y][1] = imgout[x][y][2] = 0 ;
			}
		}
	}
}

void dog3( unsigned int*** imgin, unsigned int*** imgout, unsigned int width, unsigned int height, float A, float B, float C, float D, float E, float F, float G, float H, float I) {
	unsigned int x, y ;
	float maxx = 0 ;
	float coef = A + B + C + D + E + F + G + H + I ;

	//Allocation en memoire de l'image intermediaire
	float ***inter = new float**[width] ;
	for( x = 0 ; x < width ; x++) {
		inter[x] = new float*[height] ;
		for( y = 0 ; y < height ; y++) {
			inter[x][y] = new float[3] ;
			inter[x][y][0] = inter[x][y][1] = inter[x][y][2] = 0 ; //Au passage on initialise l'image a 0 ;
		}
	}

    if ( coef == 0 )
        coef = 1 ;

	for( x = 1 ; x < width-1 ; x++) {
		for( y = 1 ; y < height-1 ; y++) {
            inter[x][y][0] = ( float(imgin[x-1][y-1][0])*A    + float(imgin[x][y-1][0])*B    + float(imgin[x+1][y-1][0])*C  +
                                float(imgin[x-1][y][0])*D      + float(imgin[x][y][0])*E      + float(imgin[x+1][y][0])*F    +
                                float(imgin[x-1][y+1][0])*G    + float(imgin[x][y+1][0])*H    + float(imgin[x+1][y+1][0])*I  ) * coef ;
            while ( inter[x][y][0] < 0 )
                inter[x][y][0] = 255 + inter[x][y][0] ;
            if ( inter[x][y][0] > maxx )
                maxx = inter[x][y][0] ;

            inter[x][y][1] = ( float(imgin[x-1][y-1][1])*A    + float(imgin[x][y-1][1])*B    + float(imgin[x+1][y-1][1])*C  +
                                float(imgin[x-1][y][1])*D      + float(imgin[x][y][1])*E      + float(imgin[x+1][y][1])*F    +
                                float(imgin[x-1][y+1][1])*G    + float(imgin[x][y+1][1])*H    + float(imgin[x+1][y+1][1])*I  ) * coef ;
            while ( inter[x][y][1] < 0 )
                inter[x][y][1] = 255 + inter[x][y][1] ;
            if ( inter[x][y][1] > maxx )
                maxx = inter[x][y][1] ;

            inter[x][y][2] = ( float(imgin[x-1][y-1][2])*A    + float(imgin[x][y-1][2])*B    + float(imgin[x+1][y-1][2])*C  +
                                float(imgin[x-1][y][2])*D      + float(imgin[x][y][2])*E      + float(imgin[x+1][y][2])*F    +
                                float(imgin[x-1][y+1][2])*G    + float(imgin[x][y+1][2])*H    + float(imgin[x+1][y+1][2])*I  ) * coef ;
            while ( inter[x][y][2] < 0 )
                inter[x][y][2] = 255 + inter[x][y][2] ;
            if ( inter[x][y][2] > maxx )
                maxx = inter[x][y][2] ;
		}
    }

    maxx = 254 / maxx ;
	for( x = 1 ; x < width-1 ; x++) {
		for( y = 1 ; y < height-1 ; y++) {
            imgout[x][y][0] = maxx * inter[x][y][0] ;
            imgout[x][y][1] = maxx * inter[x][y][1] ;
            imgout[x][y][2] = maxx * inter[x][y][2] ;

		}
	}

	//Efface l allocation en memoire de l'image intermediaire
	for( x = 0 ; x < width ; x++) {
		for( y = 0 ; y < height ; y++) {
			free( inter[x][y] );
		}
		free( inter[x] );
	}

	free( inter );
}

//dessine un cercle ou carre autour des points interessants
void drawpoint( unsigned int*** imgout, unsigned int width, unsigned int height, unsigned int x, unsigned int y, int type, int R, int color) {
	int i, j ;

	if ( type == 0 ) {
        //dessine un carre vide
        for( i = -R ; i <= R ; i++){
            for( j = -R ; j <= R ; j++){
                if( x + i >= 0 && x + i < width && y + j >= 0 && y + j < height ){
                    if( abs(i) == R || abs(j) == R ){
                        imgout[x + i][y + j][color] = 255 ;
                    }
                }
            }
        }
	} else if ( type == 1 ) {
        //dessine un carre plein
        for( i = -R ; i <= R ; i++){
            for( j = -R ; j <= R ; j++){
                if( x + i >= 0 && x + i < width && y + j >= 0 && y + j < height ){
                    imgout[x + i][y + j][color] = 255 ;
                }
            }
        }
	} else if ( type == 2 ) {
       //dessine un cercle vide
       //Algorithme de trace de cercle d'Andres
        i = 0 ;
        j = R ;
        int d = R-1 ;

        while ( j >= i ) {
            if( x + i >= 0 && x + i < width && y + j >= 0 && y + j < height )
                imgout[x + i][y + j][color] = 255 ;

            if( x + j >= 0 && x + j < width && y + i >= 0 && y + i < height )
                imgout[x + j][y + i][color] = 255 ;

            if( x - i >= 0 && x - i < width && y + j >= 0 && y + j < height )
                imgout[x - i][y + j][color] = 255 ;

            if( x - j >= 0 && x - j < width && y + i >= 0 && y + i < height )
                imgout[x - j][y + i][color] = 255 ;

            if( x + i >= 0 && x + i < width && y - j >= 0 && y - j < height )
                imgout[x + i][y - j][color] = 255 ;

            if( x + j >= 0 && x + j < width && y - i >= 0 && y - i < height )
                imgout[x + j][y - i][color] = 255 ;

            if( x - i >= 0 && x - i < width && y - j >= 0 && y - j < height )
                imgout[x - i][y - j][color] = 255 ;

            if( x - j >= 0 && x - j < width && y - i >= 0 && y - i < height )
                imgout[x - j][y - i][color] = 255 ;

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
	} else if ( type == 3 ) {
       //dessine un cercle plein
       for (int R2 = 0 ; R2 <= R ; R2++ ) {
           //Algorithme de trace de cercle d'Andres
            i = 0 ;
            j = R2 ;
            int d = R2-1 ;

            while ( j >= i ) {
                if( x + i >= 0 && x + i < width && y + j >= 0 && y + j < height )
                    imgout[x + i][y + j][color] = 255 ;

                if( x + j >= 0 && x + j < width && y + i >= 0 && y + i < height )
                    imgout[x + j][y + i][color] = 255 ;

                if( x - i >= 0 && x - i < width && y + j >= 0 && y + j < height )
                    imgout[x - i][y + j][color] = 255 ;

                if( x - j >= 0 && x - j < width && y + i >= 0 && y + i < height )
                    imgout[x - j][y + i][color] = 255 ;

                if( x + i >= 0 && x + i < width && y - j >= 0 && y - j < height )
                    imgout[x + i][y - j][color] = 255 ;

                if( x + j >= 0 && x + j < width && y - i >= 0 && y - i < height )
                    imgout[x + j][y - i][color] = 255 ;

                if( x - i >= 0 && x - i < width && y - j >= 0 && y - j < height )
                    imgout[x - i][y - j][color] = 255 ;

                if( x - j >= 0 && x - j < width && y - i >= 0 && y - i < height )
                    imgout[x - j][y - i][color] = 255 ;

                if ( d >= 2*i ) {
                    d = d - 2*i - 1 ;
                    i = i + 1 ;
                } else if ( d < 2*(R2-j) ) {
                    d = d + 2*j - 1 ;
                    j = j -1 ;
                } else {
                    d = d + 2*(j - i - 1) ;
                    j = j - 1 ;
                    i = i + 1 ;
                }
            }
        }
	}else if ( type == 4 ) {
       //dessine un cercle vide
       //Algorithme de trace de cercle d'Andres
        i = 0 ;
        j = R ;
        int d = R-1 ;

        while ( j >= i ) {
            if( x + i >= 0 && x + i < width && y + j >= 0 && y + j < height )
                imgout[x + i][y + j][0] = imgout[x + i][y + j][1] = imgout[x + i][y + j][2] = 0 ;

            if( x + j >= 0 && x + j < width && y + i >= 0 && y + i < height )
                imgout[x + j][y + i][0] = imgout[x + j][y + i][1] = imgout[x + j][y + i][2] = 0 ;

            if( x - i >= 0 && x - i < width && y + j >= 0 && y + j < height )
                imgout[x - i][y + j][0] = imgout[x - i][y + j][1] = imgout[x - i][y + j][2] = 0 ;

            if( x - j >= 0 && x - j < width && y + i >= 0 && y + i < height )
                imgout[x - j][y + i][0] = imgout[x - j][y + i][1] = imgout[x - j][y + i][2] = 0 ;

            if( x + i >= 0 && x + i < width && y - j >= 0 && y - j < height )
                imgout[x + i][y - j][0] = imgout[x + i][y - j][1] = imgout[x + i][y - j][2] = 0 ;

            if( x + j >= 0 && x + j < width && y - i >= 0 && y - i < height )
                imgout[x + j][y - i][0] = imgout[x + j][y - i][1] = imgout[x + j][y - i][2] = 0 ;

            if( x - i >= 0 && x - i < width && y - j >= 0 && y - j < height )
                imgout[x - i][y - j][0] = imgout[x - i][y - j][1] = imgout[x - i][y - j][2] = 0 ;

            if( x - j >= 0 && x - j < width && y - i >= 0 && y - i < height )
                imgout[x - j][y - i][0] = imgout[x - j][y - i][1] = imgout[x - j][y - i][2] = 0 ;

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
	}
}

//dessine le segment entre deux points
void ligne( unsigned int*** img, int xi, int yi, int xf, int yf) {
    int dx, dy, i, xinc, yinc, cumul, x, y ;

    x = xi ;
    y = yi ;
    dx = xf - xi ;
    dy = yf - yi ;
    xinc = ( dx > 0 ) ? 1 : -1 ;
    yinc = ( dy > 0 ) ? 1 : -1 ;
    dx = abs(dx) ;
    dy = abs(dy) ;

    img[x][y][2] = 255 ;

    if ( dx > dy ) {
        cumul = dx / 2 ;
        for ( i = 1 ; i <= dx ; i++ ) {
            x += xinc ;
            cumul += dy ;
            if ( cumul >= dx ) {
                cumul -= dx ;
                y += yinc ;
            }
            img[x][y][2] = 255 ;
        }
    }
    else {
        cumul = dy / 2 ;
        for ( i = 1 ; i <= dy ; i++ ) {
            y += yinc ;
            cumul += dx ;
            if ( cumul >= dy ) {
                cumul -= dy ;
                x += xinc ;
            }
            img[x][y][2] = 255 ;
        }
    }
}

//Erosion / barrycentre
void erosion( unsigned int*** img, unsigned int width, unsigned int height, unsigned int &total_pix_all, unsigned int Rmax) {
	int p1, p2 ;
    int compteur ;
    int posx, posy ;

    //On annalyse les nuages de pixels ayant ete reconnu comme etant positionne sur / proche d'un corner
    //ici j'utilise le terme d'erosion mais il est plus question de calculer le barry centre d'un nuage de point dans une fenetre d'ouverture R
    for( unsigned int x = Rmax ; x < width-Rmax ; x++) {
        for( unsigned int y = Rmax ; y < height-Rmax ; y++) {
            //On ne fait les calculs que lorsque l'on est sur un pixel valide (tant de calcul abreges
            if( img[x][y][0] == 255 ) {
                //On va incrementer la valeur de R  jusqu'a ce qu'il soirt egal a Rmax (defini par user)
                //Cette fenetre ne doit en aucun cas avoir des pixel sur sa periferie
                //On commece plus petit de facon a pouvoir englober des petits "tas" de pixel valides de facon precise
                //de cette facon un peu tres bien valider un corner sans meme avoir besoin d'atteindre une fenetre de taille Rmax en la position pixel actuel
                for ( int R = 2 ; R <= Rmax ; R++ ) {
                    if(x-R >= 0 && x+R < width && y-R >= 0 && y+R < height ) { //on fait attention de ne pas sortir de l'image
                        p1 = p2 = 0 ;

                        //On verifie d'abord que les contour du cadre d'erosion ne comporte pas de pixel valide :
                        //De cette façon on est certain d'englober une zone avec des pixel valide; on ne "coupe" pas la zone !
                        for( int i = -R ; i <= R ; i++ ) {
                            for( int j = -R ; j <= R ; j++ ) {
                                if( abs(i) == R || abs(j) == R ){
                                    if ( img[x + i][y + j][0] == 0 ) {
                                        p1++ ;
                                    }
                                    p2 ++ ;
                                }
                            }
                        }

                        if ( p1 == p2 && p1 > 0 ) { //Si Le nombre de point non valide (P1) en periferie correspond au total de pixel possible en periferie alors ok
                            compteur = posx = posy = 0 ;

                            //On compte le nombre de pixel valides dans la fenetre d'erosion et on additionne leur coordonnee
                            for( int i = -R ; i <= R ; i++) {
                                for( int j = -R ; j <= R ; j++) {
                                    if ( img[x + i][y + j][0] == 255 ) {
                                        compteur++  ;
                                        posx += (x+i) ;
                                        posy += (y+j) ;

                                        img[x + i][y + j][0] = 0 ;
                                    }
                                }
                            }

                            //On calcule la position du bary centre : de cette facon on place un point definitif et definissant sa position comme etant celle d'un corner !
                            if ( compteur > 0 ) {
                                posx /= compteur ;
                                posy /= compteur ;

                                img[posx][posy][2] = 255 ;
                                total_pix_all++ ;
                                R = Rmax+1 ;
                            }
                        }
                    }
                }
            }
        }
    }
	std::cout << "Il reste : " << total_pix_all << " pixels." << std::endl ;
}

void center_color( unsigned int*** img, unsigned int*** out, unsigned int width, unsigned int height, int hue, int gap_h, int sat, int gap_s, int val, int gap_v ) {
    int compteur = 0 ;
    int posx = 0 ;
    int posy = 0 ;
    int maxy = 0 ;
    int maxx = 0 ;
    int moy = 0 ;

	for(unsigned int x = 0 ; x < width ; x++) {
		for(unsigned int y = 0 ; y < height ; y++) {
            rgb2hsv( img[x][y][0], img[x][y][1], img[x][y][2], out[x][y][0], out[x][y][1], out[x][y][2]) ;


            //if ( img[x][y][0] > hue && img[x][y][1] > hue && img[x][y][2] > hue ) {
            if ( out[x][y][0] >= (hue-gap_h) && out[x][y][0] <= (hue+gap_h) ) {
                if ( out[x][y][1] >= (sat-gap_s) && out[x][y][1] <= (sat+gap_s) ) {
                    if ( out[x][y][2] >= (val-gap_v) && out[x][y][2] <= (val+gap_v) ) {
                        posx += x ;
                        posy += y ;
                        compteur ++ ;

                        if ( posx > maxx )
                            maxx = x ;

                        if ( posy > maxy )
                            maxy = y ;
                        hsv2rgb( hue, 100, 100, out[x][y][0], out[x][y][1], out[x][y][2]) ;
                        //hsv2rgb( out[x][y][0], out[x][y][1], out[x][y][2], out[x][y][0], out[x][y][1], out[x][y][2]) ;
                    } else {
                        out[x][y][0] = img[x][y][0] ;
                        out[x][y][1] = img[x][y][1] ;
                        out[x][y][2] = img[x][y][2] ;
                    }
                } else {
                    //out[x][y][0] = out[x][y][1] = out[x][y][2] = 0 ;
                    out[x][y][0] = img[x][y][0] ;
                    out[x][y][1] = img[x][y][1] ;
                    out[x][y][2] = img[x][y][2] ;
                }
            } else {
                out[x][y][0] = img[x][y][0] ;
                out[x][y][1] = img[x][y][1] ;
                out[x][y][2] = img[x][y][2] ;
            }
		}
	}


	if ( compteur > 50 ) {
        posx = posx / compteur ;
        posy = posy / compteur ;

        maxx -= posx ;
        maxy -= posy ;
        moy = (maxx + maxy) / 2 ;
        //printf ( "maxy = %d - maxx = %d - moy = %d\n", maxy, maxx, moy) ;
        drawpoint( out, width, height, posx, posy, 4, moy, 1) ;
	}
}

//susan
void susan9( unsigned int*** imgin, unsigned int*** imgout, unsigned int width, unsigned int height, int gap) {
	unsigned int **mask = newmask( width, height ) ;
    int *circle9 = new int[9] ; //Ce tableau contiendra les valeur h, s et v des 8 pixels autour de celui analyse
	unsigned int nb_pix, oppose ;
    unsigned int I, i ;

    //On recupere le V de l'image originale : image en noir et blan de 0 a 100 % (et non de 0 a 255) ["I" est juste la pour "remplir"]
    for( unsigned int x = 0 ; x < width ; x++)
        for( unsigned int y = 0 ; y < height ; y++)
            mask[x][y] = (MAX (imgin[x][y][0], MAX (imgin[x][y][1], imgin[x][y][2])))*100/255 ;

    for( unsigned int x = 4 ; x < width-4 ; x++) {
        for( unsigned int y = 4 ; y < height-4 ; y++) {
            nb_pix = oppose = 0 ;

            //Ce tableau contiendra les valeur h, s et v des 8 pixels autour de celui analyse
            unsigned int *circle9 = new unsigned int[9] ;

            //Enregistre le point voulu en circle9[0]
            circle9[0] = mask[x][y] ;

            //Cercle autour du pixel voulu (cercle9)
            circle9[1] = mask[x][y-2] ;
            circle9[2] = mask[x+1][y-1] ;
            circle9[3] = mask[x+2][y] ;
            circle9[4] = mask[x+1][y+1] ;
            circle9[5] = mask[x][y+2] ;
            circle9[6] = mask[x-1][y+1] ;
            circle9[7] = mask[x-2][y] ;
            circle9[8] = mask[x-1][y-1] ;

            //Vérification de v de chaque pixel par rapport au v de P
            for( i = 1 ; i < 9 ; i++ ) {
                if( abs(circle9[i] - circle9[0]) < gap ){
                    circle9[i] = 1 ; //Si le pixel est similaire alors il passe a 1
                    nb_pix ++ ; //Calcule le nombre de pixels valides dans le cercle
                } else {
                    circle9[i] = 0 ; //Si le pixel est different alors il passe a 0
                }
            }

            for ( i = 1 ; i < 9 ; i++ ) {
                I = i + (9-1)/2 ;

                if ( I > 9-1 )
                    I -= 9-1 ;

                //Si le pixel oppose est lui aussi valide alors on passe "i" a 30 => on ne validera pas img[x][y] comme etant un corner et donc on accelere le changement de pixel
                if ( circle9[i] == 1 && circle9[I] == 1 )
                    i = 30 ;
            }

            if ( nb_pix < (9-1)/2 && i == 9 ) {
                drawpoint( imgout, width, height, x, y, 3, 2, 0) ;
            }
        }
    }

	freemask( mask, width) ;
    free( circle9 );
}

void susan17( unsigned int*** imgin, unsigned int*** imgout, unsigned int width, unsigned int height, int gap) {
	unsigned int **mask = newmask( width, height ) ;
    int *circle17 = new int[17] ; //Ce tableau contiendra les valeur h, s et v des 17 pixels autour de celui analyse
	unsigned int nb_pix, oppose ;
    unsigned int I, i ;

    //On recupere le V de l'image originale : image en noir et blan de 0 a 100 % (et non de 0 a 255) ["I" est juste la pour "remplir"]
    for( unsigned int x = 0 ; x < width ; x++)
        for( unsigned int y = 0 ; y < height ; y++)
            mask[x][y] = (MAX (imgin[x][y][0], MAX (imgin[x][y][1], imgin[x][y][2])))*100/255 ;

    for( unsigned int x = 4 ; x < width-4 ; x++) {
        for( unsigned int y = 4 ; y < height-4 ; y++) {
            nb_pix = oppose = 0 ;

            //Enregistre le point voulu en circle17[0]
            circle17[0] = mask[x][y] ;

            //Cercle autour du pixel voulu (cercle17)
            circle17[1] = mask[x][y-3] ;
            circle17[2] = mask[x+1][y-3] ;
            circle17[3] = mask[x+2][y-2] ;
            circle17[4] = mask[x+3][y-1] ;
            circle17[5] = mask[x+3][y] ;
            circle17[6] = mask[x+3][y+1] ;
            circle17[7] = mask[x+2][y+2] ;
            circle17[8] = mask[x+1][y+3] ;
            circle17[9] = mask[x][y+3] ;
            circle17[10] = mask[x-1][y+3] ;
            circle17[11] = mask[x-2][y+2] ;
            circle17[12] = mask[x-3][y+1] ;
            circle17[13] = mask[x-3][y] ;
            circle17[14] = mask[x-3][y-1] ;
            circle17[15] = mask[x-2][y-2] ;
            circle17[16] = mask[x-1][y-3] ;

            //Vérification de v de chaque pixel par rapport au v de P
            for( i = 1 ; i < 17 ; i++ ) {
                if( abs(circle17[i] - circle17[0]) < gap ){
                    circle17[i] = 1 ; //Si le pixel est similaire alors il passe a 1
                    nb_pix ++ ; //Calcule le nombre de pixels valides dans le cercle
                } else {
                    circle17[i] = 0 ; //Si le pixel est different alors il passe a 0
                }
            }

            for ( i = 1 ; i < 17 ; i++ ) {
                I = i + (17-1)/2 ;

                if ( I > 17-1 )
                    I -= 17-1 ;

                //Si le pixel oppose est lui aussi valide alors on passe "i" a 30 => on ne validera pas img[x][y] comme etant un corner et donc on accelere le changement de pixel
                if ( circle17[i] == 1 && circle17[I] == 1 )
                    i = 30 ;
            }

            if ( nb_pix < (17-1)/2 && i == 17 )
                drawpoint( imgout, width, height, x, y, 3, 2, 0) ;
        }
    }

	freemask( mask, width) ;
    free( circle17 );
}

void susan21( unsigned int*** imgin, unsigned int*** imgout, unsigned int width, unsigned int height, int gap) {
	unsigned int **mask = newmask( width, height ) ;
    int *circle21 = new int[21] ; //Ce tableau contiendra les valeur h, s et v des 20 pixels autour de celui analyse
	unsigned int nb_pix, oppose ;
    unsigned int I, i ;

    //On recupere le V de l'image originale : image en noir et blan de 0 a 100 % (et non de 0 a 255) ["I" est juste la pour "remplir"]
    for( unsigned int x = 0 ; x < width ; x++)
        for( unsigned int y = 0 ; y < height ; y++)
            mask[x][y] = (MAX (imgin[x][y][0], MAX (imgin[x][y][1], imgin[x][y][2])))*100/255 ;

    for( unsigned int x = 4 ; x < width-4 ; x++) {
        for( unsigned int y = 4 ; y < height-4 ; y++) {
            nb_pix = oppose = 0 ;

            //Enregistre le point voulu en circle21[0]
            circle21[0] = mask[x][y] ;

            //Cercle autour du pixel voulu (cercle21)
            circle21[1] = mask[x][y-4] ;
            circle21[2] = mask[x+1][y-4] ;
            circle21[3] = mask[x+2][y-3] ;
            circle21[4] = mask[x+3][y-2] ;
            circle21[5] = mask[x+4][y-1] ;
            circle21[6] = mask[x+4][y] ;
            circle21[7] = mask[x+4][y+1] ;
            circle21[8] = mask[x+3][y+2] ;
            circle21[9] = mask[x+2][y+3] ;
            circle21[10] = mask[x+1][y+4] ;
            circle21[11] = mask[x][y+4] ;
            circle21[12] = mask[x-1][y+4] ;
            circle21[13] = mask[x-2][y+3] ;
            circle21[14] = mask[x-3][y+2] ;
            circle21[15] = mask[x-4][y+1] ;
            circle21[16] = mask[x-4][y] ;
            circle21[17] = mask[x-4][y-1] ;
            circle21[18] = mask[x-3][y-2] ;
            circle21[19] = mask[x-2][y-3] ;
            circle21[20] = mask[x-1][y-4] ;

            //Vérification de V de chaque pixel par rapport au V de P
            for( i = 1 ; i < 21 ; i++ ) {
                if( abs(circle21[i] - circle21[0]) < gap ){
                    circle21[i] = 1 ; //Si le pixel est similaire alors il passe a 1
                    nb_pix ++ ; //Calcule le nombre de pixels valides dans le cercle
                } else {
                    circle21[i] = 0 ; //Si le pixel est different alors il passe a 0
                }
            }

            //On verifie que chaque point valide sur le cercle n'ai pas d'oppose lui aussi valide
            for ( i = 1 ; i < 21 ; i++ ) {
                I = i + (21-1)/2 ;

                if ( I > 21-1 )
                    I -= 21-1 ;

                //Si le pixel oppose est lui aussi valide alors on passe "i" a 30 => on ne validera pas img[x][y] comme etant un corner et donc on accelere le changement de pixel
                if ( circle21[i] == 1 && circle21[I] == 1 )
                    i = 30 ;
            }

            if ( nb_pix < (21-1)/2 && i == 21 )
                drawpoint( imgout, width, height, x, y, 3, 2, 0) ;
        }
    }

	freemask( mask, width) ;
    free( circle21 );
}
