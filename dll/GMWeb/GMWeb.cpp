// GMWeb
// author: LiamMitchell
// license: MIT https://opensource.org/licenses/mit-license.php
//
// This is where pretty much everything happens.
// My sincere apologies if you are using a source code editor that
// does not support collapsing "#pragma region" blocks.
// GmxGen which we use to automate building the
// GMS Extension definitions and GML scripts
// does not yet support multiple files.
//
// Note:
// Basing this on YellowAfterlife's Steamworks.gml because it works with GMS,
// previous attempts did not have the GM ds map methods working.

#include <windows.h>
//#include <windowsx.h>

// GCC glue:
#ifdef __GNUC__
#include <stdlib.h>
// I know, and great, but that's what GMS runtime uses
#pragma GCC diagnostic ignored "-Wwrite-strings"
#define nullptr NULL
#endif


/// Different platforms, different syntax.
#if defined(WIN32)
#define dllx extern "C" __declspec(dllexport)
#elif defined(GNUC)
#define dllx extern "C" __attribute__ ((visibility("default"))) 
#else
#define dllx extern "C"
#endif

/// For debugging purposes.
#define trace(...) { printf(__VA_ARGS__); printf("\n"); fflush(stdout); }

// Shortcuts for uint32<->uint64 conversions
#ifndef uint32
#define uint32 unsigned int
#endif
#ifndef uint64
#define uint64 long unsigned int
#endif
#ifndef UINT32_MAX
#define UINT32_MAX 4294967295u
#endif
#define uint64_make(high, low) (((uint64)(high) << 32) | (uint64)(low))
#define uint64_high(value) (uint32)((value) >> 32)
#define uint64_low(value) (uint32)((value) & UINT32_MAX)

/// GameMaker has an unusual way of detecting if a value is "true".
#define gml_bool(value) ((value) >= 0.5)
#include <iostream>
#pragma region GML callbacks
// As per http://help.yoyogames.com/hc/en-us/articles/216755258:
typedef int gml_ds_map;
void(*gml_event_perform_async)(gml_ds_map map, int event_type);
int(*gml_ds_map_create_ext)(int n, ...);
bool(*gml_ds_map_set_double)(gml_ds_map map, char* key, double value);
bool(*gml_ds_map_set_string)(gml_ds_map map, char* key, char* value);
dllx double RegisterCallbacks(char* arg1, char* arg2, char* arg3, char* arg4) {
	gml_event_perform_async = (void(*)(gml_ds_map, int))arg1;
	gml_ds_map_create_ext = (int(*)(int, ...))arg2;
	gml_ds_map_set_double = (bool(*)(gml_ds_map, char*, double))arg3;
	gml_ds_map_set_string = (bool(*)(gml_ds_map, char*, char*))arg4;

	return 0;
}
gml_ds_map gml_ds_map_create() {
	return gml_ds_map_create_ext(0);
}
#pragma endregion

/* Include things we actually need for the extension to work. */
#include <d3d9.h>
#include <iostream>
#include <fstream>
#include <d3dx9.h>
#include <string>
#include <stdio.h>
#include <d3dcompiler.h>
#include <vector>
#include <sstream>
#include <map>
#include <limits>
#include <sys/types.h>
#include <sys/stat.h>

#include <Awesomium/WebCore.h>
#include <Awesomium/STLHelpers.h>
#include <Awesomium/BitmapSurface.h>
#include <Awesomium/WebViewListener.h>

#include "method_dispatcher.h"

/* Use name spaces and define things we will use. */
using namespace std;
using namespace Awesomium;

#pragma region Macros & global variables
WebCore* web_core = 0;
HANDLE hAwesomiumThread = NULL;
HWND hwnd = NULL;
bool freed = false;

ofstream *printf_file = 0;
LPDIRECT3D9 d3d;
LPDIRECT3DDEVICE9 d3ddev = 0;
boolean printf_enabled;
vector<IDirect3DTexture9*> surface_textures;
vector<UINT> surface_width;
vector<UINT> surface_height;
vector<IDirect3DSurface9*> surfaces_previous;
vector<UINT> surfaces_previous_index;
vector<UINT> textures_previous_index;
vector<IDirect3DVertexShader9*> vertex_shaders;
vector<IDirect3DPixelShader9*> pixel_shaders;
IDirect3DBaseTexture9* base_texture_previous;

