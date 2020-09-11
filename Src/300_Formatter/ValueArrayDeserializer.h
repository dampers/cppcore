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
	class CValueArrayDeserializer : public core::IFormatterT
	{
	private:
		std::vector< std::vector<std::tstring> >	m_vecValueTable;
		std::tstring	m_strQuotation;
		size_t			m_tArrayIndex;
		size_t			m_tItemIndex;
		std::tstring	m_strErrMsg;
		bool			m_bValidity;
		bool			m_bReserved[7];

	public:
		CValueArrayDeserializer(core::IChannel& channel, std::tstring strTokenDelimiter, std::tstring strQuotation, bool bSkipTitle = false);
		~CValueArrayDeserializer(void);

		bool			CheckValidity(std::tstring* pStrErrMsg)		{	if( pStrErrMsg )	*pStrErrMsg = m_strErrMsg;	return m_bValidity;		}

	private:
		size_t			_BeginDictionaryGrouping(std::tstring& strKey, const size_t tSize, bool bAllowMultiKey);
		void			_EndDictionaryGrouping();

		size_t			_BeginArrayGrouping(std::tstring& strKey, const size_t tSize);
		void			_EndArrayGrouping();

		void			_BeginArrayItem(size_t tIndex, size_t tCount);
		void			_EndArrayItem(size_t tIndex, size_t tCount);

		void			_BeginObjectGrouping(std::tstring& strKey);
		void			_EndObjectGrouping();

		void			_BeginRootGrouping();
		void			_EndRootGrouping();

		core::IFormatterT& Sync(std::tstring& strKey, std::tstring* pString);
		core::IFormatterT& Sync(std::tstring& strKey, std::ntstring* pString);
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
