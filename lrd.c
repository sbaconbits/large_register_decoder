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
// File Name: lrd.c
//

#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <getopt.h>
#include <stdbool.h>
#include <math.h>

#include "config_parser.h"
#include "register_info.h"

static char* cfg_file_name = NULL;
static char* register_name = NULL;
static uint32_t register_value = 0;
static int*  encode_list = NULL;
static struct register_info_t register_info;

static void set_config_file_name(char* name);
static void set_register_name(char* name);
static void init(void);
static void fini(void);
static void decode_register(char* value_str);
static int  str_count_char(char* s, char c);
static void encode_register(char* value_list_str);
static void print_usage(void);
static void parse_opts(int argc, char* argv[]);
static void print_decode_header(void);
static void print_pos(void);
static void print_sep(void);
static void print_register_bin(void);
static void print_decoded_segs(void);
static uint32_t seg_to_val(struct segment_data_t* s);
static uint32_t bit_pos_to_segment_index(uint32_t bit_pos);

//TODO
// - valgrind
// - handle different sized registers
// - highlight every other vertical segment

static void print_pos(void)
{
    int i;
    for(i=0; i<32; i++)
        printf("%d",(31-i)%10);
    printf("\n");
}

static void print_sep(void)
{
    int i, j;
    char c;
    int a[] = {8,4};

    for(j=0; j<(sizeof(a)/sizeof(int)); j++)
    {
        for(i=0; i<32; i++)
        {
            if( i % a[j] == 0 )
                c = '[';
            else if( i % a[j] == (a[j] - 1) )
                c = ']';
            else
                c = '-';
            printf("%c",c);
        }
        printf("\n");
    }
}


static void print_register_bin(void)
{
    uint32_t m = (1 << 31);

    while(m)
    {
        printf("%c", (register_value & m) ? '1' : '0');
        m >>= 1;
    }
    printf("\n");
}

static uint32_t seg_to_val(struct segment_data_t* s)
{
    uint32_t res=0, m;
    int i;

    for(i=s->upper; i>=s->lower; i--)
    {
        m = (1 << i);
        res |= (m & register_value);
    }

    res = res >> s->lower;
    return res;
}

static uint32_t bit_pos_to_segment_index(uint32_t bit_pos)
{
    struct segment_data_t* s;
    uint32_t i;

    for(i=0; i<register_info.entries; i++)
    {
        s = &register_info.segs[i];
        if((bit_pos <= s->upper) && (bit_pos >= s->lower))
            return i;
    }
    return 0;
}

static void print_decoded_segs(void)
{
    int i,j;
    struct segment_data_t* s;
    uint32_t val;

    for(i=0; i<register_info.entries; i++)
    {
        s = &register_info.segs[i];
        // Leading chars
        for(j=31; j>=0; j--)
        {
            if(j > s->upper)
            {
                printf(" ");
            }
            else if( (j <= s->upper) && (j >= s->lower))
            {
                printf("*");
            }
            else
            {
                if(0x1 & bit_pos_to_segment_index(j))
                    printf("|");
                else
                    printf("-");
            }
        }

        // Upper and lower and name
        if(s->upper == s->lower)
            printf(" [   %02d] %s", s->upper, s->name);
        else
            printf(" [%02d:%02d] %s", s->upper, s->lower, s->name);

        // The value
        val = seg_to_val(s);
        printf("%4d : 0x%x", val, val);

        printf("\n");
    }
}

static void print_decode_header(void)
{
    printf("Decode reg %s with value 0x%08x\n", register_name, register_value);
    print_pos();
    print_sep();
    print_register_bin();
    print_decoded_segs();
}

static void set_config_file_name(char* name)
{
    free(cfg_file_name);
    cfg_file_name = strdup(name);
    //printf("config file:%s\n", cfg_file_name);
}

static void set_register_name(char* name)
{
    free(register_name);
    register_name = strdup(name);
    //printf("register name: %s\n", register_name);
}

