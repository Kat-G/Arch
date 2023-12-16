#include "AppViewer.h"
#include <vector>

int main (int argc, char *argv[])
{
    Viewer v;
    if(!v.init())
        return 0;
    v.run();
}