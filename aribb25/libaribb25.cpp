// libaribb25.cpp: CB25Decoder クラスのインプリメンテーション
//
//////////////////////////////////////////////////////////////////////

#include "libaribb25.h"

BOOL WINAPI DllMain(HINSTANCE hinstDLL, DWORD fdwReason, LPVOID lpvReserved)
{
	if (fdwReason == DLL_PROCESS_ATTACH) {
		DisableThreadLibraryCalls(hinstDLL);
	} else if (fdwReason == DLL_PROCESS_DETACH) {
		// 未開放の場合はインスタンス開放
		if (CB25Decoder::m_pThis) {
			CB25Decoder::m_pThis->Release();
		}
	}
	return TRUE;
}

//////////////////////////////////////////////////////////////////////
// インスタンス生成メソッド
//////////////////////////////////////////////////////////////////////

extern "C"
{

__declspec(dllexport) IB25Decoder * CreateB25Decoder()
{
	// インスタンス生成
	return (CB25Decoder::m_pThis) ?
		CB25Decoder::m_pThis : dynamic_cast<IB25Decoder *>(new CB25Decoder());
}

__declspec(dllexport) IB25Decoder2 * CreateB25Decoder2()
{
	// インスタンス生成
	return (CB25Decoder::m_pThis) ?
		CB25Decoder::m_pThis : dynamic_cast<IB25Decoder2 *>(new CB25Decoder());
}

}

//////////////////////////////////////////////////////////////////////
// 構築/消滅
//////////////////////////////////////////////////////////////////////

// 静的メンバ初期化
CB25Decoder * CB25Decoder::m_pThis = NULL;

CB25Decoder::CB25Decoder(void) : _bcas(NULL), _b25(NULL), _data(NULL)
{
	m_pThis = this;
}

CB25Decoder::~CB25Decoder(void)
{
	if (_data)
		::free(_data);

	_mtx.lock();

	if (_b25)
		_b25->release(_b25);

	if (_bcas)
		_bcas->release(_bcas);

	_mtx.unlock();

	m_pThis = NULL;
}

void CB25Decoder::Release()
{
	// インスタンス開放
	delete this;
}

const BOOL CB25Decoder::Initialize(DWORD dwRound)
{
	int ret = FALSE;

	_mtx.lock();

	if (_b25)
		goto unlock;

	_bcas = create_b_cas_card();
	if (!_bcas)
		goto unlock;

	if (_bcas->init(_bcas) < 0)
		goto err;

	_b25 = create_arib_std_b25();
	if (!_b25)
		goto err;

	if (_b25->set_b_cas_card(_b25, _bcas) < 0)
		goto err;

	// success
	_b25->set_strip(_b25, 1);
	_b25->set_emm_proc(_b25, 0);
	_b25->set_multi2_round(_b25, dwRound);

	ret = TRUE;
	goto unlock;

err:
	// error
	if (_b25)
	{
		_b25->release(_b25);
		_b25 = NULL;
	}

	if (_bcas)
	{
		_bcas->release(_bcas);
		_bcas = NULL;
	}

	_errtime = ::GetTickCount();

unlock:
	_mtx.unlock();

	return ret;
}

