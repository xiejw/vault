# vim: ft=help

================================================================================
OVERVIEW~

{Banana} is a folder-level file backup system. It does not make any changes to
user's folder but could answer the following questions

1. Any file change since last check? Or historical changes in the past.
2. When does a file get added/deleted/mutated? And how?
3. With recent removed files, do they have a copy (duplicated or moved) in the
   folder?
4. (Plugin) How to recover a file, even it is a deleted?

The design was motivated by the photo repository backup system. But it should
be general enough for other important folders.

================================================================================
DESIGN~

>
                           +------------+
                           |  folder    |
                           +------------+
                                 ^
                                 |
                                 |                                    user space
    -----------------------------+----------------------------------------------
                                 |                                  banana space
                                 |
                           +------------+
                           |  banana    |
                           +------------+
                                 |
                    +------------+-----------------------+
                    |                                    | (if provided)
                    v                                    v
              +-------------+                        +----------+
              | history_log |                        |  pool    |
              +-------------+                        +----------+
<

{history_log} is an incremental, cmd log like, plain text file which records
              the change from scratch to the one (called 'a') snapshot of the
              folder. It records file add/delete in each line with checksum
              (sha256) and time stamp (epoch seconds).

              Due to the fact it is append-only, any change in history can be
              queried easily.

{folder}      is user's directory to be backed up. |banana| will never mutate
              it. It is assumed the |history_log| is never newer than
              |folder|.

{pool}        is a folder which contains all copies of the files in |folder|,
              including all the files which were deleted in the past. To make
              it easy for human checks, all files are named as

                  "<sha256_checksum>.<ext>"

              Example, file 'a/b/c.jpg' with checksum '0xabcde' is saved as

                  "0xabcde.jpg"

              As it could consume tons of spaces, it will be optional.

================================================================================
WORKFLOW~

- {step0}:  |banana| loads |history_log| and creates a in-memory snapshot 'a'.
- {step1a}: |banana| checks whether all checksums (including the
            deleted ones) are in the |pool|.

- {step1b}: In parallel (with |step0| -> |step1a|), |banana| walks the user
            folder and creates a in-memory snapshot 'b'.

- {step2}:  join |step1a| and |step1b|, then go to |step3|.

- {step3}:  |banana| generates diffs between snapshots 'a' and 'b' and
            computes some statistics (how many new files, delete files, etc).
- {step4}:  |banana| promots the user and gets permission to proceed. Depends on
            the answer, will split to two branches |step5a| and |step5b|.

- {step5a}: If no, stop.
- {step5b}: If yes, it will copy all new files into the |pool| and writes the
            diff into |history_log| so 'a_new' and 'b' are same. For file
            saving/swapping, we need to ensure it is atomic (temp file
            followed by rename).

================================================================================
INTERNAL DATA STRUCTURES~

>
                                     +--------+
                                     | Folder |
                                     +--------+
                                         |
                                         |                            user space
    -------------------------------------+--------------------------------------
                                         |                          banana space
                                    FromLocalFS
                                         |       +- ConvertTo --+
             +--> FromCmdLogs ---+       v       |              |
 +---------+ |                   |  +----------+ |              v
 | CmdLogs |-+                   +->| FileTree |-+        +------------+
 +---------+                        +----------+          | DiffResult |
      ^                                                   +------------+
      |                                                         |
      +----------------------------- ToCmdLogs <----------------+
<
