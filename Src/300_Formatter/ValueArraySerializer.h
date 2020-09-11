#pragma once

#include <string>
#include <vector>

#include "Interface.h"
#include "Pair.h"
#include "FmtTypes.h"
#include "IChannel.h"
#include "FormatterCommon.h"

namespace fmt_internal
{
	class CValueArraySerializer : public core::IFormatterT
	{
	private:
		std::tstring	m_strErrMsg;
		std::tstring	m_strSeperator;
		std::tstring	m_strQuotator;
		bool			m_bValidity;
		bool			m_bFirst;
		bool			m_bUseHexValue;
		bool			m_bReserved[5];

	public:
						CValueArraySerializer(core::IChannel& channel, std::tstring strSeperator, std::tstring strQuotator, bool bUseHex);
						~CValueArraySerializer(void);

		bool			CheckValidity(std::tstring* pStrErrMsg)		{	if( pStrErrMsg )	*pStrErrMsg = m_strErrMsg;	return m_bValidity;		}

	private:
		size_t			_BeginDictionaryGrouping(std::tstring& strKey, const size_t tSize, bool bAllowMultiKey);
		void			_EndDictionaryGrouping();

		size_t			_BeginArrayGrouping(std::tstring& strKey, const size_t tSize);
		void			_BeginArrayItem(size_t tIndex, size_t tCount);
		void			_EndArrayItem(size_t tIndex, size_t tCount);
		void			_EndArrayGrouping();

		void			_BeginObjectGrouping(std::tstring& strKey);
		void			_EndObjectGrouping();

		void			_BeginRootGrouping();
		void			_EndRootGrouping();

		core::IFormatterT& Sync(std::tstring& strKey, std::tstring* pValue);
		core::IFormatterT& Sync(std::tstring& strKey, std::ntstring* pValue);
		core::IFormatterT& Sync(std::tstring& strKey, bool* pValue);
		core::IFormatterT& Sync(std::tstring& strKey, char* pValue);
		core::IFormatterT& Sync(std::tstring& strKey, short* pValue);
		core::IFormatterT& Sync(std::tstring& strKey, int32_t* pValue);
		core::IFormatterT& Sync(std::tstring& strKey, int64_t* pValue);
		core::IFormatterT& Sync(std::tstring& strKey, BYTE* pValue);
		core::IFormatterT& Sync(std::tstring& strKey, WORD* pValue);
		core::IFormatterT& Sync(std::tstring& strKey, DWORD* pValue);
		core::IFormatterT& Sync(std::tstring& strKey, QWORD* pValue);
		core::IFormatterT& Sync(std::tstring& strKey, float* pValue);
		core::IFormatterT& Sync(std::tstring& strKey, double* pValue);
	};
}
