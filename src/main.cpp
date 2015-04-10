#include<iostream>
#include<fstream>

#include<cmath>

#define OK 0
#define ERR 1

void help()
{
	std::cerr << "\n";
	std::cerr << "-------  HELP  -------\n\n";
	std::cerr << "  Using:\n          zre_proj1_linux cb_lpc.txt cb_gain.txt in.cod out.raw\n";
	std::cerr << "\n\n  cb_lpc.txt\tname of code book for LPC parameters";
	std::cerr << "\n  cb_gain.txt\tname of code book for gain";
	std::cerr << "\n  in.cod\tname of input encoded file";
	std::cerr << "\n  out.raw\tname of output decoded file";
	std::cerr << "\n\n  Authors:  Zdenek Biberle (xbiber00)\n";
	std::cerr << "            Vit Hodes (xhodes00)\n";
	std::cerr << "            Josef Ridky (xridky00)\n\n";
	std::cerr << "\n--------------------\n\n";

}

int main(int argc, char **argv)
{
	//checking number of params
	if(argc < 5){
		std::cerr << "\n\nERROR: Required parameters are missing !!!\n";
		help();
		return ERR;
	}	

	// open files	
	std::ifstream lpc (argv[1]);
	std::ifstream gain (argv[2]);
	std::ifstream input (argv[3]);
	std::ofstream output (argv[4]);


	// close files
	lpc.close();
	gain.close();
	input.close();
	output.close();
	return OK;
}
