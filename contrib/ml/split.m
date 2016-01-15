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


function [y] = statesCnt( n )

y = ( (2 + floor(n/2 )  ) /2 ) * ( floor(n/2) + 1 ) + ( (2 + n - floor( n/2 ) ) /2 ) * (n - floor( n /2 ) - 1);

endfunction

function [z]=fullProbability(size) % size  means  network size, how many monitors are there
% probability of  given state  to happen 
% here state means how are monitors likely to  vote  base on current situation 
% with given ranking  distribution  involving  some randomness
% assumption: initiating  montor acts randomly( this  is  quite important!! )


function [y]= properColumn( attacked , defended )

y = statesCnt( attacked + defended ) - min( attacked , defended ) ;

endfunction


n = statesCnt( size );

transitions = zeros( size, n );


repetitionCnt = 100;

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
	    for z = 3 + j : 2 + m - j
	      suppDefense += rank( i , z );
	    endfor

	    if ( calculateResult( attackRatio, all , suppAttack, suppDefense ) > 0.5 )
	      attacked++;
	    else
	      defended++;
	    endif
	    
	  endfor
	  
	  column = properColumn( j + attacked , m - j + defended );
	  
	  modifier = 1;
	  if (m != 0 )
	    modifier= ( floor(m/2) + 1 ) * 2 - 1 + mod(m,2); 
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
V = zeros( ( size + 1 )* (SameState + 1), statesCnt( size )   );
endfunction

function [y] = Reward( i , j )

endfunction


function [y] = Probability( i , j, size ) 

global actions;
global SameState;

P = statesProbability( size );

trI =  mod(i - 1, size + 1 ) + 1;

p = P(:, :, trI );

FP = fullProbability(size);

probStates = createV( size );
statesProbAction = repmat ( probStates , 1,  columns( actions ));

PS = reshape( statesProbAction, rows(probStates), columns( probStates ), columns( actions ) );

for m = 1 : columns( p ) 
  id = i /( size );
  if ( floor(id) <= SameState + 1 )
    PS( ( size + 1) + i ,  j , m ) = p( trI, m );
  else
    PS(  i , j , m ) = p( trI, m );
  endif
  
  for l = trI: size    
    startState = (statesCnt( l - 1 ) ) + 1;
    
    endState = statesCnt( l );
    for h = startState : endState 
	  PS(  l + 1, h , m )= p( l+1, m )* FP( l-trI + 1, h);
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

  numberSubstates = floor( size / 2 ) + 1;

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

y = Rewards( i, j) +  staticPenal*( 1 + floor( i / (size +1) ) /2 );

endfunction


function Optymalyse( size )

global towardTime;
global actions;

CreateReward( size ); 
V = createV( size );

GivenActions = V;

Vnew = V;


VPrep = repmat( V, columns( actions) * rows(Vnew) * columns(Vnew) ,1 );  
VPres = reshape( VPrep, rows( V ), columns(V ), columns( actions),rows(Vnew), columns(Vnew) );

V

for i = 1 : rows(Vnew)

    for j = 1 : columns(Vnew)
      VPres(:,:, :, i ,j) = Probability(i, j, size);

  endfor
endfor      

      
iteration = 50;      
      
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

staticPenal = - 0.03;
towardTime = 0.9;

%Probability( 1, 1, 2 ) 
%Probability( 2, 1, 2 ) 
%fullProbability(1)
Optymalyse( 3 );

%Rewards

endfunction; 

