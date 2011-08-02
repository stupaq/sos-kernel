#ifndef FS_H
#define FS_H

#include <common.h>

#define FS_FILE        0x01
#define FS_DIRECTORY   0x02
#define FS_CHARDEVICE  0x03
#define FS_BLOCKDEVICE 0x04
#define FS_PIPE        0x05
#define FS_SYMLINK     0x06
#define FS_MOUNTPOINT  0x08 // is the file a mountpoint?

struct fs_node;

// each node have these callbacks (TODO: change to driver pointer interface)
typedef uint32_t (*read_type_t)(struct fs_node*, uint32_t, uint32_t, uint8_t*);
typedef uint32_t (*write_type_t)(struct fs_node*, uint32_t, uint32_t, uint8_t*);
typedef void (*open_type_t)(struct fs_node*);
typedef void (*close_type_t)(struct fs_node*);
typedef struct dirent* (*readdir_type_t)(struct fs_node*, uint32_t);
typedef struct fs_node* (*finddir_type_t)(struct fs_node*, char* name);

typedef struct fs_node {
	char name[128];
	uint32_t mask; // permissions
	uint32_t uid;
	uint32_t gid;
	uint32_t flags;
	uint32_t inode;
	uint32_t length; // size in bytes
	uint32_t impl; // An implementation-defined number.
	read_type_t read;
	write_type_t write;
	open_type_t open;
	close_type_t close;
	readdir_type_t readdir;
	finddir_type_t finddir;
	struct fs_node* ptr; // used by mountpoints and symlinks
} fs_node_t;

typedef struct dirent {
	char name[128];
	uint32_t inode;
} dirent_t;

extern fs_node_t* fs_root; // The root of the filesystem.

// TODO: fucked up when it comes to implement file descriptors
uint32_t read(fs_node_t* node, uint32_t offset, uint32_t size, uint8_t* buffer);
uint32_t write(fs_node_t* node, uint32_t offset, uint32_t size, uint8_t* buffer);
void open(fs_node_t* node, uint8_t read, uint8_t write);
void close(fs_node_t* node);
struct dirent* readdir(fs_node_t* node, uint32_t index);
fs_node_t* finddir(fs_node_t* node, char* name);

#endif
