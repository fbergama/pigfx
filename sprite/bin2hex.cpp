#include <iostream>
#include <fstream>

using namespace std;

int main( int argc, char* argv[] )
{
    if( argc!=3 )
    {
        std::cout << "USAGE:" << std::endl;
        std::cout << "bin2hex <source> <dest>" << std::endl;
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
    
    
    unsigned char binchar;
    char high, low;
    for (unsigned int i=0; i<filesize; i++)
    {
        inputfile.read((char*)&binchar, 1);
        high = binchar / 16;
        low = binchar % 16;
        if (high > 0)
        {
            if (high < 10) high += '0';
            else high = high + 'A' - 10;
            ofs << high;
        }
        if (low < 10) low += '0';
        else low = low + 'A' - 10;
        ofs << low;
        ofs << ';';
    }

    ofs.flush();
    std::cout << ofs.tellp() << " bytes written." << std::endl;
    ofs.close();
    inputfile.close();

    return 0;
}
