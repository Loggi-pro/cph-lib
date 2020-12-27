#pragma once
enum class TAsyncState {
	NO_RESULT = 0,
	E_WAIT,
	E_ERR,
	E_TIMEOUT,
	E_USER,
	E_SOFTWARE,
	S_DONE
};