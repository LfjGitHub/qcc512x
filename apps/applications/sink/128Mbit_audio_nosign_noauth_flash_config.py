#!/usr/bin/env python
# Copyright (c) 2017-2018 Qualcomm Technologies International, Ltd.
# The format of this file is documented in CS-00410685-UG / 80-CG297-1
{
    "flash_device": {
    "block_size": 64 * 1024,
    "boot_block_size": None,
    "alt_image_offset": 128 * 64 * 1024
},
"encrypt": False,
"layout": [
    ("curator_fs",      { "capacity" : 1024,  "authenticate": False, "src_file_signed": False}),
    ("apps_p0",         { "capacity" : 1024 * 768,  "authenticate": False, "src_file_signed": False}),
    ("apps_p1",         { "capacity" : 1024 * 512,  "authenticate": False}),
    ("audio",           { "capacity" : 620 * 1024,  "authenticate": False}),
    ("device_ro_fs",    { "capacity" : 4096,  "authenticate": False, "inline_auth_hash": True }),
    ("rw_config",       { "capacity" : 128 * 1024}),
    ("rw_fs",           { "capacity" : 256 * 1024}),
    ("ro_cfg_fs",       { "capacity" : 64 * 1024,  "authenticate": False}),
    ("ro_fs",           { "capacity" : 2 * 1024 * 1024,"authenticate": False})
]
}
