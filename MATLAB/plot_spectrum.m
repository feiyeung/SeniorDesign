function [ f, x_mag ] = plot_spectrum( x )
% https://www.mathworks.com/matlabcentral/answers/36430-plotting-the-frequency-spectrum#answer_45571

%% Time specifications:
Fs = 250;                      % samples per second
dt = 1/Fs;                     % seconds per sample

N = length(x);

%% Fourier Transform:
X = fftshift(fft(x));
%% Frequency specifications:
dF = Fs/N;                      % hertz
f = -Fs/2:dF:Fs/2-dF;           % hertz
%% Plot the spectrum:
% figure;
x_mag = abs(X)/N;
semilogy(f,x_mag,'Marker','.');
% stem(f,abs(X)/N);
xlim([1 40])
ylim([1 100])
grid on
% xlabel('Frequency (in hertz)');
% title('Magnitude Response');

end

