#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/fs.h>
#include <linux/string.h>
#include <asm/uaccess.h>
#include "xoai.h"
#include "xomain.h"
#include "xoconst.h"

static int ai_turn = TRUE;
static char winner = NO_PLAYER;
static int Major;	
static int Device_Open = 0;
static char msg[BUF_LEN];
static char *msg_Ptr;

static struct file_operations fops = {
	.read = device_read,
	.write = device_write,
	.open = device_open,
	.release = device_release
};

int init_module(void)
{
        Major = register_chrdev(0, DEVICE_NAME, &fops);

	if (Major < 0) {
	  printk(KERN_ALERT "Registering char device failed with %d\n", Major);
	  return Major;
	}

	printk(KERN_INFO "'mknod /dev/%s c %d 0'.\n", DEVICE_NAME, Major);
	init_board();

	return SUCCESS;
}

void cleanup_module(void)
{
	unregister_chrdev(Major, DEVICE_NAME);
}

static int device_open(struct inode *inode, struct file *file)
{
	char wins[WINBUF];

	if (Device_Open)
		return -EBUSY;
	
	Device_Open++;
	if (ai_turn) {
		ai_turn = FALSE;
		if (make_ai_move(PLAYER_X)) 
			winner = PLAYER_X;

	}

	if (ai_two_in_a_row(PLAYER_O) == -1)
		winner = PLAYER_O;

	print_board(msg);

	if (winner != NO_PLAYER) {
		memset(wins,0,sizeof(wins));
		sprintf(wins,"\n %c wins (send '%s' to restart')\n", winner, RESET_COMMAND);
		strcat(msg,wins);
	}

	msg_Ptr = msg;
	try_module_get(THIS_MODULE);

	return SUCCESS;
}

static int device_release(struct inode *inode, struct file *file)
{
	Device_Open--;		
	module_put(THIS_MODULE);

	return 0;
}

static ssize_t device_read(struct file *filp, char *buffer, size_t length, loff_t * offset)
{


	int bytes_read = 0;


	if (*msg_Ptr == 0)
		return 0;

	while (length && *msg_Ptr) {
		put_user(*(msg_Ptr++), buffer++);

		length--;
		bytes_read++;
	}

	return bytes_read;
}

static void reset_game(void)
{
	ai_turn = TRUE;
	winner = NO_PLAYER;
	init_board();
}

static ssize_t device_write(struct file *filp, const char *buff, size_t len, loff_t * off)
{

	if (strstr(buff,RESET_COMMAND)) {
		reset_game();
		return len;
	}

	if (ai_turn == FALSE && make_move(PLAYER_X, simple_strtol(buff, NULL, 10))) {
		ai_turn = TRUE;
		return len;
	}
	else 
		return -1;
}
