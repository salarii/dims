#ifndef ORIGIN_ADDRESS_SCANER_H
#define ORIGIN_ADDRESS_SCANER_H

namespace Self
{

class COriginAddressScaner
{
public:
	COriginAddressScaner();

	void resumeScaning();
	void scanBitcoinTransaction();


private:
	void saveBalanceToDatabase();

	void createCoinBaseTransaction();

	getHeightOfLastScanedBlock();


};

}


void scanBitcoinTransaction();
{


for (unsigned int i = 0; i < tx.vout.size(); i++)
{
	const CTxOut& txout = tx.vout[i];

	CScript::const_iterator pc = txout.scriptPubKey.begin();
	vector<unsigned char> data;
	while (pc < txout.scriptPubKey.end())
	{
		opcodetype opcode;
		if (!txout.scriptPubKey.GetOp(pc, opcode, data))
			break;

			fFound = true;

				txnouttype type;
				vector<vector<unsigned char> > vSolutions;
				if (Solver(txout.scriptPubKey, type, vSolutions) &&
					(type == TX_PUBKEY || type == TX_PUBKEYHASH))
			break;

				
		}
	}
}

if (fFound)
return true;

BOOST_FOREACH(const CTxIn& txin, tx.vin)
{
	// Match if the filter contains any arbitrary script data element in any scriptSig in tx
	CScript::const_iterator pc = txin.scriptSig.begin();
	vector<unsigned char> data;
	while (pc < txin.scriptSig.end())
	{
		opcodetype opcode;
		if (!txin.scriptSig.GetOp(pc, opcode, data))
			break;
		if (data.size() != 0 && contains(data))
			return true;
	}
//address  wysy³acza 


	create  coin base transaction 

		CPubKey pubkey;
	if (!reservekey.GetReservedKey(pubkey))
		return NULL;

	CScript scriptPubKey = CScript() << pubkey << OP_CHECKSIG;

		// Create coinbase tx
	CTransaction txNew;
	txNew.vin.resize(1);
	txNew.vin[0].prevout.SetNull();
	txNew.vout.resize(1);
	txNew.vout[0].scriptPubKey = scriptPubKeyIn;

}

return false;


#endif