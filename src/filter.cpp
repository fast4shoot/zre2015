#include "filter.h"


filter::filter()
{

}

filter::~filter()
{

}


//% A - matrix with predictor coefficients (each vector in a column, no a0 coefficient)
//takze predpokladam ze b0 je vzdy 1
float filter::do_step(std::vector<float> &fir_part, std::vector<float> &iir_part, float signal){


    //correcting the size of state and incoming coeficients to match
    while(state.size() < fir_part.size() || state.size() < iir_part.size())
        state.push_back(0.0f);

    while(fir_part.size() < iir_part.size())
        fir_part.push_back(0.0f);

    while(iir_part.size() < fir_part.size())
        iir_part.push_back(0.0f);


    //bigger index -> bigger delay

    float history = 0.0f;

    for(int i = 0; i < state.size(); i++){

        history += state[i] * fir_part[i] - state[i] * iir_part[i];

    }


    float result = signal + history;

    state.push_front(signal);   //signal pro priste se spozdenim 1
    state.pop_back();   //a posledni se zahodi..

return result;
}
