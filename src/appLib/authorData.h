#ifndef AUTHOR_DATA_H
#define AUTHOR_DATA_H

#include <vector>
#include <string>

namespace  dims
{
extern std::string HardcodedSeed;

extern std::vector< std::string > PossibleMonitors;

extern std::vector< std::string > PossibleTrackers;

extern std::string AuthorId;

extern int Value;

extern int64_t ValidityExpirationTime; // not used yet
}

#endif // AUTHOR_DATA_H
