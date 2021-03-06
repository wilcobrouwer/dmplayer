#include "../cross-platform.h"
#include "datasource_filereader.h"
#include <iostream>
#include "../error-handling.h"

/** @brief FileReaderDataSource
  *
  * @todo: document this function
  */
 FileReaderDataSource::FileReaderDataSource(std::string FileName)
{
	FileHandle = fopen(FileName.c_str(), "rb");
	if (FileHandle == NULL)
		throw FileException(std::string("Could not open ") + FileName);
	reset();
}

FileReaderDataSource::~FileReaderDataSource()
{
	dcerr("shutting down");
	fclose(FileHandle);
	dcerr("shut down");
}

uint32 FileReaderDataSource::getpos() {
	return ftell(FileHandle);
}

/** @brief reset
  *
  * @todo: document this function
  */
void FileReaderDataSource::reset()
{
	fseek (FileHandle , 0 , SEEK_SET);
}

bool FileReaderDataSource::exhausted() {
	return feof(FileHandle) != 0;
}

/** @brief read
  *
  * @todo: document this function
  */
uint32 FileReaderDataSource::getData(uint8* const buffer, uint32 len)
{
	return fread (buffer, 1, len, FileHandle);
}
