% I try to emulate a monitor attack on a weak network
% atack means a stupid action from an initiating monitor side, weak network means a messy ranking table in monitors ( lot of noise )
% assumption: an initiating  montor acts randomly( this  is  quite important!! ) 
% my goal : monitor cluster anyway after  split

function split

MaxSize = 10;

function [y]=ranking(size, baseFactor = 4, deviationFactor = 17, variationBase = 1000 )% at least 2
  typical = unidrnd(variationBase); 
  
  rand = unidrnd(typical, [size 1]).+ variationBase * baseFactor;
  ranking = repmat (rand', size, 1);

  y = normrnd( ranking, ranking ./deviationFactor );
endfunction;
  

  
function [z] = decision( strength, validity )
    z = 1;
endfunction;  
  
 
function iter( Ranking, Current, mine )

  size = size( Ranking );
  total = 0;

  for j = 1: size
    if ( mine != j )
      total += Ranking(j);
    endif
  
  endfor

  strength = 0;
  validity = 0;

  for i = 1: size

  if ( mine != j )
      if ( Current(i) == 1 )
	strength += Ranking(i)
	validity += Ranking(i)/total;
      elseif ( Current(i) == 0 )
	validity += Ranking(i)/total;
      endif
    endif

  endfor

strength = strength /( total * validity );

% now apply function for  now  assume  1 

if ( decision( strength, validity ) == 1 )

else

endif

endfunction; 



% tell split with

smallest =  realmax;

SplitWithRow = 0;

%for i = 1: Size
%  if ( smallest > rank( i , 1 ) )
  
%    smallest = rank( i , 1 );
%    SplitWithRow = i;
%   endif 
%endfor


%SplitState = ones(Size) * 0.5;
 
 
function [z] = probability( p, n , k )

combCnt = nchoosek (k, n);

elemProb =  realpow( p , n) * realpow( 1 - p , k - n );


z = elemProb * combCnt;

endfunction;  
 
global actions = [ 0.05, 0.1, 0.2, 0.3, 0.4, 0.5, 0.6, 0.7, 0.8, 0.9, 1 ];

global SameState = 3; % this + 1 is how many different actions can be  taken in basically the same internal state

function [y]=statesProbability( size )

global actions;

global SameState;

actionsSize = columns( actions ); 

probabilities = zeros( size + 1, actionsSize, size + 1 );

for k = 0 : size - 1
  
    for i = k : size

	for j = 1 : actionsSize
	  probabilities( i + 1, j, k + 1 ) = probability( actions(j), i - k , size - k );

	endfor  
    endfor
    
endfor

y = probabilities;

endfunction


function [p] = calculateResult( current, size, attackSupport, defendSupport )

function [y]= support( strength, all )

y = 0.3 * ( 1/(1 - strength/all)^2 );

endfunction

attack = current + current *support(attackSupport,size );

defense = 1 - current + ( 1 - current ) * support(defendSupport, size ); 

p = attack ./ (attack + defense );

endfunction


function [y] = levelStates( n )

y = floor(n/2 ) + 1;
endfunction

function [y] = statesCnt( n )

y = ( (2 + floor(n/2 )  ) /2 ) * ( floor(n/2) + 1 ) + ( (2 + n - floor( n/2 ) ) /2 ) * (n - floor( n /2 ) - 1)+1;

endfunction


function [y] = fullstatesCnt( n )
level = 1;
y = 0;
for i = 1 : n
level = levelStates( i ) * levelStates( i - 1 );
y += level;
endfor 

endfunction

function [z]=fullProbability(size) % size  means  network size, how many monitors are there
% probability of  given state  to happen 
% here state means how are monitors likely to  vote  base on current situation 
% with given ranking  distribution  involving  some randomness
% assumption: initiating  montor acts randomly( this  is  quite important!! )


function [y]= properColumn( attacked , defended )

y = fullstatesCnt( attacked + defended - 1 ) + levelStates( attacked + defended ) - min( attacked , defended );

endfunction


n = fullstatesCnt( size );

transitions = zeros( size, n );


repetitionCnt = 10;

for rep = 1: repetitionCnt%   fairness repetition
	
  rank = ranking(size +  2 );
  % first  get random  matrix  with  specific  size
  for k=  1 : size
  % teraz  losowanie
    for m=  0 : k - 1
      % m denotes  how many make  mind  already
      % possible states
      for j = 0 : m
      % j denotes how many agreed with  split( attacked )
	  
	  defended = 0; attacked = 0;
	  
	  howMany = k - m;
	  
	  for i =  1 : howMany
	    % i denotes how  many will make mind
	    %since this is an attack there is no corelation  what so ever with rank content, we can take any numbers from a row any row
	    attackRatio = rank( i , 1 ) / ( rank( i , 2 ) + rank( i , 1 ) );

	    all = 0;
	    for z = 1 : k
	      all += rank( i , z );
	    endfor

	    suppAttack = 0;
	    for z = 3 : 2 + j
	      suppAttack += rank( i , z );
	    endfor
	    
	    suppDefense = 0;
	    for z = 3 + j : 2 + m
	      suppDefense += rank( i , z );
	    endfor


	    
	    if ( calculateResult( attackRatio, all , suppAttack, suppDefense ) > 0.5 )
	      attacked++;
	    else
	      defended++;
	    endif
	    
	  endfor

	  column = properColumn( j + attacked , m - j + defended ) + levelStates( m + i ) * ( levelStates( m ) - min( j , m - j ) - 1 );
	    
	  modifier = 1;
	  if (  2*j != m )
	    modifier++; 
	  endif

	  transitions( howMany, column ) = transitions( howMany, column ) + 1/(repetitionCnt * modifier );
      
      endfor
    
    endfor

  endfor

endfor
      
z = transitions;

endfunction



function [V] = createV(size)
global SameState;
V = zeros( SameState + 1, statesCnt( size ) );
endfunction

function [y] = determineColumn( n )
i = 0;
States = 0;

while( levelStates( i ) < n )
n -= levelStates( i );
i++;
endwhile

y = n - 1;

endfunction

function [y] = reverseStatesCnt( state )

% don't  try to be smart
mainState = 0;

while (state > statesCnt( mainState ))
  mainState++;
endwhile

y = mainState;

endfunction

function [y] = Probability( deep, stateId, size ) 

global actions;
global SameState;

P = statesProbability( size );

nodesDecided = reverseStatesCnt( stateId )

trI =  mod(nodesDecided, size + 1 ) + 1;

p = P(:, :, nodesDecided + 1 );

FP = fullProbability(size)

probStates = createV( size );
statesProbAction = repmat ( probStates , 1,  columns( actions ));

PS = reshape( statesProbAction, rows(probStates), columns( probStates ), columns( actions ) );

for m = 1 : columns( p ) 

  if ( deep < SameState + 1 )
    PS( ( size + 1) + nodesDecided ,  j , m ) = p( trI, m );
  else
   % PS(  nodesDecided , j , m ) = p( trI, m );
  endif
  
  for l = 1 : size - nodesDecided
    
    for h = 1 : levelStates( l + nodesDecided )
      col = determineColumn(stateId)* levelStates( nodesDecided + l )  + fullstatesCnt( nodesDecided + l - 1 ) + h; 
      PS( 1 , statesCnt( l + nodesDecided - 1 ) + h  , m ) = p( nodesDecided + l + 1, m ) * FP( l, col );
    endfor 
  
  endfor 

endfor

y = PS;

endfunction

function [y] = penalty( m , n, size )
y = stdnormal_pdf( 0 )-stdnormal_pdf( ( n / (m + 0.1) - m / (n + 0.1 ) ) / 2 ) * ( (m + n)/size )^2;
endfunction

global Rewards;

function CreateReward( size )

  global Rewards;
  global SameState;
  
  Rewards = createV( size );

  allSubstates = statesCnt( size );

  numberSubstates = levelStates( size );

  for j = 1 : SameState + 1
  
    for i = 0 : numberSubstates - 1

      Rewards( j * ( size + 1), allSubstates - i ) = penalty( i , size  - i, size );
    endfor
 endfor
 
endfunction


global staticPenal;

global towardTime;

function [y] = Rs (i,j, size)

global Rewards;
global staticPenal;

y = Rewards( i, j) +  staticPenal * ( 1 +  power( floor( i / (size +1) ) /2, 3 ) );

endfunction


function Optymalyse( size )

global towardTime;
global actions;

CreateReward( size ); 
V = createV( size );

GivenActions = V;

Vnew = V;


VPrep = repmat( V, 1, columns( actions) * rows(Vnew) * columns(Vnew) );  
VPres = reshape( VPrep, rows( V ), columns(V ), columns( actions),rows(Vnew), columns(Vnew) );

for i = 1 : rows(Vnew)

    for j = 1 : columns(Vnew)
      VPres(:,:, :, i ,j) = Probability(i, j, size);
    endfor
endfor      

      
iteration = 200;      
      
for k =  1 : iteration

  Vrep = repmat( V, 1 ,columns( actions) );  
  Vres = reshape( Vrep, rows( V ), columns(V ), columns( actions) );
  
  for i = 1 : rows(Vnew)

    for j = 1 : columns(Vnew)
 
  
    Vnew(i,j) = Rs( i, j, size ) + towardTime .* max ( sum( sum( (  VPres(:,:,:,i,j) .* Vres ), 1 ), 2 ) );
	

    endfor
  endfor
  
  V = Vnew;
endfor

for i = 1 : rows(Vnew)

  for j = 1 : columns(Vnew)
      
    [ optimal, max_index ] =  max ( sum( sum( (  VPres(:,:,:,i,j) .* Vres ), 1 ), 2 ) );
     
     GivenActions(i , j) = max_index;
  endfor
endfor

V

GivenActions

endfunction

staticPenal = - 0.005;
towardTime = 1;
 
%fullProbability(5)
%Optymalyse( 5 );
%  1  2 4 6 9 
%  1   2 2 3 3

for i = 1 : 5
%c= i - 1
endfor

statesCnt( 2 + 0 - 1 ) + 1 
statesCnt( 2 + 0 - 1 ) + 2
x = statesProbability( 4 );
x(:,:, 1)


Probability(  1, 4 )

%Rewards

endfunction; 

