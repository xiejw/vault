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
void ftDump(int fd, struct ft_node *root);

static error_t listFiles(struct arr *);

#define STACK_PUSH(stk, ele) *(__typeof__(ele) *)arrPush((stk)) = (ele)
// #define STACK_POP(stk, type) (*(type *)arrPop((stk)))
#define STACK_POP_AND_ASSIGN(stk, ele) \
        (ele) = (*(__typeof__(ele) *)arrPop((stk)))

int
main()
{
        error_t err;

        logInfo("hello vault\n");

        struct arr *stack = arrNewStack(sizeof(struct ft_node *), 128, 1024);

        struct ft_node *root = ftNodeNew();
        root->root_dir       = sdsNew("tests/a");

        STACK_PUSH(stack, root);

        // *(struct ft_node **)(arrPush(stack)) = root;

        err = listFiles(stack);
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

        ftDump(1, root);

        ftFree(root);
        arrFree(stack);
        return 0;
}

// TODO join path

error_t
listFiles(struct arr *stack)
{
        error_t err;
        struct dirent *dp;

        struct ft_node *parent;
        // parent = STACK_POP(stack, struct ft_node *);
        STACK_POP_AND_ASSIGN(stack, parent);

        const char *dirpath = parent->root_dir;
        DIR *dirp           = opendir(dirpath);
        if (dirp == NULL) {
                return errNew("failed to open dir: %s", dirpath);
        }

        err = OK;
        struct ft_node *child;
        for (;;) {
                // stage 1. read entry from dirp.
                errno = 0;              // to distingush err from end-of-dir.
                dp    = readdir(dirp);  // TODO use readdir_r
                if (dp == NULL) break;  // either error or end-of-dir

                // stage 2: clean up
                //
                // TODO: skip hidden file as well.
                if (strcmp(dp->d_name, ".") == 0 ||
                    strcmp(dp->d_name, "..") == 0)
                        continue;  // skip . and ..

                // stage 3: handling
                logInfo("entry: %s", dp->d_name);

                child           = ftNodeNew();
                child->parent   = parent;
                child->root_dir = parent->root_dir;    // alias
                child->path     = sdsNew(dp->d_name);  // TODO should join
                vecPushBack(&parent->children, child);

                // stage 4: type
                switch (dp->d_type) {
                case DT_DIR:
                        logInfo("  -> is dir");

                        STACK_PUSH(stack, child);
                        break;
                case DT_REG:
                        logInfo("  -> is reg file");
                        break;
                case DT_LNK:
                        logInfo("  -> is symbolic link");
                        // We don't know whether this is a dir or file. Put it
                        // into the stack so next round we will understand it.
                        STACK_PUSH(stack, child);
                        break;
                default:
                        err = errNew("unsupported filetype: %d", dp->d_type);
                        goto exit;
                }
        }

exit:
        closedir(dirp);
        if (errno != 0) return errNew("failed to read dir");

        return err;
}

struct ft_node *
ftNodeNew(void)
{
        return calloc(1, sizeof(struct ft_node));
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

void
ftDump(int fd, struct ft_node *root)
{
        sds_t space = sdsEmpty();

        if (root->parent == NULL) dprintf(fd, "%snode (root)\n", space);

        sdsCatPrintf(&space, "    ");

        for (size_t i = 0; i < vecSize(root->children); i++) {
                struct ft_node *child = root->children[i];
                dprintf(fd, "%s+-> %s\n", space, child->path);
        }

        sdsFree(space);
}
