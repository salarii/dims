#ifndef MESSAGE_ENCRYPTION_LAYER_H
#define MESSAGE_ENCRYPTION_LAYER_H

namespace Self
{

class CAuthenticationProvider;

class CMessageEncryptionLayer
{
public:
	CMessageEncryptionLayer( CAuthenticationProvider const & _selfAuthenticationProvider );

	bool encrypt( void const * _messageIn, void const * _outOut );

	void unravelMessage( void const * _message );
	void validateMessage();

private:
		bool decriptLevel( void const * _message );
private:
	CAuthenticationProvider * m_selfAuthenticationProvider;
};

}

#endif