#include <sys/stat.h>
#include <string.h>
#include <stdio.h>
#include <fcntl.h>
#include <sys/mman.h>

#include "fs_api.h"


static int g_nerrors = 0;
static char g_scratchbuffer[SCRATCHBUFFER_SIZE];

/****************************************************************************
 * Name: findindirectory
 ****************************************************************************/

fs_node_t *findindirectory(fs_node_t *entry, const char *name)
{
  for (; entry; entry = entry->peer)
    {
      if (!entry->found && strcmp(entry->name, name) == 0)
        {
          entry->found = true;
          return entry;
        }
    }

  return NULL;
}


/****************************************************************************
 * Name: checkattributes
 ****************************************************************************/

void checkattributes(const char *path, mode_t mode, size_t size)
{
  struct stat buf;
  int ret;

  ret = stat(path, &buf);
  if (ret != 0)
    {
      printf("  -- ERROR: Failed to stat %s: %d\n", path, errno);
      g_nerrors++;
      return;
    }

  if (mode != buf.st_mode)
    {
      printf("  -- ERROR: Expected mode %08x, got %08x\n", mode,
             buf.st_mode);
      g_nerrors++;
    }

  if (size != buf.st_size)
    {
      printf("  -- ERROR: Expected size %zu, got %ju\n", size,
             (uintmax_t)buf.st_size);
      g_nerrors++;
    }
}


/****************************************************************************
 * Name: checkfile
 ****************************************************************************/

void checkfile(const char *path, fs_node_t *node)
{
  ssize_t nbytesread;
  char *filedata;
  int fd;

  /* Open the file */

  fd = open(path, O_RDONLY);
  if (fd < 0)
    {
      printf("  -- ERROR: Failed to open %s: %d\n", path, errno);
      g_nerrors++;
      return;
    }

  /* Read and verify the file contents */

  nbytesread = read(fd, g_scratchbuffer, SCRATCHBUFFER_SIZE);
  if (nbytesread < 0)
    {
      printf("  -- ERROR: Failed to read from %s: %d\n", path, errno);
      g_nerrors++;
    }
  else if (nbytesread != node->size)
    {
      printf("  -- ERROR: Read %ld bytes, expected %lu\n",
             (long)nbytesread, (unsigned long)node->size);
      g_nerrors++;
    }
  else if (memcmp(g_scratchbuffer, node->u.filecontent, node->size) != 0)
    {
      g_scratchbuffer[nbytesread] = '\0';
      printf("  -- ERROR: File content read does not match expectation:\n");
      printf("  --        Read:     [%s]\n", g_scratchbuffer);
      printf("  --        Expected: [%s]\n", node->u.filecontent);
      g_nerrors++;
    }

  /* Memory map and verify the file contents */

  filedata = (char *)mmap(NULL, node->size, PROT_READ, MAP_SHARED | MAP_FILE,
                          fd, 0);
  if (!filedata || filedata == (char *)MAP_FAILED)
    {
      printf("  -- ERROR: mmap of %s failed: %d\n", path, errno);
      g_nerrors++;
    }
  else
    {
      if (memcmp(filedata, node->u.filecontent, node->size) != 0)
        {
          memcpy(g_scratchbuffer, filedata, node->size);
          g_scratchbuffer[node->size] = '\0';
          printf("  -- ERROR: Mapped file content read does not match "
                 "expectation:\n");
          printf("  --        Memory:   [%s]\n", filedata);
          printf("  --        Expected: [%s]\n", node->u.filecontent);
          g_nerrors++;
        }

      munmap(filedata, node->size);
    }

  /* Close the file */

  if (close(fd) != OK)
    {
      printf("  -- ERROR: Failed to close %s: %d\n", path, errno);
      g_nerrors++;
    }
}

/****************************************************************************
 * Name: readdirectories
 ****************************************************************************/

void readdirectories(const char *path, fs_node_t *entry)
{
  DIR *dirp;
  fs_node_t *node;
  struct dirent *direntry;
  char *fullpath;

  printf("Traversing directory: %s\n", path);
  dirp = opendir(path);
  if (!dirp)
    {
      printf("  ERROR opendir(\"%s\") failed: %d\n", path, errno);
      g_nerrors++;
      return;
    }

  for (direntry = readdir(dirp); direntry; direntry = readdir(dirp))
    {
      if (strcmp(direntry->d_name, ".") == 0 ||
          strcmp(direntry->d_name, "..") == 0)
        {
           printf("  Skipping %s\n", direntry->d_name);
           continue;
        }

      node = findindirectory(entry, direntry->d_name);
      if (!node)
        {
          printf("  ERROR: No node found for %s\n", direntry->d_name);
          g_nerrors++;
          continue;
        }

      /* Get the full path to the entry */

      sprintf(g_scratchbuffer, "%s/%s", path, direntry->d_name);
      fullpath = strdup(g_scratchbuffer);

      if (DIRENT_ISDIRECTORY(direntry->d_type))
        {
          printf("  DIRECTORY: %s/\n", fullpath);
          if (!node->directory)
            {
              printf("  -- ERROR: Expected type directory\n");
              g_nerrors++;
            }
          else
            {
              checkattributes(fullpath, node->mode, 0);
              readdirectories(fullpath, node->u.child);
              printf("Continuing directory: %s\n", path);
            }
        }
      else if (!DIRENT_ISLINK(direntry->d_type))
        {
          printf("  FILE: %s/\n", fullpath);
          if (node->directory)
            {
              printf("  -- ERROR: Expected type file\n");
              g_nerrors++;
            }
          else
            {
              checkattributes(fullpath, node->mode, node->size);
              checkfile(fullpath, node);
            }
        }

      free(fullpath);
    }

  closedir(dirp);
}

/****************************************************************************
 * Name: checkdirectories
 ****************************************************************************/
void checkdirectories(fs_node_t *entry)
{
  for (; entry; entry = entry->peer)
    {
      if (!entry->found)
        {
          printf("ERROR: %s never found\n", entry->name);
          g_nerrors++;
        }

      if (entry->directory)
        {
          checkdirectories(entry->u.child);
        }
    }
}

