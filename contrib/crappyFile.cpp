
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


long long oper( std::string _ananas, std::vector< long long > & _result )
{
//std::reverse(_ananas.begin(),_ananas.end());
	std::string base(pszBase58);
	std::string::iterator it = _ananas.begin();
	long long number = 0;
	while ( it != _ananas.end() )
	{
		number *=58;
		number +=base.find(*it);

		it++;
	}

	number *= 0.42;
	return  number;
}



int main()
{

	std::vector<  long long  > result;
  long long  zen =oper("Rat",result);


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
