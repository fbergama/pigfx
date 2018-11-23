#define cimg_display 0
//#define cimg_OS 0
#include "CImg.h"
#include <iostream>
#include <fstream>
#include <string>
#include <stdlib.h>

using namespace cimg_library;
using std::string;
using std::cout;
using std::endl;

/** Reads a PNG file and builds a BIN font file.
 *  PNG can be any size but each pixel is considered a character pixel.
 *
 *  Width of font must be a divider of the PNG width.
 *  The PNG height must be at least the font height.
 *
 *  The PNG is scanned for up to 256 characters. They
 *  are scanned horizontally first, for sets of lines
 *  based on the font height. The PNG width divided by
 *  the font width gives the number of characters
 *  which are stored horizontally.
 *
 *  Parameters:
 *
 *  -i <Image>       : path to the PNG file
 *  -w <Font_width>  : number of pixels wide for a character
 *  -h <Font_height> : number of lines for a character
 *  -o <File>        : path to the output BIN file
 *  -c 0             : PNG displays black chareacters on white background
 *  -c 1             : PNG displays white characters on black background
 
 * Original program by Filippo Bergamasco.
 * Extension to more file and formats by Francis Pierot.
 *
 */
int main( int argc, char* argv[] )
{
	int fontwidth = 0;
	int fontheight = 0;
	bool black_ON = true; // black
	bool quiet = false;
	string pngfile;
	string binfile;

	// Parse parameters
	for (int i = 1 ; i < argc ; i++) {
		string arg = string(argv[i]);
		if (arg.compare("-i")==0) {
			i = i+1;
			if (i < argc) {
				pngfile = string(argv[i]);
			} else {
				cout << "-i <filepath.png> missing file path" << endl;
				return(1);
			}
		} else if (arg.compare("-o")==0) {
			i = i+1;
			if (i < argc) {
				binfile = string(argv[i]);
			} else {
				cout << "-o <filepath.bin> missing file path" << endl;
				return(1);
			}
		} else if (arg.compare("-w")==0) {
			i = i+1;
			if (i < argc) {
				fontwidth = atoi(argv[i]);
			} else {
				cout << "-w <font width> missing font width" << endl;
				return(1);
			}
		} else if (arg.compare("-h")==0) {
			i = i+1;
			if (i < argc) {
				fontheight = atoi(argv[i]);
			} else {
				cout << "-h <font height> missing font height" << endl;
				return(1);
			}
		} else if (arg.compare("-c")==0) {
			i = i+1;
			if (i < argc) {
				black_ON = atoi(argv[i]) == 0;
			} else {
				cout << "-c <0|1> missing ON color parameter" << endl;
				return(1);
			}
		} else if (arg.compare("-q")==0) {
			quiet = true;
		} else {
			if (arg.compare("-?") != 0) {
				cout << "Unknown parameter " << arg << endl;
				return(1);
			}
			cout << "Builds a PIGFX binary BIN font bitmap file from a PNG file. " << endl;
			cout << "Syntax:" << endl;
			cout << "    buildfont -i <pngpath> -o <binpath> -w <fontwidth> -h <fontheight> [-c <ONcolor>] [-q]" << endl;
			cout << "    -i <pngpath>      Full path to the PNG file containing an image of the font" << endl;
			cout << "    -o <binfile>      Full path to the output BIN file for PIGFX." << endl;
			cout << "    -w <font width>   Number of horizontal pixels for each character" << endl;
			cout << "    -h <font height>  Number of lines for each character" << endl;
			cout << "    -c <ONcolor>      Color value to set a pixel ON, 0 for black (default) or 1 for anything not black" << endl;
			cout << "    -q                Quiet mode (no ASCII output)" << endl;
			cout << "Exit codes:" << endl;
			cout << "    0    ok" << endl;
			cout << "    1    unknown or missing parameter" << endl;
			cout << "    2    invalid parameter value" << endl;
			cout << "    3    error while processing" << endl;
			return(0);
		}
	}

	// Check parameters values
	if (fontwidth <= 0) {
		cout << "Invalid font width (-w): " << fontwidth << endl;
		return(2);
	}
	if (fontheight <= 0) {
		cout << "Invalid font height (-w)" << endl;
		return(2);
	}

	// Load the PNG
	try {
		CImg<unsigned char> fontimg(pngfile.c_str());

		// Check the image width
		int remain = fontimg.width() % fontwidth;
		if (remain != 0) {
			cout << "PNG width is not a multiple of font width" << endl;
			return(2);
		}
		if (fontimg.height() < fontheight) {
			cout << "PNG height is less than font height" << endl;
			return(2);
		}

		// Build the image
		int charnum = 0;
		int curline = 0;

    	// output file
        std::ofstream ofs( binfile, std::ios::binary );

        // Build each line
        do {
        	// For each column
        	int nbcols = fontimg.width() / fontwidth;
        	for (int c = 0 ; (c < nbcols) && (charnum < 256); c++) {
        		// send current character :
        		//      curline to curline+fontheight
        		//      column c to c+fontwidth
        		if (!quiet) cout << "Character: " << charnum << endl;
        		for (int y = curline ; y < curline + fontheight ; y++) {
        			for (int x = c * fontwidth ; x < (c+1) * fontwidth ; x++) {
                        unsigned char v = fontimg( x,y );
                        if (black_ON)
                        	v = v==0 ? 0xFF : 0x0;
                        else
                        	v = v!=0 ? 0xFF : 0x0;
                        ofs << v;
                        if (!quiet) cout << (v==0xFF ? "*" : ".") ;
        			} // next pixel on line
        			if (!quiet) cout << endl;
        		} // next character line
               	charnum += 1;
        	} // next character on current line

        	// ignore characters after 256th
        	if (charnum > 255)
           		break; // finish now

        	// this set of lines has been read, go next
        	curline += fontheight;

        } while (curline < fontimg.height());

        // close output file
        ofs.flush(); //?? probably useless
        ofs.close();

    }  catch (CImgIOException& ex ) {
        cout << "Error: " << ex.what() << endl;
        return(3);
    }

    return 0;
}
