#include <dirent.h>
#include <errno.h>
#include <stdio.h>
#include <string.h>
#include <sys/stat.h>

// eva
#include <adt/sds.h>
#include <adt/vec.h>
#include <base/log.h>
#include <base/mm.h>
#include <base/types.h>

struct ft_node {
        struct ft_node *parent;  // unowned. if NULL, this is root.
        sds_t root_dir;          // owned by root; alias for non-root.
        sds_t path;              // owned by non-root. exclude the root_dir.
        vec_t(struct ft_node *) children;  // owned. if NULL, this must be file.
};

struct ft_node *ftNodeNew(void);
static void ftNodeFree(struct ft_node *);
void ftFree(struct ft_node *root);

static error_t listFiles(const char *dirpath);

int
main()
{
        error_t err;

        logInfo("hello vault\n");

        struct arr *arr = arrNewStack(sizeof(struct ft_node *), 128, 1024);

        struct ft_node *root = ftNodeNew();
        root->parent         = NULL;
        root->root_dir       = sdsNew("test/a");
        root->path           = NULL;

        err = listFiles("tests/a");
        if (err) {
                logFatal("fatal error");
                errDump("failed to list files.");
        }

        // // test dangling link
        // {
        //         struct stat statbuf;
        //         // const char* test_file = "tests/a/d"; // exist file
        //         // const char* test_file = "tests/a/e"; // exist dir
        //         const char *test_file = "tests/a/non-exist";  // dangling
        //         link

        //         logInfo("test file: %s", test_file);
        //         if (stat(test_file, &statbuf)) {
        //                 logFatal("error: %d (ENOENT: %d)  -- %s", errno,
        //                          errno == ENOENT, strerror(errno));
        //         } else {
        //                 logInfo("is file: %d", S_ISREG(statbuf.st_mode));
        //                 logInfo("is dir: %d", S_ISDIR(statbuf.st_mode));
        //         }
        // }

        ftFree(root);
        arrFree(arr);
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

                // stage 4: type
                switch (dp->d_type) {
                case DT_DIR:
                        logInfo("  -> is dir");
                        break;
                case DT_REG:
                        logInfo("  -> is reg file");
                        break;
                case DT_LNK:
                        logInfo("  -> is symbolic link");
                        break;
                default:
                        logFatal("  -> unknown type");
                }
        }

        closedir(dirp);
        if (errno != 0) return errNew("failed to read dir");

        return OK;
}

struct ft_node *
ftNodeNew(void)
{
        struct ft_node *node = malloc(sizeof(*node));

        // parent, root_dir, path should be set by caller.
        node->children = vecNew();
        return node;
}

void
ftNodeFree(struct ft_node *p)
{
        const int is_root = p->parent == NULL;

        if (is_root) {
                sdsFree(p->root_dir);
        } else {
                sdsFree(p->path);
        }

        vecFree(p->children);  // ftFree will free all children.
        free(p);
}

// helper
static void
ftSubTreeFree(struct ft_node *node)
{
        // dfs walk, might overflow stacks.
        vec_t(struct ft_node *) children = node->children;
        const size_t child_count         = vecSize(children);

        for (size_t i = 0; i < child_count; i++) {
                ftSubTreeFree(children[i]);
        }

        ftNodeFree(node);
}

void
ftFree(struct ft_node *root)
{
        assert(root->parent == NULL);
        ftSubTreeFree(root);
}
