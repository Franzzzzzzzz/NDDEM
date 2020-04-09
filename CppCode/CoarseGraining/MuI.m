clear all
figure(1) ; clf ; 
Marker = [''; ''; '*r'; '+b'; 'xk'; '^m'] ;  

Angles = [20 22 24 26 28 30 32 34 36 38 40] ; 
Rhog = [0, 1.2732395447351628, 1.9098593171027443,3.242277876554809, 6.079271018540266, 12.384589222348605] ;  
g = 10 ; 
diam = 2 ; 
dz=20/25 ; 

top= [0,0,0,0,0,0,0,0,0,0; ...
    11,11,11,12,12,14,15,18,19,20;...
    10,9,9,10,10,11,12,14,17,19;...
    10,9,9,9,9,10,10,11,13,15;...
    9,9,9,9,10,10,11,12,13,15;...
    9,9,9,9,9,10,11,11,11,11];

top=reshape(top, [6, 10]) ; 

for d=[2:6]
    clear VAVG RHO ; 
    load(['CoarseMUID',num2str(d),'.mat']) ; 
    for i=[1:100:size(VAVG,3)-110] 
        Vall(d,(i-1)/100+1,:) = mean(VAVG(2,:,i+50:i+99),3) ;
        Rhoall (d,(i-1)/100+1,:) = mean(RHO(:,i+50:i+99),2) ;
        Phiall(d, (i-1)/100+1, :) = Rhoall(d, (i-1)/100+1, :)/Rhog(d) ; 
        Pressure(d,(i-1)/100+1,:) = cumsum(Rhoall (d,(i-1)/100+1,:),3, 'reverse') * g * cosd(Angles((i-1)/100+1)) ;
        Gammadot(d,(i-1)/100+1,:) = (Vall(d,(i-1)/100+1,3:end) - Vall(d,(i-1)/100+1,1:end-2))/(2*dz) ; 
        Mu(d,(i-1)/100+1,:) = tand(Angles((i-1)/100+1)) * ones(1,23) ; 
    end ; 
    I(d,:,:)=Gammadot(d,:,:) * diam ./ sqrt(Pressure(d,:,2:end-1)/Rhog(d)) ; 
    I ([I<0 | isnan(I) | isinf(I)]) = nan  ; 

end ; 

figure(1) ; 
plot (squeeze(I(2,:,7))', squeeze(Rhoall(2,:,7)/Rhog(2))' , '-vm') ; hold all 
plot (squeeze(I(3,:,7))', squeeze(Rhoall(3,:,7)/Rhog(3))' , '-*r') ; hold all 
plot (squeeze(I(4,:,7))', squeeze(Rhoall(4,:,7)/Rhog(4))' , '-+g') ; hold all 
plot (squeeze(I(5,:,7))', squeeze(Rhoall(5,:,7)/Rhog(5))' , '-xk') ; hold all 
plot (squeeze(I(6,:,7))', squeeze(Rhoall(6,:,7)/Rhog(6))' , '-^b') ; hold all 

%semilogx (permute(I(3,:,:), [2,3,1]), permute(Mu(3,:,:), [2,3,1]) , '*r') ; hold all 
%semilogx (permute(I(4,:,:), [2,3,1]), permute(Mu(3,:,:), [2,3,1]) , '^b') ; hold all 
%semilogx (permute(I(5,:,:), [2,3,1]), permute(Mu(3,:,:), [2,3,1]) , '+g') ; hold all 
%semilogx (permute(I(6,:,:), [2,3,1]), permute(Mu(3,:,:), [2,3,1]) , 'xk') ; hold all 

xlim([0 5])
%% 
figure (88) ; clf 
load ../Dem/Output_MuI_D3rev/CoarseGrained.mat
imagesc(squeeze(VAVG(2,:,:))) ; hold all
for i=20:200:size(VAVG,3)
    plot ([i, i],[1, 25], 'r') ; 
end ; 

