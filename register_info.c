// Copyright (c) 2016 Steven Bacon
//
// Permission is hereby granted, free of charge, to any person obtaining a copy of
// this software and associated documentation files (the "Software"), to deal in
// the Software without restriction, including without limitation the rights to
// use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies
// of the Software, and to permit persons to whom the Software is furnished to do
// so, subject to the following conditions:
//
// The above copyright notice and this permission notice shall be included in all
// copies or substantial portions of the Software.
//
// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
// IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
// FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
// AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
// LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
// OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
// SOFTWARE.
//
// File Name: register_info.c
//
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "register_info.h"


bool validate_register_info(struct register_info_t* register_info)
{
    //TODO check the upper and lower values are in order.
    return true;
}

void normalise_name_lengths(struct register_info_t* register_info)
{
    int i;
    int longest = 0, len;
    char* tmp;
    struct segment_data_t* s;

    // First find the longest
    for(i=0; i<register_info->entries; i++)
    {
        len = strlen(register_info->segs[i].name);
        if(len > longest)
            longest = len;
    }

    len = sizeof(char) + longest;
    //printf("%s longest %d\n", __func__, longest);
    // Allocate the new strings
    for(i=0; i<register_info->entries; i++)
    {
        s =  &register_info->segs[i];
        tmp = s->name;
        s->name = malloc(len+1);
        memset(s->name, ' ', len);
        s->name[longest] = '\0';
        memcpy(s->name, tmp, strlen(tmp));
        free(tmp);
    }
}
