#include <fs/initrd.h>

// TODO: way to kfree unneded data

// data of mounted initrd
initrd_header_t* initrd_header;
initrd_file_header_t* file_headers;
fs_node_t* initrd_root; // initrd_root (will become /)
fs_node_t* initrd_dev; // directory node for /dev
fs_node_t* root_nodes;
// TODO: changed to unsigned
uint32_t nroot_nodes;

struct dirent dirent;

static uint32_t initrd_read(fs_node_t* node, uint32_t offset, uint32_t size,
		uint8_t* buffer) {
	initrd_file_header_t header = file_headers[node->inode];
	if (offset > header.length)
		return 0;
	if (offset + size > header.length)
		size = header.length - offset;
	memcpy(buffer, (uint8_t*) (header.offset + offset), size);
	return size;
}

static struct dirent* initrd_readdir(fs_node_t* node, uint32_t index) {
	if (node == initrd_root && index == 0) {
		strcpy(dirent.name, "dev");
		dirent.name[3] = 0;
		dirent.inode = 0;
		return &dirent;
	}
	if (index - 1 >= nroot_nodes)
		return 0;
	strcpy(dirent.name, root_nodes[index - 1].name);
	dirent.name[strlen(root_nodes[index - 1].name)] = 0;
	dirent.inode = root_nodes[index - 1].inode;
	return &dirent;
}

static fs_node_t* initrd_finddir(fs_node_t* node, char* name) {
	if (node == initrd_root && !strcmp(name, "dev"))
		return initrd_dev;
	for (uint32_t i = 0; i < nroot_nodes; i++)
		if (!strcmp(name, root_nodes[i].name))
			return &root_nodes[i];
	return 0;
}

fs_node_t* init_initrd(uint32_t location) {
	initrd_header = (initrd_header_t*) location;
	file_headers = (initrd_file_header_t*) (location + sizeof(initrd_header_t));

	// initialise the root directory.
	initrd_root = (fs_node_t*) kmalloc_zero(sizeof(fs_node_t));
	strcpy(initrd_root->name, "initrd");
	initrd_root->flags = FS_DIRECTORY;
	initrd_root->readdir = &initrd_readdir;
	initrd_root->finddir = &initrd_finddir;

	// initialise the /dev directory
	initrd_dev = (fs_node_t*) kmalloc_zero(sizeof(fs_node_t));
	// TODO there was: first assign name, then memset to zero all fs_node_to
	strcpy(initrd_dev->name, "dev");
	initrd_dev->flags = FS_DIRECTORY;
	initrd_dev->readdir = &initrd_readdir;
	initrd_dev->finddir = &initrd_finddir;

	root_nodes = (fs_node_t*) kmalloc(
			sizeof(fs_node_t) * initrd_header->nfiles);
	// bacuse of this, zeroing in for loop is not necessary
	memset(root_nodes, 0, sizeof(fs_node_t) * initrd_header->nfiles);
	nroot_nodes = initrd_header->nfiles;

	for (uint32_t i = 0; i < nroot_nodes; i++) {
		if (HEADER_MAGIC != file_headers[i].magic)
			panic("Initial ramdisk corrupted.");
		// offset from start of ramdisk + location = offset from begin of memory
		file_headers[i].offset += location;
		// casting: file_headers[i].name is s8int (signed)
		strcpy(root_nodes[i].name, (char*) &file_headers[i].name);
		root_nodes[i].length = file_headers[i].length;
		root_nodes[i].inode = i;
		root_nodes[i].flags = FS_FILE;
		root_nodes[i].read = &initrd_read;
	}
	return initrd_root;
}