vector<WebView*> webviews;
MethodDispatcher method_dispatcher;

// A javascript value that is only used by the sync functions. Only one can exist at a time.
JSValue& sync_value = JSValue();

/// Event Numbers
enum eOtherEventNumbers{
	EVENT_ASYNC_HTTP=62
};

const double GM_HTTP_STATUS_FAIL = -1;
const double GM_HTTP_STATUS_SUCCESS = 0;
const double GM_HTTP_STATUS_DOWNLOADING = 1;

#pragma endregion

#pragma region GML Helper Methods

// Note: using DBL_MIN because game maker http_get and http_post will be using positive values.
/// This value will be incremented each time a JS Async Exec is called.
// Will wrap around when it reaches 0.
double next_async_http_id = DBL_MIN;
void gml_event_perform_async_http(double id, double status = GM_HTTP_STATUS_SUCCESS, char* result = "", double web_view=-1, char* url = "gmweb", double http_status = 200, double contentLength = -1, double sizeDownloaded=-1) {
	gml_ds_map ds = gml_ds_map_create();         // Note: See the HTTP_EVENT help for info on these keys.
	gml_ds_map_set_double(ds, "id", id);         // http_request id
	gml_ds_map_set_double(ds, "status", status); // <0 error, 0 Success, 1 if content is being downloaded.
	gml_ds_map_set_string(ds, "result", result); // Data Received or path to downloaded file.
	gml_ds_map_set_string(ds, "url", url);       // The complete URL you requested.
	// Note: Set to "gmweb" for async js execution response.

	gml_ds_map_set_double(ds, "view", web_view); // The web view the response came from.

	// Seemingly optional.
	gml_ds_map_set_double(ds, "contentLength", contentLength);
	if (sizeDownloaded >= 0) {
		gml_ds_map_set_double(ds, "sizeDownloaded", sizeDownloaded);
	}

	gml_event_perform_async(ds, EVENT_ASYNC_HTTP);
}

#pragma endregion


#pragma region WebView Handlers and Listeners

// Bound to gmweb.test() in JavaScript
void bound_gmweb_test(WebView* caller,
					  const JSArray& args) {
	  MessageBoxA(0, "BoundMethodCalled", "Wooo", MB_OK);
}

// Bound to gmweb.event_perform_async_http(id, status, result) in JavaScript
void bound_gmweb_event_perform_async_http(WebView* caller,
					  const JSArray& args) {

	  double web_view_index = std::numeric_limits<double>::quiet_NaN();
	  
	  unsigned int i;
	  for (i = 0; i < webviews.size(); ++i) {
		  if (webviews[i] == caller) {
			  web_view_index = i;
			  break;
		  }
	  }

	  unsigned int args_length = args.size();
	  
	  double id = 0;
	  if (args_length > 0) {
		  id = args.At(0).ToDouble();
	  }

	  double status = GM_HTTP_STATUS_SUCCESS;
	  if (args_length > 1) {
		  status = args.At(1).ToDouble();
	  }

	  char* result = "";
	  if (args_length > 2) {
		  result = (char*)ToString(args.At(2).ToString()).c_str();
	  }

	  gml_event_perform_async_http(id, status, result, web_view_index);
}

// Bound to gmweb.send(from, message) in JavaScript
void bound_gmweb_send(WebView* caller,
	  const JSArray& args) {

		  double web_view_index = std::numeric_limits<double>::quiet_NaN();

		  unsigned int i;
		  for (i = 0; i < webviews.size(); ++i) {
			  if (webviews[i] == caller) {
				  web_view_index = i;
				  break;
			  }
		  }

		  unsigned int args_length = args.size();

		  gml_ds_map ds = gml_ds_map_create();

		  if (args_length >= 2) {
			gml_ds_map_set_string(ds, "from", (char*)ToString(args.At(0).ToString()).c_str());
			gml_ds_map_set_string(ds, "message", (char*)ToString(args.At(1).ToString()).c_str());
		  }
		  gml_event_perform_async(ds, EVENT_ASYNC_HTTP);
}

