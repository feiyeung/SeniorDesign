% workspace 3
% read csv exported from openvibe data
clear; clc; close all;
sig = csvread('csv_2017_02_26_sig.csv',1,0);
sti = csvread('csv_2017_02_26_sti.csv',1,0);
sig_size = size(sig)
sti_size = size(sti)

%% prepare windows and filters
window256 = hamming(256);
window512 = hamming(512);

%% extract each event
%{
32779: begins
32780: stops

33024: freq 0
33025: freq 1
33026: freq 2
33027: freq 3
%}

