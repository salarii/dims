namespace cpp monitorsScaner

typedef list<double> Vector

enum NetworkType
{
	TESTNET = 1,
	MAIN = 2
}

enum Info 
{
	TRACKERS_INFO = 1,
	MONITORS_INFO = 2
}

struct InfoRequest
{
	1:NetworkType networkType,
	2:Info info,
	3:string key,
}

exception Exception
{
	1:string msg,
}

struct Data
{
	1:i64 rows,
	2:i64 cols,
	3:list<string> data,
}

service MonitorsScaner
{
	Data getInfo (1:InfoRequest infoRequest) throws (1:Exception e),
}