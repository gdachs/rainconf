#ifndef RAINADAPTER_H_
#define RAINADAPTER_H_
#include <p8-platform/threads/mutex.h>
#include <p8-platform/threads/threads.h>
#include <p8-platform/sockets/socket.h>

class RainAdapter: public P8PLATFORM::CThread
{
public:
    RainAdapter(const char *deviceName);

    virtual ~RainAdapter(void);

    /** @name P8PLATFORM::CThread implementation */
    ///{
    void *Process(void);
    ///}
private:
    P8PLATFORM::ISocket * m_port; /**< the com port connection */
    P8PLATFORM::CMutex m_mutex;

};

#endif //RAINADAPTER_H_
