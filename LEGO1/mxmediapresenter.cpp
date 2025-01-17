#include "mxmediapresenter.h"

#include "mxactionnotificationparam.h"
#include "mxautolocker.h"
#include "mxcompositepresenter.h"
#include "mxnotificationmanager.h"
#include "mxstreamchunk.h"
#include "mxtimer.h"

DECOMP_SIZE_ASSERT(MxMediaPresenter, 0x50);

// OFFSET: LEGO1 0x1000c550
MxMediaPresenter::~MxMediaPresenter()
{
	Destroy(TRUE);
}

// OFFSET: LEGO1 0x1000c5b0
void MxMediaPresenter::Destroy()
{
	Destroy(FALSE);
}

// OFFSET: LEGO1 0x100b54e0
void MxMediaPresenter::Init()
{
	this->m_subscriber = NULL;
	this->m_chunks = NULL;
	this->m_cursor = NULL;
	this->m_currentChunk = NULL;
}

// OFFSET: LEGO1 0x100b54f0
void MxMediaPresenter::Destroy(MxBool p_fromDestructor)
{
	{
		MxAutoLocker lock(&m_criticalSection);

		if (m_currentChunk && m_subscriber)
			m_subscriber->FUN_100b8390(m_currentChunk);

		if (m_subscriber)
			delete m_subscriber;

		if (m_cursor)
			delete m_cursor;

		if (m_chunks) {
			MxStreamChunkListCursor cursor(m_chunks);
			MxStreamChunk* chunk;

			while (cursor.Next(chunk))
				chunk->Release();

			delete m_chunks;
		}

		Init();
	}

	if (!p_fromDestructor)
		MxPresenter::Destroy();
}

// OFFSET: LEGO1 0x100b5650
MxStreamChunk* MxMediaPresenter::FUN_100b5650()
{
	MxStreamChunk* result = NULL;

	if (m_subscriber) {
		result = m_subscriber->FUN_100b8360();

		if (result && result->GetFlags() & MxDSChunk::Flag_Bit3) {
			m_action->SetFlags(m_action->GetFlags() | MxDSAction::Flag_Bit7);
			m_subscriber->FUN_100b8250();
			m_subscriber->FUN_100b8390(result);
			result = NULL;
			m_previousTickleStates |= 1 << (unsigned char) m_currentTickleState;
			m_currentTickleState = TickleState_Done;
		}
	}

	return result;
}

// OFFSET: LEGO1 0x100b56b0
MxStreamChunk* MxMediaPresenter::NextChunk()
{
	MxStreamChunk* result = NULL;

	if (m_subscriber) {
		result = m_subscriber->FUN_100b8250();

		if (result && result->GetFlags() & MxDSChunk::Flag_Bit3) {
			m_action->SetFlags(m_action->GetFlags() | MxDSAction::Flag_Bit7);
			m_subscriber->FUN_100b8390(result);
			result = NULL;
			m_previousTickleStates |= 1 << (unsigned char) m_currentTickleState;
			m_currentTickleState = TickleState_Done;
		}
	}

	return result;
}

// OFFSET: LEGO1 0x100b5d10
MxResult MxMediaPresenter::Tickle()
{
	MxAutoLocker lock(&m_criticalSection);

	FUN_100b5650();

	return MxPresenter::Tickle();
}

// OFFSET: LEGO1 0x100b5d90
void MxMediaPresenter::StreamingTickle()
{
	if (!m_currentChunk) {
		m_currentChunk = NextChunk();

		if (m_currentChunk) {
			if (m_currentChunk->GetFlags() & MxDSChunk::Flag_Bit2) {
				m_subscriber->FUN_100b8390(m_currentChunk);
				m_currentChunk = NULL;
				m_previousTickleStates |= 1 << (unsigned char) m_currentTickleState;
				m_currentTickleState = TickleState_Repeating;
			}
			else if (m_action->GetFlags() & MxDSAction::Flag_Looping) {
				AppendChunk(m_currentChunk);

				if (!IsEnabled()) {
					m_subscriber->FUN_100b8390(m_currentChunk);
					m_currentChunk = NULL;
				}
			}
		}
	}
}

