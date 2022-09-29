#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "info.h"

char *baseos_name[3] =
{
        "non_os",
        "linux",
        "rtos"
};

int main(int argc, char *argv[])
{
	Info *info = info_create();

	printf("num npuos = %d\n", info->get_nest_info(info));
	printf("nos id = %d queryid = %d NOS version = %.1f\n", 0, NOS_VERSION, (float)info->get_nos_info(info, 0, NOS_VERSION)/10);
	printf("nos id = %d queryid = %d NOS base os = %s\n", 0, NOS_BASEOS, baseos_name[info->get_nos_info(info, 0, NOS_BASEOS)]);
}
