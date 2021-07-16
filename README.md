# little-git-client

### What is this?
This is a very simple Git client. I want to explore and learn `libgit2`, and this is the result of it.

This Git client is ONLY able to imitate the following `git` commands:
- Initialize git repository
```
git init
```
- Check git status
```
git status
```
- Stage ALL changes
```
git add .
```
- Commit staged changes
```
git commit -m <MESSAGE>
```
- Add remote Git repository, named `origin`
```
git remote add origin <URL_ORIGIN>
```
- Push to remote Git repository, named `origin` to its `master` branch
```
git push origin master
```

### How to compile?
Please make sure you already install `libgit2`. I was using `v1.1.1` when creating this.
```
gcc -o main main.c little-git-client.c -lgit2
```

### How to run?
In order to push to remote Git repository, this little Git client uses basic authentication. Please provide the `GITHUB_USERNAME` and `GITHUB_PASSWORD` as the environment variable.
- Check git status
```
./main <PATH_TO_REPO> status
```
- Stage ALL changes
```
./main <PATH_TO_REPO> add_all
```
- Commit staged changes
```
./main <PATH_TO_REPO> commit_all <MESSAGE>
```
- Add remote Git repository, named `origin`
```
./main <PATH_TO_REPO> add_origin <URL_ORIGIN>
```
- Push to remote Git repository, named `origin` to its `master` branch
```
./main <PATH_TO_REPO> push_to_origin
```
