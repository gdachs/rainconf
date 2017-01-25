#ifndef RAINADAPTER_H_
#define RAINADAPTER_H_

#include <map>
#include <p8-platform/threads/mutex.h>
#include <p8-platform/threads/threads.h>
#include <p8-platform/sockets/socket.h>

#define DATA_SIZE 256
#define CEC_RAINSHADOW_SERIAL_DEFAULT_BAUDRATE 115200L
/*!
 * default connection timeout in milliseconds
 */
#define CEC_DEFAULT_CONNECT_TIMEOUT     10000

class RainAdapter: public P8PLATFORM::CThread
{
public:
	RainAdapter(const char *deviceName);

	virtual ~RainAdapter(void);

	void processCommands(std::map<char, std::vector<std::string> > commandMap);

	/** @name P8PLATFORM::CThread implementation */
	///{
	void *Process(void);
	///}
private:
	bool IsOpen(void);
	bool Open(uint32_t iTimeoutMs = CEC_DEFAULT_CONNECT_TIMEOUT);
	void Close(void);

    /**
     * WriteAdapterCommand - writes the adapter command and waits for the response
     * @command: the command to write
     * @response: the response to wait for
     *
     * Return true on success, else false.
     */
    bool WriteAdapterCommand(std::string &command, const char *response);

    P8PLATFORM::ISocket *m_port; /**< the com port connection */
	P8PLATFORM::CMutex m_mutex;
	char m_response[DATA_SIZE]; /**< current response from adapter */
	P8PLATFORM::CCondition<bool> m_condition;
	bool m_gotResponse;
	std::map<char, std::string> m_commandResponseMap;
};

#endif //RAINADAPTER_H_
