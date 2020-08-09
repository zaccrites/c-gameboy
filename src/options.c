
#include <stdbool.h>
#include <string.h>

#include "options.h"


static bool starts_with(const char* string, const char* prefix)
{
    return strncmp(string, prefix, strlen(prefix)) == 0;
}


static void print_help(void)
{
    fprintf(stderr,
        "Usage: emulator <rom_path> [...options] \n"
        "  rom_path : Path to GameBoy ROM .gb file \n"
        "\n"
        "Options: \n"
        "  --small  : Size display window accurate to a real GameBoy LCD (it is shown 4x wider and taller by default) \n"
        "  --help   : Display this message and quit \n"
    );
}


int parse_options(int argc, char **argv, struct Options *options)
{
    options->exitEarly = false;
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
            else if (strcmp(arg, "--help") == 0)
            {
                options->exitEarly = true;
                print_help();
                return 0;
            }
            else
            {
                fprintf(stderr, "error: unknown option %s \n", arg);
                print_help();
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
        print_help();
        return 1;
    }

    return 0;
}
