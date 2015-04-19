#include "filter.h"
#include <cassert>

Filter::Filter(unsigned int fir_size, unsigned int iir_size):
	input_state(fir_size),
	output_state(iir_size - 1)
{
}

Filter::~Filter()
{
}


// predpokladame, ze iir_part[0] je 1.0
float Filter::do_step(std::vector<float> &fir_part, std::vector<float> &iir_part, float signal)
{
	typedef unsigned int uint;
	
	assert(fir_part.size() == input_state.size());
	assert(iir_part.size() == output_state.size() + 1);
	
	input_state.pop_back();
	input_state.push_front(signal);

	float result = 0.0;

	for (uint i = 0; i < fir_part.size(); i++)
	{
		result += input_state.at(i) * fir_part.at(i);
	}
	
	for (uint i = 0; i < iir_part.size() - 1; i++)
	{
		result -= output_state.at(i) * iir_part.at(i + 1);
	}

	output_state.pop_back();
	output_state.push_front(result);

	return result;
}
