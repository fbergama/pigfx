#include <iostream>
#include <fstream>

using namespace std;

int main( int argc, char* argv[] )
{
    if( argc!=3 )
    {
        std::cout << "USAGE:" << std::endl;
        std::cout << "bin2dec <source> <dest>" << std::endl;
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
    char high, mid, low;
    for (unsigned int i=0; i<filesize; i++)
    {
        inputfile.read((char*)&binchar, 1);
        high = binchar / 100;
        binchar = binchar % 100;
        
        mid = binchar / 10;
        
        low = binchar % 10;
        
        if (high > 0)
        {
            high += '0';
            ofs << high;
        }
        if ((high > 0) || (mid > 0))
        {
            mid += '0';
            ofs << mid;
        }
        low += '0';
        ofs << low;
        
        ofs << ';';
    }

    ofs.flush();
    std::cout << ofs.tellp() << " bytes written." << std::endl;
    ofs.close();
    inputfile.close();

    return 0;
}
