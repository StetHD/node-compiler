/*
 * Copyright (c) 2017 Minqi Pan <pmq2001@gmail.com>
 *                    Shengyuan Liu <sounder.liu@gmail.com>
 *
 * This file is part of libsquash, distributed under the MIT License
 * For full terms see the included LICENSE file
 */

#include "enclose_io_common.h"

#ifdef _WIN32
#define MAXPATHLEN 4096
#define mode_t unsigned int
#endif

sqfs *enclose_io_fs;
sqfs_path enclose_io_cwd; /* must end with a slash */
SQUASH_OS_PATH mkdir_workdir = NULL; /* must NOT end with a slash */
char *enclose_io_mkdir_scope = "/__enclose_io_memfs__"; /* must NOT end with a slash */

#ifndef _WIN32
static int mkdir_workdir_halt_rm(const char *arg1, const struct stat *ptr, int flag, struct FTW *ftwarg)
{
	if (FTW_D == flag || FTW_DNR == flag || FTW_DP == flag) {
		rmdir(arg1);
	} else {
		unlink(arg1);
	}
}
static void mkdir_workdir_halt()
{
	nftw(mkdir_workdir, mkdir_workdir_halt_rm, 20, FTW_PHYS | FTW_MOUNT | FTW_DEPTH);
}
static const char* enclose_io_mkdir_workdir()
{
	if (NULL == mkdir_workdir) {
		MUTEX_LOCK(&squash_global_mutex);
		if (NULL == mkdir_workdir) {
			mkdir_workdir = squash_tmpf(squash_tmpdir(), NULL);
			if (mkdir(mkdir_workdir, S_IRWXU)) {
				mkdir_workdir = NULL;
				return NULL;
			}
			if (atexit(mkdir_workdir_halt)) {
				mkdir_workdir = NULL;
				return NULL;
			}
		}
		MUTEX_UNLOCK(&squash_global_mutex);
	}
	return mkdir_workdir;
}

int enclose_io_lstat(const char *path, struct stat *buf)
{
	if (enclose_io_cwd[0] && '/' != *path) {
		sqfs_path enclose_io_expanded;
		size_t enclose_io_cwd_len;
		size_t memcpy_len;
		ENCLOSE_IO_GEN_EXPANDED_NAME(path);
		ENCLOSE_IO_CONSIDER_MKDIR_WORKDIR_RETURN(
			enclose_io_expanded,
			squash_lstat(enclose_io_fs, enclose_io_expanded, buf),
			lstat(mkdir_workdir_expanded, buf)
		);
	} else if (enclose_io_is_path(path)) {
		ENCLOSE_IO_CONSIDER_MKDIR_WORKDIR_RETURN(
			path,
			squash_lstat(enclose_io_fs, path, buf),
			lstat(mkdir_workdir_expanded, buf)
		);
	} else {
		return lstat(path, buf);
	}
}

ssize_t enclose_io_readlink(const char *path, char *buf, size_t bufsize)
{
	if (enclose_io_cwd[0] && '/' != *path) {
		sqfs_path enclose_io_expanded;
		size_t enclose_io_cwd_len;
		size_t memcpy_len;
		ENCLOSE_IO_GEN_EXPANDED_NAME(path);
		ENCLOSE_IO_CONSIDER_MKDIR_WORKDIR_RETURN(
			enclose_io_expanded,
			squash_readlink(enclose_io_fs, enclose_io_expanded, buf, bufsize),
			readlink(mkdir_workdir_expanded, buf, bufsize)
		);
	}
	else if (enclose_io_is_path(path)) {
		ENCLOSE_IO_CONSIDER_MKDIR_WORKDIR_RETURN(
			path,
			squash_readlink(enclose_io_fs, path, buf, bufsize),
			readlink(mkdir_workdir_expanded, buf, bufsize)
		);
	}
	else {
		return readlink(path, buf, bufsize);
	}
}

