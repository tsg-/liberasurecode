/* 
 * Copyright 2014 Tushar Gohad
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 *
 * Redistributions of source code must retain the above copyright notice, this
 * list of conditions and the following disclaimer.
 *
 * Redistributions in binary form must reproduce the above copyright notice, this
 * list of conditions and the following disclaimer in the documentation and/or
 * other materials provided with the distribution.  THIS SOFTWARE IS PROVIDED BY
 * THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED
 * WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF
 * MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO
 * EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT,
 * INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING,
 * BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
 * DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF
 * LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE
 * OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF
 * ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 *
 * liberasurecode null backend
 *
 * vi: set noai tw=79 ts=4 sw=4:
 */

#include <stdio.h>
#include <stdlib.h>

#include "erasurecode.h"
#include "erasurecode_helpers.h"
#include "erasurecode_backend.h"


/* Forward declarations */
struct ec_backend null;
struct ec_backend_op_stubs null_ops;
struct ec_backend_common backend_null;

typedef void* (*init_null_code_func)(int, int, int);
typedef int (*null_code_get_fragment_size_func)(void *, int, int, int);
typedef int (*null_code_encode_func)(void *, char **, char **, int);
typedef int (*null_code_naive_encode_func)(void *, char *, char **, int, int, int, int);
typedef int (*null_code_decode_func)(void *, char **, char **, int *, int, int);
typedef int (*null_code_naive_decode_func)(void *, char **, char *, int);
typedef int (*null_reconstruct_func)(char  **, int, uint64_t, int, char *);
typedef int (*null_code_fragments_needed_func)(void *, int *, int *, int *);

struct null_descriptor {
    /* calls required for init */
    init_null_code_func init_null_code;

    /* calls required for get fragment size */
    null_code_get_fragment_size_func null_code_get_fragment_size;

    /* calls required for encode */
    null_code_encode_func null_code_encode;

    /* calls required for encode */
    null_code_naive_encode_func null_code_naive_encode;

    /* calls required for decode */
    null_code_decode_func null_code_decode;

    /* calls required for decode */
    null_code_naive_decode_func null_code_naive_decode;

    /* calls required for reconstruct */
    null_reconstruct_func null_reconstruct;

    /* set of fragments needed to reconstruct at a minimum */
    null_code_fragments_needed_func null_code_fragments_needed;

    /* fields needed to hold state */
    int *matrix;
    int k;
    int m;
    int n;
    int w;
    int arg1;
};

#define DEFAULT_W 32

static int null_naive_encode(void *desc, char **data, char **parity,
        int blocksize)
{
    int i;
    int orig_size;
    char **encoded;
    struct null_descriptor *xdesc =
        (struct null_descriptor *)desc;
    int frag_size;

    //TODO: make inline description
    orig_size = (int)xdesc->k * blocksize;

    frag_size = xdesc->null_code_get_fragment_size(xdesc, xdesc->k, xdesc->m, orig_size);
    encoded = malloc(sizeof(char*)*xdesc->n);
    for(i=0; i<xdesc->n; i++) encoded[i] = malloc(sizeof(char)*frag_size);

    xdesc->null_code_naive_encode(
        xdesc, *data, encoded, xdesc->k, xdesc->m, frag_size, orig_size);

    //TODO: make a function to create **data, **parity from **encoded
    //      and free temprary data arrays.
    for(i=0; i<xdesc->k; i++){
        free(data[i]);
    }
    for(i=0; i<xdesc->k; i++){
        char *fragment = (char *)alloc_fragment_buffer(frag_size);
        data[i] = get_data_ptr_from_fragment(fragment);
        set_fragment_payload_size(fragment, frag_size);
        memcpy(data[i], encoded[i], frag_size);
    }
    for(i=0; i<xdesc->m; i++){
        char *fragment = (char *)alloc_fragment_buffer(frag_size);
        parity[i] = get_data_ptr_from_fragment(fragment);
        set_fragment_payload_size(fragment, frag_size);
        memcpy(parity[i], encoded[i+xdesc->k], frag_size);
    }
    // free enccoded data
    for(i=0; i < xdesc->n; i++){
        free(encoded[i]);
    }
    return 0;
}

static int null_encode(void *desc, char **data, char **parity, int blocksize)
{
    if(backend_null.skip_preprocess){
        null_naive_encode(desc, data, parity, blocksize);
    }
    return 0;
}

static int null_decode(void *desc, char **data, char **parity,
        int *missing_idxs, int blocksize)
{
    return 0;
}

static int null_reconstruct(void *desc, char **data, char **parity,
        int *missing_idxs, int destination_idx, int blocksize)
{
    return 0;
}

static int null_min_fragments(void *desc, int *missing_idxs,
        int *fragments_to_exclude, int *fragments_needed)
{
    return 0;
}

/**
 * Return the element-size, which is the number of bits stored 
 * on a given device, per codeword.  This is usually just 'w'.
 */
static int
null_element_size(void* desc)
{
    return DEFAULT_W;
}

