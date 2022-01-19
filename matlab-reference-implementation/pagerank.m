
% fileID = fopen('big-input.txt','r');
% fileID = fopen('small-input.txt','r');
% conns = fscanf(fileID, '%d %d\n', [2 inf])';

conns = load('big-input.txt');

N = max(max(conns));

M = zeros(N);
L = zeros(N, 1);

izhodne = conns(:, 1);

for k = izhodne'
    L(k) = L(k) + 1;
end

for povezava = conns'
    % Povezava:  j --> i
    j = povezava(1);
    i = povezava(2);

    % Opombe tukaj '1/L(j)' namesto 'L(j)' kot v navodilih
    M(i, j) = 1/L(j);
end

% M1 = [0 1 3 0 2; 1 0 0 0 0; 0 0 0 0 0; 0 0 3 0 2; 0 0 3 0 0]
% N = 5;

d = 0.85;
ena = ones(N, 1);
epsilon = 1e-3;


R_prev = 0 * ena;
R_curr = 1/5 * ena;

rand_factor = (1-d)/N * ena;

iter_count = 0;

while norm(R_curr - R_prev) > epsilon
    R_prev = R_curr;
    R_curr = d * M * R_prev + rand_factor;
    iter_count = iter_count + 1;
end

iter_count
R_curr

% Preverimo da smo nasli resitev
verify_pagerank(R_curr, d, L, conns, epsilon)




