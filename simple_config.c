#include "simple_config.h"

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


/*
 * Parse line to an var, the line
 * will have the start & end trimed
 *
 * @param char* line to be parsed
 * @param struct var* poin to store 
 * result
 *
 * @return VALID on success else error.
 */
static enum parse
config_parse_line(char *org_line, struct var *res)
{
	char *tmp = NULL;
	char *ptr_name = NULL;
	char *ptr_value = NULL;
	char *ptr_line = org_line;		

	if(!res)
		return INVALID_VAR_NULL;

	ptr_line = _trim(ptr_line);

	if (!strncmp("//",ptr_line, sizeof("//")-1)) {
		res->raw_line = strdup(org_line);
		if (!res->raw_line)
			return ERROR_PARSER;
		else {
			res->name = NULL;
			res->type = COMMENT;
			res->s_value = NULL;
			return VALID;
		}

	}

	if (*ptr_line == '\0') {
		res->raw_line = strdup(org_line);
		if (!res->raw_line)
			return ERROR_PARSER;
		else {
			res->name = NULL;
			res->type = COMMENT;
			res->s_value = NULL;
			return VALID;
		}
		return VALID;
	}

	ptr_name = strsep(&ptr_line, "=");
	if (!ptr_line) {
		return INVALID_VAR_SYNTAX;
	}
	ptr_value = ptr_line;
	ptr_name = _rtrim(ptr_name);
	ptr_value = _ltrim(ptr_value);

	if (*ptr_value == '\"') {
		ptr_value++;
		tmp = ptr_value;
		ptr_value = strsep(&tmp, "\"");
		if(!tmp) {
			return INVALID_STRING_VALUE;
		}
		res->type = STRING;
		res->s_value = strdup(ptr_value);
	} else { 
		res->type = INT;
		res->l_value = strtol(ptr_value, NULL, 10);	
		if(errno == ERANGE) {
			return INVALID_NUMBER;
		}
	}

	res->name = strdup(ptr_name);
	return VALID;	
}

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
static enum parse
config_parse_file(const char *path, struct s_config *res)
{
	char *line = NULL;
	size_t len = 0;
	struct var_node *var;
	FILE *fp = fopen(path, "r");

	if (!fp)
		return ERROR_FILE;

	res->lines = 0;

	while ((getline(&line, &len, fp)) != -1) {
		var = malloc(sizeof(*var));	
		if(config_parse_line(line, &var->data)) {
			free(var);
			continue;
		} 
		TAILQ_INSERT_TAIL(&res->var_list, var, entries);
		res->lines++;
	} 
	free(line);
	fclose(fp);
	return VALID;
}


enum parse 
config_init(const char *file_path, struct s_config **cfg)
{
	enum parse result;
	if (!(*cfg)) { /* If NULL */
		(*cfg) = malloc(sizeof(struct s_config));

		if (!(*cfg)) /* If NULL = Not enough memory*/
			return ERROR;
	} 
	TAILQ_INIT(&(*cfg)->var_list);		
	(*cfg)->lines = 0;
	(*cfg)->file_path = strdup(file_path);

	/* If result != VALID return the error from 
	 * config_parse_file
	 */
	if((result = config_parse_file(file_path, *cfg)))
		return result;

	return VALID;
}

void config_clear(struct s_config *cfg) 
{
	struct var_node *n1 = NULL;
	while ((n1 = TAILQ_FIRST(&cfg->var_list)))
 	{		
		TAILQ_REMOVE(&cfg->var_list,n1, entries);

		if(n1->data.type == STRING)
			free(n1->data.s_value);
		else if (n1->data.type == COMMENT) {
			free(n1->data.raw_line);	
			free(n1);
			continue;
		}

		free(n1->data.name);
		free(n1);
	}	
	free((void*)cfg->file_path);
	cfg->file_path = NULL;
	cfg->lines = 0;
}

void 
config_destroy(struct s_config **cfg) 
{
	struct var_node *n1 = NULL;
	while ((n1 = TAILQ_FIRST(&(*cfg)->var_list)))
 	{		
		TAILQ_REMOVE(&(*cfg)->var_list,n1, entries);
		if(n1->data.type == STRING)
			free(n1->data.s_value);
		free(n1->data.name);
		free(n1);
	}	
	free(*cfg);
	(*cfg) = NULL;
}

