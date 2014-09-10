/* 
 * <Copyright>
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
 * null EC backend
 *
 * vi: set noai tw=79 ts=4 sw=4:
 */

#include <stdlib.h>
#include <stdint.h>
#include <string.h>

/* calls required for init */
void* null_code_init(int k, int m, int hd)
{
    /* add your code here */
    return NULL;
}

/* calls required for encode */
int null_code_encode(void *code_desc, char **data, char **parity,
        int blocksize)
{
    /* add your code here */
    return 0;
}

int get_fragment_size(void *code_desc, int k, int m, int orig_size)
{
    /* add your code here */
    // This sample assumes non-padding fragments
    return orig_size / k;
}

int null_code_naive_encode(void *code_desc, char *data, char **encoded,
        int k, int m, int blocksize, int orig_size)
{
    /* add your code here */
    // This sample introduces naive copy to encoded data
    int i, payload_size, expected_size;
    payload_size = expected_size = 0;

    for(i=0; i<k; i++){
        expected_size = payload_size + blocksize;
        if(expected_size > orig_size) blocksize = expected_size - orig_size;
        memcpy(encoded[i], data+payload_size, blocksize);
        payload_size += blocksize;
    }

    for(i=k; i<k+m; i++){
        memset(encoded[i], 0, blocksize);
    }

    return 0;
}

/* calls required for decode */
int null_code_decode(void *code_desc, char **data, char **parity,
        int *missing_idxs, int blocksize, int decode_parity)
{
    /* add your code here */
    return 0;
}

int null_code_naive_decode(void *code_desc, char **encoded, char *out,
        int blocksize)
{
    /* add your code here */
    return 0;
}

/* calls required for reconstruct */
int null_reconstruct(char **available_fragments, int num_fragments,
        uint64_t fragment_len, int destination_idx, char* out_fragment)
{
    /* add your code here */
    return 0;
}

/* set of fragments needed to reconstruct at a minimum */
int null_code_fragments_needed(void *code_desc, int *missing_idxs,
        int *fragments_needed)
{
    /* add your code here */
    return 0;
}

