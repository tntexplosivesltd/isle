#ifndef MXNEXTACTIONDATASTART_H
#define MXNEXTACTIONDATASTART_H

#include "mxcore.h"

// VTABLE 0x100dc9a0
// SIZE 0x14
class MxNextActionDataStart : public MxCore {
public:
	// inlined constructor at 0x100c1847
	inline MxNextActionDataStart(MxU32 p_objectId, MxS16 p_unk24val, MxU32 p_data)
	{
		m_objectId = p_objectId;
		m_unk24val = p_unk24val;
		m_data = p_data;
	}

	// OFFSET: LEGO1 0x100c1900
	inline virtual const char* ClassName() const override // vtable+0xc
	{
		// 0x101025a0
		return "MxNextActionDataStart";
	}

	// OFFSET: LEGO1 0x100c1910
	inline virtual MxBool IsA(const char* name) const override // vtable+0x10
	{
		return !strcmp(name, MxNextActionDataStart::ClassName()) || MxCore::IsA(name);
	}

private:
	MxU32 m_objectId;
	MxS16 m_unk24val;
	MxU32 m_data;
};

#endif // MXNEXTACTIONDATASTART_H
