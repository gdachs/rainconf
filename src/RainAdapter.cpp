#include <iostream>
#include <p8-platform/util/util.h>

#include "serialport.h"
#include "RainAdapter.h"
#include "CreateMap.h"

using namespace P8PLATFORM;

RainAdapter::RainAdapter(const char *deviceName, std::map<char, std::vector<std::string> > commandMap) :
		m_gotResponse(false), m_commandResponseMap(
				CreateMap<char, std::string>('B', "BDR")('C', "CFG")('O', "OSD")(
						'X', "REC"))

{
	CLockObject lock(m_mutex);

	m_port = new CSerialPort(deviceName,
	CEC_RAINSHADOW_SERIAL_DEFAULT_BAUDRATE);
}

RainAdapter::~RainAdapter(void)
{
	Close();
	SAFE_DELETE(m_port);
}

bool RainAdapter::IsOpen(void)
{
	/* thread is not being stopped, the port is open and the thread is running */
	return !IsStopped() && m_port->IsOpen() && IsRunning();
}

bool RainAdapter::Open(uint32_t iTimeoutMs /* = CEC_DEFAULT_CONNECT_TIMEOUT */)
{
	bool bConnectionOpened = false;

	CLockObject lock(m_mutex);

	/* we need the port settings here */
	if (!m_port)
	{
		std::cerr << "port is NULL" << std::endl;
		return bConnectionOpened;
	}

	/* return true when the port is already open */
	if (IsOpen())
	{
		std::cerr << "port is already open" << std::endl;
		return true;
	}

	/* try to open the connection */
	std::string strError;
	CTimeout timeout(iTimeoutMs);
	while (!bConnectionOpened && timeout.TimeLeft() > 0)
	{
		if ((bConnectionOpened = m_port->Open(timeout.TimeLeft())) == false)
		{
			Sleep(250);
			std::cerr << "error opening serial port '" << m_port->GetName()
					<< "': " << m_port->GetError() << std::endl;
		}
		/* and retry every 250ms until the timeout passed */
	}

	/* return false when we couldn't connect */
	if (!bConnectionOpened)
	{
		std::cerr << strError << std::endl;

		if (m_port->GetErrorNumber() == EACCES)
		{
			std::cerr << "No permission to open the device" << std::endl;
		}
		else if (m_port->GetErrorNumber() == EBUSY)
		{
			std::cerr
					<< "The serial port is busy. Only one program can access the device directly."
					<< std::endl;
		}
		return false;
	}

	std::cerr
			<< "connection opened, clearing any previous input and waiting for active transmissions to end before starting"
			<< std::endl;

	m_response[0] = '\0';
	m_gotResponse = true;
	m_condition.Signal();

	if (!CreateThread())
	{
		bConnectionOpened = false;
		std::cerr << "could not create a communication thread" << std::endl;
	}

	if (!bConnectionOpened)
		StopThread(0);

	lock.Unlock();

	return bConnectionOpened;
}

void RainAdapter::Close(void)
{
	/* stop the reader thread */
	StopThread(0);

	CLockObject lock(m_mutex);

	/* set the ackmask to 0 before closing the connection */
	if (IsOpen() && m_port->GetErrorNumber() == 0)
	{
		std::cerr << __FUNCTION__ << " - closing the connection" << std::endl;
	}

	/* close and delete the com port connection */
	if (m_port)
		m_port->Close();
}

bool RainAdapter::WriteAdapterCommand(char *command, const char *response)
{
	if (m_port->Write(command, strlen(command)) != (ssize_t) strlen(command))
	{
		return false;
	}

	m_condition.Wait(m_mutex, m_gotResponse);
	m_gotResponse = false;

	return !strncmp(m_response, response, strlen(response));
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
		} while (m_port->GetErrorNumber() == EINTR);

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
