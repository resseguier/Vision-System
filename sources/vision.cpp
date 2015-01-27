#include <VISION/VISION.h>

//SCILAB vers 5.5.1 32bits for windows
#include <SCILAB/api_scilab.h>
#include <SCILAB/stack-c.h> /* Provide functions to access to the memory of Scilab */
#include <SCILAB/call_scilab.h> /* Provide functions to call Scilab engine */


//Escapi lib
//----------------------------------------------------
    int escapiInit() {
        printf("|-Loading Escapi...\n") ;
        // Initialize ESCAPI
        int devices = setupESCAPI();

        if (devices == 0) {
            printf("| |-ESCAPI initialization failure or no devices found.\n") ;
            return 0 ;
        }

        return devices ;
    }

    int escapiInit( int an_int ) {
        printf("|-Loading Escapi...\n") ;
        // Initialize ESCAPI
        int devices = setupESCAPI();

        if (devices == 0) {
            printf("| |-ESCAPI initialization failure or no devices found.\n") ;
            return 0 ;
        }

        for (int i = 0; i < devices; i++) {
            char temp[256] ;
            getCaptureDeviceName(i, temp, 256) ;
            printf("| |-Device %d: \"%s\"\n", i, temp) ;
        }
        printf("|\n") ;

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

    void escapiDeinitCapture(const char *argcamera) {
        int camname ;
        sscanf(argcamera, "%d", &camname) ;
        deinitCapture(camname) ;
    }

    int escapiComplete(const char *argcamera, struct SimpleCapParams &cap, Image2D &image, int width, int height) {
        int camname ;
        sscanf(argcamera, "%d", &camname) ;
        printf("|-initialization camera %d\n", camname) ;
        escapiCaptureAlloc(cap, width, height) ;
        if( !escapiCaptureInit(cap, camname) ) return -3 ;
        visionImageAlloc(image, width, height) ;
        return 0 ;
    }

    void escapiCaptureReq(int cam) {
        doCapture(cam) ;
        while ( !isCaptureDone(cam) ) {}
    }

    void escapiCaptureReq2(const char *argcamera, struct SimpleCapParams cap, Image2D image) {
        int camname ;
        sscanf(argcamera, "%d", &camname) ;
        doCapture(camname) ;
        int done = 0 ;
        while ( !isCaptureDone(camname) ) {
            done ++ ;
            if(done> 70000000)
                break ;
        }

        for(int x = 0 ; x < image.width ; x++)
            for(int y = 0 ; y < image.height ; y++)
                image.img[x][y] = 0.21*escapiPixGetRed(cap,x,y) + 0.72*escapiPixGetGreen(cap,x,y) + 0.07*escapiPixGetBlue(cap,x,y) ;
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

//SCILAB
//----------------------------------------------------
    int scilabInit() {
        printf("|-Loading Scilab...\n") ;
        #ifdef _MSC_VER
        if( StartScilab(NULL,NULL,NULL) == FALSE )
        #else
        if( StartScilab(getenv("SCI"),NULL,NULL) == FALSE )
        #endif
        {
            fprintf(stderr,"| |-Error while calling StartScilab\n") ;
            return 0 ;
        }
        return 1 ;
    }
//----------------------------------------------------

//SDL lib
//----------------------------------------------------
    void sdlGetStdout() {
        freopen( "CON", "w", stdout );
        freopen( "CON", "w", stderr );
    }

    void sdlFillStdout() {
        freopen( "stdout.txt", "w", stdout );
        freopen( "stderr.txt", "w", stderr );
    }

    void sdlCleanup() {
        TTF_Quit();
        IMG_Quit();
        SDL_Quit();
    }

    int sdlInit() {
        printf("|-Loading SDL...\n") ;
        if( SDL_Init(SDL_INIT_VIDEO) < 0 ) {
            printf("| |-error, SDL video init failed") ;
            sdlCleanup() ;
            return 0 ;
        }
        return 1 ;
    }

    void sdlImageToScr(Image2D &image, SDL_Surface *scr, int camnb) {
        for(int x = 0 ; x < image.width ; x ++ ) {
            for(int y = 0 ; y < image.height ; y ++ ) {
                Uint32* bufp = (Uint32 *)scr->pixels + (y * scr->pitch / 4) + (x + image.width*camnb) ;
                *bufp = 65793 * image.img[x][y] ;
            }
        }
    }

    void sdlImageToScrRGB(Image2DRGB &image, SDL_Surface *scr, int camnb) {
        for(int x = 0 ; x < image.width ; x ++ ) {
            for(int y = 0 ; y < image.height ; y ++ ) {
                Uint32* bufp = (Uint32 *)scr->pixels + (y * scr->pitch / 4) + (x + image.width*camnb) ;
                *bufp = 65536 * image.imgRGB[x][y][0] + 256 * image.imgRGB[x][y][1] + image.imgRGB[x][y][2] ;

            }
        }
    }

    unsigned char GetPixelComp32(SDL_Surface *scr,int x,int y,int c) {
        unsigned char *p = ((unsigned char*)scr->pixels) + y * scr->pitch + x * 4;
        return p[c];
    }
    //dessine le segment entre deux points
    //Version adapté au SDL
    //avec ajout de transparence pour visibilite
    void sdlLigneLight(SDL_Surface *scr, int width, int height, int xi, int yi, int xf, int yf, int color) {
        int dx, dy, i, xinc, yinc, cumul, x, y ;
        int tint ;

        x = xi ;
        y = yi ;
        dx = xf - xi ;
        dy = yf - yi ;
        xinc = ( dx > 0 ) ? 1 : -1 ;
        yinc = ( dy > 0 ) ? 1 : -1 ;
        dx = abs(dx) ;
        dy = abs(dy) ;

        if( x >= 0 && x < width && y >= 0 && y < height ) {
            Uint32* bufp = (Uint32 *)scr->pixels + (y * scr->pitch / 4) + x  ;
            tint = bufp[0]/65536 + color ;
            if( tint > 255 )
                tint = 255 ;
            *bufp = tint*65793 ;
        }

        if ( dx > dy ) {
            cumul = dx / 2 ;
            for ( i = 1 ; i <= dx ; i++ ) {
                x += xinc ;
                cumul += dy ;
                if ( cumul >= dx ) {
                    cumul -= dx ;
                    y += yinc ;
                }

                if( x >= 0 && x < width && y >= 0 && y < height ) {
                    Uint32* bufp = (Uint32 *)scr->pixels + (y * scr->pitch / 4) + x  ;
                    tint = bufp[0]/65536 + color ;
                    if( tint > 255 )
                        tint = 255 ;
                    *bufp = tint*65793 ;
                }
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

                if( x >= 0 && x < width && y >= 0 && y < height ) {
                    Uint32* bufp = (Uint32 *)scr->pixels + (y * scr->pitch / 4) + x  ;
                    tint = bufp[0]/65536 + color ;
                    if( tint > 255 )
                        tint = 255 ;
                    *bufp = tint*65793 ;
                }
            }
        }
    }

    void sdlLigneDark(SDL_Surface *scr, int width, int height, int xi, int yi, int xf, int yf, int color) {
        int dx, dy, i, xinc, yinc, cumul, x, y ;
        int tint ;
        int tint2 = 255-color ;

        x = xi ;
        y = yi ;
        dx = xf - xi ;
        dy = yf - yi ;
        xinc = ( dx > 0 ) ? 1 : -1 ;
        yinc = ( dy > 0 ) ? 1 : -1 ;
        dx = abs(dx) ;
        dy = abs(dy) ;

        if( x >= 0 && x < width && y >= 0 && y < height ) {
            Uint32* bufp = (Uint32 *)scr->pixels + (y * scr->pitch / 4) + x  ;
            tint = bufp[0]/65536 - color ;
            if( tint > 255 )
                tint = 255 ;
            *bufp = tint2*65793 ;
        }

        if ( dx > dy ) {
            cumul = dx / 2 ;
            for ( i = 1 ; i <= dx ; i++ ) {
                x += xinc ;
                cumul += dy ;
                if ( cumul >= dx ) {
                    cumul -= dx ;
                    y += yinc ;
                }

                if( x >= 0 && x < width && y >= 0 && y < height ) {
                    Uint32* bufp = (Uint32 *)scr->pixels + (y * scr->pitch / 4) + x  ;
                    tint = bufp[0]/65536 - color ;
                    if( tint > 255 )
                        tint = 255 ;
                    *bufp = tint2*65793 ;
                }
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

                if( x >= 0 && x < width && y >= 0 && y < height ) {
                    Uint32* bufp = (Uint32 *)scr->pixels + (y * scr->pitch / 4) + x  ;
                    tint = bufp[0]/65536 - color ;
                    if( tint > 255 )
                        tint = 255 ;
                    *bufp = tint2*65793 ;
                }
            }
        }
    }
//----------------------------------------------------

//Vision lib
//----------------------------------------------------
    void visionImageAlloc(Image2D &image, int width, int height) {
        image.width = width ;
        image.height = height ;

        image.img = new int*[width] ;
        for(int x = 0 ; x < width ; x++)
            image.img[x] = new int[height] ;
    }

    void visionImageFree(Image2D image) {
        //Efface les allocations en memoire de l'image
        for(int x = 0 ; x < image.width ; x++)
            free( image.img[x] ) ;

        free( image.img ) ;
    }

    void visionCapture2Image(struct SimpleCapParams capture, Image2D image) {
        for(int x = 0 ; x < image.width ; x++)
            for(int y = 0 ; y < image.height ; y++)
                image.img[x][y] = 0.21*escapiPixGetRed(capture,x,y) + 0.72*escapiPixGetGreen(capture,x,y) + 0.07*escapiPixGetBlue(capture,x,y) ;
    }

    int visionPicture2Image(const char *argpicture, Image2D &image, unsigned int &width, unsigned int &height) {
        if( argpicture[(strlen(argpicture)-1)] == 'p') { //If file is a BMP
            if( visionBMPproperties(argpicture, width, height) ) return -44 ;
            visionImageAlloc(image, width, height) ;
            visionBMP2Image(argpicture, image) ;
        } else { //Else it should be a PNG or it will fail to load it
            unsigned char* picture ;
            if( lodepng_decode32_file(&picture, &width, &height, argpicture) ) return -4 ;
            visionImageAlloc(image, width, height) ;
            for (int x = 0 ; x < width ; x++)
                for (int y = 0 ; y < height ; y++)
                    image.img[x][y] = 0.21*picture[y*width*4+x*4+0] + 0.72*picture[y*width*4+x*4+1] + 0.07*picture[y*width*4+x*4+2] ;
            free(picture) ;
        }
        printf("|-Picture \"%s\" has been loaded ...\n", argpicture) ;

        return 0 ;
    }

    void visionPicture3Image(unsigned char* picture, Image2D image) {
        for (int x = 0 ; x < image.width ; x++)
            for (int y = 0 ; y < image.height ; y++)
                image.img[x][y] = 0.21*picture[y*image.width*4+x*4+0] + 0.72*picture[y*image.width*4+x*4+1] + 0.07*picture[y*image.width*4+x*4+2] ;
    }

    int visionBMPproperties(const char *argbmp, unsigned int &width, unsigned int &height) {
        std::vector<unsigned char> bmp;
        lodepng::load_file(bmp, argbmp);

        static const unsigned MINHEADER = 54 ;

        if(bmp.size() < MINHEADER) return -1 ;
        if(bmp[0] != 'B' || bmp[1] != 'M') return 1 ;
        width = bmp[18] + bmp[19] * 256 ;
        height = bmp[22] + bmp[23] * 256 ;

        return 0 ;
    }

    int visionBMP2Image(const char *argbmp, Image2D image) {
        int width, height ;
        std::vector<unsigned char> bmp;
        lodepng::load_file(bmp, argbmp);

        static const unsigned MINHEADER = 54 ;

        if(bmp.size() < MINHEADER) return -1 ;
        if(bmp[0] != 'B' || bmp[1] != 'M') return 1 ;
        unsigned pixeloffset = bmp[10] + 256 * bmp[11] ;

        width = bmp[18] + bmp[19] * 256 ;
        height = bmp[22] + bmp[23] * 256 ;
        //read number of channels from BMP header
        if(bmp[28] != 24 && bmp[28] != 32) return 2 ; //only 24-bit and 32-bit BMPs are supported.
        unsigned numChannels = bmp[28] / 8 ;

        unsigned scanlineBytes = width * numChannels ;
        if(scanlineBytes % 4 != 0) scanlineBytes = (scanlineBytes / 4) * 4 + 4 ;

        unsigned dataSize = scanlineBytes * height ;
        if(bmp.size() < dataSize + pixeloffset) return 3 ;

        for(int y = 0; y < height; y++) {
            for(int x = 0; x < width; x++) {
                unsigned bmpos = pixeloffset + (height - y - 1) * scanlineBytes + numChannels * x ;
                if(numChannels == 3)
                    image.img[x][y] = 0.21*bmp[bmpos + 2] + 0.72*bmp[bmpos + 1] + 0.07*bmp[bmpos + 0] ;
                else
                    image.img[x][y] = 0.21*bmp[bmpos + 3] + 0.72*bmp[bmpos + 2] + 0.07*bmp[bmpos + 1] ;
            }
        }
        return 0 ;
    }







    void visionCloneImage(Image2D img_in, Image2D img_out) {
        for(int x = 0 ; x < img_in.width ; x++)
            for(int y = 0 ; y < img_in.height ; y++)
                img_out.img[x][y] = img_in.img[x][y] ;
    }

    void visionImageAllocRGB(Image2DRGB &image, int width, int height) {
        image.width = width ;
        image.height = height ;

        image.imgRGB = new int**[width] ;
        for(int x = 0 ; x < width ; x++) {
            image.imgRGB[x] = new int*[height] ;
            for(int y = 0 ; y < height ; y++) {
                image.imgRGB[x][y] = new int[3] ;
            }
        }
    }

    void visionImageFreeRGB(Image2DRGB image) {
        //Efface les allocations en memoire de l'image
        for(int x = 0 ; x < image.width ; x++) {
            for(int y = 0 ; y < image.height ; y++) {
                free( image.imgRGB[x][y] ) ;
            }
            free( image.imgRGB[x] ) ;
        }

        free( image.imgRGB ) ;
    }

    void visionCapture2ImageRGB(struct SimpleCapParams capture, Image2DRGB image) {
        for ( int x = 0 ; x < image.width ; x++) {
            for ( int y = 0 ; y < image.height ; y++) {
                image.imgRGB[x][y][0] = escapiPixGetRed(capture,x,y) ;
                image.imgRGB[x][y][1] = escapiPixGetGreen(capture,x,y) ;
                image.imgRGB[x][y][2] = escapiPixGetBlue(capture,x,y) ;
            }
        }
    }

    void visionPicture2ImageRGB(unsigned char* picture, Image2DRGB image) {
        for ( int x = 0 ; x < image.width ; x++) {
            for ( int y = 0 ; y < image.height ; y++) {
                image.imgRGB[x][y][0] = picture[y*image.width*4+x*4+0] ;
                image.imgRGB[x][y][1] = picture[y*image.width*4+x*4+1] ;
                image.imgRGB[x][y][2] = picture[y*image.width*4+x*4+2] ;
            }
        }
    }

    void visionCloneImageRGB(Image2DRGB img_in, Image2DRGB img_out) {
        for(int x = 0 ; x < img_in.width ; x++) {
            for(int y = 0 ; y < img_in.height ; y++) {
                img_out.imgRGB[x][y][0] = img_in.imgRGB[x][y][0] ;
                img_out.imgRGB[x][y][1] = img_in.imgRGB[x][y][1] ;
                img_out.imgRGB[x][y][2] = img_in.imgRGB[x][y][2] ;
            }
        }
    }

    void visionImageRGB2Image(Image2DRGB img_in, Image2D img_out) {
        for(int x = 0 ; x < img_in.width ; x++)
            for(int y = 0 ; y < img_in.height ; y++)
                img_out.img[x][y] = 0.21*img_in.imgRGB[x][y][0] + 0.72*img_in.imgRGB[x][y][1] + 0.07*img_in.imgRGB[x][y][2] ;

    }

    void visionImage2ImageRGB(Image2D img_in, Image2DRGB img_out) {
        for(int x = 0 ; x < img_in.width ; x++) {
            for(int y = 0 ; y < img_in.height ; y++) {
                img_out.imgRGB[x][y][0] = img_in.img[x][y] ;
                img_out.imgRGB[x][y][1] = img_in.img[x][y] ;
                img_out.imgRGB[x][y][2] = img_in.img[x][y] ;
            }
        }
    }

    void visionSusan9(Image2D img_in, Image2D img_out, int gap) {
        int *circle9 = new int[9] ; //Ce tableau contiendra les valeur h, s et v des 8 pixels autour de celui analyse
        unsigned int nb_pix, oppose ;
        unsigned int I, i ;

        for( int x = 4 ; x < img_in.width-4 ; x++) {
            for( int y = 4 ; y < img_in.height-4 ; y++) {
                nb_pix = oppose = 0 ;

                //Enregistre le point voulu en circle9[0]
                circle9[0] = img_in.img[x][y] ;

                //Cercle autour du pixel voulu (cercle9)
                circle9[1] = img_in.img[x][y-2] ;
                circle9[2] = img_in.img[x+1][y-1] ;
                circle9[3] = img_in.img[x+2][y] ;
                circle9[4] = img_in.img[x+1][y+1] ;
                circle9[5] = img_in.img[x][y+2] ;
                circle9[6] = img_in.img[x-1][y+1] ;
                circle9[7] = img_in.img[x-2][y] ;
                circle9[8] = img_in.img[x-1][y-1] ;

                //Vérification de V de chaque pixel par rapport au V de P
                for( i = 1 ; i < 9 ; i++ ) {
                    if( abs(circle9[i] - circle9[0]) < gap ){
                        circle9[i] = 1 ; //Si le pixel est similaire alors il passe a 1
                        nb_pix ++ ; //Calcule le nombre de pixels valides dans le cercle
                    } else {
                        circle9[i] = 0 ; //Si le pixel est different alors il passe a 0
                    }
                }

                //On verifie que chaque point valide sur le cercle n'ai pas d'oppose lui aussi valide
                for ( i = 1 ; i < 9 ; i++ ) {
                    I = i + (9-1)/2 ;

                    if ( I > 9-1 )
                        I -= 9-1 ;

                    //Si le pixel oppose est lui aussi valide alors on passe "i" a 30 => on ne validera pas img[x][y] comme etant un corner et donc on accelere le changement de pixel
                    if ( circle9[i] == 1 && circle9[I] == 1 )
                        i = 30 ;
                }

                if ( nb_pix < (9-1)/2 && i == 9 )
                    img_out.img[x][y] = 255 ;
                else
                    img_out.img[x][y] = 0 ;
            }
        }

        free( circle9 );
    }

    void visionSusan17(Image2D img_in, Image2D img_out, int gap) {
        int *circle17 = new int[17] ; //Ce tableau contiendra les valeur h, s et v des 16 pixels autour de celui analyse
        unsigned int nb_pix, oppose ;
        unsigned int I, i ;

        for( int x = 4 ; x < img_in.width-4 ; x++) {
            for( int y = 4 ; y < img_in.height-4 ; y++) {
                nb_pix = oppose = 0 ;

                //Enregistre le point voulu en circle17[0]
                circle17[0] = img_in.img[x][y] ;

                //Cercle autour du pixel voulu (cercle17)
                circle17[1] = img_in.img[x][y-3] ;
                circle17[2] = img_in.img[x+1][y-3] ;
                circle17[3] = img_in.img[x+2][y-2] ;
                circle17[4] = img_in.img[x+3][y-1] ;
                circle17[5] = img_in.img[x+3][y] ;
                circle17[6] = img_in.img[x+3][y+1] ;
                circle17[7] = img_in.img[x+2][y+2] ;
                circle17[8] = img_in.img[x+1][y+3] ;
                circle17[9] = img_in.img[x][y+3] ;
                circle17[10] = img_in.img[x-1][y+3] ;
                circle17[11] = img_in.img[x-2][y+2] ;
                circle17[12] = img_in.img[x-3][y+1] ;
                circle17[13] = img_in.img[x-3][y] ;
                circle17[14] = img_in.img[x-3][y-1] ;
                circle17[15] = img_in.img[x-2][y-2] ;
                circle17[16] = img_in.img[x-1][y-3] ;

                //Vérification de V de chaque pixel par rapport au V de P
                for( i = 1 ; i < 17 ; i++ ) {
                    if( abs(circle17[i] - circle17[0]) < gap ){
                        circle17[i] = 1 ; //Si le pixel est similaire alors il passe a 1
                        nb_pix ++ ; //Calcule le nombre de pixels valides dans le cercle
                    } else {
                        circle17[i] = 0 ; //Si le pixel est different alors il passe a 0
                    }
                }

                //On verifie que chaque point valide sur le cercle n'ai pas d'oppose lui aussi valide
                for ( i = 1 ; i < 17 ; i++ ) {
                    I = i + (17-1)/2 ;

                    if ( I > 17-1 )
                        I -= 17-1 ;

                    //Si le pixel oppose est lui aussi valide alors on passe "i" a 30 => on ne validera pas img[x][y] comme etant un corner et donc on accelere le changement de pixel
                    if ( circle17[i] == 1 && circle17[I] == 1 )
                        i = 30 ;
                }

                if ( nb_pix < (17-1)/2 && i == 17 )
                    img_out.img[x][y] = 255 ;
                else
                    img_out.img[x][y] = 0 ;
            }
        }

        free( circle17 );
    }

    void visionSusan21(Image2D img_in, Image2D img_out, int gap) {
        int *circle21 = new int[11] ; //Ce tableau contiendra les valeur h, s et v des 10 premiers pixels autour de celui analyse
        int nb_pix ;

        for( int x = 4 ; x < img_in.width-4 ; x++) {
            for( int y = 4 ; y < img_in.height-4 ; y++) {
                nb_pix = img_out.img[x][y] = 0 ;

                circle21[0] = img_in.img[x][y] ;

                if( abs(img_in.img[x][y-4] - circle21[0]) < gap ) { circle21[1] = 1 ; nb_pix ++ ; }
                else circle21[1] = 0 ;

                if( abs(img_in.img[x+1][y-4] - circle21[0]) < gap ) { circle21[2] = 1 ; nb_pix ++ ; }
                else circle21[2] = 0 ;

                if( abs(img_in.img[x+2][y-3] - circle21[0]) < gap ) { circle21[3] = 1 ; nb_pix ++ ; }
                else circle21[3] = 0 ;

                if( abs(img_in.img[x+3][y-2] - circle21[0]) < gap ) { circle21[4] = 1 ; nb_pix ++ ; }
                else circle21[4] = 0 ;

                if( abs(img_in.img[x+4][y-1] - circle21[0]) < gap ) { circle21[5] = 1 ; nb_pix ++ ; }
                else circle21[5] = 0 ;

                if( abs(img_in.img[x+4][y] - circle21[0]) < gap ) { circle21[6] = 1 ; nb_pix ++ ; }
                else circle21[6] = 0 ;

                if( abs(img_in.img[x+4][y+1] - circle21[0]) < gap ) { circle21[7] = 1 ; nb_pix ++ ; }
                else circle21[7] = 0 ;

                if( abs(img_in.img[x+3][y+2] - circle21[0]) < gap ) { circle21[8] = 1 ; nb_pix ++ ; }
                else circle21[8] = 0 ;

                if( abs(img_in.img[x+2][y+3] - circle21[0]) < gap ) { circle21[9] = 1 ; nb_pix ++ ; }
                else circle21[9] = 0 ;

                if( abs(img_in.img[x+1][y+4] - circle21[0]) < gap ) { circle21[10] = 1 ; nb_pix ++ ; }
                else circle21[10] = 0 ;

                if( abs(img_in.img[x][y+4] - circle21[0]) < gap ) {
                    if ( circle21[1] == 1 ) continue ;
                    nb_pix ++ ;
                }

                if( abs(img_in.img[x-1][y+4] - circle21[0]) < gap ) {
                    if ( circle21[2] == 1 ) continue ;
                    nb_pix ++ ;
                }

                if( abs(img_in.img[x-2][y+3] - circle21[0]) < gap ) {
                    if ( circle21[3] == 1 ) continue ;
                    nb_pix ++ ;
                }

                if( abs(img_in.img[x-3][y+2] - circle21[0]) < gap ) {
                    if ( circle21[4] == 1 ) continue ;
                    nb_pix ++ ;
                }

                if( abs(img_in.img[x-4][y+1] - circle21[0]) < gap ) {
                    if ( circle21[5] == 1 ) continue ;
                    nb_pix ++ ;
                }

                if( abs(img_in.img[x-4][y] - circle21[0]) < gap ) {
                    if ( circle21[6] == 1 ) continue ;
                    nb_pix ++ ;
                }

                if( abs(img_in.img[x-4][y-1] - circle21[0]) < gap ) {
                    if ( circle21[7] == 1 ) continue ;
                    nb_pix ++ ;
                }

                if( abs(img_in.img[x-3][y-2] - circle21[0]) < gap ) {
                    if ( circle21[8] == 1 ) continue ;
                    nb_pix ++ ;
                }

                if( abs(img_in.img[x-2][y-3] - circle21[0]) < gap ) {
                    if ( circle21[9] == 1 ) continue ;
                    nb_pix ++ ;
                }

                if( abs(img_in.img[x-1][y-4] - circle21[0]) < gap ) {
                    if ( circle21[10] == 1 ) continue ;
                    nb_pix ++ ;
                }

                if ( nb_pix < 10 )
                    img_out.img[x][y] = 255 ;
            }
        }

        free( circle21 );
    }

    void visionSusan21Binary(Image2D img_in, Image2D img_out) {
        int *circle21 = new int[11] ; //Ce tableau contiendra les valeur h, s et v des 10 premiers pixels autour de celui analyse
        int nb_pix ;

        for( int x = 4 ; x < img_in.width-4 ; x++) {
            for( int y = 4 ; y < img_in.height-4 ; y++) {
                nb_pix = img_out.img[x][y] = 0 ;

                circle21[0] = img_in.img[x][y] ;

                if( img_in.img[x][y-4] - circle21[0] != 0 ) { circle21[1] = 1 ; nb_pix ++ ; }
                else circle21[1] = 0 ;

                if( img_in.img[x+1][y-4] - circle21[0] != 0 ) { circle21[2] = 1 ; nb_pix ++ ; }
                else circle21[2] = 0 ;

                if( img_in.img[x+2][y-3] - circle21[0] != 0 ) { circle21[3] = 1 ; nb_pix ++ ; }
                else circle21[3] = 0 ;

                if( img_in.img[x+3][y-2] - circle21[0] != 0 ) { circle21[4] = 1 ; nb_pix ++ ; }
                else circle21[4] = 0 ;

                if( img_in.img[x+4][y-1] - circle21[0] != 0 ) { circle21[5] = 1 ; nb_pix ++ ; }
                else circle21[5] = 0 ;

                if( img_in.img[x+4][y] - circle21[0] != 0 ) { circle21[6] = 1 ; nb_pix ++ ; }
                else circle21[6] = 0 ;

                if( img_in.img[x+4][y+1] - circle21[0] != 0 ) { circle21[7] = 1 ; nb_pix ++ ; }
                else circle21[7] = 0 ;

                if( img_in.img[x+3][y+2] - circle21[0] != 0 ) { circle21[8] = 1 ; nb_pix ++ ; }
                else circle21[8] = 0 ;

                if( img_in.img[x+2][y+3] - circle21[0] != 0 ) { circle21[9] = 1 ; nb_pix ++ ; }
                else circle21[9] = 0 ;

                if( img_in.img[x+1][y+4] - circle21[0] != 0 ) { circle21[10] = 1 ; nb_pix ++ ; }
                else circle21[10] = 0 ;

                if( img_in.img[x][y+4] - circle21[0] != 0 ) {
                    if ( circle21[1] == 1 ) continue ;
                    nb_pix ++ ;
                }

                if( img_in.img[x-1][y+4] - circle21[0] != 0 ) {
                    if ( circle21[2] == 1 ) continue ;
                    nb_pix ++ ;
                }

                if( img_in.img[x-2][y+3] - circle21[0] != 0 ) {
                    if ( circle21[3] == 1 ) continue ;
                    nb_pix ++ ;
                }

                if( img_in.img[x-3][y+2] - circle21[0] != 0 ) {
                    if ( circle21[4] == 1 ) continue ;
                    nb_pix ++ ;
                }

                if( img_in.img[x-4][y+1] - circle21[0] != 0 ) {
                    if ( circle21[5] == 1 ) continue ;
                    nb_pix ++ ;
                }

                if( img_in.img[x-4][y] - circle21[0] != 0 ) {
                    if ( circle21[6] == 1 ) continue ;
                    nb_pix ++ ;
                }

                if( img_in.img[x-4][y-1] - circle21[0] != 0 ) {
                    if ( circle21[7] == 1 ) continue ;
                    nb_pix ++ ;
                }

                if( img_in.img[x-3][y-2] - circle21[0] != 0 ) {
                    if ( circle21[8] == 1 ) continue ;
                    nb_pix ++ ;
                }

                if( img_in.img[x-2][y-3] - circle21[0] != 0 ) {
                    if ( circle21[9] == 1 ) continue ;
                    nb_pix ++ ;
                }

                if( img_in.img[x-1][y-4] - circle21[0] != 0 ) {
                    if ( circle21[10] == 1 ) continue ;
                    nb_pix ++ ;
                }

                if ( nb_pix < 10 )
                    img_out.img[x][y] = 255 ;
            }
        }

        free( circle21 );
    }

    //Original rgb2hsv code :
    //http://lolengine.net/blog/2013/01/13/7-rgb-to-hsv
    void rgb2hsv(float r, float g, float b, int &h, int &s , int &v) {
        float K = 0.f;

        if( g < b ) {
            std::swap(g,b);
            K = -1.f;
        }

        if( r < g ) {
            std::swap(r,g);
            K = -2.f / 6.f - K;
        }

        float chroma = r - std::min(g,b);
        h = (fabs(K + (g - b) / (6.f * chroma + 1e-20f)))*360;
        s = (chroma / (r + 1e-20f))*100;
        v = (MAX(r, MAX(g,b)))*100/255;
    }

    void hsv2rgb(float h, float s, float v, int &r, int &g, int &b) {
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

    float rgb2h(float r, float g, float b) {
        float K = 0.f;

        if( g < b ) {
            std::swap(g,b);
            K = -1.f;
        }

        if( r < g ) {
            std::swap(r,g);
            K = -2.f / 6.f - K;
        }

        float chroma = r - std::min(g,b);
        return ((fabs(K + (g - b) / (6.f * chroma + 1e-20f)))*360) ;
    }

    //dessine le segment entre deux points
    void ligne( Image2D img, int xi, int yi, int xf, int yf, int color) {
        int dx, dy, i, xinc, yinc, cumul, x, y ;

        x = xi ;
        y = yi ;
        dx = xf - xi ;
        dy = yf - yi ;
        xinc = ( dx > 0 ) ? 1 : -1 ;
        yinc = ( dy > 0 ) ? 1 : -1 ;
        dx = abs(dx) ;
        dy = abs(dy) ;

        if( x >= 0 && x < img.width && y >= 0 && y < img.height )
            img.img[x][y] = color ;

        if ( dx > dy ) {
            cumul = dx / 2 ;
            for ( i = 1 ; i <= dx ; i++ ) {
                x += xinc ;
                cumul += dy ;
                if ( cumul >= dx ) {
                    cumul -= dx ;
                    y += yinc ;
                }
                if( x >= 0 && x < img.width && y >= 0 && y < img.height )
                    img.img[x][y] = color ;
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
                if( x >= 0 && x < img.width && y >= 0 && y < img.height )
                    img.img[x][y] = color ;
            }
        }
    }

    //dessine un cercle ou carre autour des points interessants
    void drawpoint( Image2D img_out, int x, int y, int type, int R, int color) {
        int i, j ;

        if ( type == 0 ) {
            //dessine un carre vide
            for( i = -R ; i <= R ; i++){
                for( j = -R ; j <= R ; j++){
                    if( x + i >= 0 && x + i < img_out.width && y + j >= 0 && y + j < img_out.height ){
                        if( abs(i) == R || abs(j) == R ){
                            img_out.img[x + i][y + j] = color ;
                        }
                    }
                }
            }
        } else if ( type == 1 ) {
            //dessine un carre plein
            for( i = -R ; i <= R ; i++){
                for( j = -R ; j <= R ; j++){
                    if( x + i >= 0 && x + i < img_out.width && y + j >= 0 && y + j < img_out.height ){
                        img_out.img[x + i][y + j] = color ;
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
                if( x + i >= 0 && x + i < img_out.width && y + j >= 0 && y + j < img_out.height )
                    img_out.img[x + i][y + j] = color ;

                if( x + j >= 0 && x + j < img_out.width && y + i >= 0 && y + i < img_out.height )
                    img_out.img[x + j][y + i] = color ;

                if( x - i >= 0 && x - i < img_out.width && y + j >= 0 && y + j < img_out.height )
                    img_out.img[x - i][y + j] = color ;

                if( x - j >= 0 && x - j < img_out.width && y + i >= 0 && y + i < img_out.height )
                    img_out.img[x - j][y + i] = color ;

                if( x + i >= 0 && x + i < img_out.width && y - j >= 0 && y - j < img_out.height )
                    img_out.img[x + i][y - j] = color ;

                if( x + j >= 0 && x + j < img_out.width && y - i >= 0 && y - i < img_out.height )
                    img_out.img[x + j][y - i] = color ;

                if( x - i >= 0 && x - i < img_out.width && y - j >= 0 && y - j < img_out.height )
                    img_out.img[x - i][y - j] = color ;

                if( x - j >= 0 && x - j < img_out.width && y - i >= 0 && y - i < img_out.height )
                    img_out.img[x - j][y - i] = color ;

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
                    if( x + i >= 0 && x + i < img_out.width && y + j >= 0 && y + j < img_out.height )
                        img_out.img[x + i][y + j] = color ;

                    if( x + j >= 0 && x + j < img_out.width && y + i >= 0 && y + i < img_out.height )
                        img_out.img[x + j][y + i] = color ;

                    if( x - i >= 0 && x - i < img_out.width && y + j >= 0 && y + j < img_out.height )
                        img_out.img[x - i][y + j] = color ;

                    if( x - j >= 0 && x - j < img_out.width && y + i >= 0 && y + i < img_out.height )
                        img_out.img[x - j][y + i] = color ;

                    if( x + i >= 0 && x + i < img_out.width && y - j >= 0 && y - j < img_out.height )
                        img_out.img[x + i][y - j] = color ;

                    if( x + j >= 0 && x + j < img_out.width && y - i >= 0 && y - i < img_out.height )
                        img_out.img[x + j][y - i] = color ;

                    if( x - i >= 0 && x - i < img_out.width && y - j >= 0 && y - j < img_out.height )
                        img_out.img[x - i][y - j] = color ;

                    if( x - j >= 0 && x - j < img_out.width && y - i >= 0 && y - i < img_out.height )
                        img_out.img[x - j][y - i] = color ;

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
        }
    }

    void visionBin(Image2D img_in, Image2D img_out, int threshold) {
        for( int x = 0 ; x < img_in.width ; x++ ) {
            for( int y = 0 ; y < img_in.height ; y++ ) {
                img_out.img[x][y] = 255 ;
                if( img_in.img[x][y] < threshold )
                    img_out.img[x][y] = 0 ;
            }
        }
    }

    void visionBinFindH(Image2DRGB img_in, Image2D img_out, int H, int threshold) {
        for( int x = 0 ; x < img_in.width ; x++ ) {
            for( int y = 0 ; y < img_in.height ; y++ ) {
                img_out.img[x][y] = 255 ;
                if( abs(rgb2h(img_in.imgRGB[x][y][0], img_in.imgRGB[x][y][1], img_in.imgRGB[x][y][2]) - H) < threshold ) {
                    img_out.img[x][y] = 0 ;
                }
            }
        }
    }

    void visionExcludeH(Image2DRGB img_in, Image2D img_out, int H, int threshold) {
        for( int x = 0 ; x < img_in.width ; x++ ) {
            for( int y = 0 ; y < img_in.height ; y++ ) {
                img_out.img[x][y] =  0.21*img_in.imgRGB[x][y][0] + 0.72*img_in.imgRGB[x][y][1] + 0.07*img_in.imgRGB[x][y][2] ;
                if( abs(rgb2h(img_in.imgRGB[x][y][0], img_in.imgRGB[x][y][1], img_in.imgRGB[x][y][2]) - H) < threshold ) {
                    img_out.img[x][y] = 255 ;
                }
            }
        }
    }


    void visionBlurPix(Image2D img_in, Image2D img_out, int x, int y, int R) {
        int  p = 0 ;
        img_out.img[x][y] = 0 ;

        for( int i = -R ; i <= R ; i++ ) {
            for( int j = -R ; j <= R ; j++ ) {
                if( x + i >= 0 && x + i < img_in.width && y + j >= 0 && y + j < img_in.height ) {
                    img_out.img[x][y] += img_in.img[x+i][y+j] ;
                    p++ ;
                }
            }
        }

        img_out.img[x][y] /= p ;
    }

    void visionBlurImage(Image2D img_in, Image2D img_out, int type, int R) {
        int n = 0 ;

        if ( type == 0 ) {
            for ( int x = 0 ; x < img_in.width ; x++ )
                for ( int y = 0 ; y < img_in.height ; y++ )
                    visionBlurPix(img_in, img_out, x, y, R) ;
        } else {
            Image2D inter ;
            visionImageAlloc(inter, img_in.width, img_in.height) ;

            visionCloneImage(img_in, inter) ;

            while ( n < R ) {
                n++ ;

                if ( n & 1 ) {
                    for ( int x = 0 ; x < img_in.width ; x++ )
                        for ( int y = 0 ; y < img_in.height ; y++ )
                            visionBlurPix(inter, img_out, x, y, n) ;
                }
                else {
                    for ( int x = 0 ; x < img_in.width ; x++ )
                        for ( int y = 0 ; y < img_in.height ; y++ )
                            visionBlurPix(img_out, inter, x, y, n) ;
                }
            }

            if ( (n-1) & 1 )
                visionCloneImage(inter, img_out) ;

            visionImageFree(inter) ;
        }
    }

    void visionDog2(Image2D img_in, Image2D img_out) {
        int x, y ;
        int i, j ;
        int total, p, R, max ;
        float D1, D2, z ;

        max = R = 1 ;

        for( x = 0 ; x < img_in.width ; x++) {
            for( y = 0 ; y < img_in.height ; y++) {
                total = p = 0 ;
                for( i = -R ; i <= R ; i++){
                    for( j = -R ; j <= R ; j++){
                        if(x + i >= 0 && x + i < img_in.width && y + j >= 0 && y + j < img_in.height){
                            z = abs(img_in.img[x][y] - img_in.img[x + i][y + j]) ;
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

                img_out.img[x][y] = total ;
            }
        }

        //Augmente le contraste du masque pour plus de lisibilite
        for( x = 0 ; x < img_in.width ; x++) {
            for( y = 0 ; y < img_in.height ; y++) {
                //On fait un petit tri pour supprimer les points "faibles"
                p = ((img_out.img[x][y] * 255 ) / max) * 3 ;
                if( p > 255 ) {
                    img_out.img[x][y] = 255 ;
                } else if( p > 0 ) {
                    img_out.img[x][y] = p ;
                } else {
                    img_out.img[x][y] = 0 ;
                }
            }
        }
    }

    void visionMatrice(Image2D img_in, Image2D img_out, int divider, float A, float B, float C, float D, float E, float F, float G, float H, float I) {
        for ( int x = 1 ; x < img_in.width-1 ; x++ ) {
            for ( int y = 1 ; y < img_in.height-1 ; y++ ) {
                img_out.img[x][y] = (img_in.img[x-1][y-1]*A    + img_in.img[x][y-1]*B    + img_in.img[x+1][y-1]*C  +
                                    img_in.img[x-1][y]*D      + img_in.img[x][y]*E      + img_in.img[x+1][y]*F    +
                                    img_in.img[x-1][y+1]*G    + img_in.img[x][y+1]*H    + img_in.img[x+1][y+1]*I) / divider   ;

                if ( img_out.img[x][y] > 255 )
                    img_out.img[x][y] = 255 ;

                if ( img_out.img[x][y] < 0 )
                    img_out.img[x][y] = 0 ;
            }
        }
    }

    void visionGaussian5(Image2D img_in, Image2D img_out) {
        for ( int x = 3 ; x < img_in.width-3 ; x++ ) {
            for ( int y = 3 ; y < img_in.height-3 ; y++ ) {
                img_out.img[x][y] = (img_in.img[x-2][y-2]*2 + img_in.img[x-1][y-2]*4    + img_in.img[x][y-2]*5  + img_in.img[x+1][y-2]*4    + img_in.img[x+2][y-2]*2  +
                                    img_in.img[x-2][y-1]*4  + img_in.img[x-1][y-1]*9    + img_in.img[x][y-1]*12 + img_in.img[x+1][y-1]*9    + img_in.img[x+2][y-1]*4    +
                                    img_in.img[x-2][y]*5    + img_in.img[x-1][y]*12     + img_in.img[x][y]*15   + img_in.img[x+1][y]*12     + img_in.img[x+2][y]*5    +
                                    img_in.img[x-2][y+1]*4  + img_in.img[x-1][y+1]*9    + img_in.img[x][y+1]*12 + img_in.img[x+1][y+1]*9    + img_in.img[x+2][y+1]*4    +
                                    img_in.img[x-2][y+2]*2  + img_in.img[x-1][y+2]*4    + img_in.img[x][y+2]*5  + img_in.img[x+1][y+2]*4    + img_in.img[x+2][y+2]*2) / 159   ;

                if ( img_out.img[x][y] > 255 )
                    img_out.img[x][y] = 255 ;

                if ( img_out.img[x][y] < 0 )
                    img_out.img[x][y] = 0 ;
            }
        }
    }

    void visionGaussian3(Image2D img_in, Image2D img_out) {
        for ( int x = 1 ; x < img_in.width-1 ; x++ ) {
            for ( int y = 1 ; y < img_in.height-1 ; y++ ) {
                img_out.img[x][y] = (img_in.img[x-1][y-1]*1 + img_in.img[x][y-1]*2 + img_in.img[x+1][y-1]*1 +
                                     img_in.img[x-1][y]*2   + img_in.img[x][y]*4   + img_in.img[x+1][y]*2   +
                                     img_in.img[x-1][y+1]*1 + img_in.img[x][y+1]*2 + img_in.img[x+1][y+1]*1 ) / 16   ;

                if ( img_out.img[x][y] > 255 )
                    img_out.img[x][y] = 255 ;

                if ( img_out.img[x][y] < 0 )
                    img_out.img[x][y] = 0 ;
            }
        }
    }

    int visionGaussian3pix(Image2D img_in, int x, int y) {
        int i  = (img_in.img[x-1][y-1]*1 + img_in.img[x][y-1]*2 + img_in.img[x+1][y-1]*1 +
                  img_in.img[x-1][y]*2   + img_in.img[x][y]*4   + img_in.img[x+1][y]*2   +
                  img_in.img[x-1][y+1]*1 + img_in.img[x][y+1]*2 + img_in.img[x+1][y+1]*1 ) / 16   ;

        if ( i > 255 )
            i = 255 ;

        if ( i< 0 )
            i = 0 ;

        return i ;
    }

    void visionMatriceConv(Image2D img_in, Image2D img_out, int divider, int divider2, float A, float B, float C, float D, float E, float F, float G, float H, float I, float A2, float B2, float C2, float D2, float E2, float F2, float G2, float H2, float I2) {
        int a, b ;
        for ( int x = 1 ; x < img_in.width-1 ; x++ ) {
            for ( int y = 1 ; y < img_in.height-1 ; y++ ) {
                a = (img_in.img[x-1][y-1]*A    + img_in.img[x][y-1]*B    + img_in.img[x+1][y-1]*C  +
                     img_in.img[x-1][y]*D      + img_in.img[x][y]*E      + img_in.img[x+1][y]*F    +
                     img_in.img[x-1][y+1]*G    + img_in.img[x][y+1]*H    + img_in.img[x+1][y+1]*I) / divider   ;

                b = (img_in.img[x-1][y-1]*A2    + img_in.img[x][y-1]*B2    + img_in.img[x+1][y-1]*C2  +
                     img_in.img[x-1][y]*D2      + img_in.img[x][y]*E2      + img_in.img[x+1][y]*F2    +
                     img_in.img[x-1][y+1]*G2    + img_in.img[x][y+1]*H2    + img_in.img[x+1][y+1]*I2) / divider2   ;

                img_out.img[x][y] = sqrt( a*a + b*b ) ;

                if ( img_out.img[x][y] > 255 )
                    img_out.img[x][y] = 255 ;

                if ( img_out.img[x][y] < 0 )
                    img_out.img[x][y] = 0 ;
            }
        }
    }

    void visionCanny(Image2D img_in, Image2D img_out, int gap_min, int gap_max) {
        int a, b ;
        int prec ;

        Image2D inter, inter2 ;
        visionImageAlloc(inter, img_in.width, img_in.height) ;
        visionImageAlloc(inter2, img_in.width, img_in.height) ;

        for ( int x = 1 ; x < img_in.width-1 ; x++ ) {
            for ( int y = 1 ; y < img_in.height-1 ; y++ ) {
                a = (img_in.img[x-1][y-1]*(-1)    + img_in.img[x][y-1]*0    + img_in.img[x+1][y-1]*1  +
                     img_in.img[x-1][y]*(-2)   + img_in.img[x][y]*0      + img_in.img[x+1][y]*2    +
                     img_in.img[x-1][y+1]*(-1)    + img_in.img[x][y+1]*0    + img_in.img[x+1][y+1]*1) ;

                b = (img_in.img[x-1][y-1]*(-1)    + img_in.img[x][y-1]*(-2)    + img_in.img[x+1][y-1]*(-1)  +
                     img_in.img[x-1][y]*0      + img_in.img[x][y]*0      + img_in.img[x+1][y]*0    +
                     img_in.img[x-1][y+1]*1    + img_in.img[x][y+1]*2 + img_in.img[x+1][y+1]*1) ;

                inter.img[x][y] = sqrt( a*a + b*b ) ;

                inter2.img[x][y] = atan2(b,a) ;

                img_out.img[x][y] = 0 ;
            }
        }

        for ( int x = 1 ; x < img_in.width-1 ; x++ ) {
            for ( int y = 1 ; y < img_in.height-1 ; y++ ) {
                if ( inter.img[x-1][y-1] == 255 || inter.img[x-1][y] == 255 || inter.img[x-1][y+1] == 255 || inter.img[x][y-1] == 255 || inter.img[x][y+1] == 255 || inter.img[x+1][y-1] == 255 || inter.img[x+1][y] == 255 || inter.img[x+1][y+1] == 255 )
                    prec = 1 ;
                else
                    prec = 0 ;

                if ( (inter2.img[x][y] > -pi18 && inter2.img[x][y] < pi18) || (inter2.img[x][y] > pi78 && inter2.img[x][y] < -pi78) ) { //Si autour de 0-180 deg
                    if ( inter.img[x][y] > inter.img[x-1][y] && inter.img[x][y] > inter.img[x+1][y] ) {
                        if ( inter.img[x][y] > gap_max )
                            img_out.img[x][y] = 255 ;
                        else if ( inter.img[x][y] > gap_min && inter.img[x][y] <= gap_max && prec ) {
                                img_out.img[x][y] = 128 ;
                        }
                    }
                } else if ( (inter2.img[x][y] > pi18 && inter2.img[x][y] < pi38) || (inter2.img[x][y] > -pi78 && inter2.img[x][y] < -pi58) ) { //Si autour de 45 deg
                    if ( inter.img[x][y] > inter.img[x-1][y+1] && inter.img[x][y] > inter.img[x+1][y-1] ) {
                        if ( inter.img[x][y] > gap_max )
                            img_out.img[x][y] = 255 ;
                        else if ( inter.img[x][y] > gap_min && inter.img[x][y] <= gap_max && prec ) {
                                img_out.img[x][y] = 128 ;
                        }
                    }
                } else if ( (inter2.img[x][y] > pi38 && inter2.img[x][y] < pi58) || (inter2.img[x][y] > -pi58 && inter2.img[x][y] < -pi38) ) { //Si autour de 90 deg
                    if ( inter.img[x][y] > inter.img[x][y+1] && inter.img[x][y] > inter.img[x][y-1] ) {
                        if ( inter.img[x][y] > gap_max )
                            img_out.img[x][y] = 255 ;
                        else if ( inter.img[x][y] > gap_min && inter.img[x][y] <= gap_max && prec ) {
                                img_out.img[x][y] = 128 ;
                        }
                    }
                } else if ( (inter2.img[x][y] > pi58 && inter2.img[x][y] < pi78) || (inter2.img[x][y] > -pi38 && inter2.img[x][y] < -pi18) ) { //Si autour de 135 deg
                    if ( inter.img[x][y] > inter.img[x+1][y+1] && inter.img[x][y] > inter.img[x-1][y-1] ) {
                        if ( inter.img[x][y] > gap_max )
                            img_out.img[x][y] = 255 ;
                        else if ( inter.img[x][y] > gap_min && inter.img[x][y] <= gap_max && prec ) {
                                img_out.img[x][y] = 128 ;
                        }
                    }
                }
            }
        }

        visionImageFree(inter) ;
        visionImageFree(inter2) ;
    }

    void visionMedian(Image2D img_in, Image2D img_out) {
        int k, p ;
        int tab[9] ;
        for ( int x = 1 ; x < img_in.width-1 ; x++ ) {
            for ( int y = 1 ; y < img_in.height-1 ; y++ ) {
                tab[0] = img_in.img[x-1][y-1] ;
                tab[1] = img_in.img[x][y-1] ;
                tab[2] = img_in.img[x+1][y-1] ;
                tab[3] = img_in.img[x-1][y] ;
                tab[4] = img_in.img[x][y] ;
                tab[5] = img_in.img[x+1][y] ;
                tab[6] = img_in.img[x-1][y+1] ;
                tab[7] = img_in.img[x][y+1] ;
                tab[8] = img_in.img[x+1][y+1] ;

                k = 0 ;
                while ( k < 9 ) {
                    if ( tab[k] > tab[k+1] ) {
                        p = tab[k] ;
                        tab[k] = tab[k+1] ;
                        tab[k+1] = p ;
                        k = 0 ;
                    } else
                    k++ ;
                }

                img_out.img[x][y] = tab[4] ;
            }
        }
    }

    void visionSupprMinMax(Image2D image, int gap_min, int gap_max, int bin) {
        for ( int x = 0 ; x < image.width ; x++ ) {
            for ( int y = 0 ; y < image.height ; y++ ) {
                if ( image.img[x][y] < gap_min || image.img[x][y] > gap_max ) {
                    image.img[x][y] = 0 ;
                } else {
                    if ( bin == 1 ) {
                        image.img[x][y] = 255 ;
                    }
                }
            }
        }
    }

    void visionCannyNew(Image2D img_in, Image2D img_out, int min_threshold, int max_threshold) {
        //le gradient varie entre [1;360]
        int a, b ;

        Image2DRGB inter ;
        visionImageAllocRGB(inter, img_in.width, img_in.height) ;

        for ( int x = 1 ; x < img_in.width-1 ; x++ ) {
            for ( int y = 1 ; y < img_in.height-1 ; y++ ) {
                a = (img_in.img[x-1][y-1]*(-1)    + img_in.img[x][y-1]*0    + img_in.img[x+1][y-1]*1  +
                     img_in.img[x-1][y]*(-2)   + img_in.img[x][y]*0      + img_in.img[x+1][y]*2    +
                     img_in.img[x-1][y+1]*(-1)    + img_in.img[x][y+1]*0    + img_in.img[x+1][y+1]*1) ;

                b = (img_in.img[x-1][y-1]*(-1)    + img_in.img[x][y-1]*(-2)    + img_in.img[x+1][y-1]*(-1)  +
                     img_in.img[x-1][y]*0      + img_in.img[x][y]*0      + img_in.img[x+1][y]*0    +
                     img_in.img[x-1][y+1]*1    + img_in.img[x][y+1]*2 + img_in.img[x+1][y+1]*1) ;

                inter.imgRGB[x][y][0] = sqrt( a*a + b*b ) ;

                inter.imgRGB[x][y][1]  = atan2(b,a) * 180/M_PI ;
                if( inter.imgRGB[x][y][1] < 0 )
                    inter.imgRGB[x][y][1] += 180 ;

                inter.imgRGB[x][y][2] = 0 ;

                img_out.img[x][y] = 0 ;
            }
        }

        for ( int x = 1 ; x < img_in.width-1 ; x++ ) {
            for ( int y = 1 ; y < img_in.height-1 ; y++ ) {
                if( inter.imgRGB[x][y][1] >= 0 && inter.imgRGB[x][y][1] < 22.5 ) {
                    if ( inter.imgRGB[x][y][0] > inter.imgRGB[x-1][y][0] && inter.imgRGB[x][y][0] > inter.imgRGB[x+1][y][0] ) {
                        if( inter.imgRGB[x][y][0] >= max_threshold ) {
                            img_out.img[x][y] = 255 ;
                        } else if( inter.imgRGB[x][y][0] < max_threshold && inter.imgRGB[x][y][0] >= min_threshold ){
                            img_out.img[x][y] = 100 ;
                        }
                    }
                }

                if( inter.imgRGB[x][y][1] >= 22.5 && inter.imgRGB[x][y][1] < 67.5 ) {
                    if ( inter.imgRGB[x][y][0] > inter.imgRGB[x-1][y+1][0] && inter.imgRGB[x][y][0] > inter.imgRGB[x+1][y-1][0] ) {
                        if( inter.imgRGB[x][y][0] >= max_threshold ) {
                            img_out.img[x][y] = 255 ;
                        } else if( inter.imgRGB[x][y][0] < max_threshold && inter.imgRGB[x][y][0] >= min_threshold ){
                            img_out.img[x][y] = 100 ;
                        }
                    }
                }

                if( inter.imgRGB[x][y][1] >= 67.5 && inter.imgRGB[x][y][1] < 112.5 ) {
                    if ( inter.imgRGB[x][y][0] > inter.imgRGB[x][y+1][0] && inter.imgRGB[x][y][0] > inter.imgRGB[x][y-1][0] ) {
                        if( inter.imgRGB[x][y][0] >= max_threshold ) {
                            img_out.img[x][y] = 255 ;
                        } else if( inter.imgRGB[x][y][0] < max_threshold && inter.imgRGB[x][y][0] >= min_threshold ){
                            img_out.img[x][y] = 100 ;
                        }
                    }
                }

                if( inter.imgRGB[x][y][1] >= 112.5 && inter.imgRGB[x][y][1] < 157.5 ) {
                    if ( inter.imgRGB[x][y][0] > inter.imgRGB[x-1][y-1][0] && inter.imgRGB[x][y][0] > inter.imgRGB[x+1][y+1][0] ) {
                        if( inter.imgRGB[x][y][0] >= max_threshold ) {
                            img_out.img[x][y] = 255 ;
                        } else if( inter.imgRGB[x][y][0] < max_threshold && inter.imgRGB[x][y][0] >= min_threshold ){
                            img_out.img[x][y] = 100 ;
                        }
                    }
                }

                if( inter.imgRGB[x][y][1] >= 157.5 && inter.imgRGB[x][y][1] <= 180 ) {
                    if ( inter.imgRGB[x][y][0] > inter.imgRGB[x-1][y][0] && inter.imgRGB[x][y][0] > inter.imgRGB[x+1][y][0] ) {
                        if( inter.imgRGB[x][y][0] >= max_threshold ) {
                            img_out.img[x][y] = 255 ;
                        } else if( inter.imgRGB[x][y][0] < max_threshold && inter.imgRGB[x][y][0] >= min_threshold ){
                            img_out.img[x][y] = 100 ;
                        }
                    }
                }
            }
        }

        int again = 1 ;
        int timee = 0 ;
        //while( again != 0 ) {
            again = 0 ;
            timee++ ;
            for ( int x = 1 ; x < img_in.width-1 ; x++ ) {
                for ( int y = 1 ; y < img_in.height-1 ; y++ ) {
                    if( img_out.img[x][y] == 100 ) {
                        if( img_out.img[x-1][y] == 255 || img_out.img[x+1][y] == 255 || img_out.img[x][y-1] == 255 || img_out.img[x][y+1] == 255 || img_out.img[x+1][y+1] == 255 || img_out.img[x+1][y-1] == 255 || img_out.img[x-1][y+1] == 255 || img_out.img[x-1][y-1] == 255 ) {
                            again++ ;
                            img_out.img[x][y] = 150 ;
                        }
                    }
                }
            }
            //printf("time = %d\n", timee) ;
        //}

        visionImageFreeRGB(inter) ;
    }

    void vision2Susan17(Image2D img_in, Image2D img_out, int gap) {
        int *circle17 = new int[17] ; //Ce tableau contiendra les valeur h, s et v des 16 pixels autour de celui analyse
        unsigned int nb_pix, oppose ;
        unsigned int i ;

        for( int x = 4 ; x < img_in.width-4 ; x++) {
            for( int y = 4 ; y < img_in.height-4 ; y++) {
                nb_pix = oppose = 0 ;

                //Enregistre le point voulu en circle17[0]
                circle17[0] = img_in.img[x][y] ;

                //Cercle autour du pixel voulu (cercle17)
                circle17[1] = img_in.img[x][y-3] ;
                circle17[2] = img_in.img[x+1][y-3] ;
                circle17[3] = img_in.img[x+2][y-2] ;
                circle17[4] = img_in.img[x+3][y-1] ;
                circle17[5] = img_in.img[x+3][y] ;
                circle17[6] = img_in.img[x+3][y+1] ;
                circle17[7] = img_in.img[x+2][y+2] ;
                circle17[8] = img_in.img[x+1][y+3] ;
                circle17[9] = img_in.img[x][y+3] ;
                circle17[10] = img_in.img[x-1][y+3] ;
                circle17[11] = img_in.img[x-2][y+2] ;
                circle17[12] = img_in.img[x-3][y+1] ;
                circle17[13] = img_in.img[x-3][y] ;
                circle17[14] = img_in.img[x-3][y-1] ;
                circle17[15] = img_in.img[x-2][y-2] ;
                circle17[16] = img_in.img[x-1][y-3] ;

                //Vérification de V de chaque pixel par rapport au V de P
                for( i = 1 ; i < 17 ; i++ ) {
                    if( abs(circle17[i] - circle17[0]) <= gap ){
                        circle17[i] = 1 ; //Si le pixel est similaire alors il passe a 1
                        nb_pix ++ ; //Calcule le nombre de pixels valides dans le cercle
                    } else {
                        circle17[i] = 0 ; //Si le pixel est different alors il passe a 0
                    }
                }

                if ( nb_pix < 8 && nb_pix != 0 )
                    img_out.img[x][y] = (8 - nb_pix)*30 ;
                else
                    img_out.img[x][y] = 0 ;
            }
        }

        free( circle17 );
    }

    void visionFast(Image2D img_in, Image2D img_out, int threshold, int max_fast) {
        int *fast16 = new int[16] ;
        int neg, pos ;
        int valid, suite ;

        for ( int x = 3 ; x < img_in.width-3 ; x++ ) {
            for ( int y = 3 ; y < img_in.height-3 ; y++ ) {
                img_out.img[x][y] = img_in.img[x][y]/3 ;
                valid = 0 ;

                if ( abs(img_in.img[x-3][y]-img_in.img[x][y]) > threshold )
                    valid ++ ;
                if ( abs(img_in.img[x+3][y]-img_in.img[x][y]) > threshold )
                    valid ++ ;
                if ( abs(img_in.img[x][y-3]-img_in.img[x][y]) > threshold )
                    valid ++ ;
                if ( abs(img_in.img[x][y+3]-img_in.img[x][y]) > threshold )
                    valid ++ ;

                if ( valid >= 3 ) {
                    fast16[0] = img_in.img[x][y-3]-img_in.img[x][y] ;
                    fast16[1] = img_in.img[x+1][y-3]-img_in.img[x][y] ;
                    fast16[2] = img_in.img[x+2][y-2]-img_in.img[x][y] ;
                    fast16[3] = img_in.img[x+3][y-1]-img_in.img[x][y] ;
                    fast16[4] = img_in.img[x+3][y]-img_in.img[x][y] ;
                    fast16[5] = img_in.img[x+3][y+1]-img_in.img[x][y] ;
                    fast16[6] = img_in.img[x+2][y+2]-img_in.img[x][y] ;
                    fast16[7] = img_in.img[x+1][y+3]-img_in.img[x][y] ;
                    fast16[8] = img_in.img[x][y+3]-img_in.img[x][y] ;
                    fast16[9] = img_in.img[x-1][y+3]-img_in.img[x][y] ;
                    fast16[10] = img_in.img[x-2][y+2]-img_in.img[x][y] ;
                    fast16[11] = img_in.img[x-3][y+1]-img_in.img[x][y] ;
                    fast16[12] = img_in.img[x-3][y]-img_in.img[x][y] ;
                    fast16[13] = img_in.img[x-3][y-1]-img_in.img[x][y] ;
                    fast16[14] = img_in.img[x-2][y-2]-img_in.img[x][y] ;
                    fast16[15] = img_in.img[x-1][y-3]-img_in.img[x][y] ;

                    neg = pos = 0 ;

                    for( int i = 0; i < 16 ; i++ ) {
                        if( fast16[i] < -threshold )
                            neg++ ;

                        if( fast16[i] > threshold )
                            pos++ ;
                    }

                    suite = 0 ;

                    if( neg > max_fast && neg < 16 ) {
                        for ( int i = 0 ; i < 16 ; i++ ) {
                            if( fast16[i] < -threshold ) {
                                suite ++ ;
                                if( suite > max_fast )
                                    i = 17 ;
                            }
                            else
                                suite = 0 ;
                        }

                        if ( suite > 0 ){
                            for ( int i = 0 ; i < 16 ; i++ ) {
                                if( fast16[i] < -threshold ) {
                                    suite ++ ;
                                    if( suite > max_fast )
                                        i = 17 ;
                                }
                                else
                                    suite = 0 ;
                            }
                        }

                        if( suite > max_fast )
                            img_out.img[x][y] = 200 ;
                    } else if( pos > max_fast && pos < 16 ) {
                        for ( int i = 0 ; i < 16 ; i++ ) {
                            if( fast16[i] > threshold ) {
                                suite ++ ;
                                if( suite > max_fast )
                                    i = 17 ;
                            }
                            else
                                suite = 0 ;
                        }

                        if ( suite > 0 ){
                            for ( int i = 0 ; i < 16 ; i++ ) {
                                if( fast16[i] > threshold ) {
                                    suite ++ ;
                                    if( suite > max_fast )
                                        i = 17 ;
                                }
                                else
                                    suite = 0 ;
                            }
                        }

                        if( suite > max_fast )
                            img_out.img[x][y] = 200 ;
                    }
                }
            }
        }

        free( fast16 );
    }

    void visionKeypointDetector(Image2D img_in, Image2D img_out, int threshold, int N) {
        int d, i, j ;
        int counter ;
        int surf ;

        for( int x = 0 ; x < img_in.width ; x++ ) {
            for( int y = 1 ; y < img_in.height ; y++ ) {
                img_out.img[x][y] = 0 ;
            }
        }

        //Base sur l'algorithme de cercle d'Andres
        for( int x = N ; x < img_in.width-N ; x++ ) {
            for( int y = N ; y < img_in.height-N ; y++ ) {
                counter = 0 ;
                surf = 0 ;
                i = 0 ;
                j = N ;
                d = N-1 ;

                while ( j >= i ) {
                    surf += 8 ;

                    if( x + i >= 0 && x + i < img_out.width && y + j >= 0 && y + j < img_out.height )
                        if(abs(img_in.img[x + i][y + j] - img_in.img[x][y]) < threshold )
                            counter++ ;

                    if( x + j >= 0 && x + j < img_out.width && y + i >= 0 && y + i < img_out.height )
                        if(abs(img_in.img[x + j][y + i] - img_in.img[x][y]) < threshold )
                            counter++ ;

                    if( x - i >= 0 && x - i < img_out.width && y + j >= 0 && y + j < img_out.height )
                        if(abs(img_in.img[x - i][y + j] - img_in.img[x][y]) < threshold )
                            counter++ ;

                    if( x - j >= 0 && x - j < img_out.width && y + i >= 0 && y + i < img_out.height )
                        if(abs(img_in.img[x - j][y + i] - img_in.img[x][y]) < threshold )
                            counter++ ;

                    if( x + i >= 0 && x + i < img_out.width && y - j >= 0 && y - j < img_out.height )
                        if(abs(img_in.img[x + i][y - j] - img_in.img[x][y]) < threshold )
                            counter++ ;

                    if( x + j >= 0 && x + j < img_out.width && y - i >= 0 && y - i < img_out.height )
                        if(abs(img_in.img[x + j][y - i] - img_in.img[x][y]) < threshold )
                            counter++ ;

                    if( x - i >= 0 && x - i < img_out.width && y - j >= 0 && y - j < img_out.height )
                        if(abs(img_in.img[x - i][y - j] - img_in.img[x][y]) < threshold )
                            counter++ ;

                    if( x - j >= 0 && x - j < img_out.width && y - i >= 0 && y - i < img_out.height )
                        if(abs(img_in.img[x - j][y - i] - img_in.img[x][y]) < threshold )
                            counter++ ;

                    if ( d >= 2*i ) {
                        d = d - 2*i - 1 ;
                        i = i + 1 ;
                    } else if ( d < 2*(N-j) ) {
                        d = d + 2*j - 1 ;
                        j = j -1 ;
                    } else {
                        d = d + 2*(j - i - 1) ;
                        j = j - 1 ;
                        i = i + 1 ;
                    }
                }

                if( counter < surf/2 )
                    img_out.img[x][y] = 255 ;
            }
        }
    }

    void corner21(Image2DRGB img_in, Image2D img_out, int tri_suite, int gap, int gap_hmin, int gap_hmax) {
        //--corner detector début--
        int total_pix = 0 ;
        int circle_size = 21 ;
        int circle_max_pix = 10 ;

        for( int x = 4 ; x < img_in.width-4 ; x++ ) {
            for( int y = 4 ; y < img_in.height-4 ; y++ ) {
                img_out.img[x][y] = 0 ;
                int nb_pix = 0 ;
                int suite = 0 ;
                int moy_h = 0 ;

                int circle21[circle_size][4];//Ce tableau contiendra les valeur h, s et v des 20 pixels autour de celui analysé.

                //Mise à zéro du tableau
                for( int i = 0; i < circle_size ; i++ )
                    for( int j = 0; j < 4 ; j++ )
                        circle21[i][j] = 1 ;

                //Enregistre le point voulu en circle21[0]:
                rgb2hsv(img_in.imgRGB[x][y][0],img_in.imgRGB[x][y][1],img_in.imgRGB[x][y][2],circle21[0][0],circle21[0][1],circle21[0][2]) ;

                if(circle21[0][0] < gap_hmin || circle21[0][0] > gap_hmax){
                    //cercle autour du pixel voulu:
                    rgb2hsv(img_in.imgRGB[x+1][y-4][0],img_in.imgRGB[x+1][y-4][1],img_in.imgRGB[x+1][y-4][2],circle21[1][0],circle21[1][1],circle21[1][2]) ;
                    rgb2hsv(img_in.imgRGB[x+2][y-3][0],img_in.imgRGB[x+2][y-3][1],img_in.imgRGB[x+2][y-3][2],circle21[2][0],circle21[2][1],circle21[2][2]) ;
                    rgb2hsv(img_in.imgRGB[x+3][y-2][0],img_in.imgRGB[x+3][y-2][1],img_in.imgRGB[x+3][y-2][2],circle21[3][0],circle21[3][1],circle21[3][2]) ;
                    rgb2hsv(img_in.imgRGB[x+4][y-1][0],img_in.imgRGB[x+4][y-1][1],img_in.imgRGB[x+4][y-1][2],circle21[4][0],circle21[4][1],circle21[4][2]) ;
                    rgb2hsv(img_in.imgRGB[x+4][y][0],img_in.imgRGB[x+4][y][1],img_in.imgRGB[x+4][y][2],circle21[5][0],circle21[5][1],circle21[5][2]) ;
                    rgb2hsv(img_in.imgRGB[x+4][y+1][0],img_in.imgRGB[x+4][y+1][1],img_in.imgRGB[x+4][y+1][2],circle21[6][0],circle21[6][1],circle21[6][2]) ;
                    rgb2hsv(img_in.imgRGB[x+3][y+2][0],img_in.imgRGB[x+3][y+2][1],img_in.imgRGB[x+3][y+2][2],circle21[7][0],circle21[7][1],circle21[7][2]) ;
                    rgb2hsv(img_in.imgRGB[x+2][y+3][0],img_in.imgRGB[x+2][y+3][1],img_in.imgRGB[x+2][y+3][2],circle21[8][0],circle21[8][1],circle21[8][2]) ;
                    rgb2hsv(img_in.imgRGB[x+1][y+4][0],img_in.imgRGB[x+1][y+4][1],img_in.imgRGB[x+1][y+4][2],circle21[9][0],circle21[9][1],circle21[9][2]) ;
                    rgb2hsv(img_in.imgRGB[x][y+4][0],img_in.imgRGB[x][y+4][1],img_in.imgRGB[x][y+4][2],circle21[10][0],circle21[10][1],circle21[10][2]) ;
                    rgb2hsv(img_in.imgRGB[x-1][y+4][0],img_in.imgRGB[x-1][y+4][1],img_in.imgRGB[x-1][y+4][2],circle21[11][0],circle21[11][1],circle21[11][2]) ;
                    rgb2hsv(img_in.imgRGB[x-2][y+3][0],img_in.imgRGB[x-2][y+3][1],img_in.imgRGB[x-2][y+3][2],circle21[12][0],circle21[12][1],circle21[12][2]) ;
                    rgb2hsv(img_in.imgRGB[x-3][y+2][0],img_in.imgRGB[x-3][y+2][1],img_in.imgRGB[x-3][y+2][2],circle21[13][0],circle21[13][1],circle21[13][2]) ;
                    rgb2hsv(img_in.imgRGB[x-4][y+1][0],img_in.imgRGB[x-4][y+1][1],img_in.imgRGB[x-4][y+1][2],circle21[14][0],circle21[14][1],circle21[14][2]) ;
                    rgb2hsv(img_in.imgRGB[x-4][y][0],img_in.imgRGB[x-4][y][1],img_in.imgRGB[x-4][y][2],circle21[15][0],circle21[15][1],circle21[15][2]) ;
                    rgb2hsv(img_in.imgRGB[x-4][y-1][0],img_in.imgRGB[x-4][y-1][1],img_in.imgRGB[x-4][y-1][2],circle21[16][0],circle21[16][1],circle21[16][2]) ;
                    rgb2hsv(img_in.imgRGB[x-3][y-2][0],img_in.imgRGB[x-3][y-2][1],img_in.imgRGB[x-3][y-2][2],circle21[17][0],circle21[17][1],circle21[17][2]) ;
                    rgb2hsv(img_in.imgRGB[x-2][y-3][0],img_in.imgRGB[x-2][y-3][1],img_in.imgRGB[x-2][y-3][2],circle21[18][0],circle21[18][1],circle21[18][2]) ;
                    rgb2hsv(img_in.imgRGB[x-1][y-4][0],img_in.imgRGB[x-1][y-4][1],img_in.imgRGB[x-1][y-4][2],circle21[19][0],circle21[19][1],circle21[19][2]) ;
                    rgb2hsv(img_in.imgRGB[x][y-4][0],img_in.imgRGB[x][y-4][1],img_in.imgRGB[x][y-4][2],circle21[20][0],circle21[20][1],circle21[20][2]) ;


                    for(int i = 0; i < circle_size ; i++ )
                        moy_h += circle21[i][0] ;

                    moy_h /= circle_size ;

                    if( moy_h < gap_hmin || moy_h > gap_hmax ){

                        //Vérification de v et s de chaque pixel par rapport au v et s de P:
                        for( int i = 1; i < circle_size ; i++ )
                            if( abs(circle21[i][1] - circle21[0][1]) > gap || abs(circle21[i][2] - circle21[0][2]) > gap  )
                                circle21[i][3] = 0 ;

                        //Calcule le nombre de pixels valides dans le cercle:
                        for( int i = 1; i < circle_size ; i++ )
                            nb_pix += circle21[i][3] ;

                        if( tri_suite == 1 && nb_pix > 1 && nb_pix < circle_max_pix ) {
                            for ( int i = 1 ; i < circle_size ; i++ ) {
                                if( circle21[i][3] == 1 ) {
                                    suite ++ ;
                                    if( suite == nb_pix )
                                        i = circle_size ;
                                }
                                else
                                    suite = 0 ;
                            }

                            if ( suite > 0 ) {
                                for ( int i = 1 ; i < circle_size ; i++ ) {
                                    if( circle21[i][3] == 1 ) {
                                        suite ++ ;
                                        if( suite == nb_pix )
                                            i = circle_size ;
                                    }
                                    else
                                        suite = 0 ;
                                }
                            }

                            if( suite == nb_pix )
                                img_out.img[x][y] = 255 ;
                        }
                        else{
                            if( nb_pix > 1 && nb_pix < circle_max_pix )
                                img_out.img[x][y] = 255 ;
                        }
                    }
                }
            }
            //Affiche en direct le nombre de pixel validés:
            //printf("%d pixels validés\n", total_pix) ;
        }
        //--corner detector fin--
    }

    void visionFindCanvas(Image2D img_in, Image2D img_out,PixelFound *canvas, int threshold, int Nmore) {
        int stop, p1, p2 ;
        int compteur ;
        int posx, posy ;
        int e = 0 ;
        int Nreal = 4 ;
        int N = Nreal + Nmore ;
        int surfacemin ;

        while( e < 12 ) {
            N = Nreal + Nmore ;
            surfacemin = (3.14*N*N) / 10 ;
            e = 0 ;

            for( int x = 0 ; x < img_in.width ; x++) {
                for( int y = 0 ; y < img_in.height ; y++) {
                    img_out.img[x][y] = 255 ;
                    if( img_in.img[x][y] < threshold )
                        img_out.img[x][y] = 200 ;
                }
            }

            for( int x = N ; x < img_in.width-N ; x++) {
                for( int y = N ; y < img_in.height-N ; y++) {
                    stop = p1 = p2 = 0 ;
                    if( img_in.img[x][y] < threshold && img_in.img[x-N][y] > threshold && img_in.img[x+N][y] > threshold && img_in.img[x][y-N] > threshold && img_in.img[x][y+N] > threshold) {
                        for( int i = -N ; i <= N ; i++ ) {
                            for( int j = -N ; j <= N ; j++ ) {
                                if( abs(i) == N || abs(j) == N ){
                                    if( img_in.img[x+i][y+j] >= threshold ) {
                                        p1++ ;
                                    }
                                    p2 ++ ;
                                }

                                if( img_out.img[x+i][y+j] == 0 )
                                    stop = 1 ;
                            }
                        }

                        if( p1 == p2 && p1 > 0 && stop == 0 ) {
                            compteur = posx = posy = 0 ;

                            for( int i = -N ; i <= N ; i++ ) {
                                for( int j = -N ; j <= N ; j++ ) {
                                    if ( img_in.img[x+i][y+j] < threshold ) {
                                        compteur++  ;
                                        posx += x+i ;
                                        posy += y+j ;
                                    }
                                }
                            }

                            if ( compteur > surfacemin ) {
                                posx /= compteur ;
                                posy /= compteur ;

                                img_out.img[posx][posy] = 0 ;
                                drawpoint( img_out, posx, posy, 2, N, 100) ;

                                canvas[e].x = posx ;
                                canvas[e].y = posy ;

                                if ( e < 99 )
                                    e++ ;
                            }
                        }
                    }
                }
            }

            if( e < 12 )
                Nreal++ ;

            canvas[0].nb = e ;

            if( N > 15 )
                break ;
        }
    }

    void visionFindCanvas2(Image2D img_in, Image2D img_out,PixelFound *canvas, int threshold, int Nmore, int Emax) {
        int stop, p1, p2 ;
        int compteur ;
        int posx, posy ;
        int e = 0 ;
        int Nreal = 4 ;
        int N = Nreal + Nmore ;
        int surfacemin ;

        while( e < Emax ) {
            N = Nreal + Nmore ;
            surfacemin = (3.14*N*N) / 10 ;
            e = 0 ;

            for( int x = 0 ; x < img_in.width ; x++) {
                for( int y = 0 ; y < img_in.height ; y++) {
                    img_out.img[x][y] = 255 ;
                    if( img_in.img[x][y] < threshold )
                        img_out.img[x][y] = 200 ;
                }
            }

            for( int x = N ; x < img_in.width-N ; x++) {
                for( int y = N ; y < img_in.height-N ; y++) {
                    stop = p1 = p2 = 0 ;
                    if( img_in.img[x][y] < threshold && img_in.img[x-N][y] > threshold && img_in.img[x+N][y] > threshold && img_in.img[x][y-N] > threshold && img_in.img[x][y+N] > threshold) {
                        for( int i = -N ; i <= N ; i++ ) {
                            for( int j = -N ; j <= N ; j++ ) {
                                if( abs(i) == N || abs(j) == N ){
                                    if( img_in.img[x+i][y+j] >= threshold ) {
                                        p1++ ;
                                    }
                                    p2 ++ ;
                                }

                                if( img_out.img[x+i][y+j] == 0 )
                                    stop = 1 ;
                            }
                        }

                        if( p1 == p2 && p1 > 0 && stop == 0 ) {
                            compteur = posx = posy = 0 ;

                            for( int i = -N ; i <= N ; i++ ) {
                                for( int j = -N ; j <= N ; j++ ) {
                                    if ( img_in.img[x+i][y+j] < threshold ) {
                                        compteur++  ;
                                        posx += x+i ;
                                        posy += y+j ;
                                    }
                                }
                            }

                            if ( compteur > surfacemin ) {
                                posx /= compteur ;
                                posy /= compteur ;

                                img_out.img[posx][posy] = 0 ;
                                drawpoint( img_out, posx, posy, 2, N, 100) ;

                                canvas[e].x = posx ;
                                canvas[e].y = posy ;

                                if ( e < 99 )
                                    e++ ;
                            }
                        }
                    }
                }
            }

            if( e < Emax )
                Nreal++ ;

            canvas[0].nb = e ;

            if( N > 15 )
                break ;
        }
    }

    void visionDrawCanvas(Image2D img_in, Image2D img_out, int threshold, int Nmore, int Emax) {
        int stop, p1, p2 ;
        int compteur ;
        int posx, posy ;
        int e = 0 ;
        int Nreal = 4 ;
        int N = Nreal + Nmore ;
        int surfacemin ;

        while( e < Emax ) {
            N = Nreal + Nmore ;
            surfacemin = (3.14*N*N) / 10 ;
            e = 0 ;

            for( int x = 0 ; x < img_in.width ; x++) {
                for( int y = 0 ; y < img_in.height ; y++) {
                    img_out.img[x][y] = 255 ;
                    if( img_in.img[x][y] < threshold )
                        img_out.img[x][y] = 200 ;
                }
            }

            for( int x = N ; x < img_in.width-N ; x++) {
                for( int y = N ; y < img_in.height-N ; y++) {
                    stop = p1 = p2 = 0 ;
                    if( img_in.img[x][y] < threshold && img_in.img[x-N][y] > threshold && img_in.img[x+N][y] > threshold && img_in.img[x][y-N] > threshold && img_in.img[x][y+N] > threshold) {
                        for( int i = -N ; i <= N ; i++ ) {
                            for( int j = -N ; j <= N ; j++ ) {
                                if( abs(i) == N || abs(j) == N ){
                                    if( img_in.img[x+i][y+j] >= threshold ) {
                                        p1++ ;
                                    }
                                    p2 ++ ;
                                }

                                if( img_out.img[x+i][y+j] == 0 )
                                    stop = 1 ;
                            }
                        }

                        if( p1 == p2 && p1 > 0 && stop == 0 ) {
                            compteur = posx = posy = 0 ;

                            for( int i = -N ; i <= N ; i++ ) {
                                for( int j = -N ; j <= N ; j++ ) {
                                    if ( img_in.img[x+i][y+j] < threshold ) {
                                        compteur++  ;
                                        posx += x+i ;
                                        posy += y+j ;
                                    }
                                }
                            }

                            if ( compteur > surfacemin ) {
                                posx /= compteur ;
                                posy /= compteur ;

                                img_out.img[posx][posy] = 0 ;
                                drawpoint( img_out, posx, posy, 2, N, 100) ;

                                if ( e < 99 )
                                    e++ ;
                            }
                        }
                    }
                }
            }

            if( e < Emax )
                Nreal++ ;

            if( N > 15 )
                break ;
        }
    }


    //Code de rotation de l'image
    //http://eab.abime.net/showthread.php?t=29492
    void visionRotateImage(Image2D img_in, Image2D img_out, int cx, int cy, float angle) {
        int xx, yy ;
        int xt, yt ;
        int xs, ys ;
        double sinma, cosma ;

        for( int x = 0 ; x < img_in.width ; x++ ) {
            for( int y = 0 ; y < img_in.height ; y++ ) {
                xt = x - cx;
                yt = y - cy;

                sinma = sin(angle * M_PI/180);
                cosma = cos(angle * M_PI/180);

                xs = (int)round((cosma * xt - sinma * yt) + cx);
                ys = (int)round((sinma * xt + cosma * yt) + cy);

                if( xs > 0 && xs < img_in.width && ys > 0 && ys < img_in.height )
                    img_out.img[x][y] = img_in.img[xs][ys] ;
                //else
                    //img_out.img[x][y] = 0 ;
            }
        }
    }

    void visionRotateAround(Image2D img_in, Image2D img_out, int cx, int cy, float angle, int R) {
        int xx, yy ;
        int xs, ys ;
        double sinma, cosma ;

        for( int x = -R ; x < R ; x++ ) {
            for( int y = -R ; y < R ; y++ ) {
                sinma = sin(angle * M_PI/180);
                cosma = cos(angle * M_PI/180);

                xs = (int)round((cosma * x - sinma * y) + cx);
                ys = (int)round((sinma * x + cosma * y) + cy);

                if( xs > 0 && xs < img_in.width && ys > 0 && ys < img_in.height )
                    if( x+cx > 0 && x+cx < img_in.width && y+cy > 0 && y+cy < img_in.height )
                        img_out.img[x+cx][y+cy] = img_in.img[xs][ys] ;
                //else
                    //img_out.img[x+cx][y+cy] = 0 ;
            }
        }
    }
/*
    void visionGetFeature4(Image2D img_in, Image2D img_out, int x, int y, PixelFeature *feat, int pos) {
        float c = visionGaussian3pix(img_in, x+1, y) - visionGaussian3pix(img_in, x-1, y) ;
        float d = visionGaussian3pix(img_in, x, y+1) - visionGaussian3pix(img_in, x, y-1) ;
        //feat[pos].det = sqrt( c*c + d*d )* 100/sqrt(400*400*2) ;

        feat[pos].det = atan2(d,c) * 180 / M_PI ;
        if( feat[pos].det < 0 )
            feat[pos].det += 360 ;
    }
*/
    void visionGetFeature4(Image2D img_in, Image2D img_out, int x, int y, PixelFeature *feat, int pos) {
        int xs, ys ;
        double sinma, cosma ;
        float angle ;
        int type = 2 ;

        feat[pos].x = x ;
        feat[pos].y = y ;
        feat[pos].i = img_in.img[x][y] ;
        feat[pos].moy = 0 ;
        feat[pos].ecart = 0 ;
        feat[pos].etendue = 0 ;
        feat[pos].amp = 0 ;
        feat[pos].theta = 0 ;
        feat[pos].det = 0 ;

        for( int i = 0 ; i < 9 ; i++ )
            feat[pos].histogram[i] = 0 ;

        //Calcul des gradient en X et Y
        float gx, gy ;
        if( type == 0 ) {
            gx = visionGaussian3pix(img_in, x+1, y) - visionGaussian3pix(img_in, x-1, y) ;
            gy = visionGaussian3pix(img_in, x, y+1) - visionGaussian3pix(img_in, x, y-1) ;
        } else if( type == 1 ) {
            gx = img_in.img[x+1][y] - img_in.img[x-1][y] ;
            gy = img_in.img[x][y+1] - img_in.img[x][y-1] ;
        } else if( type == 2 ) {//Faster and with same result as type == 0
            gx = (img_in.img[x+1][y-1] + 2*img_in.img[x+1][y] + img_in.img[x+1][y+1]) - (img_in.img[x-1][y-1] + 2*img_in.img[x-1][y] + img_in.img[x-1][y+1]) ;
            gy = (img_in.img[x-1][y+1] + 2*img_in.img[x][y+1] + img_in.img[x+1][y+1]) - (img_in.img[x-1][y-1] + 2*img_in.img[x][y-1] + img_in.img[x+1][y-1]) ;
        } else if( type == 3 ) {
            gx = (visionGaussian3pix(img_in, x+1, y-1) + 2*visionGaussian3pix(img_in, x+1, y) + visionGaussian3pix(img_in, x+1, y+1)) - (visionGaussian3pix(img_in, x-1, y-1) + 2*visionGaussian3pix(img_in, x-1, y) + visionGaussian3pix(img_in, x-1, y+1)) ;
            gy = (visionGaussian3pix(img_in, x-1, y+1) + 2*visionGaussian3pix(img_in, x, y+1) + visionGaussian3pix(img_in, x+1, y+1)) - (visionGaussian3pix(img_in, x-1, y-1) + 2*visionGaussian3pix(img_in, x, y-1) + visionGaussian3pix(img_in, x+1, y-1)) ;
        } else {
            printf("nope\n") ;
        }

        //Calcul de la normale du gradient
        feat[pos].amp = sqrt( gx*gx + gy*gy ) ;

        //Calcul de l'angle du grandient
        feat[pos].theta = atan2(gy,gx) ;
        if( feat[pos].theta < 0 )
            feat[pos].theta += 2*M_PI ;

        //Effectue une rotation de l'image a l'emplacement du point egale a l'angle du gradient
        for( int u = -6 ; u < 6 ; u++ ) {
            for( int v = -6 ; v < 6 ; v++ ) {
                sinma = sin(feat[pos].theta);
                cosma = cos(feat[pos].theta);

                xs = (int)round((cosma * u - sinma * v) + x);
                ys = (int)round((sinma * u + cosma * v) + y);

                if( xs > 0 && xs < img_in.width && ys > 0 && ys < img_in.height )
                    if( u+x > 0 && u+x < img_in.width && v+y > 0 && v+y < img_in.height )
                        img_out.img[u+x][v+y] = img_in.img[xs][ys] ;
            }
        }

        if( x-6 >= 0 && x+6 < img_in.width && y-6 >= 0 && y+6 < img_in.height ) {
            for( int u = -4 ; u < 4 ; u++ ) {
                for( int v = -4 ; v < 4 ; v++ ) {
                    gx = img_out.img[x+1+u][y+v]-img_out.img[x-1+u][y+v] ;
                    gy = img_out.img[x+u][y+1+v]-img_out.img[x+u][y-1+v] ;
                    angle = atan2(gy, gx) * 180 / M_PI ;
                    if( angle < 0 )
                        angle += 360 ;
                    angle = floor(angle/20) ;
                    if( angle > 17 )
                        angle = 17 ;
                    feat[pos].histogram[(int)angle] += sqrt(gx*gx+gy*gy) ;
                }
            }
        }
    }

    void visionRotateCanvas(PixelFound center, PixelFound direction, PixelFound *canvas, PixelFound *canvasnew) {
        int point_length ;
        float point_theta ;
        int diffX = direction.x - center.x ;
        int diffY = center.y - direction.y ;

        point_length = sqrt( diffX*diffX + diffY*diffY ) ;
        float theta = M_PI/2 - atan2(diffY, diffX) ;
        if( theta < 0 )
            theta += 2*M_PI ;

        printf("rotation = %f\n",theta * 180/M_PI) ;

        canvasnew[0].nb = canvas[0].nb ;
        for( int i = 0 ; i < canvas[0].nb ; i++ ) {
            diffX = canvas[i].x - center.x ;
            diffY = center.y - canvas[i].y ;
            point_length = sqrt( diffX*diffX + diffY*diffY ) ;
            point_theta = atan2(diffY, diffX) ;
            if( point_theta < 0 )
                point_theta += 2*M_PI ;

            point_theta += theta ;
            if( point_theta > 2*M_PI )
                point_theta -= 2*M_PI ;

            canvasnew[i].x = point_length*cos(point_theta) + center.x ;
            canvasnew[i].y = -point_length*sin(point_theta) + center.y ;
        }
    }

    int visionGetCanvas(Image2D img_in, Image2D img_out, PixelFound *canvas_blue, PixelFound *canvas_black, int &action, int &Nmore, int &threshold) {
        if (!action) {
            canvas_blue[0].nb = 0 ;
            canvas_black[0].nb = 0 ;
            visionFindCanvas(img_in, img_out, canvas_black, threshold, Nmore) ;
        }

        if(action) {
            while( canvas_blue[0].nb < 4 ) {
                SDL_Event event_in ;
                while(SDL_PollEvent(&event_in)) {
                    if(SDL_GetKeyState(NULL)[SDLK_DOWN]) {
                        action = 0 ;
                        return 1 ;
                    }
                    if( event_in.type == SDL_MOUSEBUTTONUP ) {
                        if(event_in.button.button == SDL_BUTTON_LEFT ) {
                            for( int i = 0 ; i < canvas_black[0].nb ; i++ ) {
                                if( abs(event_in.motion.x-canvas_black[i].x) < 15 && abs(event_in.motion.y-canvas_black[i].y) < 15 ) {
                                    canvas_blue[canvas_blue[0].nb].x = canvas_black[i].x ;
                                    canvas_blue[canvas_blue[0].nb].y = canvas_black[i].y ;

                                    drawpoint(img_out, canvas_black[i].x, canvas_black[i].y, 0, 10, 100) ;

                                    for( int j = i ; j < canvas_black[0].nb ; j++ ) {
                                        canvas_black[j].x = canvas_black[j+1].x ;
                                        canvas_black[j].y = canvas_black[j+1].y ;
                                    }

                                    canvas_blue[0].nb++ ;
                                    canvas_black[0].nb-- ;
                                    break ;
                                }
                            }
                            return 1 ;
                        } else if(event_in.button.button == SDL_BUTTON_RIGHT ) {
                            for( int i = 0 ; i < canvas_black[0].nb ; i++ ) {
                                if( abs(event_in.motion.x-canvas_black[i].x) < 15 && abs(event_in.motion.y-canvas_black[i].y) < 15 ) {
                                    drawpoint(img_out, canvas_black[i].x, canvas_black[i].y, 1, 10, 100) ;

                                    for( int j = i ; j < canvas_black[0].nb ; j++ ) {
                                        canvas_black[j].x = canvas_black[j+1].x ;
                                        canvas_black[j].y = canvas_black[j+1].y ;
                                    }

                                    canvas_black[0].nb-- ;
                                    break ;
                                }
                            }
                            return 1 ;
                        }
                    }
                }
            }

            if( canvas_blue[0].nb == 4 && canvas_black[0].nb == 8 ) {
                PixelFound canvas_blue_corrected[5] ;
                PixelFound canvas_black_corrected[9] ;
                PixelFound canvas_final[12] ;
                PixelFound center_blue ;
                PixelFound center_black ;
                int l, r, b, t ;

                canvas_final[0].nb = 0 ;

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

                action = 0 ;
                return 1 ;
            } else
                action = 0 ;
        }

        return 1 ;
    }

    int visionGetCanvas2(Image2D img_in, Image2D img_out, int campos, PixelFound *canvas_blue, PixelFound *canvas_black, int &action, int &Nmore, int Emax, int &threshold, int &finished) {
        if( action > 0 ) {
            int motionx, motiony ;
            while( canvas_blue[0].nb < 4 ) {
                SDL_Event event_in ;
                while(SDL_PollEvent(&event_in)) {
                    if(SDL_GetKeyState(NULL)[SDLK_DOWN]) {
                        action-- ;
                        return 1 ;
                    }
                    motionx = event_in.motion.x-img_in.width*campos ;
                    motiony = event_in.motion.y ;
                    if( motionx < 0 || motionx > img_in.width )
                        return 1 ;

                    if( event_in.type == SDL_MOUSEBUTTONUP ) {
                        if(event_in.button.button == SDL_BUTTON_LEFT ) {
                            for( int i = 0 ; i < canvas_black[0].nb ; i++ ) {
                                if( abs(motionx-canvas_black[i].x) < 15 && abs(motiony-canvas_black[i].y) < 15 ) {
                                    canvas_blue[canvas_blue[0].nb].x = canvas_black[i].x ;
                                    canvas_blue[canvas_blue[0].nb].y = canvas_black[i].y ;

                                    drawpoint(img_out, canvas_black[i].x, canvas_black[i].y, 0, 10, 100) ;

                                    for( int j = i ; j < canvas_black[0].nb ; j++ ) {
                                        canvas_black[j].x = canvas_black[j+1].x ;
                                        canvas_black[j].y = canvas_black[j+1].y ;
                                    }

                                    canvas_blue[0].nb++ ;
                                    canvas_black[0].nb-- ;
                                    break ;
                                }
                            }
                            return 1 ;
                        } else if(event_in.button.button == SDL_BUTTON_RIGHT ) {
                            for( int i = 0 ; i < canvas_black[0].nb ; i++ ) {
                                if( abs(motionx-canvas_black[i].x) < 15 && abs(motiony-canvas_black[i].y) < 15 ) {
                                    drawpoint(img_out, canvas_black[i].x, canvas_black[i].y, 1, 10, 100) ;

                                    for( int j = i ; j < canvas_black[0].nb ; j++ ) {
                                        canvas_black[j].x = canvas_black[j+1].x ;
                                        canvas_black[j].y = canvas_black[j+1].y ;
                                    }

                                    canvas_black[0].nb-- ;
                                    break ;
                                }
                            }
                            return 1 ;
                        }
                    }
                }
            }

            if( canvas_blue[0].nb == 4 && canvas_black[0].nb == 8 ) {
                PixelFound canvas_blue_corrected[5] ;
                PixelFound canvas_black_corrected[9] ;
                PixelFound canvas_final[12] ;
                PixelFound center_blue ;
                PixelFound center_black ;
                int l, r, b, t ;

                canvas_final[0].nb = 0 ;

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

                FILE * camera ;

                if( campos == 0 )
                    camera = fopen("2D_gauche.txt", "w") ;
                else if ( campos == 1 )
                    camera = fopen("2D_droite.txt", "w") ;

                if ( camera != NULL ) {
                    for( int i = 0 ; i < 12 ; i++ ) {
                        printf("%d = %d-%d\n", i, canvas_final[i].x, canvas_final[i].y) ;
                        fprintf(camera,"%d\t%d\n", canvas_final[i].x, canvas_final[i].y) ;
                        canvas_black[i].x = canvas_final[i].x ;
                        canvas_black[i].y = canvas_final[i].y ;
                    }
                    fclose(camera) ;
                }

                action-- ;
                finished = 1 ;
                return 1 ;
            } else
                action = -1 ;
        }

        return 1 ;
    }


    void visionGetFeature(Image2D img_in, int x, int y, PixelFeature *feat, int pos) {
        float percent = (float) 100/255 ;
        int p = 0 ;
        float a = 0 ;
        float b = 100 ;
        float c, d ;
        feat[pos].moy = 0 ;
        feat[pos].ecart = 0 ;
        float matrix[9] ;

        feat[pos].x = x ;
        feat[pos].y = y ;
        feat[pos].i = img_in.img[x][y] * percent ;

        for( int i = -1 ; i <= 1 ; i++ ) {
            for( int j = -1 ; j <= 1 ; j++ ) {
                matrix[p] = img_in.img[x+i][y+j] * percent ;
                feat[pos].moy += matrix[p] ;
                if( matrix[p] > a )
                    a = matrix[p] ;
                if( matrix[p] < b )
                    b = matrix[p] ;
                p++ ;
            }
        }
        feat[pos].moy /= 9 ;
        feat[pos].etendue = a - b ;

        for( int i = 0 ; i < p ; i++ )
            feat[pos].ecart += (matrix[i]-feat[pos].i) * (matrix[i]-feat[pos].i) ;
        feat[pos].ecart = sqrt(feat[pos].ecart / 9);

        a = matrix[7] - matrix[1] ;
        b = matrix[5] - matrix[3] ;
        feat[pos].amp = sqrt( a*a + b*b ) ;
        feat[pos].amp *= 100/sqrt(400*400*2) ;

        feat[pos].theta = atan2(b,a) * 180 / M_PI ;
        if( feat[pos].theta < 0 )
            feat[pos].theta += 360 ;
        feat[pos].theta *= (float) 100/360  ;

        //feat[pos].det = matrix[0]*(matrix[4]*matrix[8]-matrix[5]*matrix[7]) - matrix[1]*(matrix[3]*matrix[8]-matrix[6]*matrix[5]) + matrix[2]*(matrix[3]*matrix[7]-matrix[4]*matrix[6]) ;

        c = visionGaussian3pix(img_in, x+1, y) - visionGaussian3pix(img_in, x-1, y) ;
        d = visionGaussian3pix(img_in, x, y+1) - visionGaussian3pix(img_in, x, y-1) ;
        //feat[pos].det = ( a*d + b*c ) / sqrt( a*a + b*b ) ;
        feat[pos].det = sqrt( c*c + d*d )* 100/sqrt(400*400*2) ;
    }

    void visionGetFeature2(Image2D img_in, int x, int y, PixelFeature *feat, int pos) {
        float percent = (float) 100/255 ;
        int p = 0 ;
        float a = 0 ;
        float b = 100 ;
        float c, d ;
        feat[pos].moy = 0 ;
        feat[pos].ecart = 0 ;
        float matrix[9] ;

        feat[pos].x = x ;
        feat[pos].y = y ;
        feat[pos].i = img_in.img[x][y] * percent ;

        c = visionGaussian3pix(img_in, x+1, y) - visionGaussian3pix(img_in, x-1, y) ;
        d = visionGaussian3pix(img_in, x, y+1) - visionGaussian3pix(img_in, x, y-1) ;
        //feat[pos].det = sqrt( c*c + d*d )* 100/sqrt(400*400*2) ;

        feat[pos].det = atan2(d,c) * 180 / M_PI ;
        if( feat[pos].det < 0 )
            feat[pos].det += 360 ;

        if( feat[pos].det/45 < 1 ) {
            matrix[0] = img_in.img[x+1][y] * percent ;
            matrix[1] = img_in.img[x+1][y-1] * percent ;
            matrix[2] = img_in.img[x][y-1] * percent ;
            matrix[3] = img_in.img[x-1][y-1] * percent ;
            matrix[4] = img_in.img[x-1][y] * percent ;
            matrix[5] = img_in.img[x-1][y+1] * percent ;
            matrix[6] = img_in.img[x][y+1] * percent ;
            matrix[7] = img_in.img[x+1][y+1] * percent ;
            matrix[8] = img_in.img[x][y] * percent ;
            a = matrix[0] - matrix[4] ;
            b = matrix[6] - matrix[2] ;
        } else if( feat[pos].det/45 < 2 ) {
            matrix[7] = img_in.img[x+1][y] * percent ;
            matrix[0] = img_in.img[x+1][y-1] * percent ;
            matrix[1] = img_in.img[x][y-1] * percent ;
            matrix[2] = img_in.img[x-1][y-1] * percent ;
            matrix[3] = img_in.img[x-1][y] * percent ;
            matrix[4] = img_in.img[x-1][y+1] * percent ;
            matrix[5] = img_in.img[x][y+1] * percent ;
            matrix[6] = img_in.img[x+1][y+1] * percent ;
            matrix[8] = img_in.img[x][y] * percent ;
            a = matrix[7] - matrix[3] ;
            b = matrix[5] - matrix[1] ;
        } else if( feat[pos].det/45 < 3 ) {
            matrix[6] = img_in.img[x+1][y] * percent ;
            matrix[7] = img_in.img[x+1][y-1] * percent ;
            matrix[0] = img_in.img[x][y-1] * percent ;
            matrix[1] = img_in.img[x-1][y-1] * percent ;
            matrix[2] = img_in.img[x-1][y] * percent ;
            matrix[3] = img_in.img[x-1][y+1] * percent ;
            matrix[4] = img_in.img[x][y+1] * percent ;
            matrix[5] = img_in.img[x+1][y+1] * percent ;
            matrix[8] = img_in.img[x][y] * percent ;
            a = matrix[6] - matrix[2] ;
            b = matrix[4] - matrix[0] ;
        } else if( feat[pos].det/45 < 4 ) {
            matrix[5] = img_in.img[x+1][y] * percent ;
            matrix[6] = img_in.img[x+1][y-1] * percent ;
            matrix[7] = img_in.img[x][y-1] * percent ;
            matrix[0] = img_in.img[x-1][y-1] * percent ;
            matrix[1] = img_in.img[x-1][y] * percent ;
            matrix[2] = img_in.img[x-1][y+1] * percent ;
            matrix[3] = img_in.img[x][y+1] * percent ;
            matrix[4] = img_in.img[x+1][y+1] * percent ;
            matrix[8] = img_in.img[x][y] * percent ;
            a = matrix[5] - matrix[1] ;
            b = matrix[3] - matrix[7] ;
        } else if( feat[pos].det/45 < 5 ) {
            matrix[4] = img_in.img[x+1][y] * percent ;
            matrix[5] = img_in.img[x+1][y-1] * percent ;
            matrix[6] = img_in.img[x][y-1] * percent ;
            matrix[7] = img_in.img[x-1][y-1] * percent ;
            matrix[0] = img_in.img[x-1][y] * percent ;
            matrix[1] = img_in.img[x-1][y+1] * percent ;
            matrix[2] = img_in.img[x][y+1] * percent ;
            matrix[3] = img_in.img[x+1][y+1] * percent ;
            matrix[8] = img_in.img[x][y] * percent ;
            a = matrix[4] - matrix[0] ;
            b = matrix[2] - matrix[6] ;
        } else if( feat[pos].det/45 < 6 ) {
            matrix[3] = img_in.img[x+1][y] * percent ;
            matrix[4] = img_in.img[x+1][y-1] * percent ;
            matrix[5] = img_in.img[x][y-1] * percent ;
            matrix[6] = img_in.img[x-1][y-1] * percent ;
            matrix[7] = img_in.img[x-1][y] * percent ;
            matrix[0] = img_in.img[x-1][y+1] * percent ;
            matrix[1] = img_in.img[x][y+1] * percent ;
            matrix[2] = img_in.img[x+1][y+1] * percent ;
            matrix[8] = img_in.img[x][y] * percent ;
            a = matrix[3] - matrix[7] ;
            b = matrix[1] - matrix[5] ;
        } else if( feat[pos].det/45 < 7 ) {
            matrix[2] = img_in.img[x+1][y] * percent ;
            matrix[3] = img_in.img[x+1][y-1] * percent ;
            matrix[4] = img_in.img[x][y-1] * percent ;
            matrix[5] = img_in.img[x-1][y-1] * percent ;
            matrix[6] = img_in.img[x-1][y] * percent ;
            matrix[7] = img_in.img[x-1][y+1] * percent ;
            matrix[0] = img_in.img[x][y+1] * percent ;
            matrix[1] = img_in.img[x+1][y+1] * percent ;
            matrix[8] = img_in.img[x][y] * percent ;
            a = matrix[2] - matrix[6] ;
            b = matrix[0] - matrix[4] ;
        } else if( feat[pos].det/45 < 8 ) {
            matrix[1] = img_in.img[x+1][y] * percent ;
            matrix[2] = img_in.img[x+1][y-1] * percent ;
            matrix[3] = img_in.img[x][y-1] * percent ;
            matrix[4] = img_in.img[x-1][y-1] * percent ;
            matrix[5] = img_in.img[x-1][y] * percent ;
            matrix[6] = img_in.img[x-1][y+1] * percent ;
            matrix[7] = img_in.img[x][y+1] * percent ;
            matrix[0] = img_in.img[x+1][y+1] * percent ;
            matrix[8] = img_in.img[x][y] * percent ;
            a = matrix[1] - matrix[5] ;
            b = matrix[7] - matrix[3] ;
        }

        c = 0 ;
        d = 100 ;
        for( int i = 0 ; i < 9 ; i++ ) {
            feat[pos].moy += matrix[i] ;
            if( matrix[i] > c )
                c = matrix[i] ;
            if( matrix[i] < d )
                d = matrix[i] ;
            p++ ;
        }
        feat[pos].moy /= 9 ;
        feat[pos].etendue = c - d ;

        for( int i = 0 ; i < 9 ; i++ )
            feat[pos].ecart += (matrix[i]-feat[pos].i) * (matrix[i]-feat[pos].i) ;
        feat[pos].ecart = sqrt(feat[pos].ecart / 9);

        feat[pos].amp = sqrt( a*a + b*b ) ;
        feat[pos].amp *= 100/sqrt(400*400*2) ;

        feat[pos].theta = atan2(b,a) * 180 / M_PI ;
        if( feat[pos].theta < 0 )
            feat[pos].theta += 360 ;
        feat[pos].theta *= (float) 100/360  ;
    }

    void visionGetFeature3(Image2D img_in, int x, int y, PixelFeature *feat, int pos) {
        float percent = (float) 100/255 ;
        float histogram[9] ;
        float theta ;
        float normale ;
        float gx, gy ;
        int p = 0 ;

        feat[pos].x = x ;
        feat[pos].y = y ;
        feat[pos].i = img_in.img[x][y] * percent ;

        for( int i = 0 ; i < 9 ; i++ )
            feat[pos].histogram[i] = 0 ;

        for( int u = -4 ; u < 4 ; u++ ) {
            for( int v = -4 ; v < 4 ; v++ ) {
                gx = img_in.img[x+1+u][y+v]-img_in.img[x-1+u][y+v] ;
                gy = img_in.img[x+u][y+1+v]-img_in.img[x+u][y-1+v] ;
                normale = sqrt(gx*gx+gy*gy) ;
                theta = atan2(gy, gx) * 180 / M_PI ;
                if( theta < 0 ) {
                    theta += 180 ;
                    feat[pos].histogram[(int)floor(theta/20)] -= normale ;
                } else
                    feat[pos].histogram[(int)floor(theta/20)] += normale ;
                p++ ;
            }
        }
    }


    void visionPrintFeature(PixelFeature feat) {
        printf("\nx \t= %d\n", feat.x) ;
        printf("y \t= %d\n", feat.y) ;
        printf("i \t= %0.3f\n", feat.i) ;
        printf("moy \t= %0.3f\n", feat.moy) ;
        printf("ecart \t= %0.3f\n", feat.ecart) ;
        printf("etendue = %0.3f\n", feat.etendue) ;
        printf("amp \t= %0.3f\n", feat.amp) ;
        printf("theta \t= %0.3f\n", feat.theta) ;
        printf("det \t= %0.3f\n", feat.det) ;
    }

    void visionPrintFeature2(PixelFeature feat1, PixelFeature feat2) {
        printf("\nx \t= %d - %d\n", feat1.x, feat2.x) ;
        printf("y \t= %d - %d\n", feat1.y, feat2.y) ;
        printf("i \t= %0.3f - %0.3f\n", feat1.i, feat2.i) ;
        printf("moy \t= %0.3f - %0.3f\n", feat1.moy, feat2.moy) ;
        printf("ecart \t= %0.3f - %0.3f\n", feat1.ecart, feat2.ecart) ;
        printf("etendue = %0.3f - %0.3f\n", feat1.etendue, feat2.etendue) ;
        printf("amp \t= %0.3f - %0.3f\n", feat1.amp, feat2.amp) ;
        printf("theta \t= %0.3f - %0.3f\n", feat1.theta, feat2.theta) ;
        printf("det \t= %0.3f - %0.3f\n", feat1.det, feat2.det) ;
    }

    void visionPrintFeature3(PixelFeature feat1, PixelFeature feat2) {
        for( int i = 0 ; i < 9 ; i++ )
            printf("r[%d] = %0.2f - %0.2f\n", i, feat1.histogram[i], feat2.histogram[i]) ;

    }

    float visionCompareFeature(PixelFeature feat_1, PixelFeature feat_2) {
        float result = 0 ;
        result += fabs(feat_1.i - feat_2.i) ;
        result += fabs(feat_1.moy - feat_2.moy) ;
        result += fabs(feat_1.ecart - feat_2.ecart) ;
        result += fabs(feat_1.etendue - feat_2.etendue) ;
        //result += fabs(feat_1.det - feat_2.det) ;
        result += fabs(feat_1.amp - feat_2.amp) ;
        result += fabs(feat_1.theta - feat_2.theta) ;
        return result ;
    }

    float visionCompareFeature3(PixelFeature feat_1, PixelFeature feat_2) {
        float result = 0 ;
        for( int i = 0 ; i < 9 ; i++ )
            result += fabs(feat_1.histogram[i] - feat_2.histogram[i])  ;
        return result ;
    }

    float visionCompareFeature4(PixelFeature feat_1, PixelFeature feat_2) {
        float result = 0 ;
        for( int i = 0 ; i < 18 ; i++ )
            result += (feat_1.histogram[i] - feat_2.histogram[i])*(feat_1.histogram[i] - feat_2.histogram[i])  ;
        return sqrt(result) ;
    }

    void visionErosion( Image2D img_in, int Nmax) {
        int p1, p2 ;
        int compteur, posx, posy ;
        int total_point = 0 ;

        for( int x = Nmax ; x < img_in.width-Nmax ; x++) {
            for( int y = Nmax ; y < img_in.height-Nmax ; y++) {
                if( img_in.img[x][y] == 200 && img_in.img[x][y] != 255 ) {
                    for ( int N = 2 ; N <= Nmax ; N++ ) {
                        p1 = p2 = 0 ;
                        if( img_in.img[x-N][y] < 200 && img_in.img[x+N][y] < 200 && img_in.img[x][y-N] < 200 && img_in.img[x][y+N] < 200 ) {
                            for( int i = -N ; i <= N ; i++ ) {
                                for( int j = -N ; j <= N ; j++ ) {
                                    if( abs(i) == N || abs(j) == N ){
                                        if( img_in.img[x+i][y+j] < 200 ) {
                                            p1++ ;
                                        }
                                        p2 ++ ;
                                    }
                                }
                            }

                            if( p1 == p2 && p1 > 0 ) {
                                compteur = posx = posy = 0 ;

                                for( int i = -N ; i <= N ; i++ ) {
                                    for( int j = -N ; j <= N ; j++ ) {
                                        if ( img_in.img[x+i][y+j] == 200 ) {
                                            compteur++  ;
                                            posx += x+i ;
                                            posy += y+j ;
                                            img_in.img[x+i][y+j] = 0 ;
                                        }
                                    }
                                }

                                if ( compteur > 0 ) {
                                    posx /= compteur ;
                                    posy /= compteur ;
                                    if( img_in.img[posx][posy] != 255 ) {
                                        img_in.img[posx][posy] = 255 ;
                                        total_point++ ;
                                    }
                                    N = Nmax + 1 ;
                                }
                            }
                        }
                    }
                }
            }
        }
    }

    void visionErosionAndfeature(Image2D img_ori, Image2D img_in, int Nmax,PixelFeature *feat, int &nb_feat) {
        int p1, p2 ;
        int compteur, posx, posy ;
        nb_feat = -1 ;
        Image2D img_temp ;
        visionImageAlloc(img_temp, img_ori.width, img_ori.height) ;

        for( int x = Nmax ; x < img_in.width-Nmax ; x++) {
            for( int y = Nmax ; y < img_in.height-Nmax ; y++) {
                if( img_in.img[x][y] == 200 && img_in.img[x][y] != 255 ) {
                    for ( int N = 2 ; N <= Nmax ; N++ ) {
                        p1 = p2 = 0 ;
                        if( img_in.img[x-N][y] < 200 && img_in.img[x+N][y] < 200 && img_in.img[x][y-N] < 200 && img_in.img[x][y+N] < 200 ) {
                            for( int i = -N ; i <= N ; i++ ) {
                                for( int j = -N ; j <= N ; j++ ) {
                                    if( abs(i) == N || abs(j) == N ){
                                        if( img_in.img[x+i][y+j] < 200 ) {
                                            p1++ ;
                                        }
                                        p2 ++ ;
                                    }
                                }
                            }

                            if( p1 == p2 && p1 > 0 ) {
                                compteur = posx = posy = 0 ;

                                for( int i = -N ; i <= N ; i++ ) {
                                    for( int j = -N ; j <= N ; j++ ) {
                                        if ( img_in.img[x+i][y+j] == 200 ) {
                                            compteur++  ;
                                            posx += x+i ;
                                            posy += y+j ;
                                            img_in.img[x+i][y+j] = 0 ;
                                        }
                                    }
                                }

                                if ( compteur > 0 ) {
                                    posx /= compteur ;
                                    posy /= compteur ;
                                    if( img_in.img[posx][posy] != 255 ) {
                                        img_in.img[posx][posy] = 255 ;
                                        nb_feat++ ;
                                        //visionGetFeature2(img_ori, posx, posy, feat, nb_feat) ;
                                        visionGetFeature4(img_ori, img_temp, posx, posy, feat, nb_feat) ;
                                        //visionGetFeature3(img_ori, posx, posy, feat, nb_feat) ;
                                    }
                                    N = Nmax + 1 ;
                                }
                            }
                        }
                    }
                }
            }
        }

        visionImageFree(img_temp) ;
    }

//----------------------------------------------------

//Other
//----------------------------------------------------
    void visionVersion() {
        printf("\n\n---- VISION SYSTEM ----\n") ;
        printf("       %d.%d.%d\n\n", VERSIOND, VERSIONM, VERSIONY) ;
    }

    double get_time() {
        #ifdef __linux__
            struct timespec spec ;
            clock_gettime(CLOCK_REALTIME, &spec) ;
            return spec.tv_sec + (double) spec.tv_nsec/1000000000 ;
        #elif _WIN32
            SYSTEMTIME spec ;
            GetSystemTime(&spec) ;
            return spec.wMinute*60 + spec.wSecond + (double) spec.wMilliseconds/1000 ;
        #endif
    }

    void show_diff_time( double time_stop, double time_start, int type) {
        if ( type == 0 )
            printf("Execution time : %.3f s\t\t\t\t", time_stop - time_start) ;
        else if ( type == 1 ) {
            printf("%.3f fps\t\t", 1/(time_stop - time_start)) ;
        }
    }

    void do_system(const char *command) {
        sdlFillStdout() ;
        system(command) ;
        sdlGetStdout() ;
    }

    void HelpvisionGetCanvas() {
        printf("\n\n\nHi there !\n\n") ;
        printf("This program use the fonction 'visionGetCanvas'.\n") ;
        printf("What it does ?\n\n") ;
        printf("In short it will find all the points that can be on the canvas (the dot of you canvas cube).\n") ;
        printf("To do that it first use a threshold (default : 80) that you can change using the wheel of your mousse.\n") ;
        printf("You can also edit the circle used to determine if a point is a dot or not. You change this parameter by using your KeyPad Number 8 (for more) and 5 (for less) [Be carefull this parameter can give not expected responses when edited].\n") ;
        printf("When you think that your tweak is good, you can begin the real canvas finder :\n") ;
        printf("When you (human) see that all the dot of your canvas has been find push the Key 'Up Arrow'.\n") ;
        printf("The program will stop the loop and then you can delete the point than aren't needed by using the right button of your mouse.\n") ;
        printf("When you have 12 points (no more and no less!) then use the left button of the mouse in order to select the 4 dots that correspond of top of the canvas.\n") ;
        printf("When you get 4 top point and 8 points on the other face then the fonction will automaticaly find the respective coordinates of all dot.\n") ;
        printf("Then it will write a given '.txt' than you will use for the calibration in the next program.\n") ;
        printf("\nSummary of possible actions :\n") ;
        printf("-MOUSE WHEEL: increments or decrements the thresholding of the picture\n") ;
        printf("-NUM KEY 8 : increments the search size\n") ;
        printf("-NUM KEY 5 : decrements the search size\n") ;
        printf("-UP ARROW : begin the real canvas finder \n") ;
        printf("-DOWN ARROW : abort and search again for possible point\n") ;
        printf("-MOUSE LEFT BUTTON : select point as a dot of the top side\n") ;
        printf("-MOUSSE RIGHT BUTTON : delete point\n\n\n") ;
    }

    void Helpvision() {
        printf("\n\n\nHi there !\n\n") ;
        printf("This soft have been created as a standalone and it work on every platform (Windows, Linux ,[WIP OSX and LATER Android).\n") ;
        printf("All you need is at least a terminal with gcc/g++ and at best Code::Blocks\n") ;
        printf("This soft wil retrieve all your webcam and can do some image processing (POI, stereo, HOG, etc)\n\n") ;
        printf("\n\nWebcam output is 4/3 format, here are some exemple of resolution you MUST use :\n") ;
        printf("-300 / 225\n") ;
        printf("-320 / 240\n") ;
        printf("-400 / 300\n") ;
        printf("-440 / 330\n") ;
        printf("-460 / 345\n") ;
        printf("-560 / 420\n") ;
        printf("-640 / 480\n") ;
        printf("-800 / 600\n") ;
        printf("\n\nSome matrix that can be usefull :\n") ;
        printf("-Increase contrast (d = 1) :\n") ;
        printf("    0  -1   0\n") ;
        printf("   -1   5  -1\n") ;
        printf("    0  -1   0\n\n") ;
        printf("-Average / blur (d = 9) :\n") ;
        printf("    1   1   1\n") ;
        printf("    1   1   1\n") ;
        printf("    1   1   1\n\n") ;
        printf("-Gaussian blur (d = 16) :\n") ;
        printf("    1   2   1\n") ;
        printf("    2   4   2\n") ;
        printf("    1   2   1\n\n") ;
        printf("-Gaussian blur 2 (d = 33) :\n") ;
        printf("    2   4   2\n") ;
        printf("    4   9   4\n") ;
        printf("    2   4   2\n\n") ;
        printf("-Laplace (d = 1) :\n") ;
        printf("    0   1   0\n") ;
        printf("    1  -4   1\n") ;
        printf("    0   1   0\n\n") ;
        printf("-Laplace 2 (d = 1) :\n") ;
        printf("   0.5  1  0.5\n") ;
        printf("    1  -6   1\n") ;
        printf("   0.5  1  0.5\n\n") ;
        printf("-Laplace 3 (d = 1) :\n") ;
        printf("    1   1   1\n") ;
        printf("    1  -8   1\n") ;
        printf("    1   1   1\n\n") ;
        printf("-Accuraty / clearness (d = 1) :\n") ;
        printf("   -1  -1  -1\n") ;
        printf("   -1   9  -1\n") ;
        printf("   -1  -1  -1\n\n") ;
        printf("-Accuraty / clearness 2 (d = 9) :\n") ;
        printf("   -1  -1  -1\n") ;
        printf("   -1   17  -1\n") ;
        printf("   -1  -1  -1\n\n") ;
        printf("-Try 2 (d = 1) :\n") ;
        printf("    0   1   0\n") ;
        printf("   -1   0   0\n") ;
        printf("    0   0   2\n\n") ;
        printf("\nThank for using this software\n") ;
        printf("\nAbout me :\n") ;
        printf("RESSEGUIER Thomas\n") ;
        printf("Student in Industrial Vision System\n") ;
        printf("Mail : resseguier.thomas@gmail.com\n\n\n") ;
    }
//----------------------------------------------------






int*** newimg( int &width, int &height) {
	//Allocation de l'image en memoire
	int ***img = new int**[width] ;
	for(int x = 0 ; x < width ; x++) {
		img[x] = new int*[height] ;
		for(int y = 0 ; y < height ; y++) {
			img[x][y] = new int[3] ;
			img[x][y][0] = img[x][y][1] = img[x][y][2] = 0 ; //Au passage on initialise l'image a 0 ;
		}
	}

	return img ;
}


int** newmask(int &width, int &height) {
	//Allocation du masque en memoire
	int **mask = new int*[width] ;
	for(int x = 0 ; x < width ; x++) {
		mask[x] = new int[height] ;
        for(int y = 0 ; y < height ; y++)
			mask[x][y] = 0 ; //Au passage on initialise le masque a 0 ;
	}

	return mask ;
}

void freemask( int** mask, int &width) {
	//Efface les allocations en memoire du masque
	for(int x = 0 ; x < width ; x++) {
		free( mask[x] );
	}

	free( mask );
}


/*
    //int *circle21 = new int[21] ; //Ce tableau contiendra les valeur h, s et v des 20 pixels autour de celui analyse

            //Enregistre le point voulu en circle21[0]
            circle21[0] = imgin.img[x][y] ;

            //Cercle autour du pixel voulu (cercle21)
            circle21[1] = imgin.img[x][y-4] ;
            circle21[2] = imgin.img[x+1][y-4] ;
            circle21[3] = imgin.img[x+2][y-3] ;
            circle21[4] = imgin.img[x+3][y-2] ;
            circle21[5] = imgin.img[x+4][y-1] ;
            circle21[6] = imgin.img[x+4][y] ;
            circle21[7] = imgin.img[x+4][y+1] ;
            circle21[8] = imgin.img[x+3][y+2] ;
            circle21[9] = imgin.img[x+2][y+3] ;
            circle21[10] = imgin.img[x+1][y+4] ;
            circle21[11] = imgin.img[x][y+4] ;
            circle21[12] = imgin.img[x-1][y+4] ;
            circle21[13] = imgin.img[x-2][y+3] ;
            circle21[14] = imgin.img[x-3][y+2] ;
            circle21[15] = imgin.img[x-4][y+1] ;
            circle21[16] = imgin.img[x-4][y] ;
            circle21[17] = imgin.img[x-4][y-1] ;
            circle21[18] = imgin.img[x-3][y-2] ;
            circle21[19] = imgin.img[x-2][y-3] ;
            circle21[20] = imgin.img[x-1][y-4] ;

            //Vérification de I de chaque pixel par rapport au I de P
            for( i = 1 ; i < 21 ; i++ ) {
                if( abs(circle21[i] - circle21[0]) < gap ){
                    circle21[i] = 1 ; //Si le pixel est similaire alors il passe a 1
                    nb_pix ++ ; //Incremente le nombre de pixels valides dans le cercle

                    if ( circle21[i>10 ? i-10 : i+10] == 1 ) //On verifie que chaque point valide sur le cercle n'ai pas d'oppose lui aussi valide
                        i = 30 ; //Si le pixel oppose est lui aussi valide alors on passe "i" a 30 => on ne validera pas img[x][y] comme etant un corner et donc on accelere le changement de pixel
                } else
                    circle21[i] = 0 ; //Si le pixel est different alors il passe a 0
            }

            if ( nb_pix < 10 && i == 21 )
                imgout.img[x][y] = 255 ;
            else
                imgout.img[x][y] = 0 ;
*/

/*

//Difference of Gaussian
//Detection de bord
void dog(struct SimpleCapParams cap_in, struct SimpleCapParams cap_out, int R) {
	int x, y ,p , max ;
	max = 0 ;

    struct SimpleCapParams inter ;
    escapiCaptureAlloc(inter, cap_in.mWidth, cap_in.mHeight) ;
    CloneCapture(cap_in,inter) ;

	for( x = 0 ; x < cap_in.mWidth ; x++) {
		for( y = 0 ; y < cap_in.mHeight ; y++) {
            blurpix(cap_in, cap_out, x, y, R) ;
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

	for( x = 0 ; x < cap_in.mWidth ; x++) {
		for( y = 0 ; y < cap_in.mHeight ; y++) {
			imgout[x][y][0] = (imgout[x][y][0] * 255 ) / max ;
			imgout[x][y][1] = (imgout[x][y][1] * 255 ) / max ;
			imgout[x][y][2] = (imgout[x][y][2] * 255 ) / max ;
		}
	}

	for( x = 0 ; x < cap_in.mWidth ; x++){
		for( y = 0 ; y < cap_in.mHeight ; y++){
			int bw ;
			p = bw = 0 ;

			for(int i = -R ; i <= R ; i++){
				for(int j = -R ; j <= R ; j++){
					if(x + i >= 0 && x + i < cap_in.mWidth && y + j >= 0 && y + j < cap_in.mHeight){
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

	//Efface l allocation en memoir
        for( int i = 0 ; i < canvas_black_corrected[0].nb ; i++ )
            if( canvas_black_corrected[i].x < center_black.x )
                l = r = b = t = i ;e de l'image intermediaire
	for( x = 0 ; x < width ; x++) {
		for( y = 0 ; y < height ; y++) {
			free( inter[x][y] );
		}
		free( inter[x] );
	}

	free( inter );
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
*/
