#include "../include/files.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <errno.h>

static int is_text_mime(const char *mime) {
    static const char *text_types[] = {
        "application/json",
        "application/xml",
        "application/javascript",
        "application/x-javascript",
        "application/x-sh",
        "application/sql",
        "application/yaml",
        "application/toml",
        "application/xhtml+xml",
        "application/rtf",
        NULL
    };

    const char **type;

    if(strncmp(mime, "text/", 5) == 0)
        return 1;

    for(type = text_types; *type != NULL; type++) {
        if(strcmp(mime, *type) == 0)
            return 1;
    }

    return 0;
}

ViewMode detect_file_type(const char *path) {
    int fd[2];
    pid_t pid;
    int status;
    char buffer[128];
    ssize_t n;
    size_t len = 0;
    int wait_result;

    if(path == NULL)
        return VIEW_TEXT;

    if(pipe(fd) == -1)
        return VIEW_TEXT;

    pid = fork();

    if(pid == -1) {
        close(fd[0]);
        close(fd[1]);
        return VIEW_TEXT;
    }

    if(pid == 0) {
        close(fd[0]);

        if(dup2(fd[1], STDOUT_FILENO) == -1)
            _exit(EXIT_FAILURE);

        close(fd[1]);

        execlp("file", "file", "-b", "--mime-type", "--",
               path, (char *)NULL);

        _exit(EXIT_FAILURE);
    }

    close(fd[1]);

    while(len < sizeof(buffer) - 1) {
        n = read(fd[0], buffer + len, sizeof(buffer) - 1 - len);

        if(n > 0)
            len += (size_t)n;
        else if(n == 0)
            break;
        else if(errno != EINTR)
            break;
    }

    close(fd[0]);
    buffer[len] = '\0';

    do {
        wait_result = waitpid(pid, &status, 0);
    } while(wait_result == -1 && errno == EINTR);

    if(wait_result == -1)
        return VIEW_TEXT;

    if(!WIFEXITED(status) || WEXITSTATUS(status) != 0)
        return VIEW_TEXT;

    while(len > 0 && (buffer[len - 1] == '\n' ||
                      buffer[len - 1] == '\r' ||
                      buffer[len - 1] == ' ')) {
        buffer[--len] = '\0';
    }

    if(strcmp(buffer, "application/octet-stream") == 0)
        return VIEW_BINARY;

    return VIEW_TEXT;
}

int view_file(const char *path, ViewMode mode) {
    pid_t pid;
    int status;

    if(path == NULL)
        return 0;

    def_prog_mode();
    endwin();

    if(mode == VIEW_TEXT) {
        pid = fork();

        if(pid == 0) {
            execlp("less", "less", path, (char *)NULL);
            _exit(EXIT_FAILURE);
        }

        if(pid < 0)
            goto error;

        while(waitpid(pid, &status, 0) == -1) {
            if(errno != EINTR)
                goto error;
        }
    }
    else {
        int fd[2];
        pid_t xxd_pid, less_pid;
        int xxd_status, less_status;

        if(pipe(fd) == -1)
            goto error;

        xxd_pid = fork();

        if(xxd_pid == 0) {
            close(fd[0]);

            if(dup2(fd[1], STDOUT_FILENO) == -1)
                _exit(EXIT_FAILURE);

            close(fd[1]);

            execlp("xxd", "xxd", path, (char *)NULL);
            _exit(EXIT_FAILURE);
        }

        if(xxd_pid < 0) {
            close(fd[0]);
            close(fd[1]);
            goto error;
        }

        less_pid = fork();

        if(less_pid == 0) {
            close(fd[1]);

            if(dup2(fd[0], STDIN_FILENO) == -1)
                _exit(EXIT_FAILURE);

            close(fd[0]);

            execlp("less", "less", (char *)NULL);
            _exit(EXIT_FAILURE);
        }

        close(fd[0]);
        close(fd[1]);

        if(less_pid < 0) {
            waitpid(xxd_pid, &xxd_status, 0);
            goto error;
        }

        while(waitpid(less_pid, &less_status, 0) == -1) {
            if(errno != EINTR)
                break;
        }

        while(waitpid(xxd_pid, &xxd_status, 0) == -1) {
            if(errno != EINTR)
                break;
        }

        status = less_status;
    }

    reset_prog_mode();
    refresh();

    return WIFEXITED(status) && WEXITSTATUS(status) == 0;

error:
    reset_prog_mode();
    refresh();
    return 0;
}

int open_file(const char *path) {
    if(detect_file_type(path)==VIEW_TEXT){
        pid_t pid;
        int status;

        if(path == NULL)
            return 0;

        pid = fork();

        if(pid == -1)
            return 0;

        if(pid == 0) {
            execlp("xdg-open", "xdg-open", path, (char *)NULL);
            _exit(EXIT_FAILURE);
        }

        while(waitpid(pid, &status, 0) == -1) {
            if(errno != EINTR)
                return 0;
        }

        return WIFEXITED(status) && WEXITSTATUS(status) == 0;
    }
    return view_file(path,VIEW_BINARY);
}