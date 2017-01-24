#include "serialport.h"
#include <p8-platform/util/util.h>

#include "RainAdapter.h"

using namespace P8PLATFORM;

RainAdapter::RainAdapter(const char *deviceName)
{
    CLockObject lock(m_mutex);
    m_port = new CSerialPort(deviceName, 115200);
}

RainAdapter::~RainAdapter(void)
{
  /* stop the reader thread */
  StopThread(0);
}

void *RainAdapter::Process(void)
{
  return 0;
}
