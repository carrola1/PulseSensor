
N = length(data);
fs = 50;
Navg = 4;
Nsamp = 100;
Nup = 50;
thresh = 0.3;
holdOffThresh = 3;

NsampAvg = Nsamp-Navg+1;
Nsamps = floor((N-Nsamp)/Nup)+1;
samp = data(1:Nsamp);
sampAvg = zeros(1,NsampAvg);
hrAvg = zeros(1,Nsamps);
for ii = 1:Nsamps
    riseFound = 0;
    maxPk = 0;
    pks   = 0;
    tPk   = 0;
    holdOff = 0;
    
    if ii > 1
        samp(1:Nsamp-Nup) = samp(Nup+1:end);
        samp(Nsamp-Nup+1:end) = data(Nsamp+1+Nup*(ii-2):Nsamp+Nup*(ii-1));
    end
    
    for jj = 1:NsampAvg
        sampAvg(jj) = sum(samp(jj:jj+Navg-1)/Navg);
    end
    uSamp   = mean(sampAvg);
    sampAvg = sampAvg - uSamp;  % remove bias
    minSamp = min(sampAvg);
    maxSamp = max(sampAvg);
    pkThresh = maxSamp - thresh*maxSamp;
    for kk = 2:NsampAvg
        if (sampAvg(kk) > pkThresh) && (sampAvg(kk-1) <= pkThresh)
            riseFound = 1;
        end
        if riseFound == 1
            if (sampAvg(kk) < pkThresh) && (holdOff < holdOffThresh)
                riseFound = 0;
                maxPk = 0;
                tPk = 0;
                holdOff = 0;
            else
                if sampAvg(kk) > maxPk
                    maxPk = sampAvg(kk);
                    tPk   = kk;
                end
                if holdOff == holdOffThresh 
                    if (sampAvg(kk) < pkThresh) && (sampAvg(kk-1) >= pkThresh)
                        pks = [pks tPk];
                        holdOff = 0;
                        maxPk = 0;
                        riseFound = 0;
                    end
                else
                    holdOff = holdOff + 1;
                end
                
            end
        end
    end
    pks = pks(2:end);
    hr = zeros(1,length(pks)-1);
    for mm = 1:length(hr)
        hr(mm) = 1/((pks(mm+1)-pks(mm))*1/fs)*60;
    end
    hrAvg(ii) = mean(hr);
    
end
