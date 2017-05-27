% instantiate the library
disp('Loading the library...');
lib = lsl_loadlib();

global log_file;
fclose('all');
log_file = fopen('log.csv', 'a');

% resolve a stream...
disp('Resolving an EEG stream...');
result = {};
while isempty(result)
    result = lsl_resolve_byprop(lib,'name','openvibeSignal'); pause(0.05); end

% create a new inlet
disp('Opening an inlet...');
inlet = lsl_inlet(result{1});

buffer_size = 1000;
buffer = zeros(11,buffer_size);
new_data_points = 0;

disp('Now receiving chunked data...');
while true
    % get chunk from the inlet
    [chunk,stamps] = inlet.pull_chunk();
    
    for s=1:length(stamps)
        % and display it
        % fprintf('%.2f\t',chunk(:,s));
        % fprintf('%.5f\n',stamps(s));
    end
    
    c_size = size(chunk);
    fprintf('%d %d\n', c_size);
    buffer(:, (c_size(2)+1) : buffer_size) = buffer(:, 1 : (buffer_size - c_size(2)));
    buffer(:, 1:c_size(2) ) = chunk;
    new_data_points = new_data_points + c_size(2);

    if new_data_points > 100
        process(buffer);
        log10(abs(buffer(:,end)) + 1)'
        log_data(buffer(:, 1:new_data_points)');
        new_data_points = 0;
    end
        
    pause(0.1);
end

function [] = log_data (  data )
global log_file;
for i = size(data, 1)
    allOneString = sprintf('%.10f,' , data(i,:));
    fprintf(log_file, [allOneString, '\n']);
end

% csvwrite('log.csv', data)
end

function [] = process( chunk )
c_size = size(chunk);
window = hamming(c_size(2));
eeg = chunk(1:8, :);
nfft = 512;
vis = zeros(nfft/2, 100);
x = 1:100;
y = 0:255;
for ch = [3:3]
    ch_data = chunk(ch, :);
    ch_data_windowed = ch_data(:) .* window(:);
    ch_data_spe = abs(fft(ch_data_windowed, nfft));
    ch_data_spe_half = log10(ch_data_spe( 1 : nfft/2));
    vis(:,2:end) = vis(:, 1:end-1);
    vis(:,1) = ch_data_spe_half'; 
    imagesc(x,y,vis); colormap gray; colorbar; 
    caxis([0 3]); 
end

end
