#ifndef SIMPLE_CONFIG
#define SIMPLE_CONFIG

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <ctype.h>
#include <errno.h>
#include <sys/queue.h>

enum var_type {
	INT,
	STRING	
};

enum action {
	CHANGE = 0x1,
	ADD = 0x2,
};

enum parse{
	INVALID_STRING_VALUE,
	INVALID_VAR_SYNTAX,
	INVALID_NUMBER,
	INVALID_VAR_NULL,
	VALID = 0
};

struct var {
	char 		*name;
	union 
	{
		char 		*s_value;
		long 	 	 l_value;
	};
	enum var_type 	 type;
};

struct var_node {
	struct var data;		
	TAILQ_ENTRY(var_node) entries;
};

TAILQ_HEAD(list, var_node);

struct s_config {
	struct list 	var_list;	
	size_t lines;
	FILE *fp;
};

extern enum parse
config_parse_line(char *, struct var *);

extern int
config_parse_file(const char *, struct s_config*);

extern struct s_config *
config_load_file(FILE *);

extern struct s_config *
config_new(void);

extern int
config_init();

extern void
config_destroy(struct s_config **);

extern int
config_reload(struct s_config *);

extern int
config_save(struct s_config *);

extern int
config_save_file(FILE *, struct s_config *);

extern struct var *
config_get_var(const char *, struct s_config *);

extern long 
config_get_int(const char *, struct s_config *);

extern char *
config_get_string(const char *, struct s_config *);

extern int 
config_set_var(const char *, void *data, enum var_type, struct s_config *,
		enum action);

#endif /*SIMPLE_CONFIG*/