DIR * enclose_io_opendir(const char *filename)
{
	if (enclose_io_cwd[0] && '/' != *filename) {
		sqfs_path enclose_io_expanded;
		size_t enclose_io_cwd_len;
		size_t memcpy_len;
		ENCLOSE_IO_GEN_EXPANDED_NAME(filename);
		ENCLOSE_IO_CONSIDER_MKDIR_WORKDIR_RETURN(
			enclose_io_expanded,
			(DIR *)squash_opendir(enclose_io_fs, enclose_io_expanded),
			opendir(mkdir_workdir_expanded)
		);
	}
	else if (enclose_io_is_path(filename)) {
		ENCLOSE_IO_CONSIDER_MKDIR_WORKDIR_RETURN(
			filename,
			(DIR *)squash_opendir(enclose_io_fs, filename),
			opendir(mkdir_workdir_expanded)
		);
	}
	else {
		return opendir(filename);
	}
}

int enclose_io_closedir(DIR *dirp)
{
	if (squash_find_entry(dirp)) {
		return squash_closedir((SQUASH_DIR *)dirp);
	}
	else {
		return closedir(dirp);
	}
}

struct SQUASH_DIRENT * enclose_io_readdir(DIR *dirp)
{
	if (squash_find_entry(dirp)) {
		return squash_readdir((SQUASH_DIR *)dirp);
	}
	else {
		return readdir(dirp);
	}
}

long enclose_io_telldir(DIR *dirp)
{
	if (squash_find_entry(dirp)) {
		return squash_telldir((SQUASH_DIR *)dirp);
	}
	else {
		return telldir(dirp);
	}
}

void enclose_io_seekdir(DIR *dirp, long loc)
{
	if (squash_find_entry(dirp)) {
		return squash_seekdir((SQUASH_DIR *)dirp, loc);
	}
	else {
		return seekdir(dirp, loc);
	}
}

void enclose_io_rewinddir(DIR *dirp)
{
	if (squash_find_entry(dirp)) {
		return squash_rewinddir((SQUASH_DIR *)dirp);
	}
	else {
		return rewinddir(dirp);
	}
}

int enclose_io_dirfd(DIR *dirp)
{
	if (squash_find_entry(dirp)) {
		return squash_dirfd((SQUASH_DIR *)dirp);
	}
	else {
		return dirfd(dirp);
	}
}

int enclose_io_scandir(const char *dirname, struct SQUASH_DIRENT ***namelist,
	int(*select)(const struct SQUASH_DIRENT *),
	int(*compar)(const struct SQUASH_DIRENT **, const struct SQUASH_DIRENT **))
{
	if (enclose_io_cwd[0] && '/' != *dirname) {
		sqfs_path enclose_io_expanded;
		size_t enclose_io_cwd_len;
		size_t memcpy_len;
		ENCLOSE_IO_GEN_EXPANDED_NAME(dirname);
		ENCLOSE_IO_CONSIDER_MKDIR_WORKDIR_RETURN(
			enclose_io_expanded,
			squash_scandir(enclose_io_fs, enclose_io_expanded, namelist, select, compar),
			scandir(mkdir_workdir_expanded, namelist, select, compar)
		);
	}
	else if (enclose_io_is_path(dirname)) {
		ENCLOSE_IO_CONSIDER_MKDIR_WORKDIR_RETURN(
			dirname,
			squash_scandir(enclose_io_fs, dirname, namelist, select, compar),
			scandir(mkdir_workdir_expanded, namelist, select, compar)
		);
	}
	else {
		return scandir(dirname, namelist, select, compar);
	}
}

ssize_t enclose_io_pread(int d, void *buf, size_t nbyte, off_t offset)
{
	if (SQUASH_VALID_VFD(d)) {
		off_t lseek_off, backup_off;
		ssize_t read_ssize;
		backup_off = squash_lseek(d, 0, SQUASH_SEEK_CUR);
		if (-1 == backup_off) {
			return -1;
		}
		lseek_off = squash_lseek(d, offset, SQUASH_SEEK_SET);
		if (-1 == lseek_off) {
			return -1;
		}
		read_ssize = squash_read(d, buf, nbyte);
		if (-1 == read_ssize) {
			return -1;
		}
		lseek_off = squash_lseek(d, backup_off, SQUASH_SEEK_SET);
		assert(backup_off == lseek_off);
		return read_ssize;
	} else {
		return pread(d, buf, nbyte, offset);
	}
}

