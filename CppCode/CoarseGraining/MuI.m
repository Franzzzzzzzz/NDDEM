clear all
figure(1) ; clf ; 
Marker = [''; ''; '*r'; '+b'; 'xk'; '^m'] ;  

Angles = [20 22 24 26 28 30 32 34 36 38 40] ; 
Rhog = [0, 1.2732395447351628, 1.9098593171027443,3.242277876554809, 6.079271018540266, 12.384589222348605] ;  
g = 10 ; 
diam = 2 ; 
dz=20/25 ; 

for d=[2:6]
    clear VAVG RHO ; 
    load(['CoarseMUID',num2str(d),'.mat']) ; 
    for i=[1:100:size(VAVG,3)-110] 
        Vall(d,(i-1)/100+1,:) = mean(VAVG(2,:,i+50:i+99),3) ;
        Rhoall (d,(i-1)/100+1,:) = mean(RHO(:,i+50:i+99),2) ;
        Pressure(d,(i-1)/100+1,:) = cumsum(Rhoall (d,(i-1)/100+1,:),3, 'reverse') * g * cosd(Angles((i-1)/100+1)) ;
        Gammadot(d,(i-1)/100+1,:) = (Vall(d,(i-1)/100+1,3:end) - Vall(d,(i-1)/100+1,1:end-2))/(2*dz) ; 
        Mu(d,(i-1)/100+1,:) = tand(Angles((i-1)/100+1)) * ones(1,23) ; 
    end ; 
    I(d,:,:)=Gammadot(d,:,:) * diam ./ sqrt(Pressure(d,:,2:end-1)/Rhog(d)) ; 
    I ([I<0 | isnan(I) | isinf(I)]) = nan  ; 

end ; 

figure(1) ; 
plot (squeeze(I(2,:,:))', squeeze(Mu(2,:,:))' , '-vm') ; hold all 
plot (squeeze(I(3,:,:))', squeeze(Mu(3,:,:))' , '-*r') ; hold all 
plot (squeeze(I(4,:,:))', squeeze(Mu(4,:,:))' , '-+g') ; hold all 
plot (squeeze(I(5,:,:))', squeeze(Mu(5,:,:))' , '-xk') ; hold all 
plot (squeeze(I(6,:,:))', squeeze(Mu(6,:,:))' , '-^b') ; hold all 

%semilogx (permute(I(3,:,:), [2,3,1]), permute(Mu(3,:,:), [2,3,1]) , '*r') ; hold all 
%semilogx (permute(I(4,:,:), [2,3,1]), permute(Mu(3,:,:), [2,3,1]) , '^b') ; hold all 
%semilogx (permute(I(5,:,:), [2,3,1]), permute(Mu(3,:,:), [2,3,1]) , '+g') ; hold all 
%semilogx (permute(I(6,:,:), [2,3,1]), permute(Mu(3,:,:), [2,3,1]) , 'xk') ; hold all 

xlim([0 5])

%%
load ('CoarseMUID3.mat') ; 
subplot(4,1,1) ; 
%imagesc(permute(VAVG(2,:,1:end), [2,3,1]))
imagesc(RHO(:,1:end))
for i=[1:100:size(VAVG,3)] 
    line([i,i],[0,25], 'Color', 'r'); 
    Vall(3,(i-1)/100+1,:) = mean(VAVG(2,:,i+50:i+99),3) ; 
end ; 

load ('CoarseMUID4.mat') ; 
subplot(4,1,2) ; 
%imagesc(permute(VAVG(2,:,1:end), [2,3,1]))
imagesc(RHO(:,1:end))
for i=[1:100:size(VAVG,3)], 
    line([i,i],[0,25], 'Color', 'r'); 
    Vall(4,(i-1)/100+1,:) = mean(VAVG(2,:,i+50:i+99),3) ; 
end ; 

load ('CoarseMUID5.mat') ; 
subplot(4,1,3) ; 
%imagesc(permute(VAVG(2,:,1:end), [2,3,1]))
imagesc(RHO(:,1:end))
for i=[1:100:size(VAVG,3)], 
    line([i,i],[0,25], 'Color', 'r'); 
    Vall(5,(i-1)/100+1,:) = mean(VAVG(2,:,i+50:i+99),3) ; 
end ; 

subplot(4,1,4) ; 
load ('CoarseMUID6.mat') ;
%imagesc(permute(VAVG(2,:,1:end), [2,3,1]))
imagesc(RHO(:,1:end))
for i=[1:100:size(VAVG,3)],
    line([i,i],[0,25], 'Color', 'r');
    Vall(6,(i-1)/100+1,:) = mean(VAVG(2,:,i+50:i+99),3) ; 
end ; 




