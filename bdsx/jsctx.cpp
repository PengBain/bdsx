#include "jsctx.h"
#include "native.h"
#include "require.h"
#include <KR3/wl/windows.h>
#include <KR3/msg/pump.h>

using namespace kr;

Manual<JsContext> g_ctx;

namespace
{
	JsPersistent s_callbacks[0x100];

	bool s_ctxCreated = false;
	DWORD contextThreadId;
}

void createJsContext(kr::JsRawContext newContext) noexcept
{
	if (s_ctxCreated) g_ctx.remove();
	g_ctx.create(newContext);
	s_ctxCreated = true;
	contextThreadId = GetCurrentThreadId();

	g_ctx->enter();
	try
	{
		kr::JsRuntime::run(u"Promise.resolve('test').then(()=>{})");
	}
	catch (JsException&)
	{
		int a = 0;
	}

	g_native.create();

	g_ctx->exit();
}
void destroyJsContext() noexcept
{
	if (!s_ctxCreated) return;
	s_ctxCreated = false;
	JsContext::_cleanForce();
	g_ctx->enter();
	try
	{
		EventPump::getInstance()->waitAll();
	}
	catch (QuitException&)
	{
	}
	g_native.remove();
	g_ctx->exit();
	g_ctx.remove();
}
bool isContextExisted() noexcept
{
	return s_ctxCreated;
}
void checkCurrentThread() noexcept
{
	_assert(isContextThread());
}
bool isContextThread() noexcept
{
	return GetCurrentThreadId() == contextThreadId;
}
uint32_t getContextThreadId() noexcept
{
	return contextThreadId;
}

