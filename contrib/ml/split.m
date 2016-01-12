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
 
 
actions = [ 0.05, 0.1, 0.2, 0.3, 0.4, 0.5, 0.6, 0.7, 0.8, 0.9, 1 ];
 
function statesProbability( actions, size )

states = zeros( size + 3, size * 4 );

actionsSize = columns( actions ); 



for k = 2 : size
  probabilities = zeros( size, actionsSize );
    for i = k : size

	for j = 1 : actionsSize
	  probabilities( i, j ) = probability( actions(j), i - k , size - k );

	endfor  
    endfor

probabilities

endfor

endfunction


function [p] = calculateResult( current, size, attackSupport, defendSupport )

function [y]= support( strength )

y = strength + 0.3 * strength ^ 2;

endfunction

attack = current + current *( support(attackSupport)/size );

defense = 1 - current + ( 1 - current ) *( support(defendSupport)/size ); 

p = attack ./ (attack + defense );

endfunction


function fullProbability(size) % size  means  network size, how many monitors are there
% probability of  given state  to happen 
% here state means how are monitors likely to  vote  base on current situation 
% with given ranking  distribution  involving  some randomness
% assumption: initiating  montor acts randomly( this  is  quite important!! )


function [y] = statesCnt( n )

y = ( (2 + floor(n/2 )  ) /2 ) * ( floor(n/2) + 1 ) + ( (2 + n - floor( n/2 ) ) /2 ) * (n - floor( n /2 ) - 1);

endfunction

function [y]= properColumn( attacked , defended )

y = statesCnt( attacked + defended ) - min( attacked , defended ) ;

endfunction


n = statesCnt( size -2 );

transitions = zeros( size - 2, n )


repetitionCnt = 10;


% first  get random  matrix  with  specific  size
for k=  3 : size
% teraz  losowanie
  for m=  0 : k - 3
    % m denotes  how many make  mind  already
    % possible states
    for j = 0 : m
    % j denotes how many agreed with  split( attacked )
      for rep = 1:repetitionCnt % fairness repetition
	
	rank = ranking(k);% move this  outside later	
	
	defended = 0; attacked = 0;
	
	howMany = k - 2 - m;
	
	for i =  1 : howMany
	% i denotes how  many will make mind
	%since this is an attack there is no corelation  what so ever with rank content, we can take any numbers from a row any row
	attackRatio = rank( i , 1 ) / ( rank( i , 2 ) + rank( i , 1 ) );

	
	if ( calculateResult( attackRatio, k -2 , j, m - j ) > 0.5 )
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

transitions

endfunction


fullProbability(10)


endfunction; 

