#define cimg_display 0
//#define cimg_OS 0
#include "CImg.h"
#include <iostream>
#include <fstream>

using namespace cimg_library;

int main( int argc, char* argv[] )
{
    std::cout << "Loading " << argv[1] << std::endl;

    try 
    {
        CImg< unsigned char > fontimg( argv[1] );
        std::cout << "Image size: " << fontimg.width() << "x" << fontimg.height() << std::endl;

        if( fontimg.width() % 8 != 0 )
        {
            std::cout << "Width should be multiple of 8" << std::endl;
            return -1;
        }
        if( fontimg.height() % 8 != 0 )
        {
            std::cout << "Height should be multiple of 8" << std::endl;
            return -1;
        }

        int nrows = fontimg.height() / 8;
        int ncols = fontimg.width() / 8;

        if( nrows*ncols != 256 ) 
        {
            std::cout << "Image does not contain 256 characters." << std::endl;
            return -1;
        }


        std::ofstream ofs( "font8x8.bin", std::ios::binary );

        for( unsigned int i=0; i<nrows; ++i )
        {
            for( unsigned int j=0; j<ncols; ++j )
            {
                for( unsigned int y=i*8; y<i*8+8; ++y )
                {
                    for( unsigned int x=j*8; x<j*8+8; ++x )
                    {
                        unsigned char v = fontimg( x,y );
                        v = v==0 ? 0xFF : 0x0;
                        ofs << v;
                        std::cout << (v==0xFF ? "*" : " ") ;
                    }
                    std::cout << std::endl;
                }

            }
            int aa;
            std::cin >> aa;
        }
        ofs.flush();
        ofs.close();
        
    } 
    catch (CImgIOException& ex )
    {
        std::cout << "Error: " << ex.what() << std::endl;
    }


    return 0;
}
