#include<iostream>
#include<fstream>

#include<cmath>
#include <vector>
#include <sstream>
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


bool load_book(std::vector<float> &book, std::ifstream &opened_stream){

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

     } else return false;

return true;
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

    std::vector<float> LPCcodebook;
    std::vector<float> GainCodebook;


    if(not load_book(LPCcodebook, lpc)){
        std::cerr << "\n\nERROR: lpc codebook loading fail!!\n";
        return ERR;
    }

    if(not load_book(GainCodebook, gain)){
        std::cerr << "\n\nERROR: gain codebook loading fail!!\n";
        return ERR;
    }


/*
    function ss=decoder(filecod, filewav);
    % function ss=decoder(filewav, filecod);
    %
    % ZRE decoder 1050 bits per second
    % filecod - name of file with coded input.
    % filewav - name of synthesized wav file (8kHz, 1 channel, lin)
    % s - an auxiliary output with the synthesized signal.
    % makes use of codebooks cb210.txt for A coefficients and gcb64.txt for gain

    load cb210.txt
    load gcb64.txt

    % reading the file
    [asym,gsym,L] = textread (filecod,'%d%d%d');

    Adecoded = cb210(:,asym);
    Gdecoded = gcb64(:,gsym);

    % and synthesis
    ss = synthesize (Adecoded,Gdecoded,L,10,160);

    % write it out
    wavwrite (ss,8000,16,filewav);
 */

/*
    function ss = synthesize(A,G,L,P,lram);
    % ss = syntnoise(A,G,P,lram);
    %
    % synthesizes signal excited by white noise (unvoiced) or periodic pulses (voiced)
    % by a matrix with predictor parameters and vector of gains.
    % A - matrix with predictor coefficients (each vector in a column, no a0 coefficient)
    % G - vector with gains (row, each element is one gain).
    % L - lags (row, zero means unvoiced, lag is in samples)
    % P - order of predictor
    % lram - length of window. The function DOES NOT SUPPORT OVERLAPPED FRAMES!
    %
    % output:
    % ss - a long row vector with the resulting signal.
    %
    % the function does not do much error-checking, so if A, G or L are of bad sizes,
    % sorry...
    %
    % Fri May  6 13:26:22 CEST 2011 - corrected  bug lenghtening the sig by 1 frame and
    %  putting zeros first (marked !!!)



    Nram = length(G);
    init=zeros(P,1);        % initial conditions of the filter
    ss = zeros(1,Nram * lram); % preallocation is needed for long signals.

    % some initial values - position of the next pulse for voiced frames (Matlab indexing)
    nextvoiced = 1;

    from = 1; to = from + lram -1;
    for n = 1:Nram,
      a = [1; A(:,n)]; % appending with 1 for filtering
      g = G(n);
      l = L(n);

      % in case the frame is unvoiced, generate noise
      if l == 0,
        excit = randn (1,lram); % this has power one ...
      else % if it is voiced, generate some pulses
        where = nextvoiced:l:lram;
        % ok, this is for the current frame, but where should be the 1st pulse in the
        % next one ?
        nextvoiced = max(where) + l - lram;
        % generate the pulses
        excit = zeros(1,lram); excit(where) = 1;
      end
      % and set the power of excitation  to one - no necessary for noise, but anyway ...
      power = sum(excit .^ 2) / lram;
      excit = excit / sqrt(power);
      % check
    %  power = sum(excit .^ 2) / lram

      % now just generate the output
      [synt,final] = filter (g,a,excit,init);
      ss(from:to) = synt; % !!! this line was originally at the end.
      init = final;
      from = from + lram; to = from + lram -1;
    end
*/


	// close files
	lpc.close();
	gain.close();
	input.close();
	output.close();
	return OK;
}
