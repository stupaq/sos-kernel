#include <monitor.h>
#include <mm/layout.h>

#define COLUMNS 80
#define LINES 25

// we declared it as word (uint16_t) for easy access to chars with formatting
static uint16_t* video_mem = 0;
// cursor position
static uint8_t xpos = 0;
static uint8_t ypos = 0;
// background colour is black (0), foreground is white (15)
static uint8_t back_color = 0;
static uint8_t fore_color = 15;

// returns uint16_t with set vga attribute byte according to given values
static uint16_t vga_set_attr(uint8_t back, uint8_t fore) {
	// The attribute byte is made up of two nibbles - the lower being the
	// foreground colour, and the upper the background colour.
	uint8_t attr = (back << 4) | (fore & 0x0F);
	// The attribute byte is the top 8 bits of the word we have to send to the
	// VGA board.
	return (attr << 8);
}

static uint16_t vga_format_ascii(uint8_t back, uint8_t fore, char ascii) {
	uint16_t attr = vga_set_attr(back, fore);
	return ascii | attr;
}

static void update_cursor() {
	// The screen is 80 characters wide...
	uint16_t cursorLocation = ypos * 80 + xpos;
	outb(0x3D4, 14); // Tell the VGA board we are setting the high cursor byte.
	outb(0x3D5, cursorLocation >> 8); // Send the high cursor byte.
	outb(0x3D4, 15); // Tell the VGA board we are setting the low cursor byte.
	outb(0x3D5, cursorLocation); // Send the low cursor byte.
}

static void scroll() {
	int i;
	uint16_t blank = vga_format_ascii(back_color, fore_color, 0x20);
	// Row 25 is the end, this means we need to scroll up
	if (ypos >= 25) {
		// Move the current text chunk that makes up the screen
		// back in the buffer by a line
		for (i = 0 * 80; i < 24 * 80; i++)
			video_mem[i] = video_mem[i + 80];

		// The last line should now be blank. Do this by writing
		// 80 spaces to it.
		for (i = 24 * 80; i < 25 * 80; i++)
			video_mem[i] = blank;
		// The cursor should now be on the last line.
		ypos = 24;
	}
}

void init_monitor() {
	video_mem = (uint16_t*) VIDEO_MEM;
	uint16_t blank = vga_format_ascii(back_color, fore_color, 0x20);

	for (int i = 0; i < COLUMNS * LINES; i++)
		video_mem[i] = blank;

	xpos = 0;
	ypos = 0;
	update_cursor();
}

void monitor_put(char c) {
	uint16_t attribute = vga_set_attr(back_color, fore_color);
	uint16_t *location;

	// a backspace
	if (c == 0x08 && xpos)
		xpos--;
	// tab (to point divided by 4)
	else if (c == 0x09)
		xpos = (xpos + 4) & ~(4 - 1);
	// carriage return
	else if (c == '\r')
		xpos = 0;
	// newline
	else if (c == '\n') {
		xpos = 0;
		ypos++;
	}
	// other printable character.
	else if (c >= 0x20) {
		location = video_mem + (ypos * 80 + xpos);
		*location = c | attribute;
		xpos++;
	}

	// if we need to insert a new line
	if (xpos >= 80) {
		xpos = 0;
		ypos++;
	}

	// scroll and update cursor
	scroll();
	update_cursor();
}

void monitor_write(char *s) {
	while (*s)
		monitor_put(*s++);
}