static void * null_init(struct ec_backend_args *args, void *backend_sohandle)
{
    struct null_descriptor *xdesc = NULL;

    /* allocate and fill in null_descriptor */
    xdesc = (struct null_descriptor *) malloc(sizeof(struct null_descriptor));
    if (NULL == xdesc) {
        return NULL;
    }
    memset(xdesc, 0, sizeof(struct null_descriptor));

    xdesc->k = args->uargs.k;
    xdesc->m = args->uargs.m;
    xdesc->n = xdesc->k + xdesc->m;
    xdesc->w = args->uargs.w;

    if (xdesc->w <= 0)
        xdesc->w = DEFAULT_W;

    /* Sample on how to pass extra args to the backend */
    xdesc->arg1 = args->uargs.priv_args1.null_args.arg1;

    /* store w back in args so upper layer can get to it */
    args->uargs.w = DEFAULT_W;

    /* validate EC arguments */
    {
        long long max_symbols;
        if (xdesc->w != 8 && xdesc->w != 16 && xdesc->w != 32) {
            goto error;
        }
        max_symbols = 1LL << xdesc->w;
        if ((xdesc->k + xdesc->m) > max_symbols) {
            goto error;
        }
    }

    /*
     * ISO C forbids casting a void* to a function pointer.
     * Since dlsym return returns a void*, we use this union to
     * "transform" the void* to a function pointer.
     */
    union {
        init_null_code_func initp;
        null_code_get_fragment_size_func getfragsizep;
        null_code_encode_func encodep;
        null_code_naive_encode_func nencodep;
        null_code_decode_func decodep;
        null_code_naive_decode_func ndecodep;
        null_reconstruct_func reconp;
        null_code_fragments_needed_func fragsneededp;
        void *vptr;
    } func_handle = {.vptr = NULL};

    /* fill in function addresses */
    func_handle.vptr = NULL;
    func_handle.vptr = dlsym(backend_sohandle, "null_code_init");
    xdesc->init_null_code = func_handle.initp;
    if (NULL == xdesc->init_null_code) {
        goto error; 
    }

    func_handle.vptr = NULL;
    func_handle.vptr = dlsym(backend_sohandle, "get_fragment_size");
    xdesc->null_code_get_fragment_size = func_handle.getfragsizep;
    if (NULL == xdesc->null_code_get_fragment_size) {
        goto error;
    }

    func_handle.vptr = NULL;
    func_handle.vptr = dlsym(backend_sohandle, "null_code_encode");
    xdesc->null_code_encode = func_handle.encodep;
    if (NULL == xdesc->null_code_encode) {
        goto error; 
    }
    func_handle.vptr = NULL;
    func_handle.vptr = dlsym(backend_sohandle, "null_code_naive_encode");
    xdesc->null_code_naive_encode = func_handle.nencodep;
    if (NULL == xdesc->null_code_naive_encode) {
        goto error;
    }

    func_handle.vptr = NULL;
    func_handle.vptr = dlsym(backend_sohandle, "null_code_decode");
    xdesc->null_code_decode = func_handle.decodep;
    if (NULL == xdesc->null_code_decode) {
        goto error; 
    }

    func_handle.vptr = NULL;
    func_handle.vptr = dlsym(backend_sohandle, "null_code_naive_decode");
    xdesc->null_code_naive_decode = func_handle.ndecodep;
    if (NULL == xdesc->null_code_naive_decode) {
        goto error;
    }

    func_handle.vptr = NULL;
    func_handle.vptr = dlsym(backend_sohandle, "null_reconstruct");
    xdesc->null_reconstruct = func_handle.reconp;
    if (NULL == xdesc->null_reconstruct) {
        goto error; 
    }

    func_handle.vptr = NULL;
    func_handle.vptr = dlsym(backend_sohandle, "null_code_fragments_needed");
    xdesc->null_code_fragments_needed = func_handle.fragsneededp;
    if (NULL == xdesc->null_code_fragments_needed) {
        goto error; 
    }

    return (void *) xdesc;

error:
    free (xdesc);

    return NULL;
}

static int null_exit(void *desc)
{
    struct null_descriptor *xdesc = (struct null_descriptor *) desc;

    free (xdesc);
    return 0;
}

struct ec_backend_op_stubs null_op_stubs = {
    .INIT                       = null_init,
    .EXIT                       = null_exit,
    .ENCODE                     = null_encode,
    .DECODE                     = null_decode,
    .FRAGSNEEDED                = null_min_fragments,
    .RECONSTRUCT                = null_reconstruct,
    .ELEMENTSIZE                = null_element_size,
};

struct ec_backend_common backend_null = {
    .id                         = EC_BACKEND_NULL,
    .name                       = "null",
#if defined(__MACOS__) || defined(__MACOSX__) || defined(__OSX__) || defined(__APPLE__)
    .soname                     = "libnullcode.dylib",
#else
    .soname                     = "libnullcode.so",
#endif
    .soversion                  = "1.0",
    .ops                        = &null_op_stubs,
    .skip_preprocess            = 1, // if your backend want to use liberasurecode
                                     // preprocessing, set 0 this option.
};

