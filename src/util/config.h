#ifndef SYS_CONFIG_H
#define SYS_CONFIG_H

void config_save(const char *path);
int config_load(const char *path);
void config_default();

#endif