ssize_t enclose_io_readv(int d, const struct iovec *iov, int iovcnt)
{
	if (SQUASH_VALID_VFD(d)) {
		int i;
		ssize_t i_ssize;
		ssize_t ret = 0;
		for (i = 0; i < iovcnt; ++i) {
			i_ssize = squash_read(d, iov[i].iov_base, iov[i].iov_len);
			if (-1 == i_ssize) {
				return -1;
			} else if (0 == i_ssize) {
				break;
			} else {
				ret += i_ssize;
			}
		}
		return ret;
	} else {
		return readv(d, iov, iovcnt);
	}
}

void* enclose_io_dlopen(const char* path, int mode)
{
    return dlopen(enclose_io_ifextract(path, NULL), mode);
}

int enclose_io_access(const char *path, int mode)
{
	if (enclose_io_cwd[0] && '/' != *path) {
		struct stat buf;
		sqfs_path enclose_io_expanded;
		size_t enclose_io_cwd_len;
		size_t memcpy_len;
		ENCLOSE_IO_GEN_EXPANDED_NAME(path);
		ENCLOSE_IO_CONSIDER_MKDIR_WORKDIR_RETURN(
			enclose_io_expanded,
			squash_stat(enclose_io_fs, enclose_io_expanded, &buf),
			access(mkdir_workdir_expanded, &buf)
		);
	} else if (enclose_io_is_path(path)) {
		struct stat buf;
		ENCLOSE_IO_CONSIDER_MKDIR_WORKDIR_RETURN(
			path,
			squash_stat(enclose_io_fs, path, &buf),
			access(mkdir_workdir_expanded, &buf)
		);
	} else {
		return access(path, mode);
	}
}

static int enclose_io_mkdir_consult(char *path, mode_t mode) {
	int ret;
	char *head = NULL;
	char *p = NULL;
	char *p_left = NULL;
	struct stat buf;
	char *path2;

	while (strlen(path) - 1 >= 0 && '/' == path[strlen(path) - 1]) {
		path[strlen(path) - 1] = 0;
	}

	head = strstr(path, enclose_io_mkdir_scope);
	if (NULL == head || '/' != head[strlen(enclose_io_mkdir_scope)]) {
		errno = ENOENT;
		return -1;
	}

	for (p = head + strlen(enclose_io_mkdir_scope) + 1; *p; p++) {
		if (*p == '/') {
			*p = 0;
			if (0 == squash_stat(enclose_io_fs, head, &buf) && S_ISDIR(buf.st_mode)) {
				path2 = strdup(path);
				if (NULL == path2) {
					return -1;
				} else {
					memmove(
						path2 + (head-path),
						path2 + (head-path) + strlen(enclose_io_mkdir_scope),
						strlen(path2 + (head-path) + strlen(enclose_io_mkdir_scope)) + 1
					);
					mkdir(path2, mode);
					free(path2);
				}
				*p = '/';
			} else {
				*p = '/';
				break;
			}
		}
	}
	memmove(
		head,
		head + strlen(enclose_io_mkdir_scope),
		strlen(head + strlen(enclose_io_mkdir_scope)) + 1
	);
	return mkdir(path, mode);
}

int enclose_io_mkdir(const char *path, mode_t mode)
{
	if (enclose_io_cwd[0] && '/' != *path) {
		struct stat buf;
		sqfs_path enclose_io_expanded;
		size_t enclose_io_cwd_len;
		size_t memcpy_len;
		int ret;
		ENCLOSE_IO_GEN_EXPANDED_NAME(path);
		ret = squash_stat(enclose_io_fs, enclose_io_expanded, &buf);
		if (0 == ret) {
			errno = EEXIST;
			return -1;
		} else {
			int ret_inner;
			const char* workdir;
			const char* workdir_path;
			workdir = enclose_io_mkdir_workdir();
			if (NULL == workdir) {
				errno = ENOENT;
				return -1;
			}
			workdir_path = malloc(strlen(workdir) + strlen(enclose_io_expanded) + 1);
			if (NULL == workdir_path) {
				errno = ENOMEM;
				return -1;
			}
			strcpy(workdir_path, workdir);
			strcat(workdir_path, enclose_io_expanded);
			ret_inner = enclose_io_mkdir_consult(workdir_path, mode);
			free(workdir_path);
			return ret_inner;
		}
	} else if (enclose_io_is_path(path)) {
		struct stat buf;
		int ret;
		ret = squash_stat(enclose_io_fs, path, &buf);
		if (0 == ret) {
			errno = EEXIST;
			return -1;
		} else {
			int ret_inner;
			const char* workdir;
			const char* workdir_path;
			workdir = enclose_io_mkdir_workdir();
			if (NULL == workdir) {
				errno = ENOENT;
				return -1;
			}
			workdir_path = malloc(strlen(workdir) + strlen(path) + 1);
			if (NULL == workdir_path) {
				errno = ENOMEM;
				return -1;
			}
			strcpy(workdir_path, workdir);
			strcat(workdir_path, path);
			ret_inner = enclose_io_mkdir_consult(workdir_path, mode);
			free(workdir_path);
			return ret_inner;
		}
	} else {
		return mkdir(path, mode);
	}
}

