#include<iostream>
#include<fstream>

#include<cmath>
#include <vector>
#include <sstream>
#include <random>
#define OK 0
#define ERR 1
#define LPCD true
#define GAIND false

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

bool load_cod_file(std::vector<int> &LPCIndex, std::vector<int> &GainIndex, std::vector<int> &LIndex, std::ifstream &input)
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

/*
 * void decode(source, index, result, type)
 * 	copy data from source at specific index into result by choosing type of processing 
 *
 * 	params: source - vector of floats
 * 		index - vector of ints
 * 		result - vector of floats
 * 		type - boolean
 *
 * 	return: void
 */

void decode(std::vector<float> &source, std::vector<int> &index, std::vector<float> &result, bool type)
{
	if(type == LPCD){
	
		for(int i = 0; i < index.size(); i++){

			int j = (index[i]-1) * 10, end = j + 10; 

			for(; j < end; j++){
				result.push_back(source[j]);
			}
		}

	} else {

		for(int i = 0; i < index.size(); i++){
                        result.push_back(source[index[i]-1]);
                }

	}
}


// chaby pokus o prepsani synthesis funkce ......
// nejlepe smazat a udelat znovu ......
void synthesis(std::vector<float> decodedLPC, std::vector<float> decodedGains, std::vector<int> Lags){

    int P = 10, frameLength = 160;
    int no_of_frames = decodedGains.size();
	
    Filter filter;

	//initial conditions of filter
    //std::vector<float> init(P,0);  //zbytecne
    std::vector<float> ss(no_of_frames * frameLength,0);
	
	// some initial values - position of the next pulse for voiced frames (C++ indexing)
    	int nextvoiced = 0;

    //from = 1; to = from + lram -1;
    //for n = 1:Nram,
    for( int frameIndex = 0; frameIndex < no_of_frames; frameIndex++){

        //a = [1; A(:,n)]; % appending with 1 for filtering
        //g = G(n);
        //l = L(n);
        std::vector<float> a;
		a.push_back(1.0);
        for( int j = 0; j <= frameIndex * 10 + 10; j++){
            a.push_back(decodedLPC[j]);
		}

        std::vector<float> gain;
        gain.push_back(decodedGains[frameIndex]);   //sorry

        int lag = Lags[frameIndex];

        std::vector<float> excit(frameLength,0.0);
		std::default_random_engine generator;
		std::normal_distribution<float> distribution(0.0,1.0);

        /*
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
        end*/
        if(lag == 0){
            for(int i = 0; i < frameLength; i++){
                excit[i]=distribution(generator);       //hmm co to power one v komentari
            }
		} else {

            //where naplnime po krocich s delkou lag
			int step = nextvoiced;
			std::vector<int> where;
            while(step <= frameLength){
				where.push_back(step);
                step += lag;
			}

            //najdeme peak
			int maximum = where[0];			
			for(int i = 1; i < where.size(); i++){
				if(where[i] > maximum){
					maximum = where[i];
				}
			}
            //a znej odvodime kde asi bude dalsi
            nextvoiced = maximum + lag - frameLength;
		      	
            //na indexech vsech vrcholu z where generujeme 1 pulsy
			for(int i = 0; i < where.size(); i++){
                excit[where[i]] = 1.0;
            }

		}


        /*
        % and set the power of excitation  to one - no necessary for noise, but anyway ...
        power = sum(excit .^ 2) / lram;
        excit = excit / sqrt(power);
        % check
      %  power = sum(excit .^ 2) / lram

        */

        float power = 0.0f;

        for(int i = 0; i < excit.size(); i++){
            power += excit[i] * excit[i];
        }

        power /= frameLength;

        float power_sqrt = sqrt(power);

        for(int i = 0; i < excit.size(); i++){
            excit[i] = excit[i] / power_sqrt;
        }


        for(int i = 0; i < excit.size(); i++){

            ss.push_back(filter.do_step(gain, a, excit.at(i)));

        }

        /*
        % now just generate the output
        [synt,final] = filter (g,a,excit,init);
        ss(from:to) = synt; % !!! this line was originally at the end.
        init = final;
        from = from + lram; to = from + lram -1;
      end
         */



	}

/*  Vzor z matlabu
 
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


    	std::vector<float> LPCCodebook, LPCDecode;
	std::vector<float> GainCodebook, GainDecode;
	std::vector<int> LPCIndex;
	std::vector<int> GainIndex;
	std::vector<int> LIndex;
	
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
	
	// decode data from input .cod file
	decode(LPCCodebook, LPCIndex, LPCDecode, LPCD);
	decode(GainCodebook, GainIndex, GainDecode, GAIND);

	
	// synthesis
	
	synthesis(LPCDecode,GainDecode,LIndex);
	/* control prints
 
	std::cout << LPCCodebook.size() << std::endl << GainCodebook.size() << std::endl;
	std::cout << LPCIndex.size() << std::endl << GainIndex.size() << std::endl << LIndex.size() << std::endl;
	std::cout << LPCDecode.size() << std::endl << GainDecode.size() << std::endl;
	*/


/*
    ---------------------------------------------------------------------------------
     DONE BEGIN
    ---------------------------------------------------------------------------------
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
    ----------------------------------------------------------------------------------
     DONE END
    ----------------------------------------------------------------------------------	

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
    % P - order of predictor            -je to 10 vice koeficientu nemame
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

/*
y = filter(b,a,x) filters the input data, x, using a rational transfer function
defined by the numerator and denominator coefficients b and a, respectively.

Y(z) = b(1..n) / 1+ a(2..n)

init a final jsou parametry stavu filtru pocatecni a koncovy, ktery se stava novym pocatecnim..
*/
      /*
      [synt,final] = filter (g,a,excit,init);
      ss(from:to) = synt; % !!! this line was originally at the end.
      init = final;
      from = from + lram; to = from + lram -1;
    end
*/


	// close files
	lpc_file.close();
	gain_file.close();
	input_file.close();
	output.close();
	return OK;
}
