#include <iostream>

#include "serialport.h"
#include <p8-platform/util/util.h>

#include "RainAdapter.h"

using namespace P8PLATFORM;

RainAdapter::RainAdapter(const char *deviceName) :
        m_gotResponse(false)

{
    CLockObject lock(m_mutex);
    m_port = new CSerialPort(deviceName, CEC_RAINSHADOW_SERIAL_DEFAULT_BAUDRATE);
}

RainAdapter::~RainAdapter(void)
{
    /* stop the reader thread */
    StopThread(0);
}
bool RainAdapter::IsOpen(void)
{
  /* thread is not being stopped, the port is open and the thread is running */
  return !IsStopped() && m_port->IsOpen() && IsRunning();
}

void *RainAdapter::Process(void)
{
    char buf[DATA_SIZE];
    unsigned int idx = 0;
    bool started = false;
    unsigned char data;

    if (!IsOpen())
        return 0;

    while (!IsStopped())
    {
        do
        {
            /* retry Read() if it was interrupted */
            m_port->Read(&data, sizeof(uint8_t), 50);
        }
        while (m_port->GetErrorNumber() == EINTR);

        if (m_port->GetErrorNumber())
        {
            std::cerr << "No permission to open the device" << std::endl;

            break;
        }

        if (!started && data != '?')
            continue;
        if (data == '\r')
        {
            buf[idx] = '\0';

            if (!strncmp(buf, "REC", 3))
                std::cout << buf << std::endl;
            else
            {
                strncpy(m_response, buf, sizeof(m_response));

                m_gotResponse = true;
                m_condition.Signal();
            }
            idx = 0;
            started = false;
            continue;
        }
        else if (data == '?')
        {
            idx = 0;
            started = true;
            continue;
        }

        if (data == '\n')
        {
            idx = 0;
            started = false;
            continue;
        }
        if (idx >= DATA_SIZE - 1)
        {
            idx = 0;
        }
        buf[idx++] = data;
        continue;
    }
    return 0;
}
