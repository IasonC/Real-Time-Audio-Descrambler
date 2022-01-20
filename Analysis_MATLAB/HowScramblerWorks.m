%% Scrambling

[originaltime, Fso] = audioread("original.wav");
[scrambledtime, Fss] = audioread("scrambled.wav");

plot(t, originaltime)
title("Original Signal in Time")
plot(t, scrambledtime)
title("Scrambled Signal in Time")

%disp("play")
%sound(scrambledtime, Fss, 16)
%disp("end")

N = length(myRecording);
originalfreq = fft(originaltime);
scrambledfreq = fft(scrambledtime);

originalfreq = abs(originalfreq(1:N/2+1));
scrambledfreq = abs(scrambledfreq(1:N/2+1));
f = Fs*(0:(N/2))/N;

plot(f, originalfreq)
title("Original Signal in Freq")
xlim([0 6000])
plot(f, scrambledfreq)
title("Scrambled Signal in Freq")
ylim([0 700])
  
%% WHAT DOES THE SCRAMBLER DO?
% For an original signal in 0Hz - xHz
% The scrambler takes negative sideband of the 0-mean freq spectrum and shifts it up to 1000Hz - 1000+x Hz. The scrambler adds a tone at 8kHz which is white noise in time (freq impulse = sinusoid in time which oscillates at 8kHz so looks like const low-amplitude noise)

%% Descrambling
% 8kHz tone is at 8k from 0kHz to 22.05kHz, step size 0.2Hz
% so 8kHz tone is the 40001st element

f1 = setdiff(f, 8000);
recoveredfreq = setdiff(scrambledfreq, scrambledfreq(40001), 'stable');
plot(f1, recoveredfreq)
f1 = -f1 + 7000;
plot(f1, recoveredfreq) %114.863/412.668*
plot(f, originalfreq)
xlim([0 7000])
ylim([0 200])

recoveredtime = ifft(ifftshift(recoveredfreq));
T = t(N/2 + 1:N);
plot(T, recoveredtime)
ylim([-0.2 0.2])
