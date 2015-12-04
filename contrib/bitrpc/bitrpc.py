from jsonrpc import ServiceProxy
import sys
import string

# ===== BEGIN USER SETTINGS =====
# if you do not set these you will be prompted for a password for every command
rpcuser = "a"
rpcpass = "b"
# ====== END USER SETTINGS ======


if rpcpass == "":
	access = ServiceProxy("http://127.0.0.1:20100")
else:
	access = ServiceProxy("http://"+rpcuser+":"+rpcpass+"@127.0.0.1:20100")
cmd = sys.argv[1].lower()

if cmd == "backupwallet":
	try:
		path = raw_input("Enter destination path/filename: ")
		print access.backupwallet(path)
	except:
		print "\n---An error occurred---\n"

elif cmd == "getbalance":
	try:
		acct = raw_input("Enter an account (optional): ")
		mc = raw_input("Minimum confirmations (optional): ")
		try:
			print access.getbalance(acct, mc)
		except:
			print access.getbalance()
	except:
		print "\n---An error occurred---\n"

elif cmd == "help":
	try:
		cmd = raw_input("Command (optional): ")
		try:
			print access.help(cmd)
		except:
			print access.help()
	except:
		print "\n---An error occurred---\n"

elif cmd == "selfaddress":
	try:
		print access.selfAddress()
	except:
		print "\n---An error occurred---\n"

elif cmd == "synchronizebitcoin":
	try:
		print access.synchronizeBitcoin()
	except:
		print "\n---An error occurred---\n"

elif cmd == "stop":
	try:
		print access.stop()
	except:
		print "\n---An error occurred---\n"

elif cmd == "networkinfo":
	try:
		print access.networkInfo()
	except:
		print "\n---An error occurred---\n"
				
elif cmd == "connectnetwork":
	try:
		print access.connectNetwork()
	except:
		print "\n---An error occurred---\n"
		
elif cmd == "status":
	try:
		print access.status()
	except:
		print "\n---An error occurred---\n"
		
elif cmd == "enternetwork":
	try:
		addr = raw_input("Address: ")
		print access.enterNetwork(addr)
	except:
		print "\n---An error occurred---\n"
		
elif cmd == "registerinnetwork":
	try:
		addr = raw_input("Address: ")
		print access.registerInNetwork(addr)
	except:
		print "\n---An error occurred---\n"
	    
elif cmd == "validateaddress":
	try:
		addr = raw_input("Address: ")
		print access.validateaddress(addr)
	except:
		print "\n---An error occurred---\n"

elif cmd == "walletpassphrase":
	try:
		pwd = raw_input("Enter wallet passphrase: ")
		access.walletpassphrase(pwd, 60)
		print "\n---Wallet unlocked---\n"
	except:
		print "\n---An error occurred---\n"

elif cmd == "walletpassphrasechange":
	try:
		pwd = raw_input("Enter old wallet passphrase: ")
		pwd2 = raw_input("Enter new wallet passphrase: ")
		access.walletpassphrasechange(pwd, pwd2)
		print
		print "\n---Passphrase changed---\n"
	except:
		print
		print "\n---An error occurred---\n"
		print

else:
	print "Command not found or not supported"