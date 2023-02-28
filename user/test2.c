#include "type.h"
#include "const.h"
#include "protect.h"
#include "string.h"
#include "proc.h"
#include "global.h"
#include "proto.h"
#include "stdio.h"

int main(int arg, char *argv[])
{
    //set_screen(320,200);
    int pid=fork();
    if (pid)
    {
		while (1)
		{
			printf("1");
		}
        //get_win();
    }
    else
    {
  		while (1)
		{
			printf("2");
		}
    }

    return 0;
}