void BindMethods(WebView* web_view) {
	// Create a global js object named 'gmweb'
	JSValue result = web_view->CreateGlobalJavascriptObject(WSLit("gmweb"));
	if (result.IsObject()) {
		// Bind our custom method to it.
		JSObject& app_object = result.ToObject();
		method_dispatcher.Bind(app_object, WSLit("test"), JSDelegate(&bound_gmweb_test));
		method_dispatcher.Bind(app_object, WSLit("event_perform_async_http"), JSDelegate(&bound_gmweb_event_perform_async_http));
		method_dispatcher.Bind(app_object, WSLit("send"), JSDelegate(&bound_gmweb_send));
	}

	// Bind our method dispatcher to the WebView
	web_view->set_js_method_handler(&method_dispatcher);
}

#pragma endregion

#pragma region DirectX methods

int create_shader(LPCSTR vertex_shader_src, LPCSTR pixel_shader_src, IDirect3DVertexShader9 **out_vertex_shader, IDirect3DPixelShader9 **out_pixel_shader) {
    LPD3DXBUFFER vertex_shader_code, pixel_shader_code;
    LPD3DXBUFFER error_blob;
	LPD3DXCONSTANTTABLE constant_table;
	HRESULT hr;

	// Attempts compiling the vertex shader.
	hr = D3DXCompileShader(vertex_shader_src, strlen(vertex_shader_src), 0, 0, "main", "vs_3_0", D3DCOMPILE_ENABLE_BACKWARDS_COMPATIBILITY, &vertex_shader_code, &error_blob, &constant_table);
	if (printf_enabled) {
		if (FAILED(hr)) {
			printf("Failed compiling the vertex shader. %08X\n", hr);
			if (error_blob) {printf((char*) error_blob->GetBufferPointer()); error_blob->Release();}
			if (vertex_shader_code) vertex_shader_code->Release();
		}
		printf("Vertex shader compiled.\n");
    }
	if (FAILED(hr)) return -1;

	// Attempts creating the vertex shader.
	hr = d3ddev->CreateVertexShader((DWORD*) vertex_shader_code->GetBufferPointer(), out_vertex_shader);
	if (printf_enabled) {
		if (FAILED(hr)) printf("Failed creating the vertex shader. %08X\n", hr);
		printf("Vertex shader created.\n");
	}
	if (FAILED(hr)) return -1;

	// Attempts compiling the pixel shader.
	hr = D3DXCompileShader(pixel_shader_src, strlen(pixel_shader_src), 0, 0, "main", "ps_3_0", D3DCOMPILE_ENABLE_BACKWARDS_COMPATIBILITY, &pixel_shader_code, &error_blob, &constant_table);
    if (printf_enabled) {
		if (FAILED(hr)) {
			printf("Failed compiling the pixel shader. %08X\n", hr);
			if (error_blob) {printf((char*) error_blob->GetBufferPointer()); error_blob->Release();}
			if (pixel_shader_code) pixel_shader_code->Release();
		}	
		printf("Pixel shader compiled.\n");
	}
	if (FAILED(hr)) return -1;

	// Attempts creating the pixel shader.
	hr = d3ddev->CreatePixelShader((DWORD*) pixel_shader_code->GetBufferPointer(), out_pixel_shader);
	if (printf_enabled) {
		if (FAILED(hr)) printf("Failed creating the pixel shader. %08X\n", hr);
		printf("Pixel shader created.\n");
	}
	if (FAILED(hr)) return -1;

	return 1;
}

/// __gmweb_start(window_device) : Acquires the window device handle for future usage.
dllx double __gmweb_start(double window_device) {
	UINT* ptr = (UINT*)&window_device;
    // Sets the Direct3D device.
	d3ddev = (LPDIRECT3DDEVICE9)*(UINT*)&window_device;
	
	return 1;
}

