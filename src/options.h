
#ifndef OPTIONS_H
#define OPTIONS_H

#include "graphics.h"


struct Options
{
    const char *romPath;
    const char *serialOutPath;
    bool exitEarly;
    struct GraphicsOptions graphics;
};


int parse_options(int argc, char **argv, struct Options *options);


#endif
