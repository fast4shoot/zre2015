#include <iostream>
#include <fstream>
#include <cstdint>
#include <cmath>
#include <vector>
#include <sstream>
#include <random>
#include <cassert>

#define OK 0
#define ERR 1
#define LPCD true
#define GAIND false

typedef unsigned int uint;

#include "filter.h"
/*
 * void help()
 * 	print help on standard error output
 * 	
 * 	return: void
 */

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



/*
 * bool load_book(book, opened_stream)
 *	load data from opened_stream into book
 *
 *	params: book - vector of floats
 *		opened_stream - input stream to file
 *
 *	return: bool
 */

bool load_book(std::vector<float> &book, std::ifstream &opened_stream)
{

	if (opened_stream.is_open()) {
		std::string line;

		while ( getline (opened_stream,line) )
		{
			std::istringstream number_row(line);
			float tmp;

			while(number_row >> tmp){
				book.push_back(tmp);
			}
		}
	} else {
		return false;
	}

	return true;
}

/*
 * bool load_cod_file(LPCIndex, GainIndex, LIndex, input)
 * 	load indexes from input and store them into LPCIndex, GainIndex and LIndex
 * 	
 * 	params: LPCIndex - vector of ints
 * 		GainIndex - vector of ints
 * 		LIndex - vector of ints
 *		imput = input stream to file
 *
 * 	return: bool
 */

bool load_cod_file(std::vector<uint> &LPCIndex, std::vector<uint> &GainIndex, std::vector<uint> &LIndex, std::ifstream &input)
{
	if(input.is_open()){
		std::string line;

		while( getline(input,line) ){
			std::istringstream index_line(line);
			int index;

			index_line >> index; 
			LPCIndex.push_back(index);

			index_line >> index;	
			GainIndex.push_back(index);
		
			index_line >> index;
			LIndex.push_back(index);
		}	

	} else {
		return false;
	}

	return true;
}

// Decodes gains, one at a time
float decode_gain(uint index, const std::vector<uint>& indices, const std::vector<float>& gains)
{
	return gains.at(indices.at(index) - 1);
}

// Decodes LPC coeffs, one at a time
std::vector<float> decode_lpc(uint index, const std::vector<uint>& indices, const std::vector<float>& coeffs, uint order)
{
	uint step = coeffs.size() / order;
	assert(step * order == coeffs.size());
	std::vector<float> result(order + 1);
	result[0] = 1.0;
	for (uint i = 0; i < order; i++)
	{
		result[i + 1] = coeffs.at(index + i * step);
	}
	return result;
}

void write_output(const std::vector<float>& signal, std::ofstream& file)
{
	for (auto sample : signal)
	{
		if (sample > 1.0f || sample < -1.0f) std::cout << "sample: " << sample << std::endl;
		auto sample_in_range = std::max(-1.0f, std::min(1.0f, sample));
		int16_t sample_value = int16_t(sample_in_range < 0.0 ? sample_in_range * 32768 : sample_in_range * 32767);
		file.write(reinterpret_cast<char*>(&sample_value), 2);
	}
}

// chaby pokus o prepsani synthesis funkce ......
// nejlepe smazat a udelat znovu ......
std::vector<float> synthesis(
	const std::vector<float>& coeffs,
	const std::vector<uint>& coeffs_indices,
	
	const std::vector<float>& gains,
	const std::vector<uint>& gains_indices,
	
	const std::vector<uint>& lags,
	
	uint lpcOrder,
	uint frame_length)
{
	uint no_of_frames = gains_indices.size();
	
	Filter filter;
	std::vector<float> iir(0, 0.0);
	std::vector<float> ss(no_of_frames * frame_length,0);
	

	uint excitation_start = 0;
	uint prev_lag = 1;
	
	for (uint frame_idx = 0; frame_idx < no_of_frames; frame_idx++)
	{
		auto fir = decode_lpc(frame_idx, coeffs_indices, coeffs, lpcOrder);
		auto gain = decode_gain(frame_idx, gains_indices, gains);
		uint lag = lags.at(frame_idx);
		uint frame_start_idx = frame_idx * frame_length;
		
		for (uint local_sample_idx = 0; local_sample_idx < frame_length; local_sample_idx++)
		{
			uint global_sample_idx = frame_start_idx + local_sample_idx;
			float excitation = 0.0;
			
			uint used_lag;
			
			if (excitation_start < frame_start_idx)
				used_lag = prev_lag;
			else
				used_lag = lag;
			
			if (used_lag == 0)
			{
				excitation_start = global_sample_idx + 1;
			}
			else
			{
				auto excitation_cycle = global_sample_idx - excitation_start;
				if (excitation_cycle * 2 < used_lag)
					excitation = 1.0f;
				else 
					excitation = -1.0f;
					
				if (excitation_cycle == used_lag - 1) // jsme na konci cyklu?
					excitation_start += used_lag;
			}
			
			float filtered = filter.do_step(fir, iir, excitation);
			float amplified = filtered * gain;
			ss[global_sample_idx] = amplified;
		}
		
		prev_lag = lag;
	}
	
	return ss;
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
	std::ifstream lpc_file (argv[1]);
	std::ifstream gain_file (argv[2]);
	std::ifstream input_file (argv[3]);
	std::ofstream output (argv[4]);


	std::vector<float> LPCCodebook;
	std::vector<float> GainCodebook;
	std::vector<uint> LPCIndex;
	std::vector<uint> GainIndex;
	std::vector<uint> LIndex;
	
	// load LPC code book
	if(!load_book(LPCCodebook, lpc_file)){
    		std::cerr << "\n\nERROR: lpc codebook loading fail!!\n";
		return ERR;
	}

	// load Gain code book
	if(!load_book(GainCodebook, gain_file)){
        	std::cerr << "\n\nERROR: gain codebook loading fail!!\n";
	        return ERR;
    	}

	// load input .cod file
	if(!load_cod_file(LPCIndex, GainIndex, LIndex, input_file)){
		std::cerr << "\n\nERROR: input loading fail !!! \n";
		return ERR;
	}
	
	// synthesis
	auto signal = synthesis(LPCCodebook, LPCIndex, GainCodebook, GainIndex, LIndex, 10, 160);
	write_output(signal, output);
	/* control prints
 
	std::cout << LPCCodebook.size() << std::endl << GainCodebook.size() << std::endl;
	std::cout << LPCIndex.size() << std::endl << GainIndex.size() << std::endl << LIndex.size() << std::endl;
	std::cout << LPCDecode.size() << std::endl << GainDecode.size() << std::endl;
	*/

	// close files
	lpc_file.close();
	gain_file.close();
	input_file.close();
	output.close();
	return OK;
}