// __gmweb_set_window_handle(hwnd)
dllx double __gmweb_set_window_handle(double _hwnd) {
	hwnd = (HWND)(UINT)_hwnd;
	MessageBoxA(hwnd, "Handle set", "OK", MB_OK);
	return 1;
}

//
dllx double gmweb_shader_create(char* vertex_shader_code, char* pixel_shader_code) {
	IDirect3DVertexShader9 *t; IDirect3DPixelShader9 *t_1;
	int error = create_shader(vertex_shader_code, pixel_shader_code, &t, &t_1);
	if (error < 0) return -1;
	vertex_shaders.push_back(t); pixel_shaders.push_back(t_1);
	return vertex_shaders.size() - 1;
}

//
dllx double __gmweb_surface_create(double width, double height, double format) {
	IDirect3DTexture9 *t;
	d3ddev->CreateTexture((UINT) width, (UINT) height, 1, D3DUSAGE_DYNAMIC, (D3DFORMAT) (UINT) format, D3DPOOL_DEFAULT, &t, NULL);

	surface_textures.push_back(t);
	surface_width.push_back((UINT) width);
	surface_height.push_back((UINT) height);
	return surface_textures.size() - 1;
}

//
dllx double gmweb_shader_is_set() {
	IDirect3DPixelShader9* pixel_shader;
	IDirect3DVertexShader9* vertex_shader;
	d3ddev->GetPixelShader(&pixel_shader);
	if (pixel_shader != NULL) return 1;
	d3ddev->GetVertexShader(&vertex_shader);
	if (vertex_shader != NULL) return 1;
	return 0;
}

//
dllx double __gmweb_surface_set_target(double index, double surface_index) {
	for (UINT i = 0; i < textures_previous_index.size(); ++i) {
		d3ddev->SetTexture(textures_previous_index[i], NULL);
	}
	textures_previous_index.clear();
	
	IDirect3DSurface9 *surface_previous;
	d3ddev->GetRenderTarget((UINT) index, &surface_previous);
	surfaces_previous.push_back(surface_previous);
	surfaces_previous_index.push_back((UINT) index);
	
	IDirect3DSurface9 *t;
	surface_textures[(UINT) surface_index]->GetSurfaceLevel((DWORD) index, &t);
	d3ddev->SetRenderTarget((DWORD) index, t);

	return 1;
}

//
dllx double __gmweb_surface_reset_target() {
	for (UINT i = 0; i < surfaces_previous.size(); ++i) {
		d3ddev->SetRenderTarget(surfaces_previous_index[i], surfaces_previous[i]);
	}
	surfaces_previous.clear();
	surfaces_previous_index.clear();
	return 1;
}

//
dllx double gmweb_surface_set_texture(double index, double surface_index) {
	d3ddev->SetTexture((DWORD) index, surface_textures[(UINT) surface_index]);
	textures_previous_index.push_back((UINT) index);
	return 1;
}

//
dllx double gmweb_shader_set(double shader_index) {
	HRESULT hr;

	// Attempts setting the vertex shader.
	hr = d3ddev->SetVertexShader(vertex_shaders[(int) shader_index]);
	if (printf_enabled && FAILED(hr)) printf("Failed setting the vertex shader. %08X\n", hr);
	if (FAILED(hr)) return -1;

	// Attempts setting the pixel shader.
	hr = d3ddev->SetPixelShader(pixel_shaders[(int) shader_index]);
	if (printf_enabled && FAILED(hr)) printf("Failed setting the pixel shader. %08X\n", hr);
	if (FAILED(hr)) return -1;
	
	return 1;
}

//
dllx double gmweb_surface_draw(double surface_index) {
	//d3ddev->StretchRect(surface[0], NULL, destination, NULL, D3DTEXF_LINEAR);
	return 1;
}

#pragma endregion

/// gmweb_free() : Frees up resources and pointers acquired.
dllx double gmweb_free() {
	if (freed) {
		return 1;
	}

	unsigned int i;
	for (i = 0; i < vertex_shaders.size(); ++i) vertex_shaders[i]->Release();
	for (i = 0; i < pixel_shaders.size(); ++i) pixel_shaders[i]->Release();
	for (i = 0; i < surface_textures.size(); ++i) surface_textures[i]->Release();
	if (printf_enabled) {printf_file->close(); delete printf_file;}
	if (web_core) {
		web_core->Shutdown();
	}
	freed = true;
	return 1;
}


