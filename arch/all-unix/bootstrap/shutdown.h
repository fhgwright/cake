#define SD_ACTION_POWEROFF   0
#define SD_ACTION_COLDREBOOT 1

extern char bootstrapdir[];
extern char *bootstrap_bin;
extern char **bootstrap_args;
extern char cmdline[];

void Host_Shutdown(unsigned long action);
