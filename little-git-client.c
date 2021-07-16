#include "little-git-client.h"


int lgc_git_init(git_repository** repo, const char* repo_path)
{
	DIR* dir = opendir(repo_path);
	if (dir)
		closedir(dir);
	else
		return 1;

	return git_repository_init(repo, repo_path, 0);
}


int lgc_git_status(git_repository* repo, git_status_list** status_list)
{
	git_status_options status_opt = GIT_STATUS_OPTIONS_INIT;
	status_opt.show = GIT_STATUS_SHOW_INDEX_AND_WORKDIR;
	status_opt.flags = GIT_STATUS_OPT_INCLUDE_UNTRACKED | GIT_STATUS_OPT_RENAMES_HEAD_TO_INDEX | GIT_STATUS_OPT_SORT_CASE_SENSITIVELY;

	return git_status_list_new(status_list, repo, &status_opt);
}


int lgc_git_status_breakdown(git_status_list* status_list,
	git_status_entry*** staged, git_status_entry*** unstaged, git_status_entry*** untracked,
	size_t *staged_count, size_t *unstaged_count, size_t *untracked_count)
{	
	const git_status_entry *entry;
	*staged_count = 0;
	*unstaged_count = 0;
	*untracked_count = 0;
	size_t maxi = git_status_list_entrycount(status_list);

	git_status_entry **tmp_staged, **tmp_unstaged, **tmp_untracked;
	tmp_staged = (git_status_entry**) malloc(sizeof(git_status_entry *) * maxi);
	tmp_unstaged = (git_status_entry**) malloc(sizeof(git_status_entry *) * maxi);
	tmp_untracked = (git_status_entry**) malloc(sizeof(git_status_entry *) * maxi);

	for (int i = 0; i < maxi; i++) {
		entry = git_status_byindex(status_list, i);
		int s1 = entry->status & GIT_STATUS_INDEX_NEW;
		int s2 = entry->status & GIT_STATUS_INDEX_MODIFIED;
		int s3 = entry->status & GIT_STATUS_INDEX_DELETED;
		int s4 = entry->status & GIT_STATUS_INDEX_RENAMED;
		int s5 = entry->status & GIT_STATUS_INDEX_TYPECHANGE;
		int s6 = entry->status != GIT_STATUS_CURRENT;

		if ((s1 || s2 || s3 || s4 || s5) && s6) {
			tmp_staged[*staged_count] = entry;
			*staged_count = *staged_count + 1;
			continue;
		}

		int us1 = entry->status & GIT_STATUS_WT_MODIFIED;
		int us2 = entry->status & GIT_STATUS_WT_DELETED;
		int us3 = entry->status & GIT_STATUS_WT_RENAMED;
		int us4 = entry->status & GIT_STATUS_WT_TYPECHANGE;
		int us5 = entry->status != GIT_STATUS_CURRENT && entry->index_to_workdir != NULL;

		if ((us1 || us2 || us3 || us4) && us5) {
			tmp_unstaged[*unstaged_count] = entry;
			*unstaged_count = *unstaged_count + 1;
			continue;
		}

		int ut11 = entry->status == GIT_STATUS_WT_NEW;

		if (ut11) {
			tmp_untracked[*untracked_count] = entry;
			*untracked_count = *untracked_count + 1;
			continue;
		}
	}

	*staged = tmp_staged;
	*unstaged = tmp_unstaged;
	*untracked = tmp_untracked;

	return 0;
}


int lgc_git_add_all(git_repository* repo)
{
	git_index* index;
	char *path = ".";
	git_strarray array = {
		&path,
		1
	};
	int error;

	error = git_repository_index(&index, repo);
	if (error)
		return error;
	
	error = git_index_add_all(index, &array, 0, NULL, NULL);
	if (error)
		return error;

	git_index_write(index);
	git_index_free(index);

	return 0;
}


int lgc_git_commit(git_repository* repo, char* message)
{
	git_oid commit_oid, tree_oid;
	git_signature *signature;
	git_index *index;
	git_tree *tree;
	git_object *parent = NULL;
	git_reference *ref = NULL;
	int ret, error;

	error = git_revparse_ext(&parent, &ref, repo, "HEAD");
	if (error) {
		if (error != GIT_ENOTFOUND) {
			return error;
		}
		else {
			parent = NULL;
		}		
	}
	
	error = git_repository_index(&index, repo);
	if (error)
		return error;
	
	error = git_index_write_tree(&tree_oid, index);
	if (error)
		return error;
	
	error = git_index_write(index);
	if (error)
		return error;
	
	error = git_tree_lookup(&tree, repo, &tree_oid);
	if (error)
		return error;
	
	error = git_signature_default(&signature, repo);
	if (error)
		return error;
	
	error = git_commit_create_v(
		&commit_oid,
		repo,
		"HEAD",
		signature,
		signature,
		NULL,
		message,
		tree,
		parent ? 1 : 0,
		parent
	);
	if (error)
		return error;

	git_index_free(index);
	git_signature_free(signature);
	git_tree_free(tree);

	return 0;
}


int lgc_git_remote_add_origin(git_repository* repo, char* url)
{
	git_remote *remote;
	return git_remote_create(&remote, repo, "origin", url);
}


int lgc_git_push_origin_auth_callback(struct git_credential** out, const char *url, const char *username_from_url, unsigned int allowed_types, void *payload)
{
	char* github_username = getenv("GITHUB_USERNAME");
	char* github_password = getenv("GITHUB_PASSWORD");
	return git_credential_userpass_plaintext_new(out, github_username, github_password);
}


int lgc_git_push_origin(git_repository* repo) 
{
	int error;
	char *refspec = "refs/heads/master";
	const git_strarray refspecs = {
		&refspec,
		1
	};

	struct git_remote_callbacks cb = GIT_REMOTE_CALLBACKS_INIT;
	cb.credentials = lgc_git_push_origin_auth_callback;

	git_push_options options = GIT_PUSH_OPTIONS_INIT;
	options.callbacks = cb;
	
	git_remote *remote;
	error = git_remote_lookup(&remote, repo, "origin");
	if (error)
		return error;

	error = git_remote_push(remote, &refspecs, &options);

    return error;
}
