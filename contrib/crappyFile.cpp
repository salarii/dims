
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




long long oper( std::string _ananas, std::vector< long long > & _result )
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
			{
				if ( check > 1000 )
					return 0;
				_result.push_back( check );
			}
			si=base.find(*it);

			loop++;

		}

		it = _ananas.begin();

		check++;
	}

}



int main()
{

	std::vector<  long long  > result;
  long long  zen =oper("r",result);


  std::string  dance ="";
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
