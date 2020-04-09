%% ======================================
clear all ; 
Rhog = [0, 1.2732395447351628, 1.9098593171027443,3.242277876554809, 6.079271018540266, 12.384589222348605] ; 
g = 10 ; 
diam = 2 ; 

Test{1}.id='MuI_D2rev' ; 
Test{1}.d=2 ; 
Test{1}.firstangle = 35 ;
Test{1}.deltaangle = -1 ;
Test{1}.deltatime = 500 ;
Test{1}.avgtime = 300 ; 
Test{1}.rhog = Rhog(Test{1}.d) ; 
Test{1}.dz = 20/50 ; 

Test{2}.id='MuI_D3rev' ; 
Test{2}.d=3 ; 
Test{2}.firstangle = 35 ;
Test{2}.deltaangle = -1 ;
Test{2}.deltatime = 500 ;
Test{2}.avgtime = 300 ; 
Test{2}.rhog = Rhog(Test{2}.d) ; 
Test{2}.dz = 20/50 ; 

Test{3}.id='MuI_D4rev' ; 
Test{3}.d=4 ; 
Test{3}.firstangle = 35 ;
Test{3}.deltaangle = -1 ;
Test{3}.deltatime = 500 ;
Test{3}.avgtime = 300 ; 
Test{3}.rhog = Rhog(Test{3}.d) ; 
Test{3}.dz = 20/50 ; 

Test{4}.id='MuI_D5rev' ; 
Test{4}.d=5 ; 
Test{4}.firstangle = 35 ;
Test{4}.deltaangle = -1 ;
Test{4}.deltatime = 200 ;
Test{4}.avgtime = 100 ; 
Test{4}.rhog = Rhog(Test{3}.d) ; 
Test{4}.dz = 20/50 ; 

Test{5}.id='MuI_D6rev' ; 
Test{5}.d=6 ; 
Test{5}.firstangle = 35 ;
Test{5}.deltaangle = -1 ;
Test{5}.deltatime = 200 ;
Test{5}.avgtime = 50 ; 
Test{5}.rhog = Rhog(Test{5}.d) ; 
Test{5}.dz = 20/50 ; 

Test{6}.id='MuI_D2revb' ; 
Test{6}.d=2 ; 
Test{6}.firstangle = 35 ;
Test{6}.deltaangle = -1 ;
Test{6}.deltatime = 500 ;
Test{6}.avgtime = 300 ; 
Test{6}.rhog = Rhog(Test{6}.d) ; 
Test{6}.dz = 20/50 ; 

Test{7}.id='MuI_D5revb' ; 
Test{7}.d=5 ; 
Test{7}.firstangle = 40 ;
Test{7}.deltaangle = -1 ;
Test{7}.deltatime = 500 ;
Test{7}.avgtime = 300 ; 
Test{7}.rhog = Rhog(Test{7}.d) ; 
Test{7}.dz = 20/50 ; 

Test{10}.id='MuI_D4rev_16' ; 
Test{10}.d=4 ; 
Test{10}.firstangle = 35 ;
Test{10}.deltaangle = -1 ;
Test{10}.deltatime = 500 ;
Test{10}.avgtime = 300 ; 
Test{10}.rhog = Rhog(Test{10}.d) ; 
Test{10}.dz = 20/50 ; 

Test{11}.id='MuI_D4rev_Local' ; 
Test{11}.d=4 ; 
Test{11}.firstangle = 35 ;
Test{11}.deltaangle = -1 ;
Test{11}.deltatime = 500 ;
Test{11}.avgtime = 300 ; 
Test{11}.rhog = Rhog(Test{11}.d) ; 
Test{11}.dz = 20/50 ; 

Test{12}.id='MuI_D4rev_8' ; 
Test{12}.d=4 ; 
Test{12}.firstangle = 35 ;
Test{12}.deltaangle = -1 ;
Test{12}.deltatime = 500 ;
Test{12}.avgtime = 300 ; 
Test{12}.rhog = Rhog(Test{12}.d) ; 
Test{12}.dz = 20/50 ; 

