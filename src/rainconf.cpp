#include <iostream>
#include <getopt.h>
#include <map>

#include "Split.h"

#include "RainAdapter.h"

#define no_argument 0
#define required_argument 1
#define optional_argument 2
#define DELIMITER ','

int main(int argc, char * argv[])
{
    std::map<char, std::vector<std::string> > commandMap;

    const struct option longopts[] =
    {
      {"get-physical-address",   no_argument,        0, 'p'},
      {"set-physical-address",   required_argument,  0, 'P'},
      {0,0,0,0},
    };

    int index;
    int iarg=0;

    //turn off getopt error message
    opterr=1;

    while(iarg != -1)
    {
      iarg = getopt_long(argc, argv, "pP:", longopts, &index);

      switch (iarg)
      {
        case 'p':
            commandMap[(char) iarg] = std::vector<std::string>();
          break;

        case 'P':
            commandMap[(char) iarg] = split(optarg, DELIMITER);
          break;
      }
    }

    if (optind < argc)
    {
        RainAdapter rainAdapter(argv[optind], commandMap    );
    }

    return 0;
}
