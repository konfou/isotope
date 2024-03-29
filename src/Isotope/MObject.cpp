/*
	Copyright (C) 2006, Edumetrics Institute

	This program is free software; you can redistribute it and/or
	modify it under the terms of the GNU General Public License
	as published by the Free Software Foundation; either version 2
	of the License, or (at your option) any later version.

	see http://www.gnu.org/copyleft/gpl.html
*/

#include "MObject.h"
#include "Main.h"
#include "Model.h"
#include "MStore.h"
#include "MAnimation.h"
#include "../GClasses/GString.h"
#include "../Gasp/BuiltIns/GaspFloat.h"
#include "../Gasp/Engine/EType.h"
#include "../GClasses/GBillboardCamera.h"
#include "MGameClient.h"
#include "MRealm.h"
#include "MScriptEngine.h"

/*static*/ int AllocCounter::s_allocs = 0;
/*static*/ int AllocCounter::s_deallocs = 0;

AllocCounter::AllocCounter()
{
	s_allocs++;
}

AllocCounter::~AllocCounter()
{
	s_deallocs++;
}

// ---------------------------------------------------------------------------


MObject::MObject(MScriptEngine* pScriptEngine)
 : 
#ifdef _DEBUG
 AllocCounter(),
#endif // _DEBUG
 m_vh(pScriptEngine->GetEngine())
{
	GAssert(m_vh.m_szID = "MObject.m_vh", "");
	m_pScriptEngine = pScriptEngine;
	m_flags = MOB_Tangible;
	m_nPivotHeight = 0;
}

MObject::~MObject()
{
	// We need to explicitly call the destructor for m_vh because we explicitly called
	// the constructor for it
	m_vh.~VarHolder();
}

GImage* MObject::GetFrame(GRect* pRect, GBillboardCamera* pCamera)
{
	return m_pScriptEngine->CallGetFrame(GetGObject(), pRect, pCamera->GetDirection());
}

void MObject::GetPos(float* px, float* py)
{
	ObjectObject* pOb = (ObjectObject*)m_vh.GetGObject();
	*px = (float)((GaspFloat*)pOb->arrFields[MOB_FIELD_X])->m_value;
	*py = (float)((GaspFloat*)pOb->arrFields[MOB_FIELD_Y])->m_value;
}

void MObject::SetGhostPos(float x, float y)
{
	m_drawPos.x = x;
	m_drawPos.y = y;
}

void MObject::SetPos(float x, float y, float z)
{
	ObjectObject* pOb = (ObjectObject*)m_vh.GetGObject();
	((GaspFloat*)pOb->arrFields[MOB_FIELD_X])->m_value = x;
	((GaspFloat*)pOb->arrFields[MOB_FIELD_Y])->m_value = y;
	((GaspFloat*)pOb->arrFields[MOB_FIELD_Z])->m_value = z;
}

GPosSize* MObject::GetGhostPos()
{
	ObjectObject* pOb = (ObjectObject*)m_vh.GetGObject();
	float tmp;
	tmp = (float)((GaspFloat*)pOb->arrFields[MOB_FIELD_X])->m_value;
	if(absfloat(tmp - m_drawPos.x) < (float)INTERPOLATE_DISTANCE)
		m_drawPos.x = tmp;
	else
		m_drawPos.x = ((float)INTERPOLATE_RATE * m_drawPos.x) + (((float)1 - (float)INTERPOLATE_RATE) * tmp);
	tmp = (float)((GaspFloat*)pOb->arrFields[MOB_FIELD_Y])->m_value;
	if(absfloat(tmp - m_drawPos.y) < (float)INTERPOLATE_DISTANCE)
		m_drawPos.y = tmp;
	else
		m_drawPos.y = ((float)INTERPOLATE_RATE * m_drawPos.y) + (((float)1 - (float)INTERPOLATE_RATE) * tmp);
	m_drawPos.z = (float)((GaspFloat*)pOb->arrFields[MOB_FIELD_Z])->m_value;
	m_drawPos.sx = (float)((GaspFloat*)pOb->arrFields[MOB_FIELD_SX])->m_value;
	m_drawPos.sy = (float)((GaspFloat*)pOb->arrFields[MOB_FIELD_SY])->m_value;
	m_drawPos.sz = (float)((GaspFloat*)pOb->arrFields[MOB_FIELD_SZ])->m_value;
	return &m_drawPos;
}

void MObject::SetSize(float sx, float sy, float sz)
{
	ObjectObject* pOb = (ObjectObject*)m_vh.GetGObject();
	((GaspFloat*)pOb->arrFields[MOB_FIELD_SX])->m_value = sx;
	((GaspFloat*)pOb->arrFields[MOB_FIELD_SY])->m_value = sy;
	((GaspFloat*)pOb->arrFields[MOB_FIELD_SZ])->m_value = sz;
}

void MObject::Update(double time)
{
	m_pScriptEngine->CallUpdate(GetGObject(), time);
}

double MObject::GetTime()
{
	ObjectObject* pOb = (ObjectObject*)m_vh.GetGObject();
	return ((GaspFloat*)pOb->arrFields[MOB_FIELD_TIME])->m_value;
}

void MObject::SetTime(double time)
{
	ObjectObject* pOb = (ObjectObject*)m_vh.GetGObject();
	((GaspFloat*)pOb->arrFields[MOB_FIELD_TIME])->m_value = time;
}

int MObject::GetUid()
{
	ObjectObject* pOb = (ObjectObject*)m_vh.GetGObject();
	return ((IntObject*)pOb->arrFields[MOB_FIELD_UID])->m_value;
}

void MObject::SetUid(int uid)
{
	ObjectObject* pOb = (ObjectObject*)m_vh.GetGObject();
	((IntObject*)pOb->arrFields[MOB_FIELD_UID])->m_value = uid;
}

/*static*/ int MObject::GetUid(ObjectObject* pObj)
{
	return ((IntObject*)pObj->arrFields[MOB_FIELD_UID])->m_value;
}

float MObject::GetDistanceSquared(MObject* pThat)
{
	float x1, y1, x2, y2;
	GetPos(&x1, &y1);
	pThat->GetPos(&x2, &y2);
	x2 -= x1;
	y2 -= y1;
	return x2 * x2 + y2 * y2;
}

float MObject::GetDistanceSquared(float x, float y)
{
	float x1, y1;
	GetPos(&x1, &y1);
	x -= x1;
	y -= y1;
	return x * x + y * y;
}

bool MObject::IsChatCloud()
{
	GObject* pOb = m_vh.GetGObject();
	if(!pOb)
		return false;
	if(strcmp(pOb->GetType()->GetName(), "ChatCloud") == 0)
		return true;
	return false;
}

const char* MObject::GetTypeName()
{
	return m_vh.GetGObject()->GetType()->GetName();
}

bool MObject::SanityCheck()
{
	ObjectObject* pOb = (ObjectObject*)m_vh.GetGObject();
	if(!pOb)
		return false; // MObject wraps NULL
	double dTime = GetTime();
	if(dTime < 1137777000 /*The year 2006*/ || dTime > 2147483647 /*The year 2038*/)
		return false; // time out of range. 86400 = number of seconds in a day
	return true;
}
