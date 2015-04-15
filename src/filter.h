#ifndef FILTER_H
#define FILTER_H

/*
Filtr, stav si uchovava sam.
Velikost jeho stavu zavisi na poctu vlozenych koeficientu, ktery by se ale nemel menit..
*/
class filter
{
public:
    filter();

    ~filter();

    std::deque state;

    float do_step(std::vector<float> &fir_part, std::vector<float> &iir_part, float signal);
};

#endif // FILTER_H