#pragma region Awesomium Helper Methods

// Awesomium Things


void UpdateApplication() {
	web_core->Update();
}

/////////////////////////////////////////////////////////////////////////////////////////
// Game Maker Mouse Codes to Awesomium Mouse Button Codes.
// Awesomium Codes
// 0 kMouseButton_Left
// 1 kMouseButton_Middle
// 2 kMouseButton_Right
// 
// Game Maker Codes
// ? mb_none No mouse button
// ? mb_any Any of the mouse buttons 
// 1  mb_left The left mouse button 
// 2  mb_middle The middle mouse button (this may not be valid for all target platforms) 
// 3  mb_right The right mouse button 
MouseButton gmMouseButtonToAwesomium(double button) {
	return (MouseButton)(UINT)(button-1);
}

#pragma endregion


#pragma region GMWeb Awesomium Exports

// gmweb_awesomium_start(log_path = "", log_level = 0, debug_port = 0)
dllx double gmweb_awesomium_start(char * log_path = "", double log_level = 0, double debug_port = 0) {
	if (d3ddev == NULL) {
		if (printf_enabled) printf("Failed to start Awesomium d3ddev is NULL please run gmweb_init script from GM first.\n");
		return 0;
	}

	if (web_core != NULL) {
		if (printf_enabled) printf("Awesomium already started.\n");
		return 0;
	}

	WebConfig config;

	config.log_level = kLogLevel_Verbose;//kLogLevel_Normal;
	if ((string) log_path != "") {
		config.log_path = WSLit(log_path);
		config.log_level = (LogLevel)(UINT)log_level;//kLogLevel_Normal;
	}
	
	config.remote_debugging_port = (int)debug_port;

	//Specify additional command line options to pass to the web core
	WebStringArray configOptions = WebStringArray();

	 configOptions.Push(WSLit("--use-gl=desktop"));
	 configOptions.Push(WSLit("--enable-gamepad"));
	 configOptions.Push(WSLit("--enable-media-stream"));
	 configOptions.Push(WSLit("--enable-media-source"));
	 configOptions.Push(WSLit("--enable-video-fullscreen"));
	 configOptions.Push(WSLit("--enable-threaded-compositing"));
	 configOptions.Push(WSLit("--enable-video-track"));

	config.additional_options = configOptions;
	web_core = WebCore::Initialize(config);
	
	return 1;
}

//
dllx double gmweb_awesomium_update() {
	web_core->Update();
	return 1;
}

//
dllx double gmweb_awesomium_create_webview(double width, double height) {
	if (web_core == NULL) {
		if (printf_enabled) printf("Failed to create Awesomium WebView the web_core is NULL please run gmweb_awesomium_start first.\n");
		return 0;
	}
	
	//////////////////////////////////////////////////////////
	//0 kWebViewType_Offscreen
	// This type will render continuously to a buffer
	// (Surface). You must display the Surface and pass
	// all input (mouse/keyboard events) yourself.
	//
	//1 kWebViewType_Window
	// This type will create a native Window to display
	// the WebView and will capture all input itself.
	//////////////////////////////////////////////////////////

	//Set a bunch of prefs, ensuring that WebGL will be enabled and Awesomium won't be handicapped by default
	WebPreferences sessionPrefs;
	sessionPrefs.enable_gpu_acceleration				= true;
	sessionPrefs.enable_web_gl							= true;
	sessionPrefs.enable_javascript						= true;
	sessionPrefs.allow_file_access_from_file_url		= true;
	sessionPrefs.allow_running_insecure_content			= true;
	sessionPrefs.allow_universal_access_from_file_url	= true;
	sessionPrefs.enable_web_security					= false;
	sessionPrefs.enable_smooth_scrolling				= true;
	sessionPrefs.allow_scripts_to_access_clipboard		= true;
	sessionPrefs.allow_scripts_to_close_windows			= false;
	sessionPrefs.enable_local_storage					= true;
	sessionPrefs.enable_plugins							= true;
	sessionPrefs.enable_databases						= true;
	sessionPrefs.enable_web_audio						= true;

	//Create the web session and add preferences to it
	WebString sessionStr = WebString::CreateFromUTF8("", strlen(""));
	WebSession* session = web_core->CreateWebSession(sessionStr, sessionPrefs);

	//Create an Offscreen WebView, and pass it our session
	WebView* view = web_core->CreateWebView((int)width, (int)height, session); // , 0, kWebViewType_Offscreen
	view->SetTransparent(true);

	BindMethods(view);

	webviews.push_back(view);

	return webviews.size() - 1;
}

