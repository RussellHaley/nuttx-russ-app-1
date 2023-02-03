#ifndef RUSSAPP_FS_API_H
#define RUSSAPP_FS_API_H

#include <stdint.h>
#include <stddef.h>
#include <stdbool.h>

 
#define WRITABLE_MODE      (S_IWOTH|S_IWGRP|S_IWUSR)
#define READABLE_MODE      (S_IROTH|S_IRGRP|S_IRUSR)
#define EXECUTABLE_MODE    (S_IXOTH|S_IXGRP|S_IXUSR)

#define DIRECTORY_MODE     (S_IFDIR|READABLE_MODE|EXECUTABLE_MODE)
#define FILE_MODE          (S_IFREG|READABLE_MODE)


/* Configuration settings */

#ifndef CONFIG_EXAMPLES_ROMFS_RAMDEVNO
#  define CONFIG_EXAMPLES_ROMFS_RAMDEVNO 1
#endif

#ifndef CONFIG_EXAMPLES_ROMFS_SECTORSIZE
#  define CONFIG_EXAMPLES_ROMFS_SECTORSIZE 64
#endif

#ifndef CONFIG_EXAMPLES_ROMFS_MOUNTPOINT
#  define CONFIG_EXAMPLES_ROMFS_MOUNTPOINT "/usr/local/share"
#endif

#ifdef CONFIG_DISABLE_MOUNTPOINT
#  error "Mountpoint support is disabled"
#endif

#ifndef CONFIG_FS_ROMFS
#  error "ROMFS support not enabled"
#endif

#define NSECTORS(b)        (((b)+CONFIG_EXAMPLES_ROMFS_SECTORSIZE-1)/CONFIG_EXAMPLES_ROMFS_SECTORSIZE)
#define STR_RAMDEVNO(m)    #m
#define MKMOUNT_DEVNAME(m) "/dev/ram" STR_RAMDEVNO(m)
#define MOUNT_DEVNAME      MKMOUNT_DEVNAME(CONFIG_EXAMPLES_ROMFS_RAMDEVNO)

#define SCRATCHBUFFER_SIZE 1024


/****************************************************************************
 * Private Types
 ****************************************************************************/

typedef struct fs_node_s
{
	struct fs_node_s *peer;      /* Next node in this directory */
	bool           directory; /* True: directory */
	bool           found;     /* True: found and verified */
	const char    *name;      /* Node name */
	mode_t         mode;      /* Expected permissions */
	size_t         size;      /* Expected size */
	union
    {
      const char    *filecontent; /* Context of text file */
	  struct fs_node_s *child;       /* Subdirectory start */
    } u;
}fs_node_t;


fs_node_t *findindirectory( fs_node_t *entry, const char *name);
void checkattributes(const char *path, mode_t mode, size_t size);
void checkfile(const char *path,  fs_node_t *node);
void readdirectories(const char *path,  fs_node_t *entry);
void checkdirectories( fs_node_t *entry);

#endif //RUSSAPP_FS_API_H
