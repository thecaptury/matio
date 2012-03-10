/*
 * Copyright (C) 2005-2011   Christopher C. Hulbert
 *
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 *
 *    1. Redistributions of source code must retain the above copyright notice,
 *       this list of conditions and the following disclaimer.
 *
 *    2. Redistributions in binary form must reproduce the above copyright
 *       notice, this list of conditions and the following disclaimer in the
 *       documentation and/or other materials provided with the distribution.
 *
 * THIS SOFTWARE IS PROVIDED BY CHRISTOPHER C. HULBERT ``AS IS'' AND ANY
 * EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
 * WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
 * DISCLAIMED. IN NO EVENT SHALL CHRISTOPHER C. HULBERT OR CONTRIBUTORS BE
 * LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
 * CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
 * SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
 * INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
 * CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
 * ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
 * POSSIBILITY OF SUCH DAMAGE.
 */
#include <stdlib.h>
#include <stdarg.h>
#include <stdio.h>
#include <string.h>
#include <math.h>
#include <getopt.h>
#include "matio_private.h"
#if !defined(HAVE_STRCASECMP)
#   define strcasecmp(a,b) strcmp(a,b)
#endif

static const char *optstring = "df:hvHV";
static struct option options[] = {
    {"data",    no_argument,      NULL,'d'},
    {"format",  required_argument,NULL,'f'},
    {"human",   no_argument,      NULL,'h'},
    {"verbose", optional_argument,NULL,'v'},
    {"help",    no_argument,      NULL,'H'},
    {"version", no_argument,      NULL,'V'},
    {NULL,0,NULL,0}
};

static const char *helpstr[] = {
"",
"Usage: matdump [OPTIONS] mat_file [var1 var2 ...]",
"",
"Runs various test on the Matlab I/O library libmatio",
"",
"OPTIONS",
"-d,--data         Print data with header information",
"-h,--human        Human readable sizes in 'whos' display mode",
"-v,--verbose      Turn on verbose messages",
"-H,--help         This output",
"-V,--version      version information",
"",
"mat_file          name of the MAT file to dump",
"var1 var2 ...     If specified, dumps only listed variables",
"",
"Report bugs to <cch@isl-inc.com>.",
NULL
};

static char *byteswapping[2] = {"No","Yes"};
static char *mxclass[13] = { "mxCELL_CLASS", "mxSTRUCT_CLASS", "mxOBJECT_CLASS",
                             "mxCHAR_CLASS", "mxSPARSE_CLASS", "mxDOUBLE_CLASS",
                             "mxSINGLE_CLASS", "mxINT8_CLASS", "mxUINT8_CLASS",
                             "mxINT16_CLASS", "mxUINT16_CLASS", "mxINT32_CLASS",
                             "mxUINT32_CLASS"
                            };
static int printdata = 0;
static int human_readable = 0;
static int print_whos_first = 1;

/* Print Functions */
static void print_whos(matvar_t *matvar);
static void print_default(matvar_t *matvar);

static void (*printfunc)(matvar_t *matvar) = NULL;

static void
print_whos(matvar_t *matvar)
{
    int i;
    int nbytes;
    char size[32] = {'\0',};

    if ( print_whos_first ) {
        printf("%-20s       %-10s     %-10s     %-18s\n\n","Name","Size","Bytes","Class");
        print_whos_first = 0;
    }
    printf("%-20s", matvar->name);
    if ( matvar->rank > 0 ) {
        int cnt = 0;
        printf("%8d", matvar->dims[0]);
        nbytes = matvar->dims[0];
        for ( i = 1; i < matvar->rank; i++ ) {
            if ( ceil(log10(matvar->dims[i]))+1 < 32 )
                cnt += sprintf(size+cnt,"x%d", matvar->dims[i]);
            nbytes *= matvar->dims[i];
        }
        printf("%-10s",size);
        nbytes *= Mat_SizeOfClass(matvar->class_type);
    } else {
        printf("                    ");
    }
    if ( human_readable ) {
        if ( nbytes > 1073741824L )
            printf(" %10.1fG",(double)nbytes/1073741824.0);
        else if ( nbytes > 1048576 )
            printf(" %10.1fM",(double)nbytes/1048576.0);
        else if ( nbytes > 1024 )
            printf(" %10.1fK",(double)nbytes/1024.0);
        else
            printf(" %10dB",nbytes);
    } else {
        printf("%  10d",nbytes);
    }
    printf("  %-18s\n",mxclass[matvar->class_type-1]);

    return;
}

static int indent = 0;

static void
default_printf_func(int log_level,char *message)
{
    int i;

    for ( i = 0; i < indent; i++ )
        printf("    ");
    printf("%s\n",message);
}

