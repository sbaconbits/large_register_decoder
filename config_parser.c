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
// File Name: config_parser.c
//

#include <stdio.h>
#include <stdbool.h>
#include <libconfig.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>

#include "config_parser.h"

bool populate_register_info_from_config(char* register_name,
                                        char* cfg_file_name,
                                        struct register_info_t* register_info)
{
    config_t cfg;
    int ret, count, i, path_len;
    config_setting_t* sett_names;
    config_setting_t* sett_poss;
    bool return_val = true;
    char* path = NULL;

    config_init(&cfg);

    if(NULL == register_name)
    {
        printf("Error: register name not given\n");
        return_val = false;
        goto cleanup;
    }

    ret = config_read_file(&cfg, cfg_file_name);
    if(CONFIG_FALSE == ret)
    {
        printf("Failed to read config reason (%s) line (%d)\n", config_error_text(&cfg), config_error_line(&cfg));
        return_val = false;
        goto cleanup;
    }

    path_len = strlen(register_name) + strlen("names") + 2; // Null term + seperator
    path = malloc(sizeof(char) * path_len);
    snprintf(path, path_len, "%s.names", register_name);

    // Getting the "names" for the segments
    sett_names = config_lookup(&cfg, path);
    if(NULL == sett_names)
    {
        printf("Failed to lookup (%s) in (%s) reason (%s) line %d\n",
                path, cfg_file_name, config_error_text(&cfg), config_error_line(&cfg));
        return_val = false;
        goto cleanup;
    }

    // Check we've found an array
    if(CONFIG_FALSE == config_setting_is_array(sett_names))
    {
        printf("Error in config file (%s) path (%s) is not an array\n", cfg_file_name, path);
        return_val = false;
        goto cleanup;
    }

    free(path);

    path_len = strlen(register_name) + strlen("positions") + 2; // Null term + seperator
    path = malloc(sizeof(char) * path_len);
    snprintf(path, path_len, "%s.positions", register_name);

    // Getting the "positions" for the segments
    sett_poss = config_lookup(&cfg, path);
    if(NULL == sett_poss)
    {
        printf("Failed to lookup (%s) in (%s) reason (%s) line %d\n",
                path, cfg_file_name, config_error_text(&cfg), config_error_line(&cfg));
        return_val = false;
        goto cleanup;
    }

    // Check we've found an array
    if(CONFIG_FALSE == config_setting_is_array(sett_poss))
    {
        printf("Error in config file (%s) path (%s) is not an array\n", cfg_file_name, path);
        return_val = false;
        goto cleanup;
    }

    // Check the arrarys are the same length
    count = config_setting_length(sett_names);
    if(count != config_setting_length(sett_poss))
    {
        printf("Error the names and the positions don't have the same number of elements\n");
        return_val = false;
        goto cleanup;
    }

    register_info->entries = count;
    register_info->segs = malloc(sizeof(struct segment_data_t) * count);
    // Itterate over all of the array elements and add to our store.
    for(i=0; i<count; i++)
    {
        int len;
        const char* p;
        //printf("%d - %s  %s\n", i,
        //        config_setting_get_string_elem(sett_names, i),
        //        config_setting_get_string_elem(sett_poss,  i));

        len = strlen(config_setting_get_string_elem(sett_names, i)) + 1 ;
        register_info->segs[i].name = malloc(sizeof(char) * len);
        strncpy(register_info->segs[i].name, config_setting_get_string_elem(sett_names,i), len);
        //printf("%d - %s\n", i, register_info->segs[i].name);

        p = config_setting_get_string_elem(sett_poss, i);
        register_info->segs[i].upper = atoi(p);
        p = strchr(p, ':');
        if(NULL == p)
        {
            register_info->segs[i].lower = register_info->segs[i].upper;
        }
        else
        {
            //printf("p %s\n",p+1);
            //TODO handle error if empty after ':' in segments pos
            register_info->segs[i].lower = atoi(p+1);
        }
        //printf("upper:%d\n", register_info->segs[i].upper);
        //printf("lower:%d\n", register_info->segs[i].lower);

    }

cleanup:
    free(path);
    config_destroy(&cfg);
    return return_val;
}
