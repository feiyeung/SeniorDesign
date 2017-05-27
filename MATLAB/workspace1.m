% workspace 1
clear; clc; close all;
data = csvread('OBCI_03_wired_csv.txt');

%% filter design (band pass and notch)

tic;

% https://www.mathworks.com/help/dsp/ref/iirnotch.html
% 2nd order notch
wo = 60/(250/2);  bw = wo/35;
[b,a] = iirnotch(wo,bw);
% fvtool(b,a,'Fs',250);

% https://www.mathworks.com/help/signal/ug/remove-the-60-hz-hum-from-a-signal.html?searchHighlight=notch&s_tid=doc_srchtitle
% butterworth notch filter
d = designfilt('bandstopiir','FilterOrder',20, ...
               'HalfPowerFrequency1',58,'HalfPowerFrequency2',62, ...
               'DesignMethod','butter','SampleRate',250);
% fvtool(d,'Fs',250);

bpFilt = designfilt('bandpassiir','FilterOrder',20, ...
         'HalfPowerFrequency1',2,'HalfPowerFrequency2',40, ...
         'SampleRate',250);
% fvtool(bpFilt)

window = hamming(256);
% fvtool(window)

toc;

%% get epoch

% target time in seconds
t_tar = 468;

% target ind
ind_tar = 468 * 250;

epoc_ind_range = [-5 5] .* 250;
bounds = ind_tar + epoc_ind_range;

ep_data = data(bounds(1) : bounds(2), :);
eq_ind = epoc_ind_range(1) : epoc_ind_range(2);

%% get each channel from the epoch
ep_ch = {};
for i = [3:8]
    ep_ch{i} = ep_data(:, i+1);
end

%% get a subset for fft
chan4 = ep_ch{4};
chan4_subset = chan4( find( 0 <= eq_ind & eq_ind < 256 ) );

%%
% close all; 
tic; figure;
title('frequency analysis');
subplot(4,1,1)
plot_spectrum(chan4_subset);
title('raw signal')
d_1_windowed = chan4_subset .* window;

subplot(4,1,2)
plot_spectrum(d_1_windowed);
title('windowed')

d_2_notched = filter(d, d_1_windowed);

subplot(4,1,3)
plot_spectrum(d_2_notched);
title('notched')

d_3_bped = filter(bpFilt,d_2_notched);

subplot(4,1,4)
plot_spectrum(d_3_bped);
title('bandpassed')



toc