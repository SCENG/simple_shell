#include "shell.h"

int shellby_env(char **args, char __attribute__((__unused__)) **front);
int shellby_setenv(char **args, char __attribute__((__unused__)) **front);
int shellby_unsetenv(char **args, char __attribute__((__unused__)) **front);

/**
* shellby_env - Prints the current environment.
* @args: An array of arguments passed to the shell
* @front: A double pointer to the beginning of args
*
* Return: If an error occurs - -1.
*	   Otherwise - 0.
*
* Description: Prints one variable per line in the
*              format 'variable'='value'.
*/
int shellby_env(char **args, char __attribute__((__unused__)) **front)
{
int index;
char nc = '\n';
if (!environ)
return (-1);
for (index = 0; environ[index]; index++)
{
write(STDOUT_FILENO, environ[index], _strlen(environ[index]));
write(STDOUT_FILENO, &nc, 1);
}
(void)args;
return (0);
}

/**
* shellby_setenv - Changes or adds an environmental variable to the PATH.
* @args: An array of arguments passed to the shell.
* @front: A double pointer to the beginning of args.
* Description: args[1] is the name of the new or existing PATH variable.
*              args[2] is the value to set the new or changed variable to.
*
* Return: If an error occurs - -1.
*         Otherwise - 0.
*/
int shellby_setenv(char **args, char __attribute__((__unused__)) **front)
{
char **env_var = NULL, **new_environ, *new_value;
size_t size;
int index;
if (!args[0] || !args[1])
return (create_error(args, -1));
new_value = malloc(_strlen(args[0]) + 1 + _strlen(args[1]) + 1);
if (!new_value)
return (create_error(args, -1));
_strcpy(new_value, args[0]);
_strcat(new_value, "=");
_strcat(new_value, args[1]);
env_var = _getenv(args[0]);
if (env_var)
{
free(*env_var);
*env_var = new_value;
return (0);
}
for (size = 0; environ[size]; size++)
;
new_environ = malloc(sizeof(char *) * (size + 2));
if (!new_environ)
{
free(new_value);
return (create_error(args, -1));
}
for (index = 0; environ[index]; index++)
new_environ[index] = environ[index];
free(environ);
environ = new_environ;
environ[index] = new_value;
environ[index + 1] = NULL;
return (0);
}

/**
* shellby_unsetenv - Deletes an environmental variable from the PATH.
* @args: An array of arguments passed to the shell.
* @front: A double pointer to the beginning of args.
* Description: args[1] is the PATH variable to remove.
*
* Return: If an error occurs - -1.
*         Otherwise - 0.
*/
int shellby_unsetenv(char **args, char __attribute__((__unused__)) **front)
{
char **env_var, **new_environ;
size_t size;
int index, index2;
if (!args[0])
return (create_error(args, -1));
env_var = _getenv(args[0]);
if (!env_var)
return (0);
for (size = 0; environ[size]; size++)
;
new_environ = malloc(sizeof(char *) * size);
if (!new_environ)
return (create_error(args, -1));
for (index = 0, index2 = 0; environ[index]; index++)
{
if (*env_var == environ[index])
{
free(*env_var);
continue;
}
new_environ[index2] = environ[index];
index2++;
}
free(environ);
environ = new_environ;
environ[size - 1] = NULL;
return (0);
}



#include "shell.h"

int cant_open(char *file_path);
int proc_file_commands(char *file_path, int *exe_ret);

/**
 * cant_open - If the file doesn't exist or lacks proper permissions, print
 * a cant open error.
 * @file_path:Path to the supposed file.
 *
 * Return:127.
 */

int cant_open(char *file_path)
{
	char *error, *hist_str;
	int len;

	hist_str = _itoa(hist);
	if (!hist_str)
		return (127);

	len = _strlen(name) + _strlen(hist_str) + _strlen(file_path) + 16;
	error = malloc(sizeof(char) * (len + 1));
	if (!error)
	{
		free(hist_str);
		return (127);
	}

	_strcpy(error, name);
	_strcat(error, ": ");
	_strcat(error, hist_str);
	_strcat(error, ": Can't open ");
	_strcat(error, file_path);
	_strcat(error, "\n");

	free(hist_str);
	write(STDERR_FILENO, error, len);
	free(error);
	return (127);
}

/**
 * proc_file_commands - Takes a file and attempts to run the commands stored
 * within.
 * @file_path: Path to the file.
 * @exe_ret: Return value of the last executed command.
 *
 * Return: If file couldn't be opened - 127.
 *	   If malloc fails - -1.
 *	   Otherwise the return value of the last command ran.
 */
int proc_file_commands(char *file_path, int *exe_ret)
{
	size_t file, b_read, i;
	unsigned int line_size = 0;
	unsigned int old_size = 120;
	char *line, **args, **front;
	char buffer[120];
	int ret;

	hist = 0;
	file = open(file_path, O_RDONLY);
	if (file == -1)
	{
		*exe_ret = cant_open(file_path);
		return (*exe_ret);
	}
	line = malloc(sizeof(char) * old_size);
	if (!line)
		return (-1);
	do {
		b_read = read(file, buffer, 119);
		if (b_read == 0 && line_size == 0)
			return (*exe_ret);
		buffer[b_read] = '\0';
		line_size += b_read;
		line = _realloc(line, old_size, line_size);
		_strcat(line, buffer);
		old_size = line_size;
	} while (b_read);
	for (i = 0; line[i] == '\n'; i++)
		line[i] = ' ';
	for (; i < line_size; i++)
	{
		if (line[i] == '\n')
		{
			line[i] = ';';
			for (i += 1; i < line_size && line[i] == '\n'; i++)
				line[i] = ' ';
		}
	}
	variable_replacement(&line, exe_ret);
	handle_line(&line, line_size);
	args = _strtok(line, " ");
	free(line);
	if (!args)
		return (0);
	if (check_args(args) != 0)
	{
		*exe_ret = 2;
		free_args(args, args);
		return (*exe_ret);
	}
	front = args;

	for (i = 0; args[i]; i++)
	{
		if (_strncmp(args[i], ";", 1) == 0)
		{
			free(args[i]);
			args[i] = NULL;
			ret = call_args(args, front, exe_ret);
			args = &args[++i];
			i = 0;
}
}
ret = call_args(args, front, exe_ret);

free(front);
return (ret);
}
