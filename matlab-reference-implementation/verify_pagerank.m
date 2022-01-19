function [matches] = verify_pagerank(R,d,L,conns,tol)
% VERIFY_PAGERANK
% Input:
% R: izračunani rank
% d: teleportacijski faktor
% L: vektor stevil izhodnih povezav {L(i) = t => i ima t izhodnih povezav}
% conns: matrika 2 x N, kjer vrstica i j pomeni, da obstaja povezava i->j
% tol: toleranca
%
% Output:
% true: če za vektor R velja 
%       R(i) = (1-d)/N + 
%               d * vsota{po straneh j, ki imajo povezavo do strani i}
%                       (R(j) / L(j))
% false: sicer
    N = size(R, 1);
    
    for k = 1:N
        calculated_r = (1 - d) / N;
        for povezava = conns'
            % Povezava:  j --> i
            j = povezava(1);
            i = povezava(2);
    
            if i == k
                calculated_r = calculated_r + d * R(j) / L(j);
            end
        end
    
        if abs(calculated_r - R(k)) > tol
            matches = false;
            return
        end
    end
    
    
    
    
    matches = true;
end