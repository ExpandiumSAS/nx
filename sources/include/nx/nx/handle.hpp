#ifndef __NX_HANDLE_H__
#define __NX_HANDLE_H__

#include <nx/config.h>

namespace nx {

class NX_API handle
{
public:
    handle(int fh);
    virtual ~handle();

private:
    int fh_;
};

} // namespace nx

#endif // __NX_HANDLE_H__
