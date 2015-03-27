#ifndef __NX_HANDLE_CAST_H__
#define __NX_HANDLE_CAST_H__

namespace nx {

template <typename Class, typename Watcher>
inline
Class&
watcher_cast(Watcher* w)
{ return *static_cast<Class*>(w->data); }

} // namespace nx

#endif // __NX_HANDLE_CAST_H__
