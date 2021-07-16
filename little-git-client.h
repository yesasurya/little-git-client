#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <dirent.h>
#include <git2.h>


// Equivalent git command: git init
int lgc_git_init(git_repository** repo, const char* repo_path);


// Equivalent git command: git status
int lgc_git_status(git_repository* repo, git_status_list** status_list);


// lgc_git_status return a status list which contains all the files (STAGED, UNSTAGED, UNTRACKED).
// This function split a status list into three different categories.
int lgc_git_status_breakdown(git_status_list* status_list, 
	git_status_entry*** staged, git_status_entry*** unstaged, git_status_entry*** untracked,
	size_t *staged_count, size_t *unstaged_count, size_t *untracked_count);


// Equivalent git command: git add .
int lgc_git_add_all(git_repository* repo);


// Equivalent git command: git commit -m <message>
int lgc_git_commit(git_repository* repo, char* message);


// Equivalent git command: git remote add origin <url>
int lgc_git_remote_add_origin(git_repository* repo, char* url);


// lgc_git_push_origin require authentication. The authentication is performed using this callback function.
// Currently, the authentication used is username and password.
int lgc_git_push_origin_auth_callback(struct git_credential** out, const char *url, const char *username_from_url, unsigned int allowed_types, void *payload);


// Equivalent git command: git push origin master
int lgc_git_push_origin(git_repository* repo);
