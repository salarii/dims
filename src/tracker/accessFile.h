// Copyright (c) 2014 Dims dev-team
// Distributed under the MIT/X11 software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.


#ifndef ACCESS_FILE_H
#define ACCESS_FILE_H

#include <boost/filesystem.hpp>
#include <boost/filesystem/fstream.hpp>

#include "common/ratcoinParams.h"

#include "serialize.h"
#include "version.h"
#include "simpleBuddy.h"

namespace tracker
{

template < class T >
struct CSerializedTypes
{
	static unsigned int const m_key;
};

class CAccessFile
{
public:
	CAccessFile();
	template< class T >
	bool loadSegmentFromFile( unsigned int _index, std::string const & _fileName, T & _t );

	template< class T >
	void saveSegmentToFile( unsigned int _index, std::string const & _fileName, T const & _block );

	bool fileExist(std::string const & _name ) const;

	FILE* openDiskFile(long int const pos, std::string _path, bool fReadOnly);
private:
	template < class T >
	unsigned int getSerialSize( T const & _t);
private:
	std::map< unsigned int , unsigned int > m_serializeSize;

};

template< class T >
bool
CAccessFile::loadSegmentFromFile( unsigned int _index, std::string const & _fileName, T & _t )
{
	if (!fileExist(_fileName))
		return false;

	FILE * fileStream ;

	unsigned int serialSize = getSerialSize( _t );

	fileStream = openDiskFile( serialSize * _index, _fileName, false);

	unsigned int bufferedSize = (1 << KiloByteShift) * 4;
	CBufferedFile blkdat(fileStream, 2*serialSize, serialSize, SER_DISK, CLIENT_VERSION);

	blkdat >> _t;

	return true;
}

template < class T >
unsigned int
CAccessFile::getSerialSize( T const & _t )
{
	unsigned int serialSize = 0;

	std::map< unsigned int , unsigned int >::iterator sizeIter = m_serializeSize.find( CSerializedTypes<T>::m_key );
	if ( sizeIter == m_serializeSize.end() )
	{
		serialSize = ::GetSerializeSize( _t, SER_DISK, CLIENT_VERSION );

		m_serializeSize.insert( std::make_pair( CSerializedTypes<T>::m_key, serialSize) );
	}
	else
	{
		serialSize = sizeIter->second;
	}
	return serialSize;
}

template< class T >
void
CAccessFile::saveSegmentToFile( unsigned int _index, std::string const & _fileName, T const & _block )
{
	unsigned int serialSize = getSerialSize( _block );

	FILE * fileStream;
		fileStream = openDiskFile( serialSize * _index, _fileName, false);

	CAutoFile autoFile(fileStream, SER_DISK, CLIENT_VERSION);
	autoFile << _block;
}

}

#endif // ACCESS_FILE_H
