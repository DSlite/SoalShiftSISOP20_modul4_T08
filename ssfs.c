#define FUSE_USE_VERSION 28
#include <fuse.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <dirent.h>
#include <errno.h>
#include <sys/time.h>
#include <time.h>

static const char *dirpath = "/home/umum/Documents/SisOpLab/FUSE";
// static const char *key = "9(ku@AW1[Lmvgax6q`5Y2Ry?+sF!^HKQiBXCUSe&0M.b%rI'7d)o4~VfZ*{#:}ETt$3J-zpc]lnh8,GwP_ND|jO";

void getFileName(char buff[], char *fpath) {
  char *token = strtok(fpath, "/");
  while (token != NULL) {
    sprintf(buff, "%s", token);
    token = strtok(NULL, "/");
  }
}

void getDirAndFile(char *dir, char *file, char *path) {
  char buff[1000];
  memset(dir, 0, strlen(dir));
  memset(file, 0, strlen(file));
  strcpy(buff, path);
  char *token = strtok(buff, "/");
  while(token != NULL) {
    sprintf(file, "%s", token);
    token = strtok(NULL, "/");
    if (token != NULL) {
      sprintf(dir, "%s/%s", dir, file);
    }
  }
}

void nextSync(char *syncDirPath) {
  char buff[1000];
  char construct[1000];
  memset(construct, 0, strlen(construct));
  strcpy(buff, syncDirPath);
  int state = 0;
  char *token = strtok(buff, "/");
  while (token != NULL) {
    char tBuff[1000];
    char get[1000];
    strcpy(tBuff, token);
    if (!state) {
      if (strstr(tBuff, "sync_")-tBuff == 0) {
        strcpy(get, tBuff+5);
      } else {
        sprintf(get, "sync_%s", tBuff);
        state = 1;
      }
    } else {
      strcpy(get, tBuff);
    }
    sprintf(construct, "%s/%s", construct, get);
    token = strtok(NULL, "/");
  }
  memset(syncDirPath, 0, strlen(syncDirPath));
  sprintf(syncDirPath, "%s", construct);
}

void changePath(char *fpath, const char *path) {
  if (strcmp(path, "/") == 0) {
    sprintf(fpath, "%s", dirpath);
  } else {
    sprintf(fpath, "%s%s", dirpath, path);
  }
}

void logFile(char *level, char *cmd, int res, int lenDesc, const char *desc[]) {
  FILE *f = fopen("/home/umum/Documents/SisOpLab/fuse_log.txt", "a");
  time_t t;
  struct tm *tmp;
  char timeBuff[100];

  time(&t);
  tmp = localtime(&t);
  strftime(timeBuff, sizeof(timeBuff), "%y%m%d-%H:%M:%S", tmp);

  fprintf(f, "%s::%s::%s::%d", level, timeBuff, cmd, res);
  for (int i = 0; i < lenDesc; i++) {
    fprintf(f, "::%s", desc[i]);
  }
  fprintf(f, "\n");

  fclose(f);
}


static int _getattr(const char *path, struct stat *stbuf)
{
	char fpath[1000];
	changePath(fpath, path);

	int res;

	res = lstat(fpath, stbuf);

  const char *desc[] = {path};
  logFile("INFO", "GETATTR", res, 1, desc);

	if (res == -1) return -errno;


	return 0;
}

static int _access(const char *path, int mask)
{
	char fpath[1000];
	changePath(fpath, path);

	int res;

	res = access(fpath, mask);

  const char *desc[] = {path};
  logFile("INFO", "ACCESS", res, 1, desc);

	if (res == -1) return -errno;


	return 0;
}

static int _readlink(const char *path, char *buf, size_t size)
{
	char fpath[1000];
	changePath(fpath, path);

	int res;

	res = readlink(fpath, buf, size - 1);

  const char *desc[] = {path};
  logFile("INFO", "READLINK", res, 1, desc);

	if (res == -1) return -errno;

	buf[res] = '\0';
	return 0;
}


