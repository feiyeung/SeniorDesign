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
alpha = 0.25;
nfft = 512;
last_frame = {0,0,0,0,0,0,0,0};
freq = -250/2 : 250/nfft : (250/2 - 250/nfft);

target_freq = [16.13, 21.74, 25];
k_freq = {};
ind_freq = [];
for f = target_freq
    diff = abs(f - freq);
    diff_sorted = sort(diff);
    t1 = find(diff == diff_sorted(1)); % closest freq
    t2 = find(diff == diff_sorted(2)); % 2nd closest freq
    
    if diff_sorted(1) < (0.2 * 250/nfft)
        out = [t1(1)];
        ind_freq(end+1) = t1(1);
    elseif (length(t1)==1)
        out = [t1(1), t2(1)];
        ind_freq(end+1) = t1(1);
        ind_freq(end+1) = t2(1);
    else
        out = [t1(1), t1(2)];
        ind_freq(end+1) = t1(1);
        ind_freq(end+1) = t1(2);
    end
    k_freq{end+1} = out;
end
ind_freq = sort(ind_freq);

%%
% find raw stft

stft_val = {[],[],[],[],[],[],[],[]};
stft_time = [];
stft_full = {[],[],[],[],[],[],[],[]};
nfft = 512;
for i = [1 : 25 : length(eq_ind)-256]
    stft_time = [stft_time; t_tar + eq_ind(i) / 250 ];
    % calc for each channel
    this_frame = {};
    for c = [3:8]
        chan = ep_ch{c};
        chan_subset = chan( i : i+255 );
        d_0_offset = chan_subset - mean(chan_subset);
        d_1_windowed = d_0_offset .* window;
        % d_2_notched = filter(d, d_1_windowed);
        % d_3_bped = filter(bpFilt,d_2_notched);
        % d_4_offset = d_1_windowed - mean(d_1_windowed);
        this_frame{c} = abs(fftshift(fft(d_1_windowed, nfft)));
        % stft_val{c} = [stft_val{c}; this_frame{c}(ind_freq)'];
        stft_full{c} = [stft_full{c}; this_frame{c}( (end/2)+1 : end  )'];
    end
end

%% 2d plot
% figure; imagesc(stft_full{3}); colormap gray;
% freq_axis = [-256:255] ./ 255 .* (250/2); xlabel(freq_axis);



tmp = stft_full{5}';

freq_axis = linspace(0,125,256);
time_axis = [1 : size(stft_full{3}, 1)] ./ (250 / 25); 
figure; imagesc(time_axis,freq_axis,log10(tmp)); colormap gray;
caxis([2 2.8]);



tmp_avg = log10(tmp);
for i = 1:size(tmp,1)
    tmp_avg(i,:) = movmean(log10(tmp(i,:)), 30);
end

freq_axis = linspace(0,125,256);
time_axis = [1 : size(stft_full{3}, 1)] ./ (250 / 25); 
figure; imagesc(time_axis,freq_axis,tmp_avg); colormap gray;
caxis([2 2.8]);


%% plot

close all


legend_text = {};
for v = freq(ind_freq)
    legend_text{end+1} = num2str(v);
end


stft_val = stft_full;

ch = 4;
alpha = 0.2;
alpha_ref = alpha * 0.1;

data_tmp = stft_val{ch};
data_f1 = data_tmp(:,1);
data_f2 = mean([data_tmp(:,2), data_tmp(:,3)]')';
data_f3 = mean([data_tmp(:,4), data_tmp(:,5)]')';
data_ch = [data_f1, data_f2, data_f3];
legend_text = {'1', '2', '3'};

data_ch_log10 = log10(data_ch);
data_fil = filter(alpha, [1 alpha-1], data_ch_log10 );
% data_ref = filter(alpha_ref, [1 alpha_ref-1], data_ch_log10 );
data_ref = mean(data_fil')';
stft_plot_data = data_fil - data_ref;

figure;
plot(stft_time, stft_plot_data);
title('diff from ref');
grid on;
xlabel('time (sec)');
legend(legend_text)

figure;
plot(stft_time, data_fil);
title('sig');
grid on;
xlabel('time (sec)');
legend(legend_text)