const BOOL CB25Decoder::Decode(BYTE *pSrcBuf, const DWORD dwSrcSize, BYTE **ppDstBuf, DWORD *pdwDstSize)
{
	if (!_b25)
	{
		DWORD now = ::GetTickCount();
		DWORD interval = (now - _errtime) / 1000;
		if (interval > RETRY_INTERVAL) {
			if (Initialize() < 0)
				_errtime = now;
		}

		if (!_b25) {
			if (*ppDstBuf != pSrcBuf) {
				*ppDstBuf = pSrcBuf;
				*pdwDstSize = dwSrcSize;
			}
			return FALSE;
		}
	}

	if (_data) {
		::free(_data);
		_data = NULL;
	}

	ARIB_STD_B25_BUFFER buf;
	buf.data = pSrcBuf;
	buf.size = dwSrcSize;
	const int rc = _b25->put(_b25, &buf);
	if (rc < 0) {
		if (rc >= ARIB_STD_B25_ERROR_NO_ECM_IN_HEAD_32M) {
			// pass through
			_b25->release(_b25);
			_b25 = NULL;
			_bcas->release(_bcas);
			_bcas = NULL;
			if (*ppDstBuf != pSrcBuf) {
				*ppDstBuf = pSrcBuf;
				*pdwDstSize = dwSrcSize;
			}
		} else {
			BYTE *p = NULL;
			_b25->withdraw(_b25, &buf);	// withdraw src buffer
			if (buf.size > 0)
				p = (BYTE *)::malloc(buf.size + dwSrcSize);

			if (p) {
				::memcpy(p, buf.data, buf.size);
				::memcpy(p + buf.size, pSrcBuf, dwSrcSize);
				*ppDstBuf = p;
				*pdwDstSize = buf.size + dwSrcSize;
				_data = p;
			} else {
				if (*ppDstBuf != pSrcBuf) {
					*ppDstBuf = pSrcBuf;
					*pdwDstSize = dwSrcSize;
				}
			}

			if (rc == ARIB_STD_B25_ERROR_ECM_PROC_FAILURE) {
				// pass through
				_b25->release(_b25);
				_b25 = NULL;
				_bcas->release(_bcas);
				_bcas = NULL;
			}
		}
		_errtime = ::GetTickCount();
		return FALSE;	// error
	}
	_b25->get(_b25, &buf);
	*ppDstBuf = buf.data;
	*pdwDstSize = buf.size;
	return TRUE;	// success
}

const BOOL CB25Decoder::Flush(BYTE **ppDstBuf, DWORD *pdwDstSize)
{
	BOOL ret = TRUE;
	
	if (_b25) {
		int rc = _b25->flush(_b25);
		ret = (rc < 0) ? FALSE : TRUE;
	}

	*ppDstBuf = NULL;
	*pdwDstSize = 0;

	return ret;
}

const BOOL CB25Decoder::Reset(void)
{
	BOOL ret = TRUE;
	
	if (_b25) {
		int rc = _b25->reset(_b25);
		ret = (rc < 0) ? FALSE : TRUE;
	}

	return ret;
}

void CB25Decoder::DiscardNullPacket(const bool bEnable)
{
	// NULLパケット破棄の有無を設定
	_b25->set_strip(_b25, bEnable);
}

void CB25Decoder::DiscardScramblePacket(const bool bEnable)
{
}

void CB25Decoder::EnableEmmProcess(const bool bEnable)
{
	// EMM処理の有効/無効を設定
	_b25->set_emm_proc(_b25, bEnable);
}

void CB25Decoder::SetMulti2Round(const int32_t round)
{
	// ラウンド段数を設定
	_b25->set_multi2_round(_b25, round);
}

const DWORD CB25Decoder::GetDescramblingState(const WORD wProgramID)
{
	// 指定したプログラムIDの復号状態を返す
	return 0;
}

void CB25Decoder::ResetStatistics(void)
{
}

const DWORD CB25Decoder::GetPacketStride(void)
{
	// パケット周期を返す
	return 0;
}

const DWORD CB25Decoder::GetInputPacketNum(const WORD wPID)
{
	// 入力パケット数を返す　※TS_INVALID_PID指定時は全PIDの合計を返す
	return 0;
}

const DWORD CB25Decoder::GetOutputPacketNum(const WORD wPID)
{
	// 出力パケット数を返す　※TS_INVALID_PID指定時は全PIDの合計を返す
	return 0;
}

const DWORD CB25Decoder::GetSyncErrNum(void)
{
	// 同期エラー数を返す
	return 0;
}

const DWORD CB25Decoder::GetFormatErrNum(void)
{
	// フォーマットエラー数を返す
	return 0;
}

const DWORD CB25Decoder::GetTransportErrNum(void)
{
	// ビットエラー数を返す
	return 0;
}

const DWORD CB25Decoder::GetContinuityErrNum(const WORD wPID)
{
	// ドロップパケット数を返す
	return 0;
}

const DWORD CB25Decoder::GetScramblePacketNum(const WORD wPID)
{
	// 復号漏れパケット数を返す
	return 0;
}

const DWORD CB25Decoder::GetEcmProcessNum(void)
{
	// ECM処理数を返す(B-CASカードアクセス回数)
	return 0;
}

const DWORD CB25Decoder::GetEmmProcessNum(void)
{
	// EMM処理数を返す(B-CASカードアクセス回数)
	return 0;
}