#endif // !_WIN32

int enclose_io_dos_return(int statement) {
#ifdef _WIN32
	int ret = (statement);
	if (-1 == ret) {
		ENCLOSE_IO_SET_LAST_ERROR;
		return ret;
	} else {
		return ret;
	}
#else
	return (statement);
#endif // _WIN32
}

short enclose_io_if(const char* path)
{
	if (enclose_io_cwd[0] && '/' != *path) {
		return 1;
	} else if (enclose_io_is_path(path)) {
		return 1;
	} else {
		return 0;
	}
}

SQUASH_OS_PATH enclose_io_ifextract(const char* path, const char* ext_name)
{
    if (enclose_io_cwd[0] && '/' != *path) {
        sqfs_path enclose_io_expanded;
        size_t enclose_io_cwd_len;
        size_t memcpy_len;
        ENCLOSE_IO_GEN_EXPANDED_NAME(path);
        return squash_extract(enclose_io_fs, enclose_io_expanded, ext_name);
    }
    else if (enclose_io_is_path(path)) {
        return squash_extract(enclose_io_fs, path, ext_name);
    }
    else {
        return path;
    }
}

void enclose_io_chdir_helper(const char *path)
{
        size_t memcpy_len = strlen(path);
        if (SQUASHFS_PATH_LEN - 1 < memcpy_len) {
        	memcpy_len = SQUASHFS_PATH_LEN - 1;
        }
        memcpy(enclose_io_cwd, path, memcpy_len);
        while ('/' == enclose_io_cwd[memcpy_len - 1]) {
        	memcpy_len--;
        }
        enclose_io_cwd[memcpy_len] = '/';
        enclose_io_cwd[memcpy_len + 1] = '\0';
}

int enclose_io_chdir(const char *path)
{
	if (enclose_io_is_path(path)) {
		if (mkdir_workdir) {
			sqfs_path mkdir_workdir_expanded;
			char *mkdir_workdir_expanded_head;
			size_t mkdir_workdir_len;
			size_t memcpy_len;
			struct stat mkdir_workdir_buf;
			mkdir_workdir_len = strlen(mkdir_workdir);
			memcpy(mkdir_workdir_expanded, mkdir_workdir, mkdir_workdir_len);
			memcpy_len = strlen(path);
			if (SQUASHFS_PATH_LEN - mkdir_workdir_len < memcpy_len) {
				memcpy_len = SQUASHFS_PATH_LEN - mkdir_workdir_len;
			}
			memcpy(&mkdir_workdir_expanded[mkdir_workdir_len], (path), memcpy_len);
			mkdir_workdir_expanded[mkdir_workdir_len + memcpy_len] = '\0';
			mkdir_workdir_expanded_head = strstr(mkdir_workdir_expanded, enclose_io_mkdir_scope);
			if (mkdir_workdir_expanded_head && '/' == mkdir_workdir_expanded_head[strlen(enclose_io_mkdir_scope)]) {
				memmove(
					mkdir_workdir_expanded_head,
					mkdir_workdir_expanded_head + strlen(enclose_io_mkdir_scope),
					strlen(mkdir_workdir_expanded_head + strlen(enclose_io_mkdir_scope)) + 1
				);
				if (0 == stat(mkdir_workdir_expanded, &mkdir_workdir_buf)) {
					int ret = chdir(mkdir_workdir_expanded);
					if (0 == ret) {
						enclose_io_chdir_helper(path);
					}
					return ret;
				}
			}
		}

		struct stat st;
		int ret;

		ret = squash_stat(enclose_io_fs, path, &st);
		if (-1 == ret) {
			#ifdef _WIN32
			ENCLOSE_IO_SET_LAST_ERROR;
			#endif
			return -1;
		}
		if (S_ISDIR(st.st_mode)) {
			enclose_io_chdir_helper(path);
			return 0;
		} else {
			errno = ENOENT;
			#ifdef _WIN32
			ENCLOSE_IO_SET_LAST_ERROR;
			#endif
			return -1;
		}
	} else {
		int ret;

		ret = chdir(path);
		if (0 == ret) {
			enclose_io_cwd[0] = '\0';
		}
		return ret;
	}
}

