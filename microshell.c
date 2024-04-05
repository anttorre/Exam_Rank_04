/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   microshell.c                                       :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: anttorre <atormora@gmail.com>              +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/03/26 14:16:53 by anttorre          #+#    #+#             */
/*   Updated: 2024/04/02 14:33:24 by anttorre         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include <signal.h>
#include <sys/wait.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

int	msg_err(char *s)
{
	while (*s)
		write(2, s++, 1);
	return (1);
}

int	exec_cd(char **argv, int i)
{
	if (i != 2)
		return (msg_err("error: cd: bad arguments\n"));
	else if (chdir(argv[1]) == -1)
		return (msg_err("error: cd: cannot change directory to "), msg_err(argv[1]), msg_err("\n"));
	return (0);
}

int	exec_cmds(char **argv, char **env, int i, int f_pipe)
{
	int		status;
	int		fd[2];
	pid_t	pid;

	f_pipe = argv[i] && !strcmp(argv[i], "|");
	if (f_pipe && pipe(fd) == -1)
		return (msg_err("error: fatal\n"));
	pid = fork();
	if (pid == 0)
	{
		argv[i] = NULL;
		if (f_pipe && close(fd[0]) == -1)
			return (msg_err("error: fatal\n"));
		if (f_pipe && dup2(fd[1], 1) == -1)
			return (msg_err("error: fatal\n"));
		if (f_pipe && close(fd[1]) == -1)
			return (msg_err("error: fatal\n"));
		execve(*argv, argv, env);
		return (msg_err("error: cannot execute "), msg_err(*argv), msg_err("\n"));
	}
	if (f_pipe && close(fd[1]) == -1)
		return (msg_err("error: fatal\n"));
	if (f_pipe && (dup2(fd[0], 0) == -1))
		return (msg_err("error: fatal\n"));
	if (f_pipe && close(fd[0]) == -1)
		return (msg_err("error: fatal\n"));
	waitpid(pid, &status, 0);
	return (WIFEXITED(status) && WEXITSTATUS(status));
}

int	main(int argc, char **argv, char **env)
{
	int	i;
	int	status;

	i = 0;
	status = 0;
	if (argc > 1)
	{
		while (argv[i] && argv[++i])
		{
			argv += i;
			i = 0;
			while (argv[i] && strcmp(argv[i], "|") && strcmp(argv[i], ";"))
				i++;
			if (!strncmp(*argv, "cd\0", 3))
				status = exec_cd(argv, i);
			else if (i)
				status = exec_cmds(argv, env, i, 0);
		}
	}
	return (status);
}