Test{13}.id='MuI_D4rev_12' ; 
Test{13}.d=4 ; 
Test{13}.firstangle = 35 ;
Test{13}.deltaangle = -1 ;
Test{13}.deltatime = 500 ;
Test{13}.avgtime = 300 ; 
Test{13}.rhog = Rhog(Test{13}.d) ; 
Test{13}.dz = 20/50 ; 

%% Spato-temporal data plotting 
figure (1) ; clf 
Tmp = Test{10} ; 
load(['../CppCode/Dem/Output_DEBUG/Output_' Tmp.id '/CoarseGrained.mat']) ; 
%load(['../CppCode/Dem/Output_' Tmp.id '/CoarseGrained.mat']) ; 
imagesc(squeeze(VAVG(2,:,:))/Tmp.rhog) ; hold all
for i=0:Tmp.deltatime:size(VAVG,3)
    plot ([i, i],[1, 25], 'r') ; 
    text(i+10, 10, num2str(Tmp.firstangle + Tmp.deltaangle*(i/Tmp.deltatime))) ; 
end ; 
%caxis([0 10])

%% Multiproc issue ... artemis
figure() ; limits=[0,100] ; 
subplot(2,8,1)
load(['../CppCode/Dem/Output_' Test{3}.id '/CoarseGrained.mat']) ; 
imagesc(squeeze(VAVG(2,:,1:300))) ; caxis(limits) ;
title('Local 1p') ; 

subplot(2,8,2)
load(['../CppCode/Dem/Output_DEBUG/Output_' 'MuI_D4rev_Local' '/CoarseGrained.mat']) ; 
imagesc(squeeze(VAVG(2,:,1:300))) ; caxis(limits) ; 
title ('Local 16p') ;

subplot(2,8,3)
load(['../CppCode/Dem/Output_DEBUG/Output_' 'MuI_D4rev_1' '/CoarseGrained.mat']) ; 
imagesc(squeeze(VAVG(2,:,1:300))) ; caxis(limits) ;
title ('Hpc 1p') ;

subplot(2,8,4)
load(['../CppCode/Dem/Output_DEBUG/Output_' 'MuI_D4rev_8' '/CoarseGrained.mat']) ; 
imagesc(squeeze(VAVG(2,:,1:300))) ; caxis(limits) ;
title ('Hpc 8p') ;

subplot(2,8,5)
load(['../CppCode/Dem/Output_DEBUG/Output_' 'MuI_D4rev_12' '/CoarseGrained.mat']) ; 
imagesc(squeeze(VAVG(2,:,1:300))) ; caxis(limits) ;
title ('Hpc 12p') ;

subplot(2,8,6)
load(['../CppCode/Dem/Output_DEBUG/Output_' 'MuI_D4rev_15' '/CoarseGrained.mat']) ; 
imagesc(squeeze(VAVG(2,:,1:300))) ; caxis(limits) ;
title ('Hpc 15p') ;

subplot(2,8,7)
load(['../CppCode/Dem/Output_DEBUG/Output_' 'MuI_D4rev_16' '/CoarseGrained.mat']) ; 
imagesc(squeeze(VAVG(2,:,1:300))) ; caxis(limits) ;
title ('Hpc 16p') ;

subplot(2,8,8)
load(['../CppCode/Dem/Output_DEBUG/Output_' 'MuI_D4rev_1_16' '/CoarseGrained.mat']) ; 
imagesc(squeeze(VAVG(2,:,:))) ; caxis(limits) ;
title ('Hpc 1p16t') ;

subplot(2,8,9)
load(['../CppCode/Dem/Output_DEBUG/Output_' 'MuI_D4rev_16_J' '/CoarseGrained.mat']) ; 
imagesc(squeeze(VAVG(2,:,:))) ; caxis(limits) ;
title ('Hpc 16p J') ;

subplot(2,8,10)
load(['../CppCode/Dem/Output_DEBUG/Output_' 'MuI_D4rev_16_K' '/CoarseGrained.mat']) ; 
imagesc(squeeze(VAVG(2,:,:))) ; caxis(limits) ;
title ('Hpc 16p K') ;

subplot(2,8,11)
load(['../CppCode/Dem/Output_DEBUG/Output_' 'MuI_D4rev_16_L' '/CoarseGrained.mat']) ; 
imagesc(squeeze(VAVG(2,:,:))) ; caxis(limits) ;
title ('Hpc 16p L') ;