char *enclose_io_getcwd(char *buf, size_t size)
{
	if (enclose_io_cwd[0]) {
		size_t memcpy_len = strlen(enclose_io_cwd);
		if (NULL == buf) {
			buf = malloc((memcpy_len + 1) * sizeof(char));
			if (NULL == buf) {
				errno = ENOMEM;
				#ifdef _WIN32
				ENCLOSE_IO_SET_LAST_ERROR;
				#endif
				return NULL;
			}
		} else {
			if (size - 1 < memcpy_len) {
				memcpy_len = size - 1;
			}
		}
		memcpy(buf, enclose_io_cwd, memcpy_len);
		assert(memcpy_len - 1 >= 0);
		buf[memcpy_len - 1] = '\0';
		return buf;
	} else {
		return getcwd(buf, size);
	}
}

char *enclose_io_getwd(char *buf)
{
	return enclose_io_getcwd(buf, MAXPATHLEN);
}

int enclose_io_stat(const char *path, struct stat *buf)
{
	if (enclose_io_cwd[0] && '/' != *path) {
		sqfs_path enclose_io_expanded;
		size_t enclose_io_cwd_len;
		size_t memcpy_len;
		ENCLOSE_IO_GEN_EXPANDED_NAME(path);
		ENCLOSE_IO_CONSIDER_MKDIR_WORKDIR_RETURN(
			enclose_io_expanded,
			enclose_io_dos_return(squash_stat(enclose_io_fs, enclose_io_expanded, buf)),
			stat(mkdir_workdir_expanded, buf)
		);
	} else if (enclose_io_is_path(path)) {
		ENCLOSE_IO_CONSIDER_MKDIR_WORKDIR_RETURN(
			path,
			enclose_io_dos_return(squash_stat(enclose_io_fs, path, buf)),
			stat(mkdir_workdir_expanded, buf)
		);
	} else {
		return stat(path, buf);
	}
}

int enclose_io_fstat(int fildes, struct stat *buf)
{
	if (SQUASH_VALID_VFD(fildes)) {
		return enclose_io_dos_return(squash_fstat(fildes, buf));
	} else {
		return fstat(fildes, buf);
	}
}

