#include <stdlib.h>
#include <iostream>
#include <getopt.h>
#include <map>

#include "Split.h"

#include "RainAdapter.h"

#define no_argument 0
#define required_argument 1
#define optional_argument 2
#define DELIMITER ','

void usage()
{
   std::cout <<
		   "Usage" 				             											<< std::endl << std::endl <<
		   "  rainconf [options] <com port>" 											<< std::endl << std::endl <<
		   "Sends some commands to the RainShadow adapter" 								<< std::endl << std::endl <<
		   "Options"                            										<< std::endl <<
		   "-a --get-logical-address"													<< std::endl <<
		   "-A --set-logical-address <logical address>[,bit field>]" 					<< std::endl <<
		   "-b --get-persisted-logical-address" 										<< std::endl <<
		   "-B --set-persisted-logical-address <logical address>[,bit field>]"		 	<< std::endl <<
		   "-c --get-configuration-bits" 												<< std::endl <<
		   "-C --set-configuration-bits <configuration bits>" 							<< std::endl <<
		   "-M --mirror-string <string to mirror>"										<< std::endl <<
		   "-o --get-osd-name" 															<< std::endl <<
		   "-O --set-osd-name <osd name>"												<< std::endl <<
		   "-p --get-physical-address" 													<< std::endl <<
		   "-P --set-physical-address <physical address>[,<device type>]"				<< std::endl <<
		   "-q --get-current-retry-count" 												<< std::endl <<
		   "-Q --set-current-retry-count <retry count>" 								<< std::endl <<
           "-r --get-revision"                                                          << std::endl <<
		   "-X --send-raw-command \"<raw command>\"" 									<< std::endl <<
		   "-h --help" 																	<< std::endl;
}

int main(int argc, char * argv[])
{
	std::map<char, std::vector<std::string> > commandMap;

	const struct option longopts[] =
	{
	{ "get-logical-address", no_argument, 0, 'a' },
	{ "set-logical-address", required_argument, 0, 'A' },
	{ "get-persisted-logical-address", no_argument, 0, 'b' },
	{ "set-persisted-logical-address", required_argument, 0, 'B' },
	{ "get-configuration-bits", no_argument, 0, 'c' },
	{ "set-configuration-bits", required_argument, 0, 'C' },
	{ "mirror-string", required_argument, 0, 'M' },
	{ "get-osd-name", no_argument, 0, 'o' },
	{ "set-osd-name", required_argument, 0, 'O' },
	{ "get-physical-address", no_argument, 0, 'p' },
	{ "set-physical-address", required_argument, 0, 'P' },
	{ "get-current-retry-count", no_argument, 0, 'q' },
	{ "set-current-retry-count", required_argument, 0, 'Q' },
    { "get-revision", no_argument, 0, 'r' },
	{ "send-raw-command", required_argument, 0, 'X' },
	{ "help", no_argument, 0, 'h' },
	{ 0, 0, 0, 0 }, };

	int index;
	int iarg = 0;

	//turn off getopt error message
	opterr = 1;

	while (iarg != -1)
	{
		std::vector<std::string> vec;
		vec.clear();

		iarg = getopt_long(argc, argv, "abcopqrA:B:C:M:O:P:Q:X:h", longopts,
				&index);

		switch (iarg)
		{
		case 'a':
		case 'b':
		case 'c':
		case 'o':
		case 'p':
		case 'q':
		case 'r':
			commandMap[toupper((char) iarg)] = vec; 			// no arguments, vec is empty
			break;
		case 'M':
		case 'O':
		case 'Q':
		case 'X':
			vec.push_back(std::string(optarg));					// a single argument
			commandMap[(char) iarg] = vec;
			break;
		case 'A':
		case 'B':
		case 'C':
		case 'P':
			commandMap[(char) iarg] = split(optarg, DELIMITER); // multiple arguments separated by DELIMITER
			break;
		case 'h':
			usage();
			exit(EXIT_SUCCESS);
		}
	}

	if (optind + 1 != argc)
	{
		usage();
		exit(EXIT_FAILURE);
	}

	RainAdapter rainAdapter(argv[optind]);
	rainAdapter.processCommands(commandMap);

	return 0;
}
