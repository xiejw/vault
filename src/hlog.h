#ifndef HLOG_H_
#define HLOG_H_

// eva
#include <adt/vec.h>

// vault
#include <ft.h>

// -----------------------------------------------------------------------------
// public history log data structures
// -----------------------------------------------------------------------------

// hlog file spec
//
// hlog file is a plain text file. It is line oriented and each line must end
// with a new line character.
//
// The line order is the chronological. It is append-only mode. By following
// the lines to reconstruct the file tree ('ft'), the snapshot should be
// identical to the current file tree.
//
// Empty lines or lines only consist empty spaces (tabs) are not allowed.
// Trailing spaces are not allowed. This basically means the file name cannot
// end with spaces.
//
// Each line must start with either "+" (adding a file) and "-" (deleting a
// file), followed by single empty space.
//   - renaming a file can be recorded with a "-" and "+"
//   - changing a file can also be recorded with a "-" and "+"
//   - all these are not required to be placed next to each other.
//
// After that, (immediately), it is a valid epoch (in seconds) time stamp for
// this operation (adding or deleting), followed by single empty space.
//
// After that, (immediately), it is a valid 64-length characters for sha256
// checksum for the file, followed by single empty space.
//
// After that, (immediately), it is a single string without any quotes for the
// file name, without leading/trailing slash. Empty spaces are allowed for any
// component but not for the suffix for the final component. This must be a
// file. Directory is implied.
//
// A quick summary could be:
//
// clang-format off
//
// "+ 1652901470 e3b0c44298fc1c149afbf4c8996fb92427ae41e4649b934ca495991b7852b855 foo/bar"
//
// clang-format on

#define HLOG_ADD 1
#define HLOG_DEL 0

struct hlog {
        int cmd;                         // 1: addition and 0: deletion.
        u64_t timestamp;                 // epoch in seconds.
        sds_t path;                      // file path (exclude root_dir).
        unsigned char checksum[64 + 1];  // 0-ended sha256 checksum.
};

// convert the hlog list into an ft_node tree.
//
// Empty dir will be removed. But it is not sorted.
extern error_t hlogToFt(_moved_in_ sds_t root_dir, vec_t(struct hlog *) hlogs,
                        _out_ struct ft_node **root);

#endif  // HLOG_H_
