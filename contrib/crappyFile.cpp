
#define RSA_KEY_LENGTH 2048
#define PUB_EXP     3
 #include <openssl/rsa.h>
 #include <openssl/sha.h>
#include <stdio.h>
#include <string.h>
#include <algorithm> 
#include <string>
#include <sstream>
static const char* pszBase58 = "123456789ABCDEFGHJKLMNPQRSTUVWXYZabcdefghijkmnopqrstuvwxyz";
#include <boost/variant.hpp>
#include <vector>

#include <iomanip>
#include <algorithm> 

#include  <boost/variant.hpp>

/*
bool matchAll( long long si, long long n )
{
	int  pat;
	int i = 0;
	while(1)
	{


		if ( si && !n || !si && n )
			return false;
		else if( si == 0 && n == 0 )
			return true;

		pat  = si % 10;
		si /= 10;

		if ( pat != n%10 )
			return false;
		n /=10;
	}

}


bool comp( char sign, int check, int loop )
{

	long long workSi =sign;
long long n = check * 1000000;
for ( int i = 0;i<28 + loop; i++ )
{
	n /= 58;
if ( n < 1000000 )
{
	n *= 1000;
}
}

int  pat = workSi % 10;
workSi /= 10;

bool flag =false;
while ( 1 )
{
	
	if ( pat == n%10  )
	{
		if( matchAll( workSi, n/10 ) )
			return true;
	}

	n/=10;
	if ( n == 0 )
		break;

}

return false;
}




long long oper( std::string _ananas )
{
std::reverse(_ananas.begin(),_ananas.end());
	std::string base(pszBase58);
	std::string::iterator it = _ananas.begin();
	long long si= 0;
	int  loop =0;
	int check = 1 ;
	
for ( int j = 1; j <  _ananas.size(); j++ )
{
check *= 58;
}

	while( 1 )
	{
		si=base.find(*it);
		

		if ( it == _ananas.end() )
			break;
		while ( comp( si,check, loop  ) )
		{
			it++;

			if ( it == _ananas.end())
				return check;
			si=base.find(*it);

			loop++;

		}

		it = _ananas.begin();

		check++;
	}

}

*/
/*
bool condition( unsigned int _leadinZeroNumber,unsigned char const (&_hash)[SHA256_DIGEST_LENGTH])
{
	for ( unsigned int i = SHA256_DIGEST_LENGTH - 1; i >= 0; --i )
	{
		if ( _leadinZeroNumber >= 8 )
		{
			if ( _hash[ i ] != 0 )
				return false;
			_leadinZeroNumber -= 8;
		}
		else
		{
			unsigned char compare = 0xff;
			compare >>= _leadinZeroNumber;
			if ( compare < _hash[ i ] )
				return false;
			else
				return true;
		}
	}
	return true;

}


bool
proofOfWorkChecking(std::vector< unsigned char > & _payload, unsigned int _leadinZeroNumber )
{
	SHA256_CTX sha256;
	SHA256_Init(&sha256);
	SHA256_Update(&sha256, &_payload[0], _payload.size());
	SHA256_Final(hash, &sha256);

	return condition( _leadinZeroNumber, hash);
}


struct CIncreaseCell
{
	static bool m_increase;
	CIncreaseCell(){}
	unsigned char operator() ( unsigned char & _cell ) const
	{
		if ( m_increase )
		{
			_cell = ++_cell % 0x100;

			m_increase = !_cell;
		}
		return _cell;
	}
};

bool CIncreaseCell::m_increase = false;



void
sha256(std::vector< unsigned char > & _payload, unsigned int _leadinZeroNumber )
{
	unsigned char hash[SHA256_DIGEST_LENGTH];

	do
	{	
		CIncreaseCell::m_increase = true;
		std::transform (_payload.rbegin(), _payload.rend(), _payload.rbegin(), CIncreaseCell());
		if ( CIncreaseCell::m_increase )
			_payload.push_back( 0 );

		SHA256_CTX sha256;
		SHA256_Init(&sha256);
		SHA256_Update(&sha256, &_payload[0], _payload.size());
		SHA256_Final(hash, &sha256);

	}
	while( !condition( _leadinZeroNumber, hash) );

}
*/

boost::variant< int  , float  >  emu((float)1.1);

template < class T >
class CResponseVisitorBase : public boost::static_visitor<int>
{
public:
	virtual T operator()(int _transactionStatus ) const
	{
		throw std::exception();
	}

	virtual T operator()(float _accountBalance ) const
	{
		throw std::exception();
	}

};

class CGetTransactionStatus : public CResponseVisitorBase< bool >
{
public:
	virtual bool operator()(float _accountBalance ) const
	{
		return true;
	}

};





int main()
{
	CResponseVisitorBase< bool > const & responseVisitorBase((CResponseVisitorBase< bool > const &)CGetTransactionStatus());
	boost::apply_visitor( responseVisitorBase, emu );

  //long long  zen =oper("R");

	// std::vector<unsigned char> vec;
	 //sha256(vec,0);

/*	RSA *keypair = RSA_generate_key(RSA_KEY_LENGTH, PUB_EXP, NULL, NULL);


	char msg[2048/8] = "1234567A1234567B1234567C1234567D";
	printf("msg: %s\n", msg);
	printf("size msg (asumo 256): %lu\n", sizeof(msg));
	unsigned char * encrypt = (unsigned char*)malloc(1024);
	memset(encrypt, 0, 1024);
	int temp;
	temp = RSA_public_encrypt((int)strlen(msg), (unsigned char*)msg, (unsigned char*)encrypt, keypair, RSA_PKCS1_OAEP_PADDING);
	printf("size encrypt(asumo 256):    %i\n", temp);

	printf((  char const * )encrypt);


	char * decrypt = (char*)malloc(1024);
	memset(decrypt, 0, 1024);
	temp = RSA_private_decrypt(temp, (unsigned char*)encrypt, (unsigned char*)decrypt,keypair , RSA_PKCS1_OAEP_PADDING);
	printf("size decrypt (asumo 32):    %i\n", temp);
	printf("decrypted: %s\n", decrypt);*/
	return 0;
}
// 58