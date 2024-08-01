#pragma once
//graphics exception macros
#define GFX_EXCEPT_NOINFO(hrcall) Graphics::HrException(__LINE__,__FILE__,(hrcall))
#define GFX_THROW_NOINFO(hrcall) if( FAILED(hr = (hrcall)) ) throw GFX_EXCEPT_NOINFO(hr)

//graphics exception macros with dxgi info manager
#if !defined NDEBUG
#define GFX_EXCEPT(hrcall) Graphics::HrException(__LINE__,__FILE__,(hrcall),infoManager.GetMessages())
#define GFX_THROW_INFO(hrcall) infoManager.Set(); if( FAILED(hr = (hrcall)) ) throw GFX_EXCEPT(hr)
#define GFX_THROW_ONLY_INFO(call) infoManager.Set(); (call); auto msgs = infoManager.GetMessages(); if(!msgs.empty()) {throw Graphics::OnlyInfoException(__LINE__,__FILE__,msgs);}
#define GFX_DEVICE_REMOVED_EXCEPT(hrcall) Graphics::DeviceRemovedException(__LINE__,__FILE__,(hrcall),infoManager.GetMessages())
#else // release mode exception macros
#define GFX_EXCEPT(hrcall) Graphics::HrException(__LINE__,__FILE__,(hrcall))
#define GFX_THROW_INFO(hrcall) GFX_THROW_NOINFO(hrcall)
#define GFX_THROW_ONLY_INFO(call) (call)
#define GFX_DEVICE_REMOVED_EXCEPT(hrcall) Graphics::DeviceRemovedException(__LINE__,__FILE__,(hrcall))
#endif