int enclose_io_open(int nargs, const char *pathname, int flags, ...)
{
	if (enclose_io_cwd[0] && '/' != *pathname) {
		sqfs_path enclose_io_expanded;
		size_t enclose_io_cwd_len;
		size_t memcpy_len;
		ENCLOSE_IO_GEN_EXPANDED_NAME(pathname);
		if (!(O_CREAT & flags)) {
			ENCLOSE_IO_CONSIDER_MKDIR_WORKDIR_RETURN(
				enclose_io_expanded,
				enclose_io_dos_return(squash_open(enclose_io_fs, enclose_io_expanded)),
				open(mkdir_workdir_expanded, flags)
			);
		} else {
			va_list args;
			mode_t mode;
			assert(3 == nargs);
			va_start(args, flags);
			mode = va_arg(args, mode_t);
			va_end(args);
			if (mkdir_workdir) {
				sqfs_path mkdir_workdir_expanded;
				char *mkdir_workdir_expanded_head;
				size_t mkdir_workdir_len;
				size_t memcpy_len;
				struct stat mkdir_workdir_buf;
				mkdir_workdir_len = strlen(mkdir_workdir);
				memcpy(mkdir_workdir_expanded, mkdir_workdir, mkdir_workdir_len);
				memcpy_len = strlen(enclose_io_expanded);
				if (SQUASHFS_PATH_LEN - mkdir_workdir_len < memcpy_len) {
					memcpy_len = SQUASHFS_PATH_LEN - mkdir_workdir_len;
				}
				memcpy(&mkdir_workdir_expanded[mkdir_workdir_len], enclose_io_expanded, memcpy_len);
				mkdir_workdir_expanded[mkdir_workdir_len + memcpy_len] = '\0';
				mkdir_workdir_expanded_head = strstr(mkdir_workdir_expanded, enclose_io_mkdir_scope);
				if (mkdir_workdir_expanded_head && '/' == mkdir_workdir_expanded_head[strlen(enclose_io_mkdir_scope)]) {
					memmove(
						mkdir_workdir_expanded_head,
						mkdir_workdir_expanded_head + strlen(enclose_io_mkdir_scope),
						strlen(mkdir_workdir_expanded_head + strlen(enclose_io_mkdir_scope)) + 1
					);
					return enclose_io_dos_return(open(mkdir_workdir_expanded, flags, mode));
				} else {
					errno = ENOENT;
					return enclose_io_dos_return(-1);
				}
			} else {
				errno = ENOENT;
				return enclose_io_dos_return(-1);
			}
		}
	} else if (enclose_io_is_path(pathname)) {
		if (!(O_CREAT & flags)) {
			ENCLOSE_IO_CONSIDER_MKDIR_WORKDIR_RETURN(
				pathname,
				enclose_io_dos_return(squash_open(enclose_io_fs, pathname)),
				open(mkdir_workdir_expanded, flags)
			);
		} else {
			va_list args;
			mode_t mode;
			assert(3 == nargs);
			va_start(args, flags);
			mode = va_arg(args, mode_t);
			va_end(args);
			if (mkdir_workdir) {
				sqfs_path mkdir_workdir_expanded;
				char *mkdir_workdir_expanded_head;
				size_t mkdir_workdir_len;
				size_t memcpy_len;
				struct stat mkdir_workdir_buf;
				mkdir_workdir_len = strlen(mkdir_workdir);
				memcpy(mkdir_workdir_expanded, mkdir_workdir, mkdir_workdir_len);
				memcpy_len = strlen(pathname);
				if (SQUASHFS_PATH_LEN - mkdir_workdir_len < memcpy_len) {
					memcpy_len = SQUASHFS_PATH_LEN - mkdir_workdir_len;
				}
				memcpy(&mkdir_workdir_expanded[mkdir_workdir_len], pathname, memcpy_len);
				mkdir_workdir_expanded[mkdir_workdir_len + memcpy_len] = '\0';
				mkdir_workdir_expanded_head = strstr(mkdir_workdir_expanded, enclose_io_mkdir_scope);
				if (mkdir_workdir_expanded_head && '/' == mkdir_workdir_expanded_head[strlen(enclose_io_mkdir_scope)]) {
					memmove(
						mkdir_workdir_expanded_head,
						mkdir_workdir_expanded_head + strlen(enclose_io_mkdir_scope),
						strlen(mkdir_workdir_expanded_head + strlen(enclose_io_mkdir_scope)) + 1
					);
					return enclose_io_dos_return(open(mkdir_workdir_expanded, flags, mode));
				} else {
					errno = ENOENT;
					return enclose_io_dos_return(-1);
				}
			} else {
				errno = ENOENT;
				return enclose_io_dos_return(-1);
			}
		}
	} else {
		if (2 == nargs) {
			return open(pathname, flags);
		} else {
			va_list args;
			mode_t mode;
			va_start(args, flags);
			mode = va_arg(args, mode_t);
			va_end(args);
			return open(pathname, flags, mode);
		}
	}
}

int enclose_io_close(int fildes)
{
	if (SQUASH_VALID_VFD(fildes)) {
		return enclose_io_dos_return(squash_close(fildes));
	} else {
		return close(fildes);
	}
}

ssize_t enclose_io_read(int fildes, void *buf, size_t nbyte)
{
	if (SQUASH_VALID_VFD(fildes)) {
		return enclose_io_dos_return(squash_read(fildes, buf, nbyte));
	} else {
		return read(fildes, buf, nbyte);
	}
}

