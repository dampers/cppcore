#include "stdafx.h"
#include "XMLDeserializer.h"
#include "XMLFunctions.h"

namespace fmt_internal
{
	//////////////////////////////////////////////////////////////////////////
	CXMLDeserializer::CXMLDeserializer(core::IChannel& channel)
	: IFormatter(channel)
	, m_vecObjectCountStack()
	, m_stackTraverse()
	, m_bValidity(false)
	, m_strErrMsg()
	, m_stRoot()
	{
		if( !(m_bValidity = channel.CheckValidity(m_strErrMsg)) )
			return;

		std::tstring strContext;
		while(1)
		{
			const size_t tTokenSize = 512;
			TCHAR szBuff[tTokenSize+1] = { 0, };
			size_t tReadSize = channel.Access(szBuff, sizeof(TCHAR) * tTokenSize);
			if( 0 == tReadSize )
				break;

			strContext += szBuff;
		}

		m_bValidity = fmt_internal::ParseXmlContext(strContext, m_stRoot, m_strErrMsg);
	}

	//////////////////////////////////////////////////////////////////////////
	CXMLDeserializer::~CXMLDeserializer(void)
	{
	}

	//////////////////////////////////////////////////////////////////////////
	size_t CXMLDeserializer::BeginDictionary(std::tstring& strKey, const size_t tSize, bool bAllowMultiKey)
	{
		ST_XML_NODE* pNode = &m_stDummy;

		if( m_stackTraverse.top()->strKey == strKey )
		{
			pNode = m_stackTraverse.top();
		}
		else
		{
			size_t i;
			for(i=0; i<m_stackTraverse.top()->vecChild.size(); i++)
			{
				if( m_stackTraverse.top()->vecChild[i].strKey != strKey )
					continue;

				pNode = &m_stackTraverse.top()->vecChild[i];
				break;
			}
		}

		sGroupingData stGroupingData(GT_DICTIONARY, pNode->vecChild.size());
		m_vecObjectCountStack.push_back(stGroupingData);
		m_stackTraverse.push(pNode);
		return pNode->vecChild.size();
	}

	//////////////////////////////////////////////////////////////////////////
	void CXMLDeserializer::EndDictionary()
	{
		m_vecObjectCountStack.pop_back();
		m_stackTraverse.pop();
	}

	//////////////////////////////////////////////////////////////////////////
	size_t CXMLDeserializer::BeginArray(std::tstring& strKey, const size_t tSize)
	{
		size_t tCount = 0;
		size_t i;
		for(i=0; i<m_stackTraverse.top()->vecChild.size(); i++)
		{
			if( m_stackTraverse.top()->vecChild[i].strKey != strKey )
				continue;
			tCount++;
		}

		sGroupingData stGroupingData(GT_ARRAY, tCount);
		m_vecObjectCountStack.push_back(stGroupingData);
		return tCount;
	}

	//////////////////////////////////////////////////////////////////////////
	void CXMLDeserializer::EndArray()
	{
		m_vecObjectCountStack.pop_back();
	}

	//////////////////////////////////////////////////////////////////////////
	void CXMLDeserializer::BeginObject(std::tstring& strKey)
	{
		sGroupingData stGroupingData(GT_OBJECT, 0xFFFFFFFF);
		m_vecObjectCountStack.push_back(stGroupingData);

		sGroupingData& refParentGroupData = m_vecObjectCountStack[m_vecObjectCountStack.size()-2];

		ST_XML_NODE* pCurNode = m_stackTraverse.top();

		size_t tFoundIndex = 0;
		size_t i;
		for(i=0; i<pCurNode->vecChild.size(); i++)
		{
			if( (GT_DICTIONARY == refParentGroupData.nType) )
			{
				if( tFoundIndex < refParentGroupData.tSequence )
				{
					tFoundIndex++;
					continue;
				}

				strKey = pCurNode->vecChild[i].strKey;
			}
			else
			{
				if( pCurNode->vecChild[i].strKey != strKey )
					continue;

				if( (GT_ARRAY == refParentGroupData.nType) && (tFoundIndex < refParentGroupData.tSequence) )
				{
					tFoundIndex++;
					continue;
				}
			}

			refParentGroupData.tSequence++;
			m_stackTraverse.push(&pCurNode->vecChild[i]);
			return;
		}
		
		m_stackTraverse.push(&m_stDummy);
	}

	//////////////////////////////////////////////////////////////////////////
	void CXMLDeserializer::EndObject()
	{
		m_vecObjectCountStack.pop_back();
		m_stackTraverse.pop();
	}

	//////////////////////////////////////////////////////////////////////////
	void CXMLDeserializer::BeginRoot()
	{
		sGroupingData stGroupingData(GT_ROOT, 0xFFFFFFFF);
		m_vecObjectCountStack.push_back(stGroupingData);
		m_stackTraverse.push(&m_stRoot);
	}

	//////////////////////////////////////////////////////////////////////////
	void CXMLDeserializer::EndRoot()
	{
		m_vecObjectCountStack.pop_back();
		m_stackTraverse.pop();
	}