///gmweb_file_exists(file_path) : Check if a file exists. Unlike the GameMaker version, this is not restricted to a sandbox!
dllx double gmweb_file_exists(char* file_path) {
	if (ifstream(file_path)){
		return 1;
	}
	return 0;
}

///gmweb_folder_exists(folder_path) : Check if a folder exists. Unlike the GameMaker version, this is not restricted to a sandbox!
dllx double gmweb_folder_exists(const char *folder_path) {
    struct stat info;

	if(stat( folder_path, &info ) != 0){
		return 0;
	}
	else if(info.st_mode & S_IFDIR){
		return 1;
	}
	else{
		return 0;
	}
}

//
dllx double gmweb_awesomium_load_url(double webview_index, char* _url) {
	WebView* view = webviews[(UINT) webview_index];
	
	WebURL url(WSLit(_url));
	if (!url.IsValid()) {
		if (printf_enabled) printf("URL '%s' is invalid.\n", _url);
		return 0;
	}
	
	view->LoadURL(url);
	
	return 1;
}



//
dllx double gmweb_awesomium_destroy(double webview_index) {
	WebView* view = webviews[(UINT) webview_index];
	view->Destroy();
	return 1;
}

//
dllx double gmweb_awesomium_mouse_move(double webview_index, double x, double y) {
	// Inject a mouse-movement event to screen-space (0,0 is top-left of screen).
	webviews[(UINT) webview_index]->InjectMouseMove((int)x, (int)y);
	return 1;
}

//
dllx double gmweb_awesomium_mouse_down(double webview_index, double button) {
	// Inject a mouse button down event.
	WebView* view = webviews[(UINT) webview_index];
	view->Focus(); // Focus the view just in case.
	view->InjectMouseDown(gmMouseButtonToAwesomium(button));	
	return 1;
}

//
dllx double gmweb_awesomium_mouse_up(double webview_index, double button) {
	// Inject a mouse button up event.
	webviews[(UINT) webview_index]->InjectMouseUp(gmMouseButtonToAwesomium(button));
	return 1;
}

//
dllx double gmweb_awesomium_mouse_wheel(double webview_index, double pxVert, double pxHorz) {
	// Inject a mouse button up event.
	webviews[(UINT) webview_index]->InjectMouseWheel((int)pxVert, (int)pxHorz);
	return 1;
}

void prepareWebKeyboardEvent(WebKeyboardEvent* e, WebKeyboardEvent::Type type, int vkCode, char* typedChar = NULL) {
	e->virtual_key_code = vkCode;
	e->type = type;

	// Copy over the typedChar if set.
	if (typedChar == NULL) {
		e->text[0] = 0x00;
	} else {
		e->text[0] = typedChar[0];
		// Note: They are nice and give is up to four characters which we could use.
		// Might need to for Asian / Other support.
	}

	char* buf = e->key_identifier;
	GetKeyIdentifierFromVirtualKeyCode(e->virtual_key_code, &buf);
};

//
dllx double gmweb_awesomium_key_press(double webview_index, double vkCode, char* typedChar = NULL) {
	// Inject a mouse button up event.
	WebView* view = webviews[(UINT) webview_index];
	WebKeyboardEvent e;

	prepareWebKeyboardEvent(&e, WebKeyboardEvent::Type::kTypeChar, (int)vkCode, typedChar);
	view->InjectKeyboardEvent(e);
	return 1;
}