static void init(void)
{
    //printf("%s\n", __func__);
    register_info.entries = 0;
    register_info.segs = NULL;
    set_config_file_name("~/lrd.cfg");
}

static void fini(void)
{
    int i;
    //printf("%s\n", __func__);
    for(i=0; i<register_info.entries; i++)
        free(register_info.segs[i].name);
    free(register_info.segs);
    free(cfg_file_name);
    free(register_name);
    free(encode_list);
}

static void decode_register(char* value_str)
{
    bool ret;

    if(1 != sscanf(value_str, "0x%8x", &register_value))
    {
        printf("Error: incorrect argument to decode (%s)\n", value_str);
        return;
    }

    //TODO check the returns
    (void)ret;
    ret = populate_register_info_from_config(register_name, cfg_file_name, &register_info);
    ret = validate_register_info(&register_info);
    normalise_name_lengths(&register_info);
    print_decode_header();
}

static int str_count_char(char* s, char c)
{
    int ret = 0;
    while('\0' != *s)
        if(*(s++) == c) ++ret;
    return ret;
}

static void encode_register(char* value_list_str)
{
    int elements = str_count_char(value_list_str, ',') + 1;
    char* tokenp = NULL;
    int i = 0;
    bool ret;
    struct segment_data_t* s;
    uint32_t reg_value=0;

    // Tokenise the segment data list
    encode_list = malloc(sizeof(int) * elements);
    //printf("%s %s elements %d \n", __func__, value_list_str, elements);
    while((tokenp = strsep(&value_list_str, ",")))
    {
        if(tokenp[0] == '\0')
            break;

        encode_list[i] = atoi(tokenp);
        //printf("token:%s %d\n", tokenp, encode_list[i]);
        i++;
    }

    // Populate the register info for the register we are interested in.
    ret = populate_register_info_from_config(register_name, cfg_file_name, &register_info);
    if(false == ret)
    {
        printf("Error encoding: failed to populate register info from config\n");
        return;
    }

    if(elements != register_info.entries)
    {
        printf("Error number of elements given (%d) are the same as the"
               "number of segments for the register (%d)\n", elements, register_info.entries);
    }

    for(i=0; i<elements; i++)
    {
        int seg_max;
        s = &register_info.segs[i];
        seg_max = pow(2, s->upper - s->lower + 1) - 1;
        if(encode_list[i] > seg_max)
        {
            printf("Error ecoding segment %d has a value (%d) great than max (%d)\n",
                    i, encode_list[i], seg_max);
            return;
        }
        //printf("seg max %d\n", seg_max);
        //printf("val = 
        reg_value += encode_list[i] << s->lower;
    }
    printf("Encoded value 0x%x\n", reg_value);

}

static void print_usage(void)
{
    printf("Usage:\n");
    printf("-c <config_file>    (~/.lrd.conf by default.\n");
    printf("-r <reg_name>       As defined in the config file.\n");
    printf("-d <reg_val>        Decode register value (hexadecimal) e.g. 0x12341234\n");
    printf("-e <value_list>     Encode list (decimal) e.g 1,2,10,2\n"); //TODO hex in future
    printf("\n");
}

static void parse_opts(int argc, char* argv[])
{
    int ret, running = 1;

    while(running)
    {
        ret = getopt(argc, argv, "c:r:d:e:h");
        if(ret == -1)
        {
            // Parsed all args returning
            break;
        }

        switch(ret)
        {
            case 'c':
                set_config_file_name(optarg);
                break;
            case 'r':
                set_register_name(optarg);
                break;
            case 'd':
                decode_register(optarg);
                break;
            case 'e':
                encode_register(optarg);
                break;
            case 'h':
            default:
                print_usage();
                running = 0;
                break;
        }
    }
}


int main(int argc, char* argv[])
{
    init();
    parse_opts(argc, argv);
    fini();
    return 0;
}
