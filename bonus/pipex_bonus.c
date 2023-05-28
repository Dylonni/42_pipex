/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   pipex_bonus.c                                      :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: dylan <dylan@student.42.fr>                +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2023/05/19 15:31:00 by daumis            #+#    #+#             */
/*   Updated: 2023/05/28 15:44:17 by dylan            ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "../pipex.h"

/*Initialize the infile fd*/
void	ft_init_infile(t_data *data)
{
	data->io_fds[0] = open(data->inf_name, O_RDONLY);
	if (data->io_fds[0] == -1)
	{
		ft_putstr_fd(data->inf_name, 2);
		ft_putstr_fd(" : No such file or directory", 2);
		write(2, "\n", 1);
		data->no_infile = 1;
		ft_cleanup(data);
		exit (1);
	}
	dup2(data->io_fds[0], STDIN_FILENO);
	close(data->io_fds[0]);
}

/*Initialize the outfile fd*/
void	ft_init_outfile(t_data *data)
{
	if (data->no_heredoc)
	{
		data->io_fds[1] = open(data->otf_name, \
		O_CREAT | O_WRONLY | O_TRUNC, 0644);
		dup2(data->io_fds[1], STDOUT_FILENO);
		close(data->io_fds[1]);
	}
	else
	{
		data->io_fds[1] = open(data->otf_name, \
		O_CREAT | O_WRONLY | O_APPEND, 0644);
		dup2(data->io_fds[1], STDOUT_FILENO);
		close(data->io_fds[1]);
	}
	if (data->io_fds[1] == -1)
	{
		ft_cleanup(data);
		perror("Open :");
		exit(1);
	}
}

/*Function called to pipe and fork processess, which will continue the program*/
void	ft_execute(t_data *data, char *env[], int argc, int *i)
{
	pid_t	id;

	if (pipe(data->pipe_fd) == -1)
		return (perror("Pipe "));
	id = fork();
	if (id == -1)
		return (perror("Fork "));
	if (id == 0)
	{
		if ((*i == 0) && (data->no_heredoc == 1))
			ft_init_infile(data);
		else if ((*i == 0) && (data->no_heredoc == 0))
			heredoc_redirect(data);
		child_process(data, env, argc, i);
	}
	else
		parent_process(data, i);
	wait(NULL);
}

/*Process called by fork , inherits data of ft_execute params*/
void	child_process(t_data *data, char *env[], int argc, int *i)
{
	make_command(data, i);
	if (*i == 0)
	{
		dup2(data->pipe_fd[1], STDOUT_FILENO);
		close(data->pipe_fd[1]);
		close(data->pipe_fd[0]);
	}
	else
	{
		dup2(data->prev_fd[0], STDIN_FILENO);
		dup2(data->pipe_fd[1], STDOUT_FILENO);
		close(data->prev_fd[0]);
		close(data->prev_fd[1]);
		close(data->pipe_fd[0]);
		close(data->pipe_fd[1]);
	}
	if (*i == (argc - 5))
		ft_init_outfile(data);
	execve(data->fused_cmd, data->args, env);
	if (!data->cmd_exists)
		ft_cleanup(data);
	exit(1);
}

/*Process called by fork , inherits data of ft_execute params*/
void	parent_process(t_data *data, int *i)
{
	if (*i != 0)
	{
		close(data->prev_fd[0]);
		close(data->prev_fd[1]);
	}
	data->prev_fd[0] = data->pipe_fd[0];
	data->prev_fd[1] = data->pipe_fd[1];
}