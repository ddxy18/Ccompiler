//
// Created by dxy on 2020/7/23.
//

#include <fcntl.h>
#include <unistd.h>

#include "Ccompiler.h"

int main(int argc, char *argv[]) {
    if (argc != 2) {
        return -1;
    }

    int fd;
    if ((fd = open(argv[1], O_RDONLY)) != -1) {
        InitLex(fd);
        while (AnalyseLexical(fd)!=NULL);
        close(fd);
        //TODO: generate object code
        return 0;
    }
    return -1;
}