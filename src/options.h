
#ifndef OPTIONS_H
#define OPTIONS_H

#include "graphics.h"


struct Options
{
    const char *romPath;
    struct GraphicsOptions graphics;
    bool exitEarly;
};


int parse_options(int argc, char **argv, struct Options *options);


#endif