static void
print_default(matvar_t *matvar)
{
    if ( NULL == matvar )
        return;

    switch ( matvar->class_type ) {
        case MAT_C_DOUBLE:
        case MAT_C_SINGLE:
        case MAT_C_INT64:
        case MAT_C_UINT64:
        case MAT_C_INT32:
        case MAT_C_UINT32:
        case MAT_C_INT16:
        case MAT_C_UINT16:
        case MAT_C_INT8:
        case MAT_C_UINT8:
        case MAT_C_CHAR:
        case MAT_C_SPARSE:
            Mat_VarPrint(matvar, printdata);
            break;
        case MAT_C_STRUCT:
        {
            matvar_t **fields = (matvar_t **)matvar->data;
            int        nfields;
            int        i;
            size_t     nmemb;

            if ( matvar->name )
                Mat_Message("      Name: %s", matvar->name);
            Mat_Message("      Rank: %d", matvar->rank);
            if ( matvar->rank == 0 )
                return;
            Mat_Message("Class Type: Structure");
            nfields = Mat_VarGetNumberOfFields(matvar);
            nmemb = matvar->dims[0];
            for ( i = 1; i < matvar->rank; i++ )
                nmemb *= matvar->dims[i];
            if ( nfields > 0 && nmemb < 1 ) {
                char * const *fieldnames = Mat_VarGetStructFieldnames(matvar);
                Mat_Message("Fields[%d] {", nfields);
                indent++;
                for ( i = 0; i < nfields; i++ )
                    Mat_Message("    Name: %s", matvar->name);
                indent--;
                Mat_Message("}");
            } else if ( nfields > 0 && nmemb > 0 ) {
                Mat_Message("Fields[%d] {", nfields);
                indent++;
                for ( i = 0; i < nfields*nmemb; i++ )
                    print_default(fields[i]);
                indent--;
                Mat_Message("}");
            }
            break;
        }
        case MAT_C_CELL:
        {
            matvar_t **cells = (matvar_t **)matvar->data;
            size_t     ncells;
            int        i;

            if ( matvar->name )
                Mat_Message("      Name: %s", matvar->name);
            Mat_Message("      Rank: %d", matvar->rank);
            if ( matvar->rank == 0 )
                return;
            ncells = matvar->dims[0];
            for ( i = 1; i < matvar->rank; i++ )
                ncells *= matvar->dims[i];
            Mat_Message("Class Type: Cell Array");
            Mat_Message("{");
            indent++;
            for ( i = 0; i < ncells; i++ )
                print_default(cells[i]);
            indent--;
            Mat_Message("}");
            break;
        }
        default:
            Mat_Message("Empty");
    }
}

int
main (int argc, char *argv[])
{
    char *prog_name = "matdump";
    int   i, k, c, err = EXIT_SUCCESS;
    mat_t    *mat;
    matvar_t *matvar;

    Mat_LogInitFunc(prog_name,default_printf_func);

    printfunc = print_default;

    while ((c = getopt_long(argc,argv,optstring,options,NULL)) != EOF) {
        switch (c) {
            case 'd':
                printdata = 1;
                Mat_VerbMessage(1,"Printing data\n");
                break;
            case 'f':
                if ( NULL != optarg && !strcmp(optarg,"whos") ) {
                    printfunc = print_whos;
                    break;
                }
                Mat_Warning("%s is not a recognized output format. "
                              "Using default\n", optarg);
                break;
            case 'h':
                human_readable = 1;
                break;
            case 'v':
                Mat_SetVerbose(1,0);
                break;
            case 'H':
                Mat_Help(helpstr);
                exit(EXIT_SUCCESS);
            case 'V':
                printf("%s %d.%d.%d\n"
                       "Written by Christopher Hulbert\n\n"
                       "Copyright(C) 2006 Christopher C. Hulbert",
                       prog_name,MATIO_MAJOR_VERSION,MATIO_MINOR_VERSION,
                       MATIO_RELEASE_LEVEL);
                exit(EXIT_SUCCESS);
            default:
                printf("%c not a valid option\n", c);
                break;
        }
    }

    if ( (argc-optind) < 1 )
        Mat_Error("Must specify at least one argument");

    mat = Mat_Open( argv[optind],MAT_ACC_RDONLY );
    if ( NULL == mat ) {
        Mat_Error("Error opening %s\n", argv[optind]);
        return EXIT_FAILURE;
    }

    optind++;

    if ( optind < argc ) {
        /* variables specified on the command line */
        if ( printdata ) {
            for ( i = optind; i < argc; i++ ) {
                matvar = Mat_VarRead(mat,argv[i]);
                if ( matvar ) {
                    (*printfunc)(matvar);
                    Mat_VarFree(matvar);
                    matvar = NULL;
                } else {
                    Mat_Warning("Couldn't find variable %s in the MAT file",
                          argv[i]);
                }
            }
        } else {
            for ( i = optind; i < argc; i++ ) {
                matvar = Mat_VarReadInfo(mat,argv[i]);
                if ( matvar ) {
                    (*printfunc)(matvar);
                    Mat_VarFree(matvar);
                    matvar = NULL;
                } else {
                    Mat_Warning("Couldn't find variable %s in the MAT file",
                          argv[i]);
                }
            }
        }
    } else {
        /* print all variables */
        if ( printdata ) {
            while ( (matvar = Mat_VarReadNext(mat)) != NULL ) {
                (*printfunc)(matvar);
                Mat_VarFree(matvar);
                matvar = NULL;
            }
        } else {
            while ( (matvar = Mat_VarReadNextInfo(mat)) != NULL ) {
                (*printfunc)(matvar);
                Mat_VarFree(matvar);
                matvar = NULL;
            }
        }
    }

    Mat_Close(mat);

    Mat_LogClose();

    return err;
}
