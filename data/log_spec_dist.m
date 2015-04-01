function D=log_spec_dist(s1,s2);
% Syntax: D=log_spec_dist(s1,s2);
%
% computes average log. spectral distance between 2 signals using FFT. 
% uses frames of 160 samples with 0 overlap. The FFT is done on 256 points. 
% In case the lengths are not equal, shorter length is used. The function warns if the 
% difference in lengths is more than 5%. 
% The function does centering of input signals. 

sm = s1 - mean(s1); 
S1 = frame(sm, 160,0); 
sm = s2 - mean(s2); 
S2 = frame(sm, 160,0); 
N1 = size(S1,2); N2 = size(S2,2); N = min(N1,N2); 
if (abs(N2 - N1) / N >= 0.05) 
  disp ('warning, difference in lengths is bigger than 5%'); 
end

 
D = 0; Nactual = 0 ; 
for n = 1:N 
  % log power spec densities   
  x = S1(:,n);  
  X = fft([x' zeros(1,256-160)]); Gdft= 1/160 *abs(X) .^ 2; 
  Gdftlog1 = 10 * log10 (Gdft); 
  x = S2(:,n);  
  X = fft([x' zeros(1,256-160)]); Gdft= 1/160 *abs(X) .^ 2; 
  Gdftlog2 = 10 * log10 (Gdft); 
%  plot (1:256, Gdftlog1, 1:256, Gdftlog2); pause 
  % log spec. distance - if we do not have inifinities ! 
  if (isempty(find(abs(Gdftlog1) == Inf)) && isempty(find(abs(Gdftlog2) == Inf)))
    d2=sum((Gdftlog1 - Gdftlog2).^2)/256;
    d=sqrt(d2); 
    D = D + d;
    Nactual = Nactual +1;
  end   
end
D = D / Nactual; 
display (['D=',num2str(D),' dB, N=',num2str(N),', Nactual=',num2str(Nactual)]); 
