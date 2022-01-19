
% Podatki:
% https://snap.stanford.edu/data/web-Google.html

tic;
% Malo hitrejsa varianta (~10%), ampak ne zna pohandlat primera, 
% ko je na zacetku se kaksna vrstica, ki ne predstavlja podatkov
% fileID = fopen('./data/web-Google.txt','r');
% conns = fscanf(fileID, '%d %d\n', [2 inf])';

conns = load("./data/web-Google.txt");
load_time = toc;


tic;

% +1 => stejemo se niclo
N = max(max(conns)) + 1;

L = zeros(N, 1);

for k = conns(:, 1)'
    L(k+1) = L(k+1) + 1;
end

% I, J, V; Ne poznamo velikosti v naprej,
% malo ugibamo da potem ne bo prevec realokacij
I = zeros(N, 1);
J = zeros(N, 1);
V = zeros(N, 1);

t = 1;
for povezava = conns'
    % Povezava:  j --> i
    % Popravimo indeks na matlabovo foro
    % saj zacenjamo z vozliscem 0
    j = povezava(1) + 1;
    i = povezava(2) + 1;
    
    % Opombe tukaj '1/L(j)' namesto 'L(j)' kot v navodilih
    I(t) = i;
    J(t) = j;
    V(t) = 1/L(j);

    t = t + 1;
end

I = I(1:t-1);
J = J(1:t-1);
V = V(1:t-1);

S = sparse(I, J, V, N, N);
preparation_time = toc;



tic;
d = 0.85;
ena = ones(N, 1);

% Razlika norm dveh zaporednih vektorjev
epsilon = 1e-8;
% Napaka na posameznem elementu vektorja
epsilon2 = 1e-8;


R_prev = 0 * ena;
R_curr = 1/N * ena;

rand_factor = (1-d) / N * ena;

iter_count = 0;

while norm(R_curr - R_prev) > epsilon
    R_prev = R_curr;
    R_curr = S * (d * R_prev) + rand_factor;
    iter_count = iter_count + 1;
end
calculation_time = toc;

fprintf("N: %d; Iterations: %d; Epsilon: %d\n", N, iter_count, epsilon);

% R_curr
tic;
% Preverimo da smo nasli resitev

verify_number = 3;
for i = 1:verify_number
    row_num = floor(rand*N);
    if row_verify_pagerank(R_curr, d, L, conns, epsilon2, row_num)
        fprintf("Row %d is correctly calculated\n", row_num);
    else
        fprintf("Row %d is INCORRECT\n", row_num);
    end
end

verification_time = toc;

fprintf("Load time: %.3f s\n", load_time);
fprintf("Preparation time: %.3f s\n", preparation_time);
fprintf("Calculation time: %.3f s\n", calculation_time);
fprintf("Verification time: %.3f s\n", verification_time);

