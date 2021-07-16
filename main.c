#include "little-git-client.h"


int parse_args(char** repo_path, char** command, int argc, char* argv[])
{
	int error = 1;
	char* valid_commands[] = {"status", "add_all", "commit_all", "add_origin", "push_to_origin"};

	if (argc == 3 || argc == 4) {
		char* arg_command = argv[2];
		for (int i = 0; i < 5; i++) {
			if (strcmp(arg_command, valid_commands[i]) == 0) {
				error = 0;
				break;
			}
		}
	}

	if (error) {
		printf("You need to follow this format: ./main <repo_path> <command>\n");
		printf("Here are list of commands recognized:\n"
			"1. status\n"
			"2. add_all\n"
			"3. commit_all <commit_message>\n"
			"4. add_origin <origin_url>\n"
			"5. push_to_origin\n"
		);
	} else {
		*repo_path = argv[1];
		*command = argv[2];	
	}

	return error;
}


int check_required_env_vars()
{
	char* github_username = getenv("GITHUB_USERNAME");
	char* github_password = getenv("GITHUB_PASSWORD");
	if (github_username == NULL || github_password == NULL) {
		printf("Please provide required environment variables: GITHUB_USERNAME, GITHUB_PASSWORD\n");
		return 1;
	}

	return 0;
}


int main(int argc, char* argv[])
{
	int error = 0;
	char *repo_path;
	char *command;
	git_repository* repo;

	error = parse_args(&repo_path, &command, argc, argv);
	if (error)
		return error;

	error = check_required_env_vars();
	if (error)
		return error;

	git_libgit2_init();

	error = lgc_git_init(&repo, repo_path);
	if (error) {
		printf("Failed to open directory specified as repository path\n");
		return error;
	}

	if (strcmp(command, "status") == 0) {
		git_status_list *status_list;
		error = lgc_git_status(repo, &status_list);
		if (error)
			return error;

		git_status_entry **staged;
		git_status_entry **unstaged;
		git_status_entry **untracked;
		size_t staged_count, unstaged_count, untracked_count;
		error = lgc_git_status_breakdown(status_list, &staged, &unstaged, &untracked, &staged_count, &unstaged_count, &untracked_count);
		if (error)
			return error;

		printf("--STAGED--\n");
		for (int i = 0; i < staged_count; i++) {
			printf("%s\n", staged[i]->head_to_index->new_file.path);
		}
		printf("\n");

		printf("--UNSTAGED--\n");
		for (int i = 0; i < unstaged_count; i++) {
			printf("%s\n", unstaged[i]->index_to_workdir->new_file.path);
		}
		printf("\n");

		printf("--UNTRACKED--\n");
		for (int i = 0; i < untracked_count; i++) {
			printf("%s\n", untracked[i]->index_to_workdir->old_file.path);
		}
		printf("\n");
	}

	else if (strcmp(command, "add_all") == 0) {
		error = lgc_git_add_all(repo);
		if (error)
			return error;
	}

	else if (strcmp(command, "commit_all") == 0) {
		char *commit_message = argv[3];
		if (commit_message) {
			error = lgc_git_commit(repo, commit_message);
			if (error) {
				printf("Failed to commit\n");
				return error;
			}
		} else {
			error = 1;
			printf("Please provide commit message.\n");
			return error;
		}
	}

	else if (strcmp(command, "add_origin") == 0) {
		char *origin_url = argv[3];
		if (origin_url) {
			error = lgc_git_remote_add_origin(repo, origin_url);
			if (error) {
				printf("Failed to add origin\n");
				return error;
			}
		} else {
			error = 1;
			printf("Please provide origin URL.\n");
			return error;
		}
	}

	else if (strcmp(command, "push_to_origin") == 0) {
		error = lgc_git_push_origin(repo);
		if (error) {
			printf("Failed to push\n");
			return error;
		}
	}

	printf("Exiting successfully\n");
	return 0;
}