// OFFSET: LEGO1 0x100b5e10
void MxMediaPresenter::RepeatingTickle()
{
	if (IsEnabled() && !m_currentChunk) {
		if (m_cursor)
			if (!m_cursor->Next(m_currentChunk))
				m_cursor->Next(m_currentChunk);

		if (m_currentChunk) {
			MxLong time = m_currentChunk->GetTime();
			if (time <= m_action->GetElapsedTime() % m_action->GetLoopCount()) {
				m_previousTickleStates |= 1 << (unsigned char) m_currentTickleState;
				m_currentTickleState = TickleState_unk5;
			}
		}
		else {
			if (m_action->GetElapsedTime() <= m_action->GetStartTime() + m_action->GetDuration()) {
				m_previousTickleStates |= 1 << (unsigned char) m_currentTickleState;
				m_currentTickleState = TickleState_unk5;
			}
		}
	}
}

// OFFSET: LEGO1 0x100b5ef0
void MxMediaPresenter::DoneTickle()
{
	m_previousTickleStates |= 1 << m_currentTickleState;
	m_currentTickleState = TickleState_Idle;
	EndAction();
}

// OFFSET: LEGO1 0x100b6030
void MxMediaPresenter::Enable(MxBool p_enable)
{
	if (IsEnabled() != p_enable) {
		MxPresenter::Enable(p_enable);

		if (p_enable) {
			MxLong time = Timer()->GetTime();
			m_action->SetUnkTimingField(time);
			SetTickleState(TickleState_Repeating);
		}
		else {
			if (m_cursor)
				m_cursor->Reset();
			m_currentChunk = NULL;
			SetTickleState(TickleState_Done);
		}
	}
}

// OFFSET: LEGO1 0x100b5700
MxResult MxMediaPresenter::StartAction(MxStreamController* p_controller, MxDSAction* p_action)
{
	MxResult result = FAILURE;
	MxAutoLocker lock(&m_criticalSection);

	if (MxPresenter::StartAction(p_controller, p_action) == SUCCESS) {
		if (m_action->GetFlags() & MxDSAction::Flag_Looping) {
			m_chunks = new MxStreamChunkList;
			m_cursor = new MxStreamChunkListCursor(m_chunks);

			if (!m_chunks && !m_cursor)
				goto done;
		}

		if (p_controller) {
			m_subscriber = new MxDSSubscriber;

			if (!m_subscriber ||
				m_subscriber->FUN_100b7ed0(p_controller, p_action->GetObjectId(), p_action->GetUnknown24()) != SUCCESS)
				goto done;
		}

		result = SUCCESS;
	}

done:
	return result;
}

// OFFSET: LEGO1 0x100b5bc0
void MxMediaPresenter::EndAction()
{
	MxAutoLocker lock(&m_criticalSection);

	if (!m_action)
		return;

	m_currentChunk = NULL;

	if (m_action->GetFlags() & MxDSAction::Flag_World &&
		(!m_compositePresenter || !m_compositePresenter->VTable0x64(2))) {
		MxPresenter::Enable(FALSE);
		SetTickleState(TickleState_Idle);
	}
	else {
		MxDSAction* action = m_action;
		MxPresenter::EndAction();

		if (m_subscriber) {
			delete m_subscriber;
			m_subscriber = NULL;
		}

		if (action && action->GetUnknown8c()) {
			NotificationManager()->Send(
				action->GetUnknown8c(),
				&MxEndActionNotificationParam(c_notificationEndAction, this, action, FALSE)
			);
		}
	}
}

// OFFSET: LEGO1 0x100b5f10
void MxMediaPresenter::AppendChunk(MxStreamChunk* p_chunk)
{
	MxStreamChunk* chunk = new MxStreamChunk;

	MxU32 length = p_chunk->GetLength();
	chunk->SetLength(length);
	chunk->SetData(new MxU8[length]);
	chunk->SetTime(p_chunk->GetTime());

	memcpy(chunk->GetData(), p_chunk->GetData(), chunk->GetLength());
	m_chunks->Append(chunk);
}
