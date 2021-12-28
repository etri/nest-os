/*********************************************************************
        DO WHAT THE FUCK YOU WANT TO PUBLIC LICENSE
                   Version 2, December 2004

Copyright (C) 2004 Sam Hocevar <sam@hocevar.net>

Everyone is permitted to copy and distribute verbatim or modified
copies of this license document, and changing it is allowed as long
as the name is changed.

           DO WHAT THE FUCK YOU WANT TO PUBLIC LICENSE
  TERMS AND CONDITIONS FOR COPYING, DISTRIBUTION AND MODIFICATION

 0. You just DO WHAT THE FUCK YOU WANT TO.
**********************************************************************/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "error.h"
#include "cwd.h"

static void get_labels(char **names, int n)
{
        FILE *fp;
        char *path = WORK_DIR()"data/coco.names";
        int i = 0;

        fp = fopen(path, "r");
        if (!fp)
	{
		printf("file does not exist");
		exit(1);
	}

        while (!feof(fp))
        {
                fgets(names[i], sizeof(char)*256, fp);
                i++;
        }
        fclose(fp);

        for (int j=0; j<n; j++)
        {
                names[j][strlen(names[j])-1] = '\0';
        }
}

char **load_coconames80(void)
{
	const int classes = 80;
	char **names = calloc(classes, sizeof(char *));
        for (int i=0; i<classes; i++)
        {
                names[i] = calloc(256, sizeof(char));
        }
        get_labels(names, classes);
	
	return names;
}