subplot(2,8,12)
load(['../CppCode/Dem/Output_DEBUG/Output_' 'MuI_D4rev_16_E' '/CoarseGrained.mat']) ; 
imagesc(squeeze(VAVG(2,:,:))) ; caxis(limits) ;
title ('Hpc 16p E') ;

subplot(2,8,13)
load(['../CppCode/Dem/Output_DEBUG/Output_' 'MuI_D4rev_16_F' '/CoarseGrained.mat']) ; 
imagesc(squeeze(VAVG(2,:,:))) ; caxis(limits) ;
title ('Hpc 16p F') ;

subplot(2,8,14)
load(['../CppCode/Dem/Output_DEBUG/Output_' 'MuI_D4rev_16_G' '/CoarseGrained.mat']) ; 
imagesc(squeeze(VAVG(2,:,:))) ; caxis(limits) ;
title ('Hpc 16p G') ;

subplot(2,8,15)
load(['../CppCode/Dem/Output_DEBUG/Output_' 'MuI_D4rev_16_H' '/CoarseGrained.mat']) ; 
imagesc(squeeze(VAVG(2,:,:))) ; caxis(limits) ;
title ('Hpc 16p H') ;

subplot(2,8,16)
load(['../CppCode/Dem/Output_DEBUG/Output_' 'MuI_D4rev_16_I' '/CoarseGrained.mat']) ; 
imagesc(squeeze(VAVG(2,:,:))) ; caxis(limits) ;
title ('Hpc 16p I') ;


%% ======================================
for i=1:size(Test,2)
    clear VAVG RHO ; 
    load(['../CppCode/Dem/Output_' Test{i}.id '/CoarseGrained.mat']) ; 
    n=1 ; 
    for j=[1:Test{i}.deltatime:size(VAVG,3)-Test{i}.deltatime-1] 
        deb = j+Test{i}.deltatime-Test{i}.avgtime-1 ; fin = j+Test{i}.deltatime ; 
        Angle{i}(n) = Test{i}.firstangle + (n-1) * Test{i}.deltaangle ;
        Vall{i} (n,:)    = mean(VAVG(2,:,deb:fin),3) ;
        Rhoall{i} (n,:)  = mean(RHO (:,  deb:fin),2) ;
        Phiall{i} (n,:)  = Rhoall{i}(n,:)/Test{i}.rhog ; 
        Pressure{i}(n,:) = cumsum(Rhoall{i}(n,:),2, 'reverse') * g * cosd(Angle{i}(n)) ;
        Gammadot{i}(n,:) = (Vall{i}(n,3:end) - Vall{i}(n,1:end-2))/(2*Test{i}.dz) ; 
        Mu{i}(n,:) = tand(Angle{i}(n)) * ones(1,size(RHO,1)-2) ; 
        I{i}(n,:) = Gammadot{i}(n,:) * diam ./ sqrt(Pressure{i}(n,2:end-1)/Test{i}.rhog) ; 
        n=n+1 ; 
    end ; 
    I{i}([I{i}<0 | isnan(I{i}) | isinf(I{i})]) = nan  ; 
end ;




figure(4) ; clf ; 
cm = pycolors('inferno') ; 
axes() ; 
set(gca, 'ColorOrder', cm(1:256/20:256,:));  hold all
for i=1:13
    %plot (I{1}(i,2:end), Phiall{1}(i,3:end-1), '*r') ; hold all
    plot (Phiall{1}(i,:), '-') ; hold all
end 

%%
for i=2:13
    plot (I{2}(i,1:end), Mu{2}(i,1:end), 'ob-') ; hold all
end 
for i=2:10
    plot (I{3}(i,1:end), Mu{3}(i,1:end), '*-k') ; hold all
end 



%% 
dmp1588=textread('../CppCode/Dem/Output_MuI_D6rev/dmp1588_pos');
dmp1588=dmp1588(:,1:25) ;
dmp1588=reshape(dmp1588',1,[]) ;
dmp1588(1:10)
dmp1588=dmp1588(1:8570*6) ;
dmp1588_ok=reshape(dmp1588,8570,6)  ;
dmp1588_ok(1,:)

