//
dllx double gmweb_awesomium_key_down(double webview_index, double vkCode, char* typedChar = NULL) {
	// Inject a mouse button up event.
	WebView* view = webviews[(UINT) webview_index];
	WebKeyboardEvent e;
	
	prepareWebKeyboardEvent(&e, WebKeyboardEvent::Type::kTypeKeyDown, (int)vkCode, typedChar);

	view->Focus();
	view->InjectKeyboardEvent(e);

	// Call this to actually have text characters typed.
	gmweb_awesomium_key_press(webview_index, vkCode, typedChar);
	return 1;
}

//
dllx double gmweb_awesomium_key_up(double webview_index, double vkCode, char* typedChar = NULL) {
	// Inject a mouse button up event.
	WebView* view = webviews[(UINT) webview_index];
	WebKeyboardEvent e;
	prepareWebKeyboardEvent(&e, WebKeyboardEvent::Type::kTypeKeyUp, (int)vkCode, typedChar);

	view->Focus();
	view->InjectKeyboardEvent(e);
	return 1;
}



// gmweb_awesomium_render(webview_index, surface_index)
dllx double gmweb_awesomium_render(double webview_index, double surface_index) {
	WebView* view = webviews[(UINT) webview_index];
	IDirect3DTexture9* d3dtex;
	
	d3dtex = surface_textures[(UINT) surface_index];
	
	BitmapSurface* surface = static_cast<BitmapSurface *>(view->surface());

	// Make sure our surface is not NULL, eg NULL if WebView process has crashed.
	if (surface != NULL) {
		if (!surface->is_dirty()) {
			return 0;
		}

		IDirect3DSurface9* d3dsurf;
		d3dtex->GetSurfaceLevel(0,&d3dsurf);

		D3DLOCKED_RECT lockedRect;
		d3dsurf->LockRect(&lockedRect, NULL, 0);

		surface->CopyTo((unsigned char *)lockedRect.pBits, surface->row_span(), 4, false, false);
		d3dsurf->UnlockRect();

		return 0;
	} else {
		if (view->IsCrashed()) {
			//trace("Web View has crashed.");
		}
		return 1;
	}
	return 1;
}

//
dllx double gmweb_create_object(double webview_index) {
	// Inject a mouse button up event.
	WebView* view = webviews[(UINT) webview_index];
	WebKeyboardEvent e;
	
	JSObject obj;
	return 1;
}


// gmweb_execute_js_sync(webview_index, script, ?frame_xpath = "")
dllx double gmweb_execute_js_sync(double webview_index, char* script, char* frame_xpath = "") {
	// Call JS string sync returning sync value if it is a number.
	WebView* web_view = webviews[(UINT) webview_index];
	sync_value = web_view->ExecuteJavascriptWithResult(WSLit(script), WSLit(frame_xpath));
	
	// Just as a nicer helper, if the result is a single number (Integer or Double) then return it.
	return sync_value.ToDouble();
}

// gmweb_execute_js(webview_index, script, ?frame_xpath = "")
dllx double gmweb_execute_js(double webview_index, char* script, char* frame_xpath = "") {
	double async_http_id = next_async_http_id;
	// Increment the next_async_http_id. (Handle wrap at 0 back to DBL_MIN).
	if (next_async_http_id++ == 0) {
		next_async_http_id = DBL_MIN;
	}

	WebView* view = webviews[(UINT) webview_index];
	//JSValue value = 
		view->ExecuteJavascript(WSLit(script), WSLit(frame_xpath));
	/// @return  Returns the result (if any). Any JSObject returned from this
	///          method will be a remote proxy for an object contained within
	///          the WebView process. If this call fails, JSValue will have an
	///          Undefined type. You can check WebView::last_error() for more
	///          information about the failure.
	///
	/// @note  You should never call this from within any of the following
	///        callbacks:
	///
	///        - JSMethodHandler::OnMethodCall
	///        - JSMethodHandler::OnMethodCallWithReturnValue
	///        - DataSource::OnRequest
	///

	return async_http_id;
}


//
dllx char* gmweb_get_sync_string() {	
	return (char*)ToString(sync_value.ToString()).c_str();
}

#pragma endregion