	//////////////////////////////////////////////////////////////////////////
	template<typename T>
	static inline void __XMLDeserializerSync(const std::stack<ST_XML_NODE*>& stackTraverse, std::vector<sGroupingData>& vecGroupingData, std::tstring& strKey, T* pValue)
	{
		sGroupingData& refGroupData = vecGroupingData.back();
		if( refGroupData.tSequence >= refGroupData.tTotalCount )
			return;

		size_t tIndex = refGroupData.tSequence++;
		size_t tFoundIndex = 0;

		if( GT_DICTIONARY == refGroupData.nType )
		{
			const std::vector<ST_XML_NODE>& vecChildren = stackTraverse.top()->vecChild;
			strKey = vecChildren[tIndex].strKey;
			*pValue = ValueFrom<T>(vecChildren[tIndex].strValue);
			return;
		}

		{	// Search on children
			const std::vector<ST_XML_NODE>& vecChildren = stackTraverse.top()->vecChild;

			size_t i;
			for(i=0; i<vecChildren.size(); i++)
			{
				if( vecChildren[i].strKey != strKey )
					continue;

				if( GT_ARRAY == refGroupData.nType && tIndex > tFoundIndex++ )
					continue;

				*pValue = ValueFrom<T>(vecChildren[i].strValue);
				return;
			}
		}
		
		{	// Search on attributes
			const std::vector<ST_XML_ATTR>& vecAttrs = stackTraverse.top()->vecAttr;

			size_t i;
			for(i=0; i<vecAttrs.size(); i++)
			{
				if( vecAttrs[i].strKey != strKey )
					continue;

				if( GT_ARRAY == refGroupData.nType && tIndex > tFoundIndex++ )
					continue;

				*pValue = ValueFrom<T>(vecAttrs[i].strValue);
				return;
			}
		}
	}

	//////////////////////////////////////////////////////////////////////////
	core::IFormatterT& CXMLDeserializer::Sync(std::tstring& strKey, std::tstring* pValue)
	{
		__XMLDeserializerSync(m_stackTraverse, m_vecObjectCountStack, strKey, pValue);
		return *this;
	}

	//////////////////////////////////////////////////////////////////////////
	core::IFormatterT & CXMLDeserializer::Sync(std::tstring & strKey, std::ntstring * pValue)
	{
		__XMLDeserializerSync(m_stackTraverse, m_vecObjectCountStack, strKey, pValue);
		return *this;
	}

	//////////////////////////////////////////////////////////////////////////
	core::IFormatterT& CXMLDeserializer::Sync(std::tstring& strKey, bool* pValue)
	{
		__XMLDeserializerSync(m_stackTraverse, m_vecObjectCountStack, strKey, pValue);
		return *this;
	}

	//////////////////////////////////////////////////////////////////////////
	core::IFormatterT& CXMLDeserializer::Sync(std::tstring& strKey, char* pValue)
	{
		__XMLDeserializerSync(m_stackTraverse, m_vecObjectCountStack, strKey, pValue);
		return *this;
	}

	//////////////////////////////////////////////////////////////////////////
	core::IFormatterT& CXMLDeserializer::Sync(std::tstring& strKey, short* pValue)
	{
		__XMLDeserializerSync(m_stackTraverse, m_vecObjectCountStack, strKey, pValue);
		return *this;
	}

	//////////////////////////////////////////////////////////////////////////
	core::IFormatterT& CXMLDeserializer::Sync(std::tstring& strKey, int32_t* pValue)
	{
		__XMLDeserializerSync(m_stackTraverse, m_vecObjectCountStack, strKey, pValue);
		return *this;
	}

	//////////////////////////////////////////////////////////////////////////
	core::IFormatterT& CXMLDeserializer::Sync(std::tstring& strKey, int64_t* pValue)
	{
		__XMLDeserializerSync(m_stackTraverse, m_vecObjectCountStack, strKey, pValue);
		return *this;
	}

	//////////////////////////////////////////////////////////////////////////
	core::IFormatterT& CXMLDeserializer::Sync(std::tstring& strKey, BYTE* pValue)
	{
		__XMLDeserializerSync(m_stackTraverse, m_vecObjectCountStack, strKey, pValue);
		return *this;
	}

	//////////////////////////////////////////////////////////////////////////
	core::IFormatterT& CXMLDeserializer::Sync(std::tstring& strKey, WORD* pValue)
	{
		__XMLDeserializerSync(m_stackTraverse, m_vecObjectCountStack, strKey, pValue);
		return *this;
	}

	//////////////////////////////////////////////////////////////////////////
	core::IFormatterT& CXMLDeserializer::Sync(std::tstring& strKey, DWORD* pValue)
	{
		__XMLDeserializerSync(m_stackTraverse, m_vecObjectCountStack, strKey, pValue);
		return *this;
	}

	//////////////////////////////////////////////////////////////////////////
	core::IFormatterT& CXMLDeserializer::Sync(std::tstring& strKey, QWORD* pValue)
	{
		__XMLDeserializerSync(m_stackTraverse, m_vecObjectCountStack, strKey, pValue);
		return *this;
	}

	//////////////////////////////////////////////////////////////////////////
	core::IFormatterT& CXMLDeserializer::Sync(std::tstring& strKey, float* pValue)
	{
		__XMLDeserializerSync(m_stackTraverse, m_vecObjectCountStack, strKey, pValue);
		return *this;
	}

	//////////////////////////////////////////////////////////////////////////
	core::IFormatterT& CXMLDeserializer::Sync(std::tstring& strKey, double* pValue)
	{
		__XMLDeserializerSync(m_stackTraverse, m_vecObjectCountStack, strKey, pValue);
		return *this;
	}

	//////////////////////////////////////////////////////////////////////////
	core::IFormatterT& CXMLDeserializer::Sync(std::tstring& strKey, BYTE* pData, size_t tLen)
	{
		// Ignore
		return *this;
	}
}
