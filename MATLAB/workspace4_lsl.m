clear; close all;

global pool;

lib = lsl_loadlib();

result = {};
while isempty(result)
    result = lsl_resolve_byprop(lib,'name','openvibeSignal'); 
    pause(0.05);
end
inlet = lsl_inlet(result{1});

buffer_size = 512;
buffer = zeros(11,buffer_size);
new_data_points = 0;
global pool nfft;
nfft = 512;
tmp = zeros(1,nfft/2);
pool = {tmp, tmp, tmp, tmp, tmp, tmp, tmp, tmp};

while true
    % get chunk from the inlet
    [chunk,stamps] = inlet.pull_chunk();
    c_size = size(chunk);
    if (c_size(2) > buffer_size)
        chunk = chunk(:, 1:buffer_size);
        c_size = size(chunk);
    end
    % chunk( abs(chunk) > 1e3 ) = 1e3;
    buffer(:, (c_size(2)+1) : buffer_size) = buffer(:, 1 : (buffer_size - c_size(2)));
    buffer(:, 1:c_size(2) ) = chunk;
    new_data_points = new_data_points + c_size(2);
    
    if new_data_points > 50
        process(buffer);
        new_data_points = 0;
    end
        
    pause(0.05);
    
end


function [] = process( chunk )
global pool nfft;
c_size = size(chunk);
window = hamming(c_size(2));
eeg = chunk(1:8, :);
% nfft = 512;
freq = 0:(nfft/2-1);
freq_ind = (4<freq & freq<30);
alpha = 0.3;
hold off;
for ch=[1:8]
    ch_data = chunk(ch, :);
    % ch_data( abs(ch_data) > 1e7 ) = 1e7;
    ch_shifted = ch_data - mean(ch_data);
    ch_data_windowed = ch_shifted(:) .* window(:);
    ch_data_spe = abs(fft(ch_data_windowed, nfft));
    ch_data_spe(abs(ch_data_spe)<0.01) = 0.01;
    ch_data_spe_half = log10(ch_data_spe( 1 : nfft/2));
    pool{ch} = pool{ch} .* (1-alpha) + ch_data_spe_half .* alpha;
    plot(freq(freq_ind), pool{ch}(freq_ind), 'Marker','.','LineWidth',3);
    hold on; grid on; ylim([1 5]);
end
xlabel('freq (Hz)'); ylabel('log10 abs(FT)');
plot([12 12],[0 5], '--k');
plot([16 16],[0 5], '--k');
plot([19 19],[0 5], '--k');
plot([22 22],[0 5], '--k');
ylim([1 5]);


%{
vis = zeros(nfft/2, c_size(2));
x = 1:c_size(2);
y = 0:255;
for ch = [3:8]
    ch_data = chunk(ch, :);
    ch_data_windowed = ch_data(:) .* window(:);
    ch_data_spe = abs(fft(ch_data_windowed, nfft));
    ch_data_spe_half = log10(ch_data_spe( 1 : nfft/2));
    vis(:,2:end) = vis(:, 1:end-1);
    vis(:,1) = ch_data_spe_half'; 
    imagesc(x,y,vis); colormap gray; colorbar; 
    caxis([0 3]); 
end
%}
end