static int _readdir(const char *path, void *buf, fuse_fill_dir_t filler,
		       off_t offset, struct fuse_file_info *fi)
{
	char fpath[1000];
	changePath(fpath, path);

	DIR *dp;
	struct dirent *de;

	(void) offset;
	(void) fi;

	dp = opendir(fpath);
	if (dp == NULL) {
    const char *desc[] = {path};
    logFile("INFO", "READDIR", -1, 1, desc);
    return -errno;
  }

	while ((de = readdir(dp)) != NULL) {
		struct stat st;
		memset(&st, 0, sizeof(st));
		st.st_ino = de->d_ino;
		st.st_mode = de->d_type << 12;
		if (filler(buf, de->d_name, &st, 0)) break;
	}

  const char *desc[] = {path};
  logFile("INFO", "READDIR", 0, 1, desc);

	closedir(dp);
	return 0;
}

static int _mkdir(const char *path, mode_t mode)
{
	char fpath[1000];
	changePath(fpath, path);

	int res;

	res = mkdir(fpath, mode);


  // Soal 3
  char syncOrigPath[1000];
  char syncDirPath[1000];
  char syncFilePath[1000];
  memset(syncOrigPath, 0, strlen(syncOrigPath));
  strcpy(syncOrigPath, path);
  getDirAndFile(syncDirPath, syncFilePath, syncOrigPath);
  memset(syncOrigPath, 0, strlen(syncOrigPath));
  strcpy(syncOrigPath, syncDirPath);
  do {
    char syncPath[1000];
    memset(syncPath, 0, strlen(syncPath));
    nextSync(syncDirPath);
    if (strcmp(syncDirPath, syncOrigPath) == 0) break;
    changePath(syncPath, syncDirPath);
    if (access(syncPath, F_OK) == -1) continue;
    sprintf(syncPath, "%s/%s", syncPath, syncFilePath);
    mkdir(syncPath, mode);
  } while (1);


  const char *desc[] = {path};
  logFile("INFO", "MKDIR", res, 1, desc);

	if (res == -1) return -errno;

	return 0;
}

static int _unlink(const char *path)
{
	char fpath[1000];
	changePath(fpath, path);

	int res;

	res = unlink(fpath);

  char syncOrigPath[1000];
  char syncDirPath[1000];
  char syncFilePath[1000];
  memset(syncOrigPath, 0, strlen(syncOrigPath));
  strcpy(syncOrigPath, path);
  getDirAndFile(syncDirPath, syncFilePath, syncOrigPath);
  memset(syncOrigPath, 0, strlen(syncOrigPath));
  strcpy(syncOrigPath, syncDirPath);
  do {
    char syncPath[1000];
    memset(syncPath, 0, strlen(syncPath));
    nextSync(syncDirPath);
    if (strcmp(syncDirPath, syncOrigPath) == 0) break;
    changePath(syncPath, syncDirPath);
    if (access(syncPath, F_OK) == -1) continue;
    sprintf(syncPath, "%s/%s", syncPath, syncFilePath);
    unlink(syncPath);
  } while (1);

  const char *desc[] = {path};
  logFile("WARNING", "UNLINK", res, 1, desc);

	if (res == -1) return -errno;

	return 0;
}

static int _rmdir(const char *path)
{
	char fpath[1000];
	changePath(fpath, path);
	int res;

	res = rmdir(fpath);

  char syncOrigPath[1000];
  char syncDirPath[1000];
  char syncFilePath[1000];
  memset(syncOrigPath, 0, strlen(syncOrigPath));
  strcpy(syncOrigPath, path);
  getDirAndFile(syncDirPath, syncFilePath, syncOrigPath);
  memset(syncOrigPath, 0, strlen(syncOrigPath));
  strcpy(syncOrigPath, syncDirPath);
  do {
    char syncPath[1000];
    memset(syncPath, 0, strlen(syncPath));
    nextSync(syncDirPath);
    if (strcmp(syncDirPath, syncOrigPath) == 0) break;
    changePath(syncPath, syncDirPath);
    if (access(syncPath, F_OK) == -1) continue;
    sprintf(syncPath, "%s/%s", syncPath, syncFilePath);
    rmdir(syncPath);
  } while (1);

  const char *desc[] = {path};
  logFile("WARNING", "RMDIR", res, 1, desc);

	if (res == -1) return -errno;

	return 0;
}

