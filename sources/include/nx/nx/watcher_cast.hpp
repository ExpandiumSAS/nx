#ifndef __NX_HANDLE_CAST_H__
#define __NX_HANDLE_CAST_H__

namespace nx {

template <typename Class, typename Watcher>
inline
Class&
watcher_cast(Watcher* h)
{ return *static_cast<Class*>(h); }

} // namespace nx

#endif // __NX_HANDLE_CAST_H__
