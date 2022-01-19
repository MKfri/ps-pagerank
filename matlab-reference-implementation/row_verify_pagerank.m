function [matches] = row_verify_pagerank(R,d,L,conns,tol,row_index)
% ROW_VERIFY_PAGERANK
% Input:
% R: izračunani rank
% d: teleportacijski faktor
% L: vektor stevil izhodnih povezav {L(j) = t => j ima t izhodnih povezav}
% conns: matrika 2 x N, kjer vrstica i j pomeni, da obstaja povezava i->j
% tol: toleranca
% row_index: indeks vrstice, ki jo zelimo preveriti
%
% Output:
% 1: če za vektor R velja 
%       R(row_index) = (1-d)/N + 
%               d * vsota{po straneh j, ki imajo povezavo do strani row_index}
%                       (R(j) / L(j))
% 0: sicer
    N = size(R, 1);
    
    calculated_r = (1 - d) / N;
    for povezava = conns'
        % Povezava:  j --> i
        j = povezava(1) + 1;
        i = povezava(2) + 1;

        if i == row_index
            calculated_r = calculated_r + d * R(j) / L(j);
        end
    end

    if abs(calculated_r - R(row_index)) > tol
        matches = false;
    else
        matches = true;
    end
end