static int _symlink(const char *from, const char *to)
{
	char ffrom[1000];
	char fto[1000];
	changePath(ffrom, from);
	changePath(fto, to);

	int res;

	res = symlink(ffrom, fto);

  const char *desc[] = {from, to};
  logFile("INFO", "SYMLINK", res, 2, desc);

	if (res == -1) return -errno;

	return 0;
}

static int _rename(const char *from, const char *to)
{
	char ffrom[1000];
	char fto[1000];
	changePath(ffrom, from);
	changePath(fto, to);

	int res;

	res = rename(ffrom, fto);

  const char *desc[] = {from, to};
  logFile("INFO", "RENAME", res, 2, desc);

	if (res == -1) return -errno;

	return 0;
}

static int _link(const char *from, const char *to)
{
	char ffrom[1000];
	char fto[1000];
	changePath(ffrom, from);
	changePath(fto, to);

	int res;

	res = link(ffrom, fto);

  const char *desc[] = {from, to};
  logFile("INFO", "LINK", res, 2, desc);

	if (res == -1) return -errno;

	return 0;
}

static int _chmod(const char *path, mode_t mode)
{
	char fpath[1000];
	changePath(fpath, path);

	int res;

	res = chmod(fpath, mode);

  char modeBuff[100];
  sprintf(modeBuff, "%d", mode);
  const char *desc[] = {path, modeBuff};
  logFile("INFO", "CHMOD", res, 2, desc);

	if (res == -1) return -errno;

	return 0;
}

static int _chown(const char *path, uid_t uid, gid_t gid)
{
	char fpath[1000];
	changePath(fpath, path);

	int res;

	res = lchown(fpath, uid, gid);

  char uidBuff[100];
  char gidBuff[100];
  sprintf(uidBuff, "%d", uid);
  sprintf(gidBuff, "%d", gid);
  const char *desc[] = {path, uidBuff, gidBuff};
  logFile("INFO", "CHOWN", res, 3, desc);

	if (res == -1) return -errno;

	return 0;
}

static int _truncate(const char *path, off_t size)
{
	char fpath[1000];
	changePath(fpath, path);

	int res;

	res = truncate(fpath, size);

  const char *desc[] = {path};
  logFile("INFO", "TRUNCATE", res, 1, desc);

	if (res == -1) return -errno;

	return 0;
}

static int _utimens(const char *path, const struct timespec ts[2])
{
	char fpath[1000];
	changePath(fpath, path);

	int res;

	/* don't use utime/utimes since they follow symlinks */
	res = utimensat(0, fpath, ts, AT_SYMLINK_NOFOLLOW);

  const char *desc[] = {path};
  logFile("INFO", "UTIMENSAT", res, 1, desc);

	if (res == -1) return -errno;

	return 0;
}

static int _create(const char *path, mode_t mode,
		      struct fuse_file_info *fi)
{
	char fpath[1000];
	changePath(fpath, path);

	int res;

	res = open(fpath, fi->flags, mode);

  char syncOrigPath[1000];
  char syncDirPath[1000];
  char syncFilePath[1000];
  memset(syncOrigPath, 0, strlen(syncOrigPath));
  strcpy(syncOrigPath, path);
  getDirAndFile(syncDirPath, syncFilePath, syncOrigPath);
  memset(syncOrigPath, 0, strlen(syncOrigPath));
  strcpy(syncOrigPath, syncDirPath);
  do {
    char syncPath[1000];
    memset(syncPath, 0, strlen(syncPath));
    nextSync(syncDirPath);
    if (strcmp(syncDirPath, syncOrigPath) == 0) break;
    changePath(syncPath, syncDirPath);
    if (access(syncPath, F_OK) == -1) continue;
    sprintf(syncPath, "%s/%s", syncPath, syncFilePath);
    close(open(syncPath, fi->flags, mode));
  } while (1);