long 
config_get_int(const char *var_name, struct s_config *cfg)
{
	struct var_node *tmp = NULL;
	TAILQ_FOREACH(tmp, &cfg->var_list, entries) {
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
	TAILQ_FOREACH(tmp, &cfg->var_list, entries) {
		if(tmp->data.type == STRING &&
			!strcmp(tmp->data.name, var_name))
			return tmp->data.s_value;
	}
	return NULL;
}

struct var *
config_get_var(const char *var_name, struct s_config *cfg)
{	struct var_node *tmp = NULL;
	TAILQ_FOREACH(tmp, &cfg->var_list, entries) {
		if(tmp->data.type == STRING &&
			!strcmp(tmp->data.name, var_name))
			return &tmp->data;
	}
	return NULL;
}


enum parse
config_set_var(const char *var_name, void *data, enum var_type type,
	       	struct s_config *cfg, enum action action) 
{
	struct var_node *tmp = NULL;
	struct var_node *var = NULL;
	int found = 0;

	if(!data)
		return INVALID_VALUE;

	TAILQ_FOREACH(tmp, &cfg->var_list, entries) {
		if(tmp->data.type == COMMENT ||
				strcmp(tmp->data.name, var_name)) {
			continue;
		} else {
			found = 1;
			break;
		}
	}
	if (action == (CHANGE | ADD)) {
		if(found) {
			if (type == STRING) {
				char *res_str = strdup(data);
				if(tmp->data.type == STRING)
					free(tmp->data.s_value);
				tmp->data.s_value = res_str;
			} else if (type == INT) {
				if(tmp->data.type == STRING)
					free(tmp->data.s_value);
				tmp->data.type = INT;
				tmp->data.l_value = *(long*)data;
			} else {
				return INVALID_TYPE;	
			}
			return CHANGE_OK;
		} else {
			var = malloc(sizeof(*var));	
			if(!var)
				return ERROR;

			var->data.name = strdup(var_name);

			if (type == COMMENT) {
				char *res_str = strdup(data);
				var->data.raw_line = res_str;
				var->data.type = COMMENT;
			} else if (type == STRING) {
				char *res_str = strdup(data);
				var->data.s_value= res_str;
				var->data.type = STRING;
			} else if (type == INT) {
				var->data.type = INT;
				var->data.l_value = *(long*)data;
			} else {
				free(var);
				return INVALID_TYPE;	
			}
			TAILQ_INSERT_TAIL(&cfg->var_list, var, entries);
			cfg->lines++;
			return ADD_OK;
		}		
	} else if (action == CHANGE) {
		if(found) {
			if (type == STRING) {
				char *res_str = strdup(data);
				if(tmp->data.type == STRING)
					free(tmp->data.s_value);
				tmp->data.s_value = res_str;
			} else if (type == INT) {
				if(tmp->data.type == STRING)
					free(tmp->data.s_value);
				tmp->data.type = INT;
				tmp->data.l_value = *(long*)data;
			} else {
				return INVALID_TYPE;	
			}
			return CHANGE_OK;
		} 
		return NOT_FOUND;	
	} else if (action == ADD) {
		if(found) {
			return DUPLICATED;	
		}
		var = malloc(sizeof(*var));	
		var->data.name = strdup(var_name);
		if(!var)
			return ERROR;

		if (type == COMMENT) {
			char *res_str = strdup(data);		
			var->data.raw_line = res_str;
			var->data.type = COMMENT;
		} else if (type == STRING) {
			char *res_str = strdup(data);		
			var->data.s_value= res_str;
			var->data.type = STRING;
		} else if (type == INT) {
			var->data.type = INT;
			var->data.l_value = *(long*)data;
		} else {
			free(var);
			return INVALID_TYPE;	
		}
		TAILQ_INSERT_TAIL(&cfg->var_list, var, entries);
		cfg->lines++;
		return ADD_OK;

	} 
	return INVALID_ACTION;	
}

enum parse
config_save_file(const char *pathname, struct s_config *cfg)
{
	struct var_node *tmp = NULL;
	FILE *fp = fopen(pathname, "w");

	if(!fp)
		return ERROR_FILE;

	TAILQ_FOREACH(tmp, &cfg->var_list, entries) {
		if (tmp->data.type == COMMENT)
			fprintf(fp, "%s\n", tmp->data.raw_line);
		else if (tmp->data.type == STRING) {
			fprintf(fp, "%s = \"%s\"\n",
				tmp->data.name,
				tmp->data.s_value);
		} else if (tmp->data.type == INT) {
			fprintf(fp, "%s = %ld\n", 
				tmp->data.name,
				tmp->data.l_value);
		}
	}

	fclose(fp);
	return VALID;
}

enum parse
config_save(struct s_config *cfg)
{
	struct var_node *tmp = NULL;

	if(!cfg->file_path)
		return ERROR_PATH;

	FILE *fp = fopen(cfg->file_path, "w");

	if(!fp)
		return ERROR_FILE;

	TAILQ_FOREACH(tmp, &cfg->var_list, entries) {
		if (tmp->data.type == COMMENT)
			fprintf(fp, "%s\n", tmp->data.raw_line);
		else if (tmp->data.type == STRING) {
			fprintf(fp, "%s = \"%s\"\n",
				tmp->data.name,
				tmp->data.s_value);
		} else if (tmp->data.type == INT) {
			fprintf(fp, "%s = %ld\n", 
				tmp->data.name,
				tmp->data.l_value);
		}
	}

	fclose(fp);
	return VALID;
}
