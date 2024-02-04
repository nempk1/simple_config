#include "simple_config.h"
#include <sys/queue.h>

static char *_ltrim(char *s)
{
    	while(isspace(*s))
	    	s++;
    	return s;
}

static char *_rtrim(char *s)
{
	char* back = s + strlen(s);

	/* Loop backwards until 
	 * non space character
	 */
    	while(isspace(*--back));
		

	/* New 'end' set */
	*(back+1) = '\0';
	return s;
}

static char *_trim(char *s)
{
    	return _rtrim(_ltrim(s));
}


enum parse
config_parse_line(char *org_line, struct var *res)
{
	if(!res)
		return INVALID_VAR_NULL;

	char *tmp = NULL;
	char *ptr_name = NULL;
	char *ptr_value = NULL;
	char *ptr_line = org_line;		
	ptr_line = _trim(ptr_line);

	ptr_name = strsep(&ptr_line, "=");
	if(!ptr_line) {
		return INVALID_VAR_SYNTAX;
	}
	ptr_value = ptr_line;
	ptr_name = _rtrim(ptr_name);
	ptr_value = _ltrim(ptr_value);

	if(*ptr_value == '\"') {
		ptr_value++;
		tmp = ptr_value;
		ptr_value = strsep(&tmp, "\"");
		if(!tmp) {
			return INVALID_STRING_VALUE;
		}
		res->type = STRING;
		res->s_value = strdup(ptr_value);
	}
	else { 
		res->type = INT;
		res->l_value = strtol(ptr_value, NULL, 10);	
		if(errno == ERANGE) {
			return INVALID_NUMBER;
		}
	}

	res->name = strdup(ptr_name);
	return VALID;	
}

int
config_parse_file(const char *path, struct s_config *res)
{
	char *line = NULL;
	size_t len = 0;
	size_t length;
	struct var_node *var;
	FILE *fp = fopen(path, "r");

	while ((length = getline(&line, &len, fp)) != -1) {
		var = malloc(sizeof(*var));	
		if(config_parse_line(line, &var->data)) {
			free(var);
			continue;
		} 
		LIST_INSERT_HEAD(&res->var_list, var, entries);
	} 
	free(line);
	fclose(fp);
	return 0;
}

struct s_config *
config_new(void)
{
	struct s_config *result = malloc(sizeof(*result));
	LIST_INIT(&result->var_list);
	return result;
}

long 
config_get_int(const char *var_name, struct s_config *cfg)
{
	struct var_node *tmp = NULL;
	LIST_FOREACH(tmp, &cfg->var_list, entries) {
		if(tmp->data.type == INT &&
			!strcmp(tmp->data.name, var_name))
			return tmp->data.l_value;
	}
	return 0;
}

char *
config_get_string(const char *var_name, struct s_config *cfg)
{
	struct var_node *tmp = NULL;
	LIST_FOREACH(tmp, &cfg->var_list, entries) {
		if(tmp->data.type == STRING &&
			!strcmp(tmp->data.name, var_name))
			return tmp->data.s_value;
	}
	return NULL;
}

void config_destroy(struct s_config **cfg) 
{
	struct var_node *n1 = NULL;
	while (!LIST_EMPTY(&(*cfg)->var_list)) {		
		n1 = LIST_FIRST(&(*cfg)->var_list);
		LIST_REMOVE(n1, entries);
		if(n1->data.type == STRING)
			free(n1->data.s_value);
		free(n1->data.name);
		free(n1);
	}	
	free(*cfg);
	(*cfg) = NULL;
}
