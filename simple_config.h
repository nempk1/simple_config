#ifndef SIMPLE_CONFIG
#define SIMPLE_CONFIG

#include <sys/queue.h>

#include <ctype.h>
#include <err.h>
#include <errno.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>

enum var_type {
	COMMENT,
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
	INVALID_TYPE,
	INVALID_VALUE,
	ERROR_PARSER,
	ERROR,
	CHANGE_OK,
	ADD_OK,
	NOT_FOUND,
	VALID = 0
};

struct var {
	char 		*raw_line;
	char 		*name;
	union 
	{
		char 		*s_value;
		long 	 	 l_value;
	};
	enum var_type 	 type;
};

struct var_node {
	struct var 		data;		
	TAILQ_ENTRY(var_node) 	entries;
};

TAILQ_HEAD(list, var_node);

struct s_config {
	struct list 	 var_list;	
	FILE 		*fp;
	size_t 		 lines;
};

/*
 * Parse line to an var, the line
 * will have the start, end trimed
 *
 * @param char* line to be parsed
 * @param struct var* poin to store 
 * result
 *
 * @return VALID on success else error.
 */
extern enum parse
config_parse_line(char *, struct var *);

/*
 * This function uses the config_parse_line
 * basicale stores every line in 
 * the struct s_config param that at the end
 * will have buffered all the lines from the file
 * to be edited.
 *
 * @param const char* Path off file to be parsed
 *                     
 * @param struct s_config* Buffer of loaded and 
 *                         parsed files
 *
 * @return 0 on successs, else error;
 */
extern int
config_parse_file(const char *, struct s_config*);

/*
 * Simple allocate struct s_config
 * and set some of the initial 
 * value.
 *
 * @return Pointer to s_config on success,
 *         NULL on error.
 */
extern struct s_config *
config_new(void);

/*
 * Will free all the memory used by
 * the s_config even the struct itself.
 *
 * On success will set the param to NULL.
 *
 * @param Pointer to s_config * pointer,
 */
extern void
config_destroy(struct s_config **);

// TODO
extern int
config_reload(struct s_config *);

// TODO
extern int
config_save(struct s_config *);

/*
 * Will write the all the data in the param
 * using the 'ideal' syntax to the filepath 
 *
 * @param const char* Path to file to be written
 * 		      it will override if the file
 * 		      already exists
 *
 * @param struct s_config* Config struct with the data.
 * 
 * @return VALID on success, else error.
 */
extern enum parse
config_save_file(const char*, struct s_config *);

/*
 * Will get the reference to the var struct 
 * according to the var_name in the param
 *
 * @param const char* Variable name to be searched
 *
 * @param struct s_config* Config refer to search.
 * 
 * @return Reference the var on success,
 *         else NULL.
 */
extern struct var *
config_get_var(const char *, struct s_config *);

/*
 * Will search for the var name and
 * return value if found.
 *
 * @param const char* var name.
 * @param struct s_config* Config to be searched.
 *
 * @return Value if found, else 0.
 * 	   Theres no way to know if your var
 * 	   have 0 in it or if didnt found.
 */
extern long 
config_get_int(const char *, struct s_config *);

/*
 * Will search for the var name and 
 * return pointer to string if found.
 *
 * @param const char* var name.
 * @param struct s_config* Config to be searched.
 *
 * @return Pointer if found, else NULL.
 */
extern char *
config_get_string(const char *, struct s_config *);

/* Will ADD or CHANGE variable value.
 *
 * @param var name
 * @param value
 * @param specifie type of var
 * @param Config reference where to store
 * @param ADD or CHANGE, never both. (for now)
 *
 * @return CHANGE_OK or ADD_OK on success,
 *         else ERROR.
 */
extern enum parse
config_set_var(const char *, void *, enum var_type, struct s_config *,
		enum action);

#endif /*SIMPLE_CONFIG*/