off_t enclose_io_lseek(int fildes, off_t offset, int whence)
{
	if (SQUASH_VALID_VFD(fildes)) {
		return enclose_io_dos_return(squash_lseek(fildes, offset, whence));
	} else {
		return lseek(fildes, offset, whence);
	}
}

short enclose_io_is_path(char *pathname)
{
	if (strnlen((pathname), 21) >= 21) {
		if (0 == strncmp((pathname), "/__enclose_io_memfs__", 21)
#ifdef _WIN32
			|| 0 == strncmp((pathname), "\\__enclose_io_memfs__", 21)
#endif
			) {
				return 1;
			}
	}
#ifdef _WIN32
	if (strnlen((pathname), 23) >= 23) {
		if (0 == strncmp((pathname) + 1, ":/__enclose_io_memfs__", 22)
			|| 0 == strncmp((pathname) + 1, ":\\__enclose_io_memfs__", 22)
			) {
				return 1;
			}
	}
	if (strnlen((pathname), 24) >= 24) {
		if (0 == strncmp((pathname), "\\\\?\\__enclose_io_memfs__", 24) ||
			0 == strncmp((pathname), "//?/__enclose_io_memfs__", 24)) {
				return 1;
			}
	}
	if (strnlen((pathname), 27) >= 27) {
		if (0 == strncmp((pathname), "\\\\?\\", 4) ||
			0 == strncmp((pathname), "//?/", 4)) {
			if (0 == strncmp((pathname) + 5, ":\\__enclose_io_memfs__", 22) ||
				0 == strncmp((pathname) + 5, ":/__enclose_io_memfs__", 22)) {
					return 1;
				}
		}
	}
#endif
	return 0;
}

#ifdef _WIN32
short enclose_io_is_path_w(wchar_t *pathname)
{
	if (wcsnlen((pathname), 21) >= 21) {
		if (0 == wcsncmp((pathname), L"/__enclose_io_memfs__", 21)
			|| 0 == wcsncmp((pathname), L"\\__enclose_io_memfs__", 21)
			) {
				return 1;
			}
	}
	if (wcsnlen((pathname), 23) >= 23) {
		if (0 == wcsncmp((pathname) + 1, L":/__enclose_io_memfs__", 22)
			|| 0 == wcsncmp((pathname) + 1, L":\\__enclose_io_memfs__", 22)
			) {
				return 1;
			}
	}
	if (wcsnlen((pathname), 24) >= 24) {
		if (0 == wcsncmp((pathname), L"\\\\?\\__enclose_io_memfs__", 24) ||
			0 == wcsncmp((pathname), L"//?/__enclose_io_memfs__", 24)) {
				return 1;
			}
	}
	if (wcsnlen((pathname), 27) >= 27) {
		if (0 == wcsncmp((pathname), L"\\\\?\\", 4) ||
			0 == wcsncmp((pathname), L"//?/", 4)) {
			if (0 == wcsncmp((pathname) + 5, L":\\__enclose_io_memfs__", 22) ||
				0 == wcsncmp((pathname) + 5, L":/__enclose_io_memfs__", 22)) {
					return 1;
				}
		}
	}
	return 0;
}

short enclose_io_is_relative_w(wchar_t *pathname)
{
	if (L'\\' == (pathname)[0] ||
		L'/' == (pathname)[0]) {
		return 0;
	}
	if (wcsnlen(pathname, 3) >= 3) {
		if (0 == wcsncmp((pathname) + 1, L":\\", 2) ||
			0 == wcsncmp((pathname) + 1, L":/", 2)) {
			return 0;
		}
	}
	if (wcsnlen(pathname, 4) >= 4) {
		if (0 == wcsncmp((pathname), L"\\\\?\\", 4) ||
			0 == wcsncmp((pathname), L"//?/", 4)) {
			return 0;
		}
	}
	if (wcsnlen(pathname, 7) >= 7) {
		if (0 == wcsncmp((pathname), L"\\\\?\\", 4) && 0 == wcsncmp((pathname) + 5, L":\\", 2) ) {
				return 0;
			}
		if (0 == wcsncmp((pathname), L"//?/", 4) && 0 == wcsncmp((pathname) + 5, L":/", 2) ) {
				return 0;
			}
	}
	return 1;
}
#endif
