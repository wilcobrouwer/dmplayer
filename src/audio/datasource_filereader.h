#ifndef DATASOURCE_FILEREADER_H_INCLUDED
#define DATASOURCE_FILEREADER_H_INCLUDED

#include <string>
#include "datasource_interface.h"

class FileReaderDataSource: public IDataSource
{
	public:
		FileReaderDataSource(std::string FileName);
		~FileReaderDataSource();

		void reset();
		bool exhausted();
		uint32 getpos();
		uint32 getData(uint8* buffer, uint32 len);
	private:
		FILE* FileHandle;
};


#endif // DATASOURCE_FILEREADER_H_INCLUDED
