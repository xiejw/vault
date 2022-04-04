#include <dirent.h>
#include <stdio.h>
#include <string.h>
#include <sys/errno.h>

// eva
#include <base/log.h>
#include <base/types.h>

static error_t listFiles(const char *dirpath);

int
main()
{
        error_t err;

        logInfo("hello vault\n");

        err = listFiles(".");
        if (err) {
                logFatal("fatal error");
                errDump("failed to list files.");
        }
        return 0;
}

error_t
listFiles(const char *dirpath)
{
        struct dirent *dp;
        DIR *dirp = opendir(dirpath);
        if (dirp == NULL) {
                return errNew("failed to open dir: %s", dirpath);
        }

        for (;;) {
                // stage 1. read entry from dirp.
                errno = 0;              // to distingush err from end-of-dir.
                dp    = readdir(dirp);  // TODO use readdir_r
                if (dp == NULL) break;  // either error or end-of-dir

                // stage 2: clean up
                if (strcmp(dp->d_name, ".") == 0 ||
                    strcmp(dp->d_name, "..") == 0)
                        continue;  // skip . and ..

                // stage 3: handling
                logInfo("entry: %s", dp->d_name);
        }

        closedir(dirp);
        if (errno != 0) return errNew("failed to read dir");

        return OK;
}
