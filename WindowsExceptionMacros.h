#pragma once
#define GGE_EXCEPT( hr ) Window::HrException( __LINE__,__FILE__,(hr) )
#define GGE_LAST_EXCEPT() Window::HrException( __LINE__,__FILE__,GetLastError() )
#define GGE_NOGFX_EXCEPT() Window::NoGfxException( __LINE__,__FILE__ )