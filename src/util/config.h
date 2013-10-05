#ifndef SYS_CONFIG_H
#define SYS_CONFIG_H

void config_default();

void config_save_local();
int config_load_local();
void config_save_global();
int config_load_global();

#endif
