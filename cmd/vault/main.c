#include <dirent.h>
#include <errno.h>
#include <stdio.h>
#include <string.h>
#include <sys/stat.h>

// eva
#include <adt/sds.h>
#include <adt/vec.h>
#include <base/fpath.h>
#include <base/log.h>
#include <base/mm.h>
#include <base/types.h>

struct ft_node {
        struct ft_node *parent;  // unowned. if NULL, this is root.
        sds_t root_dir;          // owned by root; alias for non-root.
        sds_t path;              // owned. exclude the root_dir. "" for root.
        vec_t(struct ft_node *) children;  // owned. if NULL, this must be file.
};

struct ft_node *ftNodeNew(void);
static void ftNodeFree(struct ft_node *);
void ftFree(struct ft_node *root);
void ftDump(int fd, struct ft_node *root);

#define STACK_PUSH(stk, ele) *(__typeof__(ele) *)arrPush((stk)) = (ele)
// #define STACK_POP(stk, type) (*(type *)arrPop((stk)))
#define STACK_POP_AND_ASSIGN(stk, ele) \
        (ele) = (*(__typeof__(ele) *)arrPop((stk)))

#define FT_ERROR_OUT 0
#define FT_WARNING   1
#define FT_SILENT    2

struct ft_walk_config {
        // values:
        //   - 0 error out (default)
        //   - 1 warning
        //   - 2 do nothing
        int dangling_sym_link;
};

static error_t listFiles(struct arr *, const struct ft_walk_config *);

int
main()
{
        error_t err;

        logInfo("hello vault\n");

        struct arr *stack = arrNewStack(sizeof(struct ft_node *), 128, 1024);

        struct ft_node *root = ftNodeNew();
        root->root_dir       = sdsNew("tests/a");
        root->path           = sdsEmptyWithCap(0);  // never grow

        STACK_PUSH(stack, root);

        // *(struct ft_node **)(arrPush(stack)) = root;

        struct ft_walk_config cfg = {
            .dangling_sym_link = FT_WARNING,
        };

        err = listFiles(stack, &cfg);
        if (err) {
                logFatal("fatal error");
                errDump("failed to list files.");
        }

        ftDump(1, root);

        ftFree(root);
        arrFree(stack);
        return 0;
}

// check whether the symbolic link embeded in 'node' is a dir.
//
// - if the link is dangling, depending on dangling_sym_link flag, options will
//   be different. See struct ft_walk_config for details.
//
// return:
//     output flag (upon success):
//         - 0 is file
//         - 1 is dir
//         - 2 should skip this node.
static error_t
checkSymLinkType(int dangling_sym_link, struct ft_node *node,
                 _out_ int *output_flag)
{
        struct stat statbuf;
        sds_t fs_path = fpJoinSds(node->root_dir, node->path);
        logDebug("checking symbolic path: %s", fs_path);

        if (stat(fs_path, &statbuf) == 0) {
                *output_flag = S_ISDIR(statbuf.st_mode);
                return OK;
        }

        // now handling error cases.
        if (errno == ENOENT) {  // this is a dangling link
                errno = 0;      // reset
                if (dangling_sym_link == FT_ERROR_OUT) {
                        return errNew("sym link does not exist: %s", fs_path);
                } else if (dangling_sym_link == FT_WARNING) {
                        logWarn(
                            "sym link does not exist (but process will "
                            "continue): %s",
                            fs_path);
                        *output_flag = 2;  // should skip
                        return OK;
                } else {
                        assert(dangling_sym_link == FT_SILENT);
                        logDebug("sym link does not exist: %s", fs_path);
                        *output_flag = 2;  // should skip
                        return OK;
                }
        } else {
                return errNew("failed to stat the link: %s", strerror(errno));
        }
}

error_t
listFiles(struct arr *stack, const struct ft_walk_config *cfg)
{
        error_t err;
        struct dirent *dp;

        struct ft_node *parent;

        STACK_POP_AND_ASSIGN(stack, parent);

        const char *dirpath = parent->root_dir;
        DIR *dirp           = opendir(dirpath);
        if (dirp == NULL) {
                return errNew("failed to open dir: %s", dirpath);
        }

        err = OK;
        struct ft_node *child;
        sds_t parent_path      = parent->path;  // alias
        size_t parent_path_len = sdsLen(parent_path);

        int output_flag;

        for (;;) {
                // stage 1. read entry from dirp.
                errno = 0;              // to distingush err from end-of-dir.
                dp    = readdir(dirp);  // TODO use readdir_r
                if (dp == NULL) break;  // either error or end-of-dir

                // stage 2: clean up
                // skip hidden file, "." and "..".
                const char *d_name = dp->d_name;
                assert(dp->d_name[0] != 0);
                if (d_name[0] == '.') continue;

                // stage 3: handling
                child           = ftNodeNew();
                child->parent   = parent;
                child->root_dir = parent->root_dir;  // alias
                child->path     = fpJoin(parent_path, parent_path_len, d_name,
                                         strlen(d_name));

                vecPushBack(&parent->children, child);

                // stage 4: type
                switch (dp->d_type) {
                case DT_DIR:
                        logDebug("entry [d]: %s", d_name);
                        STACK_PUSH(stack, child);
                        break;
                case DT_REG:
                        logDebug("entry [f]: %s", d_name);
                        break;
                case DT_LNK:
                        logDebug("entry [l]: %s", d_name);
                        err = checkSymLinkType(cfg->dangling_sym_link, child,
                                               &output_flag);

                        if (err) {
                                // we leave the child in the tree and fast
                                // return
                                goto exit;
                        }

                        switch (output_flag) {
                        case 0:  // file
                                break;
                        case 1:  // dir
                                STACK_PUSH(stack, child);
                                break;
                        case 2:  // skip item
                                // we will remove child from the tree.
                                child = vecPopBack(parent->children);
                                ftNodeFree(child);
                                break;
                        default:
                                errPanic("should not reach not");
                        }

                        break;
                default:
                        err = errNew("unsupported filetype: %d", dp->d_type);
                        goto exit;
                }
        }

exit:
        closedir(dirp);
        if (errno != 0)
                return errNew("failed to read dir: %s", strerror(errno));

        return err;
}

// create a new node with zeros filled in all fields.
struct ft_node *
ftNodeNew(void)
{
        return calloc(1, sizeof(struct ft_node));
}

// free a single node (will not touch the children it owns).
void
ftNodeFree(struct ft_node *p)
{
        const int is_root = p->parent == NULL;

        if (is_root) {
                sdsFree(p->root_dir);
        }
        sdsFree(p->path);
        vecFree(p->children);  // ftFree will free all children.
        free(p);
}

// helper
//
// free the subtree at 'node', where 'node' can be a leaf.
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

// free the entire tree rooted at 'root'
void
ftFree(struct ft_node *root)
{
        assert(root->parent == NULL);
        ftSubTreeFree(root);
}

// dump the tree representation to fd (say stdout).
void
ftDump(int fd, struct ft_node *root)
{
        sds_t space = sdsEmpty();

        if (root->parent == NULL) dprintf(fd, "%snode (root)\n", space);

        sdsCatPrintf(&space, "    ");

        for (size_t i = 0; i < vecSize(root->children); i++) {
                struct ft_node *child = root->children[i];
                int non_empty_dir = vecSize(child->children) > 1;
                dprintf(fd, "%s+-> %s %s\n", space, child->path,
                                non_empty_dir ? "(+)" : "");
        }

        sdsFree(space);
}