%% Plot velocity fields
figure (12) ; clf ; 
cm = pycolors('inferno') ; 
axes() ; 
set(gca, 'ColorOrder', cm(1:256/5:256,:));  hold all
for i=1:2:10
    plot (squeeze(Vall(2,i,1:top(2,i)))', '-') 
end ;
for i=1:2:10
    plot (squeeze(Vall(3,i,1:top(3,i)))', ':') 
end ;
for i=1:2:10
    plot (squeeze(Vall(4,i,1:top(4,i)))', '^') 
end ;
for i=1:2:10
    plot (squeeze(Vall(5,i,1:top(5,i)))', 'p') 
end ;
for i=1:2:10
    plot (squeeze(Vall(6,i,1:top(6,i)))', 'v') 
end ;
%plot (squeeze(Vall(3,:,:))', '--')
%plot (squeeze(Vall(4,:,:))', '.-')
%plot (squeeze(Vall(5,:,:))', '+')
%plot (squeeze(Vall(6,:,:))', 'x')


%% Mu(I), Phi(I) 
figure(14) ; clf 
plotstring={'', '+g', 'xb', '*k', 'pr', 'vm'} ; 
for d=2:6
    for i=1:10
        %plot (squeeze(I(d,i,2:top(d,i)-1))', squeeze(Mu(d,i,2:top(d,i)-1))' , plotstring{d}) ; hold all 
        errorbar (nanmean(squeeze(I(d,i,2:top(d,i)-1))'), mean(squeeze(Mu(d,i,2:top(d,i)-1))'), nanstd(squeeze(I(d,i,2:top(d,i)-1))')/sqrt(top(d,i)-4), 'horizontal', plotstring{d}) ; hold all 
        %plot (squeeze(mean(I(d,i,2:top(d,i)-1),3))', squeeze(mean(Mu(d,i,2:top(d,i)-1),3))' , plotstring{d}) ; hold all 
        If{d}(i) = nanmean(squeeze(I(d,i,2:top(d,i)-1))') ;
        Muf{d}(i) = mean(squeeze(Mu(d,i,2:top(d,i)-1))') ;
        Ifstd{d}(i) = nanstd(squeeze(I(d,i,2:top(d,i)-1))')/sqrt(top(d,i)-4);
    end ;
    %Mu0(d) = max(Muf(d, If(d,:)<0.1))
    %plot (0, Mu0(d), 'o', 'MarkerSize', 15) ; 
    if (d==6) If{d}=If{d}(1:7) ; Muf{d}=Muf{d}(1:7) ; Ifstd{d}=Ifstd{d}(1:7); end ; 
        
    c=fit(If{d}',Muf{d}', 'a + b/(c/x+1)', 'Weights', Ifstd{d}) ; 
    plot(c, plotstring{d})
    Mu0(d) = c.a ; 
    MuDelta(d) = c.b ; 
    I0(d) = c.c ;
    %clear c ; 
end ; 

%%
figure(15) ; clf 
plotstring={'', '+g', 'xb', '*k', 'pr', 'vm'} ; 
clear Phi ; 
for d=2:6
    tmp=[] ; tmp2=[] ;
    for i=1:10
        plot (squeeze(I(d,i,2:top(d,i)-1))', squeeze(Phiall(d,i,2:top(d,i)-1))' , plotstring{d}) ; hold all 
        tmp=[tmp, squeeze(Phiall(d,i,2:top(d,i)-1))'] ; 
        tmp2=[tmp2, squeeze(I(d,i,2:top(d,i)-1))'] ; 
    end ;
    Phi{d}(:,2)=tmp ;
    Phi{d}(:,1)=tmp2 ;
    Phi{d}=rmmissing(Phi{d}) ; 
    Phi{d} = Phi{d}(Phi{d}(:,2)>0.1, :)
    clear c ; 
    c=fit(Phi{d}(:,1), Phi{d}(:,2), 'a*x+b', 'StartPoint', [-0.1, 0.5]) ; 
    plot (c) ; 
    phimax(d) = c.b ; 
    phidelta(d) = c.a ; 
end ; 


%% Get the min/max height that seem relevent
figure (10) ; clf ; clear top ;
for d=[2:6]
    for i=[1:10]
        plot (squeeze(Vall(d,i,:))) ; 
        [X,Y]=ginput(1) ; 
        %bottom(d)= round(X(1)) ; 
        top(d,i) = round(X(1)) ; 
    end ;
end ; 














