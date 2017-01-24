#ifndef RAINADAPTER_H_
#define RAINADAPTER_H_
#include <p8-platform/threads/mutex.h>
#include <p8-platform/threads/threads.h>
#include <p8-platform/sockets/socket.h>

#define DATA_SIZE 256
#define CEC_RAINSHADOW_SERIAL_DEFAULT_BAUDRATE 115200L

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
    bool IsOpen(void);

    P8PLATFORM::ISocket *m_port; /**< the com port connection */
    P8PLATFORM::CMutex m_mutex;
    char                          m_response[DATA_SIZE]; /**< current response from adapter */
    P8PLATFORM::CCondition<bool>  m_condition;
    bool                          m_gotResponse;
};

#endif //RAINADAPTER_H_
