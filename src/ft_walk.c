#include "ft_walk.h"

#include <dirent.h>
#include <errno.h>
#include <string.h>
#include <sys/stat.h>

// eva
#include <base/fpath.h>
#include <base/log.h>
#include <base/mm.h>
#include <base/types.h>

static void ftNodeFree(struct ft_node *);
static struct ft_node *ftNodeNew(void);
static error_t listFiles(struct arr *, const struct ft_walk_config *);

//------------------------------------------------------------------------------
// stack macros
//
// considere to moved to eva.
//------------------------------------------------------------------------------
#define STACK_PUSH(stk, ele) *(__typeof__(ele) *)arrPush((stk)) = (ele)
// #define STACK_POP(stk, type) (*(type *)arrPop((stk)))
#define STACK_POP_AND_ASSIGN(stk, ele) \
        (ele) = (*(__typeof__(ele) *)arrPop((stk)))

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
        logTrace("checking symbolic path: %s", fs_path);

        if (stat(fs_path, &statbuf) == 0) {
                *output_flag = S_ISDIR(statbuf.st_mode);
                return OK;
        }

        // now handling error cases.
        if (errno != ENOENT)
                return errNew("failed to stat the symbolic link: %s",
                              strerror(errno));

        // this is a dangling link
        if (dangling_sym_link == FT_ERROR_OUT) {
                return errNew("sym link does not exist: %s", fs_path);
        }

        if (dangling_sym_link == FT_WARNING) {
                logWarn(
                    "sym link does not exist (but process will "
                    "continue): %s",
                    fs_path);
                goto out;
        }

        assert(dangling_sym_link == FT_SILENT);
        logDebug("sym link does not exist: %s", fs_path);

out:
        *output_flag = 2;  // should skip
        return OK;
}

error_t
walkTree(struct ft_node *root, const struct ft_walk_config *cfg)
{
        error_t err       = OK;
        struct arr *stack = arrNewStack(sizeof(struct ft_node *), 128, 1024);
        STACK_PUSH(stack, root);

        // TODO check root is truelly a dir.
        while (!arrIsEmpty(stack)) {
                err = listFiles(stack, cfg);
                if (err) break;
        }
        arrFree(stack);
        return err;
}

error_t
listFiles(struct arr *stack, const struct ft_walk_config *cfg)
{
        error_t err = OK;

        struct ft_node *parent;
        STACK_POP_AND_ASSIGN(stack, parent);

        const sds_t parent_path      = parent->path;  // alias
        const size_t parent_path_len = sdsLen(parent_path);

        const char *dirpath = fpJoinSds(parent->root_dir, parent_path);
        DIR *dirp           = opendir(dirpath);
        if (dirp == NULL) return errNew("failed to open dir: %s", dirpath);

        logTrace("readdir: %s", dirpath);

        struct ft_node *child;
        for (;;) {
                // ------------------------------------------------------------
                // stage 1. read entry from dirp.
                errno             = 0;  // to distingush err from end-of-dir.
                struct dirent *dp = readdir(dirp);
                if (dp == NULL) break;  // either error or end-of-dir

                // ------------------------------------------------------------
                // stage 2: skip hidden file, "." and "..".
                const char *d_name = dp->d_name;
                assert(dp->d_name[0] != 0);
                if (d_name[0] == '.') continue;

                // ------------------------------------------------------------
                // stage 3: fill the child node
                child           = ftNodeNew();
                child->parent   = parent;
                child->root_dir = parent->root_dir;  // alias
                child->path     = fpJoin(parent_path, parent_path_len, d_name,
                                         strlen(d_name));
                child->is_dir   = 0;  // will be set later.

                vecPushBack(&parent->children, child);

                // ------------------------------------------------------------
                // stage 4: handle symb link
                unsigned int dtype = dp->d_type;
                if (dtype == DT_LNK) {
                        int output_flag;
                        err = checkSymLinkType(cfg->dangling_sym_link, child,
                                               &output_flag);

                        errno = 0;  // reset

                        // we leave the child in the tree (for resource
                        // clean up) and fast return
                        if (err) goto exit;

                        switch (output_flag) {
                        case 0:  // file
                                dtype = DT_REG;
                                break;
                        case 1:  // dir
                                dtype = DT_DIR;
                                break;
                        case 2:  // skip item
                                // we will
                                // - remove child from the tree.
                                // - leave dtype as link to skip the handle in
                                // next stage
                                child = vecPopBack(parent->children);
                                ftNodeFree(child);
                                break;
                        default:
                                errPanic("should not reach not");
                        }

                        // if no rewrite, it is skipped item.
                        if (dtype == DT_LNK) continue;
                }

                // ------------------------------------------------------------
                // stage 5: handle dir vs file
                switch (dtype) {
                case DT_DIR:
                        logTrace("entry [d]: %s", d_name);
                        child->is_dir = 1;
                        STACK_PUSH(stack, child);
                        break;
                case DT_REG:
                        logTrace("entry [f]: %s", d_name);
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

struct ft_node *
ftRootNew(sds_t root_dir)
{
        struct ft_node *root = ftNodeNew();
        root->root_dir       = root_dir;
        root->path           = sdsEmptyWithCap(0);  // never grow
        root->is_dir         = 1;

        // TODO (check is a dir)
        return root;
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

static void
ftDumpImpl(int fd, struct ft_node *node, sds_t space)
{
        int is_dir;
        // append 4 spaces.
        sdsCatPrintf(&space, "    ");

        for (size_t i = 0; i < vecSize(node->children); i++) {
                struct ft_node *child = node->children[i];
                is_dir                = child->is_dir;
                dprintf(fd, "%s+-> %s%s\n", space, child->path,
                        is_dir ? " (+)" : "");

                if (is_dir) {
                        ftDumpImpl(fd, child, space);
                }
        }

        // remove 4 spaces
        size_t len = sdsLen(space);
        sdsSetLen(space, len - 4);
        space[len - 4] = 0;
}

// dump the tree representation to fd (say stdout).
void
ftDump(int fd, struct ft_node *root)
{
        sds_t space = sdsEmpty();
        dprintf(fd, "%sroot - %s\n", space, root->root_dir);

        ftDumpImpl(fd, root, space);
        sdsFree(space);
}
