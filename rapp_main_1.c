/****************************************************************************
 * apps/examples/hello/hello_main.c
 *
 * Licensed to the Apache Software Foundation (ASF) under one or more
 * contributor license agreements.  See the NOTICE file distributed with
 * this work for additional information regarding copyright ownership.  The
 * ASF licenses this file to you under the Apache License, Version 2.0 (the
 * "License"); you may not use this file except in compliance with the
 * License.  You may obtain a copy of the License at
 *
 *   http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS, WITHOUT
 * WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.  See the
 * License for the specific language governing permissions and limitations
 * under the License.
 *
 ****************************************************************************/

/****************************************************************************
 * Included Files
 ****************************************************************************/

#include <nuttx/config.h>
#include <sys/mount.h>
#include <sys/boardctl.h>
#include <nuttx/drivers/ramdisk.h>
#include <stdio.h>
#include <sched.h>
#include <unistd.h>
#include <lua.h>
#include <lualib.h>
#include <lauxlib.h>
#include <errno.h>
#include <fcntl.h>
#include <dirent.h>


#include "fs_api.h"
#include "scripts.h"

/****************************************************************************
 * Public Functions
 ****************************************************************************/

int lua_task_1(int argc, char* argv[])
{
	
    lua_State* L;
    // initialize Lua interpreter
    L = luaL_newstate();
    // load Lua base libraries (print / math / etc)
    luaL_openlibs(L);
	char *lua_cmd_str = "print('hello from lua')";
    printf("Starting lua\n");
	int result = 0;
    if((result = luaL_dostring(L, lua_cmd_str))  != LUA_OK)
    {
        printf("Failed to load or run lua. Got Error code %d\n", result);
        return result * -1;
    }
    
    sleep(1);
//     luaL_dofile(L, "myFile.lua");
     
    // Cleanup:  Deallocate all space assocatated with the lua state */
    lua_close(L);

    return 0;    
}

void open_fs_items(void)
{
	fs_node_t node;
	fs_node_t *scriptsDir;
	scriptsDir = findindirectory(&node, "scripts");
	if(scriptsDir == NULL)
	{
		printf("failed to find scripts dir.\n");
	}
}


static int open_romdisk_2(void)
{
	int ret = 0;
	 ret = romdisk_register(0, // /dev/ram0
                          scripts_img, //var in the xxd header
                          NSECTORS(scripts_img_len),
                          CONFIG_EXAMPLES_ROMFS_SECTORSIZE);
	if (ret < 0)
	{
		printf("Failed to create a romdisk.\n");
		printf("Error Number is %d\n", errno);
		printf("ERROR: Failed to create RAM disk: %s\n", strerror(errno));
		return 1;
	}
   
   ret = nx_mount("/dev/ram0", "/romfs", "romfs", MS_RDONLY, NULL);
   
   if (ret < 0)
    {
		printf("Failed to mount ramdisk.\n");
		printf("Error NUmber is %d\n", errno);
		printf("ERROR: Failed to create RAM disk: %s\n", strerror(errno));
		return 1;
    }
}

static int open_romdisk(void)
{
	int ret = 0;
	struct boardioc_mkrd_s ramdisk;
	struct boardioc_romdisk_s desc;	
	
	ramdisk.minor = CONFIG_EXAMPLES_ROMFS_RAMDEVNO;
	ramdisk.nsectors = NSECTORS(scripts_img_len);
	ramdisk.sectsize = CONFIG_EXAMPLES_ROMFS_SECTORSIZE;

	ret = boardctl(BOARDIOC_MKRD, (uintptr_t)&ramdisk);
	
	if (ret < 0)
    {
		printf("Failed to create a ramdisk.\n");
		printf("Error NUmber is %d\n", errno);
		printf("ERROR: Failed to create RAM disk: %s\n", strerror(errno));
		return 1;
    }
    
	/* Create a RAM disk for the test */
	desc.minor    = CONFIG_EXAMPLES_ROMFS_RAMDEVNO;         /* Minor device number of the ROM disk. */
	desc.nsectors = NSECTORS(scripts_img_len);              /* The number of sectors in the ROM disk */
	desc.sectsize = CONFIG_EXAMPLES_ROMFS_SECTORSIZE;       /* The size of one sector in bytes */
	desc.image    = (FAR uint8_t *)scripts_img;             /* File system image */
	
	ret = boardctl(BOARDIOC_ROMDISK, (uintptr_t)&desc);
	
	
	if (ret < 0)
    {
		printf("Error NUmber is %d\n", errno);
		printf("ERROR: Failed to create ROM disk: %s\n", strerror(errno));
		return 1;
    }
		/* Mount the test file system */

	printf("Mounting ROMFS filesystem at target=%s with source=%s\n",
			CONFIG_EXAMPLES_ROMFS_MOUNTPOINT, MOUNT_DEVNAME);

	ret = mount(MOUNT_DEVNAME, CONFIG_EXAMPLES_ROMFS_MOUNTPOINT, "romfs",
				MS_RDONLY, NULL);
	if (ret < 0)
	{
		printf("Error NUmber is %d\n", errno);
		printf("ERROR: Mount failed: %s\n", strerror(errno));
		return 1;
	}

	return 0;
}
/****************************************************************************
 * main
 ****************************************************************************/

int main(int argc, FAR char *argv[])
{
	printf("Starting Russells App 1...\n");
// 	int res = task_create("LuaTask", 3, 100, &lua_task_1, NULL);
// 	if(res == 0)
// 	{
// 		printf("Failed to create task\n");
// 	}

	printf("Rom Disk Images says: %s\n", scripts_img+139);
// 	open_romdisk();
	open_romdisk_2();
// 	for(int count = 2; count > 0; count--)
// 	{
// 		printf("%d Main thred...\n", count);
// 		sleep(2);
// 	}
	return 0;
}
