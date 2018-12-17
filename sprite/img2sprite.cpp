#define cimg_display 0
//#define cimg_OS 0
#include "CImg.h"
#include <iostream>
#include <fstream>

using namespace cimg_library;

int main( int argc, char* argv[] )
{
    if( argc!=3 )
    {
        std::cout << "USAGE:" << std::endl;
        std::cout << "img2sprite <imagefile> <binfile>" << std::endl;
    }


    try 
    {
        std::cout << "Loading " << argv[1] << std::endl;
        CImg< unsigned char > fontimg( argv[1] );
        std::cout << "Image size: " << fontimg.width() << "x" << fontimg.height() << std::endl;


        unsigned int dimensions[2] = {fontimg.width(), fontimg.height()};
        std::ofstream ofs( argv[2], std::ios::binary );
        ofs.write( (char*)dimensions, 2*sizeof(unsigned int) );

        cimg_for( fontimg, ptr, unsigned char ) { 
            unsigned char val = (*ptr)>128 ? 0xFF : 0x00;
            ofs << val;
        }

        ofs.flush();

        std::cout << ofs.tellp() << " bytes written." << std::endl;
        ofs.close();
        
    } 
    catch (CImgIOException& ex )
    {
        std::cout << "Error: " << ex.what() << std::endl;
    }


    return 0;
}
