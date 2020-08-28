/*

  Naive Memory Allocator   v.1.0  (C)  By Filippo Bergamasco  2014

  Usage:

    1) Set the memory area to be used by the allocator:
        nmalloc_set_memory_area(  pBuff, size );

    2) Call
        nmalloc_malloc( size )
       to obtain a memory chunk of size "size"

    3) When an allocated chunk "a" is no longer needed, call
        nmalloc_free( &a );

        the pointer a is automatically set to 0.


 ----------------------------------------------------------------------------

    Copyright 2014 Filippo Bergamasco

    Licensed under the Apache License, Version 2.0 (the "License");
    you may not use this file except in compliance with the License.
    You may obtain a copy of the License at

        http://www.apache.org/licenses/LICENSE-2.0

    Unless required by applicable law or agreed to in writing, software
    distributed under the License is distributed on an "AS IS" BASIS,
    WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
    See the License for the specific language governing permissions and
    limitations under the License.

 ----------------------------------------------------------------------------
*/

#ifndef _NMALLOC_000_H
#define _NMALLOC_000_H


/* size_T datatype, used as an unsigned integer to keep track of memory
 * block size. This type can be modified as needed by the target architecture
 */
typedef unsigned int size_T;


/* Use the following definition to enable debug (it needs printf function) */

//#define NMALLOC_DEBUG




extern void  nmalloc_set_memory_area( void* pBuff, size_T max_size );
extern void* nmalloc_malloc( size_T size );
extern void  nmalloc_free( void* ptr );


/* The following functions are available only in debug mode */
#ifdef NMALLOC_DEBUG

extern void nmalloc_print_blocks( void );
extern size_T nmalloc_free_space( void );
extern size_T nmalloc_num_free_blocks( void );

#endif

#endif
