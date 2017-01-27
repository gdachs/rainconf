#include <iostream>
#include <p8-platform/util/util.h>

#include "serialport.h"
#include "RainAdapter.h"
#include "CreateMap.h"
#include "Split.h"

using namespace P8PLATFORM;

RainAdapter::RainAdapter(const char *deviceName) :
		m_gotResponse(false), m_commandResponseMap(
				CreateMap<char, std::string>('A', "ADR")('B', "BDR")('C', "CFG")(
						'O', "OSD")('P', "PHY")('Q', "QTY")('R', "REV")('X',
						"STA"))
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

void RainAdapter::processCommands(
		std::map<char, std::vector<std::string> > commandMap)
{
	if (!IsOpen())
	{
		Open();
	}
	for (std::map<char, std::vector<std::string> >::iterator cmdIt =
			commandMap.begin(); cmdIt != commandMap.end(); ++cmdIt)
	{
		std::string command("!");

		command += cmdIt->first;
		for (std::vector<std::string>::iterator vecIt = cmdIt->second.begin();
				vecIt != cmdIt->second.end(); ++vecIt)
		{
			command += " " + *vecIt;
		}
		command += '~';

		if (WriteAdapterCommand(command,
				m_commandResponseMap[cmdIt->first].c_str()))
		{
		    printResponse();
		}
		else
		{
			std::cerr << "got wrong response for command: " << command
					<< " response: " << m_response << std::endl;
		}
	}
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

	m_response[0] = '\0';
//	m_gotResponse = true;
//	m_condition.Signal();

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

bool RainAdapter::WriteAdapterCommand(std::string &command, const std::string &responseType)
{
	CLockObject lock(m_mutex);

	m_gotResponse = false;

	if (m_port->Write((void *)command.c_str(), command.length()) != (ssize_t) command.length())
	{
		return false;
	}

	m_condition.Wait(m_mutex, m_gotResponse);

    return !m_response.substr(0, responseType.length()).compare(responseType);
}

void RainAdapter::printResponse()
{
    std::vector<std::string> respVec;

    switch (m_response[0])
    {
    case 'A':
    case 'B':
        respVec = split(m_response, ' ');
        std::cout << "LOGICAL_ADDRESS=" << respVec[1] << std::endl << "BIT_FIELD=" << respVec[2] << std::endl;
        break;
    case 'C':
        std::cout << "CONFIGURATION_BITS=" <<  m_response.substr(4) << std::endl;
        break;
    case 'M':
        std::cout << "MIRROR=\"" <<  m_response.substr(4) << "\"" << std::endl;
        break;
    case 'O':
        std::cout << "OSD_NAME=\"" <<  m_response.substr(4) << "\"" << std::endl;
        break;
    case 'P':
        respVec = split(m_response, ' ');
        std::cout << "PHYSICAL_ADDRESS=" << respVec[1] << std::endl << "DEVICE_TYPE=" << respVec[2] << std::endl;
        break;
    case 'Q':
        std::cout << "RETRY_COUNT=" <<  m_response.substr(4) << std::endl;
        break;
    case 'R':
        std::cout << "REVISION=" <<  m_response.substr(4) << std::endl;
        break;
    case 'S':
        std::cout << "SENDING_STATUS=" <<  m_response.substr(4) << std::endl;
        break;
    default:
        /* do nothing */
        break;
    }
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
				m_response = buf;
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
