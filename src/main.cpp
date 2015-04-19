#include <iostream>
#include <fstream>
#include <cstdint>
#include <cmath>
#include <vector>
#include <sstream>
#include <random>
#include <cassert>
#include <algorithm>

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
	std::cerr << "  Using:\n          zre_proj1_linux cb_lpc.txt cb_gain.txt in.cod out.wav\n";
	std::cerr << "\n\n  cb_lpc.txt\tname of code book for LPC parameters";
	std::cerr << "\n  cb_gain.txt\tname of code book for gain";
	std::cerr << "\n  in.cod\tname of input encoded file";
	std::cerr << "\n  out.wav\tname of output decoded file";
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
 * 	params: LPCIndex - vector of uints
 * 		GainIndex - vector of uints
 * 		LIndex - vector of uints
 *		input = input stream to file
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

// normalizuje zesileni
void normalize(std::vector<float>& gains)
{
	auto minmax = std::minmax_element(gains.begin(), gains.end());
	auto mult = 1.0 / std::max(std::abs(*minmax.first), std::abs(*minmax.second));
	for (auto& gain : gains) gain *= mult;
}

// Decodes gains, one at a time
float decode_gain(uint index, const std::vector<uint>& indices, const std::vector<float>& gains)
{
	return gains.at(indices.at(index) - 1);
}

// Decodes LPC coeffs, one at a time
std::vector<float> decode_lpc(uint index, const std::vector<uint>& indices, const std::vector<float>& coeffs, uint order)
{
	index = indices.at(index) - 1;
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
	// Tohle predpoklada little-endian masinu, ale to snad prezijeme, zatim
	
	uint16_t bytes_per_sample = 2;
	
	uint32_t file_size = signal.size() * bytes_per_sample + 36; // 36 je velikost hlavicky bez RIFF hlavicky
	uint32_t wave_section_chunk_size = 16;
	uint16_t wave_type_format = 1; // PCM
	uint16_t number_of_channels = 1;
	uint32_t sample_rate = 8000;
	uint32_t bytes_per_second = sample_rate * bytes_per_sample; // dva bajty na sample
	uint16_t alignment = 1 * bytes_per_sample; // jeden kanal, dva bajty na sample
	uint16_t bits_per_sample = bytes_per_sample * 8;
	uint32_t data_size = signal.size() * bytes_per_sample;
	
	file.write("RIFF", 4);
	file.write(reinterpret_cast<char*>(&file_size), 4);
	file.write("WAVE", 4);
	file.write("fmt ", 4);
	file.write(reinterpret_cast<char*>(&wave_section_chunk_size), 4);
	file.write(reinterpret_cast<char*>(&wave_type_format), 2);
	file.write(reinterpret_cast<char*>(&number_of_channels), 2);
	file.write(reinterpret_cast<char*>(&sample_rate), 4);
	file.write(reinterpret_cast<char*>(&bytes_per_second), 4);
	file.write(reinterpret_cast<char*>(&alignment), 2);
	file.write(reinterpret_cast<char*>(&bits_per_sample), 2);
	file.write("data", 4);
	file.write(reinterpret_cast<char*>(&data_size), 4);
	
	
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
	
	uint lpc_order,
	uint frame_length)
{
	uint no_of_frames = gains_indices.size();
	
	Filter filter(1, lpc_order + 1);
	std::vector<float> ss(no_of_frames * frame_length,0);
	
	std::default_random_engine generator;
	std::normal_distribution<float> distribution(0.0,1.0);

	uint excitation_start = 0;
	uint prev_lag = 0;
	
	for (uint frame_idx = 0; frame_idx < no_of_frames; frame_idx++)
	{
		auto gain = decode_gain(frame_idx, gains_indices, gains);
		auto fir = std::vector<float>(1, gain);
		auto iir = decode_lpc(frame_idx, coeffs_indices, coeffs, lpc_order);
		
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
				excitation = distribution(generator);
			}
			else
			{
				auto excitation_cycle = global_sample_idx - excitation_start;
				if (excitation_cycle < 2)// * 8 < used_lag)
					excitation = 1.0f;
				else 
					excitation = -1.0f;
					
				if (excitation_cycle == used_lag - 1) // jsme na konci cyklu?
					excitation_start = global_sample_idx + 1;
			}
			
			ss[global_sample_idx] = excitation;
		}
		
		float frame_power = 0.0f;
		for (uint local_sample_idx = 0; local_sample_idx < frame_length; local_sample_idx++)
		{
			uint global_sample_idx = frame_start_idx + local_sample_idx;
			auto sample = ss.at(global_sample_idx);
			frame_power += sample * sample;
		}
		
		float sample_power_mult = 1.0f / std::sqrt(frame_power * frame_length);
		for (uint local_sample_idx = 0; local_sample_idx < frame_length; local_sample_idx++)
		{
			uint global_sample_idx = frame_start_idx + local_sample_idx;
			float sample = ss.at(global_sample_idx) * sample_power_mult;
			float filtered = filter.do_step(fir, iir, sample);
			ss.at(global_sample_idx) = filtered;
		}
		
		
		prev_lag = lag;
	}
	
	normalize(ss);
	return ss;
}


int main(int argc, char **argv)
{
	//checking number of params
	if(argc < 5){
		std::cerr << "\n\nERROR: Required parameters are missing\n";
		help();
		return ERR;
	}	

	// open files	
	std::ifstream lpc_file (argv[1]);
	std::ifstream gain_file (argv[2]);
	std::ifstream input_file (argv[3]);
	std::ofstream output (argv[4], std::ofstream::binary);


	std::vector<float> LPCCodebook;
	std::vector<float> GainCodebook;
	std::vector<uint> LPCIndex;
	std::vector<uint> GainIndex;
	std::vector<uint> LIndex;
	
	// load LPC code book
	if(!load_book(LPCCodebook, lpc_file)){
    		std::cerr << "\n\nERROR: lpc codebook loading fail\n";
		return ERR;
	}

	// load Gain code book
	if(!load_book(GainCodebook, gain_file)){
        	std::cerr << "\n\nERROR: gain codebook loading fail\n";
	        return ERR;
    	}

	// load input .cod file
	if(!load_cod_file(LPCIndex, GainIndex, LIndex, input_file)){
		std::cerr << "\n\nERROR: input loading fail\n";
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
