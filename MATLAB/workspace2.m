% worksapce 2
% moving


data = csvread('OBCI_03_wired_csv.txt');

%% filter design (band pass and notch)

tic;

% https://www.mathworks.com/help/dsp/ref/iirnotch.html
% 2nd order notch
wo = 60/(250/2);  bw = wo/35;
[b,a] = iirnotch(wo,bw);
% fvtool(b,a,'Fs',250);

% https://www.mathworks.com/help/signal/ug/remove-the-60-hz-hum-from-a-signal.html?searchHighlight=notch&s_tid=doc_srchtitle
% butterworth notch filter at 60Hz
d = designfilt('bandstopiir','FilterOrder',20, ...
               'HalfPowerFrequency1',58,'HalfPowerFrequency2',62, ...
               'DesignMethod','butter','SampleRate',250);
% fvtool(d,'Fs',250);


bpFilt = designfilt('bandpassiir','FilterOrder',80, ...
         'HalfPowerFrequency1',3,'HalfPowerFrequency2',50, ...
         'SampleRate',250);
% fvtool(bpFilt)

window = hamming(256);
% fvtool(window)

toc

%% get epoch

% target time in seconds
% t_tar = 468;
t_tar = 300;

% target ind
ind_tar = t_tar * 250;

epoc_ind_range = [-50 50] .* 250;
bounds = ind_tar + epoc_ind_range;

ep_data = data(bounds(1) : bounds(2), :);
eq_ind = epoc_ind_range(1) : epoc_ind_range(2);

%% get each channel from the epoch
ep_ch = {};
for i = [3:8]
    ep_ch{i} = ep_data(:, i+1);
end


%% 
close all;
% slide the window
figure;
alpha = 0.25;
nfft = 512;
last_frame = {0,0,0,0,0,0,0,0};
freq = -250/2 : 250/nfft : (250/2 - 250/nfft);

for i = [1 : 63 : length(eq_ind)-256]
    % calc for each channel
    tic;
    start = now;
    frame = {};
    % hold on;
    for c = [3:8]
        chan = ep_ch{c};
        chan_subset = chan( i : i+255 );
        d_0_offset = chan_subset - mean(chan_subset);
        d_1_windowed = d_0_offset .* window;
        d_2_notched = filter(d, d_1_windowed);
        d_3_bped = filter(bpFilt,d_2_notched);
        % d_4_offset = d_1_windowed - mean(d_1_windowed);
        
        % plot_spectrum(d_3_bped);
        frame{c} = abs(fftshift(fft(d_3_bped, nfft)));
        this_frame{c} = log10(frame{c}) .* alpha + last_frame{c} .* (1-alpha);
        plot(freq, this_frame{c}, 'Marker', '.');
        grid on;
        
        if c==3
            hold on;
        end
    end
    for f = [16.13, 21.74, 25]
        plot([f f]', [0 4]');
    end
    hold off;
    legend('3','4','5','6','7','8');
    xlim([0,40]);
    ylim([0, 4]);
    last_frame = this_frame;
    time_from_beginning = t_tar + eq_ind(i) / 250; % sec
    title(sprintf('%f sec', time_from_beginning) );
    pause(0.25 - (now - start)*3600*24);
    iter_time = toc;
    fprintf('%f secs into data begins. this frame time %f secs\n', time_from_beginning, iter_time);
end


return
%% 

cwt(ep_ch{8},250,'amor');

%% cwt

this_data = ep_ch{6};
this_data = filter(bpFilt,   filter(d, hamming(length(this_data)) .* this_data));
[wt, f] = cwt(this_data,250,'amor');
cwt(this_data,250,'amor'); caxis([0 10]);
wt_sel = db( abs( wt( find( 10 < f & f < 30 ), :) ));
f_sel = f( 10 < f & f < 30 );

figure;
hold on;
freq_cmp = [16.1, 21.7, 25];
alpha = 0.2 ;
for this_f = freq_cmp
    f_closest_ind = find( min(abs(f_sel - this_f)) == abs(f_sel - this_f) )
    f_closest_ind = f_closest_ind(1);
    f_closest = f_sel(f_closest_ind)
    sig = abs(wt( f_closest_ind, : ));
    sig = filter(alpha, [1 alpha-1], sig);
    sig2 = log10(sig);
    sig3 = sig2 - mean(sig2);
    sig4 = sig3 ./ filter(0.5, [1 1-0.5], sig3);
    plot( [1:size(wt, 2)]./250, sig4 );
    xlim([45,55]);
    
    
    
end
hold off;
legend('16','21','25')










