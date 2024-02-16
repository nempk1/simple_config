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
	INVALID_ACTION,
	ERROR_PARSER,
	ERROR_FILE,
	ERROR_PATH,
	ERROR,
	DUPLICATED,
	NOT_FOUND,
	CHANGE_OK,
	ADD_OK,
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
	const char 	*file_path;
	size_t 		 lines;
};

/*
 * If receives NULL cfg will allocate
 * and parse the file from file_path
 *
 * @param File path to be parsed
 * @param cfg pointer ref to be used
 *
 * @return VALID on success, else ERROR*
 * 	   and errno set to error code
 * 	   of open or malloc.
 */
extern enum parse 
config_init(const char *, struct s_config**);

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

/*
 * Will free all the memory used by
 * the s_config. It can be reused.
 *
 * On success will set the param to NULL.
 *
 * @param Pointer to s_config,
 */
extern void
config_clear(struct s_config*);

/*
 * Will write the all the data in the param
 * using the 'ideal' syntax to the filepath 
 * store in the cfg struct
 *
 * @param Config struct with the data.
 * 
 * @return VALID on success, else error.
 */
extern enum parse
config_save(struct s_config *);

/*
 * Will write the all the data in the param
 * using the 'ideal' syntax to the filepath 
 *
 * @param Path to file to be written
 * 		      it will override if the file
 * 		      already exists
 *
 * @param Config struct with the data.
 * 
 * @return VALID on success, else error.
 */
extern enum parse
config_save_file(const char*, struct s_config *);

/*
 * Will get the reference to the var struct 
 * according to the var_name in the param
 *
 * @param Variable name to be searched
 *
 * @param Config refer to search.
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
 * @param var name.
 * @param Config to be searched.
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
 * @param var name.
 * @param Config to be searched.
 *
 * @return Pointer if found, else NULL.
 */
extern char *
config_get_string(const char *, struct s_config *);

/* Will ADD or CHANGE variable value.
 *
 * @param var name
 * @param Value to store
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
