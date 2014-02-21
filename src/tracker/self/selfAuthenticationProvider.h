#ifndef SELF_AUTHENTICATION_PROVIDER_H
#define SELF_AUTHENTICATION_PROVIDER_H

namespace Self
{

class CSelfAuthenticationProvider
{
public:
	CSelfAuthenticationProvider();

	bool hasKeys() const;
	void enableAccess() const;

	bool regenerateKeys();

private:
};

}

#endif // SELF_AUTHENTICATION_PROVIDER_H