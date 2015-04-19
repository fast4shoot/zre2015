#ifndef FILTER_H
#define FILTER_H

#include <deque>
#include <vector>

/*
Filtr, stav si uchovava sam.
Velikost jeho stavu zavisi na poctu vlozenych koeficientu, ktery by se ale nemel menit..
*/
class Filter
{
public:
    Filter(unsigned int fir_size, unsigned int iir_size);
    ~Filter();


    float do_step(std::vector<float> &fir_part, std::vector<float> &iir_part, float signal);
    
    std::deque<float> input_state;
    std::deque<float> output_state;
};

#endif // FILTER_H
