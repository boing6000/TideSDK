/**
 * Appcelerator Titanium - licensed under the Apache Public License 2
 * see LICENSE in the root folder for details on the license.
 * Copyright (c) 2009 Appcelerator, Inc. All Rights Reserved.
 */

#ifndef _SCRIPT_EVALUATOR_H_
#define _SCRIPT_EVALUATOR_H_

#include <kroll/kroll.h>
#if defined(OS_OSX)
#	import <WebKit/WebKit.h>
#	import <WebKit/WebViewPrivate.h>
#	import <WebKit/WebInspector.h>
#	import <WebKit/WebScriptDebugDelegate.h>
#	import <WebKit/WebScriptObject.h>
#	import <WebKit/WebPreferencesPrivate.h>
#else
#	include <webkit/webkit.h>
#	if defined(OS_WIN32)
#		include <WebKit/WebKitCOMAPI.h>
#		include <WebKit/WebKitTitanium.h>
#		include <comutil.h>
#	endif
#endif

/**
 * Script evaluators are responsible for matching and evaluating custom <script> types.
 * In Titanium, we have 1 instance that checks against the binding API using these rules:
 * - Most language mimeTypes will be in the form "text/language". We strip "text/" from the string.
 * - We look for a defined object under the global context that matches "language", "Language", or "LANGUAGE"
 * - If an object is found, a bound method named "evaluate" must exist on that object.
 * If a binding for the language is succesfully found, we call the evaluate method with these arguments:
 * - evaluate(string mimeType, string sourceCode, Object windowContext) 
 */
namespace ti
{
	class ScriptEvaluator
#if defined(OS_WIN32)
		: public IWebScriptEvaluator, public COMReferenceCounted
#elif defined(OS_LINUX)
		: public WebKitWebScriptEvaluator, public ReferenceCounted
#else
		: public ReferenceCounted
#endif
	{
	public:
#if defined(OS_WIN32)
		virtual HRESULT STDMETHODCALLTYPE QueryInterface(REFIID riid, void** ppvObject) { return COMReferenceCounted::QueryInterface(riid,ppvObject); }
		virtual ULONG STDMETHODCALLTYPE AddRef(void) { return COMReferenceCounted::AddRef(); }
		virtual ULONG STDMETHODCALLTYPE Release(void) { return COMReferenceCounted::Release(); }
#endif
		static void Initialize();
		SharedValue FindScriptModule(std::string type);
		bool MatchesMimeType(std::string mimeType);
		SharedValue Evaluate(std::string mimeType, std::string sourceCode, JSContextRef context);
		
#if defined(OS_WIN32)
		// IWebScriptEvaluator
		virtual HRESULT STDMETHODCALLTYPE matchesMimeType(BSTR mimeType, BOOL *result)
		{
			*result = this->MatchesMimeType((const char *)_bstr_t(mimeType));
			return S_OK;
		}
		virtual HRESULT STDMETHODCALLTYPE evaluate(BSTR mimeType, BSTR sourceCode, int* context)
		{
			this->Evaluate((const char *)_bstr_t(mimeType), (const char *)_bstr_t(sourceCode),
				reinterpret_cast<JSContextRef>(context));
			return S_OK;
		}
#elif defined(OS_LINUX)
		// WebKitWebScriptEvaluator
		virtual bool matchesMimeType(const gchar *mime_type) { return this->MatchesMimeType(mime_type); }
		virtual void evaluate(const gchar *mime_type, const gchar *source_code, void* context) { this->Evaluate(mime_type, source_code, context); }
#endif
	protected:
		static AutoPtr<ScriptEvaluator> instance;
		ScriptEvaluator() {}
	};
}

#if defined(OS_OSX)
@interface OSXScriptEvaluator : NSObject
{
	kroll::ScriptEvaluator* delegate;
}
-(OSXScriptEvaluator*) initWithEvaluator:(kroll::ScriptEvaluator*)evaluator;
-(BOOL) matchesMimeType:(NSString*)mimeType;
-(void) evaluate:(NSString *)mimeType sourceCode:(NSString*)sourceCode context:(void *)context;
@end
#elif defined(OS_LINUX)
	extern void addScriptEvaluator(WebKitWebScriptEvaluator *evaluator);
#endif

#endif