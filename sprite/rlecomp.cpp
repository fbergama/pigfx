#include <iostream>
#include <fstream>

using namespace std;

int main( int argc, char* argv[] )
{
    if( argc!=3 )
    {
        std::cout << "USAGE:" << std::endl;
        std::cout << "rlecomp <source> <dest>" << std::endl;
        return 0;
    }

    std::cout << "Loading " << argv[1] << std::endl;
    ifstream inputfile;
    inputfile.open(argv[1], ios::binary | ios::ate);
    unsigned int filesize = inputfile.tellg();
    std::cout << "Input is " << filesize << " bytes" << std::endl;
    inputfile.close();
    inputfile.open(argv[1], ios::binary | ios::in);
    
    std::ofstream ofs( argv[2], std::ios::binary );
    
    unsigned int n = 1;
    char ch = 0;
    char last_char;
    inputfile.read(&last_char, 1);
    for (unsigned int i=1; i<=filesize; i++)
    {
        if (i<filesize) inputfile.read(&ch, 1);
        if ((ch != last_char) || (n == 255) || i==filesize) /* Symbol/Wiederholungen-Tupel ausgeben, falls ein anderes Symbol kommt oder die maximal darstellbare Anzahl erreicht. */
        {
            //if (i==filesize) std::cout << "byte " << (int)last_char << (int)ch << " " << n << " times" << std::endl;
            ofs << last_char;
            ofs << (char)n;
            /*ofs.write( &prev_ch, 1 );
            ofs.write( &n, 1 );*/

            n = 0; /* Beginn einer neuen Folge */
        }
        last_char = ch;
        n++;
    }
    ofs.flush();
    std::cout << ofs.tellp() << " bytes written." << std::endl;
    ofs.close();
    inputfile.close();

    /*int dimensions[2] = {fontimg.width(), fontimg.height()};
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
    ofs.close();*/

    return 0;
}
