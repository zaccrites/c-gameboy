
#include <stdbool.h>
#include <string.h>

#include "options.h"


static bool starts_with(const char* string, const char* prefix)
{
    return strncmp(string, prefix, strlen(prefix)) == 0;
}


int parse_options(int argc, char **argv, struct Options *options)
{
    options->romPath = NULL;
    options->graphics.smallWindow = false;

    for (int i = 0; i < argc; i++)
    {
        const char *arg = argv[i];
        if (starts_with(arg, "--"))
        {
            if (strcmp(arg, "--small") == 0)
            {
                options->graphics.smallWindow = true;
            }
            else
            {
                fprintf(stderr, "error: unknown option %s \n", arg);
                return 1;
            }
        }
        else
        {
            if (options->romPath == NULL)
            {
                options->romPath = arg;
            }
        }
    }

    if (options->romPath == NULL)
    {
        fprintf(stderr, "error: please provide path to ROM file \n");
        return 1;
    }

    return 0;
}
