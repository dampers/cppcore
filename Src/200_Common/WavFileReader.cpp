#include "stdafx.h"
#include "WavFileReader.h"
#include "MemoryMappedFile.h"

namespace core
{
	ECODE ReadWavFile(std::tstring strWavFile, ST_WAVE_FORMATEX* outFormat, std::vector<unsigned char>& vecPCM)
	{
		ECODE nRet = EC_SUCCESS;
		CMemoryMappedFile MemFile;

		try
		{
			nRet = MemFile.Create(strWavFile.c_str(), PAGE_READWRITE_, FILE_MAP_READ_);
			if (EC_SUCCESS != nRet)
				throw exception_format(TEXT("Creating MemMappedFile(%s) failure, %d"), strWavFile.c_str(), nRet);

			LPCBYTE pWavData = MemFile.Ptr();
			size_t tWavSize = MemFile.Size();

			size_t tPos = 0;
			ST_WAV_HEADER* pHeader = (ST_WAV_HEADER*)(pWavData + tPos);
			tPos += sizeof(ST_WAV_HEADER);

			while (tPos + 8 < tWavSize)
			{
				ST_WAV_CHUNK* pChunkHeader = (ST_WAV_CHUNK*)(pWavData + tPos);
				if (0 == strncmp(pChunkHeader->szChunkID, "fmt ", 4))
				{
					ST_WAV_CHUNK_FMT* pFmt = (ST_WAV_CHUNK_FMT*)(pWavData + tPos);
					outFormat->cbSize = sizeof(*outFormat);
					outFormat->wFormatTag = pFmt->wAudioFormat;
					outFormat->nChannels = pFmt->wNumOfChannel;
					outFormat->nSamplesPerSec = pFmt->dwSampleRate;
					outFormat->nAvgBytesPerSec = pFmt->dwByteRate;
					outFormat->nBlockAlign = pFmt->wBlockAlign;
					outFormat->wBitsPerSample = pFmt->wBitPerSample;
				}
				if (0 == strncmp(pChunkHeader->szChunkID, "LIST", 4))
				{
					// [TODO]
				}
				if (0 == strncmp(pChunkHeader->szChunkID, "JUNK", 4))
				{
					// [TODO]
				}
				if (0 == strncmp(pChunkHeader->szChunkID, "data", 4))
				{
					ST_WAV_CHUNK_DATA* pData = (ST_WAV_CHUNK_DATA*)(pWavData + tPos);
					vecPCM.resize(pData->dwChunkSize);

					if (0 == pData->dwChunkSize)
						throw exception_format("ZERO data size");

					LPCBYTE pPCMSrc = (LPCBYTE)(pWavData + tPos + 8);
					memcpy(&vecPCM[0], pPCMSrc, pData->dwChunkSize);
				}

				tPos += sizeof(ST_WAV_CHUNK) + pChunkHeader->dwChunkSize;
			}

			MemFile.Destroy();
		}
		catch (std::exception& e)
		{
			Log_Error("%s", e.what());
			MemFile.Destroy();
			return nRet;
		}

		return EC_SUCCESS;
	}
}
