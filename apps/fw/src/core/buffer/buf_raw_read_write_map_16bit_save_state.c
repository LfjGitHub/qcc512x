/* Copyright (c) 2016 Qualcomm Technologies International, Ltd. */
/*   Part of 6.3.0 */
/**
 * \file 
 * Contains the corresponding function.
 */

#include "buffer/buffer_private.h"

uint16 *
buf_raw_read_write_map_16bit_save_state(BUFFER *buf, buf_mapping_state *save_state)
{
    buf_save_state( save_state );
    mmu_read_port_open(buf->handle);
    mmu_write_port_open(buf->handle);
    (void) mmu_read_port_map_16bit_le(buf->handle, buf->outdex);
    return mmu_write_port_map_16bit_le(buf->handle, buf->outdex);
}
