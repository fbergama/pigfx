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
        std::cout << "png2xterm256 <imagefile> <binfile>" << std::endl;
        return 0;
    }


    try 
    {
        std::cout << "Loading " << argv[1] << std::endl;
        CImg< unsigned char > fontimg( argv[1] );
        std::cout << "Image size: " << fontimg.width() << "x" << fontimg.height() << std::endl;


        int dimensions[2] = {fontimg.width(), fontimg.height()};
        std::ofstream ofs( argv[2], std::ios::binary );
        //ofs.write( (char*)dimensions, 2*sizeof(unsigned int) );

        unsigned int x, y;
        unsigned char r,g,b;
        for (y=0; y<fontimg.height(); y++)
        {
            for (x=0; x<fontimg.width(); x++)
            {
                r = fontimg(x, y, 0, 0);
                g = fontimg(x, y, 0, 1);
                b = fontimg(x, y, 0, 2);
                unsigned char val = 16 + (round((double)r / 255 * 5) * 36) + (round((double)g / 255 * 5) * 6) + round((double)b / 255 * 5);
                ofs << val;
                //std::cout << "y:" << y << ", x:" << x << ", r:" << (int)r << ", g:" << (int)g << ", b:" << (int)b << ", val:" << (int)val << std::endl;
            }
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
