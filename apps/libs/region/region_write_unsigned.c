/*******************************************************************************
Copyright (c) 2015 Qualcomm Technologies International, Ltd.
 Part of 6.3.0
*******************************************************************************/

#include "region.h"

void RegionWriteUnsigned(const Region *r, uint32 value)
{
    uint8 *p = (uint8 *) (r->end);
    while(p != r->begin)
    {
        *--p = (uint8) (value & 0xFF);
        value >>= 8;
    }
}