  const char *desc[] = {path};
  logFile("INFO", "CREAT", res, 1, desc);

	if (res == -1) return -errno;

	fi->fh = res;
	return 0;
}

static int _open(const char *path, struct fuse_file_info *fi)
{
	char fpath[1000];
	changePath(fpath, path);
	int res;

	res = open(fpath, fi->flags);

  const char *desc[] = {path};
  logFile("INFO", "OPEN", res, 1, desc);

	if (res == -1) return -errno;

	fi->fh = res;
	return 0;
}

static int _read(const char *path, char *buf, size_t size, off_t offset,
		    struct fuse_file_info *fi)
{
	char fpath[1000];
	changePath(fpath, path);

	int fd;
	int res;

	if(fi == NULL) fd = open(fpath, O_RDONLY);
	else fd = fi->fh;

	if (fd == -1) return -errno;

	res = pread(fd, buf, size, offset);

  const char *desc[] = {path};
  logFile("INFO", "READ", res, 1, desc);

	if (res == -1) res = -errno;

	if(fi == NULL) close(fd);
	return res;
}

static int _write(const char *path, const char *buf, size_t size,
		     off_t offset, struct fuse_file_info *fi)
{
	char fpath[1000];
	changePath(fpath, path);

	int fd;
	int res;

	(void) fi;
	if(fi == NULL) fd = open(fpath, O_WRONLY);
	else fd = fi->fh;

	if (fd == -1) return -errno;

	res = pwrite(fd, buf, size, offset);

  char syncOrigPath[1000];
  char syncDirPath[1000];
  char syncFilePath[1000];
  memset(syncOrigPath, 0, strlen(syncOrigPath));
  strcpy(syncOrigPath, path);
  getDirAndFile(syncDirPath, syncFilePath, syncOrigPath);
  memset(syncOrigPath, 0, strlen(syncOrigPath));
  strcpy(syncOrigPath, syncDirPath);
  do {
    char syncPath[1000];
    int syncFd;
    memset(syncPath, 0, strlen(syncPath));
    nextSync(syncDirPath);
    if (strcmp(syncDirPath, syncOrigPath) == 0) break;
    changePath(syncPath, syncDirPath);
    if (access(syncPath, F_OK) == -1) continue;
    sprintf(syncPath, "%s/%s", syncPath, syncFilePath);
    syncFd = open(syncPath, O_WRONLY);
    if (syncFd == -1) return -errno;
    pwrite(syncFd, buf, size, offset);
    close(syncFd);
  } while (1);

  const char *desc[] = {path};
  logFile("INFO", "WRITE", res, 1, desc);

	if (res == -1) res = -errno;

	if(fi == NULL) close(fd);
	return res;
}

static int _statfs(const char *path, struct statvfs *stbuf)
{
	char fpath[1000];
	changePath(fpath, path);
	int res;

	res = statvfs(fpath, stbuf);

  const char *desc[] = {path};
  logFile("INFO", "STATFS", res, 1, desc);

	if (res == -1) return -errno;

	return 0;
}

static int _release(const char *path, struct fuse_file_info *fi)
{
	(void) path;
	close(fi->fh);
  const char *desc[] = {path};
  logFile("INFO", "RELEASE", 0, 1, desc);
	return 0;
}


static const struct fuse_operations _oper = {
	.getattr	= _getattr,
	.access		= _access,
	.readlink	= _readlink,
	.readdir	= _readdir,
	.mkdir		= _mkdir,
	.symlink	= _symlink,
	.unlink		= _unlink,
	.rmdir		= _rmdir,
	.rename		= _rename,
	.link		  = _link,
	.chmod		= _chmod,
	.chown		= _chown,
	.truncate	= _truncate,
	.utimens	= _utimens,
	.open		  = _open,
	.create 	= _create,
	.read		  = _read,
	.write		= _write,
	.statfs		= _statfs,
	.release	= _release,
};

int main(int argc, char *argv[])
{
	umask(0);
	return fuse_main(argc, argv, &_oper, NULL);
}
