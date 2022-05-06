# vim: ft=help

================================================================================
OVERVIEW~

{vault} is a folder-level file backup system. It does not make any changes to
user's folder but could answer the following questions efficiently

1. Any file change since last check?
    - or any historical changes in the past.
2. When does a file get added/deleted/mutated?
3. With recent removed files, do they have a copy in the folder?
    - so this is a safe move rather than unexpected deletion.
4. How to recover a file, even it is a deleted?

The design was motivated by the photo repository backup system and git system.
But it should be general enough for other important folders, especially with
file system organized by symbolic links (link to external hard disk due to
space limitation).

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
                                 |                                   vault space
                                 |
                           +------------+
                           |  vault     |
                           +------------+
                                 |
                    +------------+-----------------------+
                    |                                    |
                    v                                    v
              +-------------+                        +----------+
              | history_log |                        |  pool    |
              +-------------+                        +----------+
<

{folder}      is user's directory to be backed up. |vault| will never mutate
              it. It is assumed the |history_log|(|hlog|) is never newer than
              |folder|.

{history_log} is an incremental, change log like, plain text file which records
{hlog}        the change from scratch to the snapshot of the folder. It
              records file add/delete in each line with checksum (sha256) and
              time stamp (epoch seconds).

              Due to the fact it is append-only, any change in history can be
              queried easily.

{pool}        is a folder which contains all copies of the files in |folder|,
              including all the files which were deleted in the past. To make
              it easy for human checks, all files are named as

                  "<sha256_checksum>.<ext>"

              Example, file 'a/b/c.jpg' with checksum '0xabcde' is saved as

                  "0xabcde.jpg"

              So limit the number of files in one folder, we copy the git
              design and store the file like

                  "0x"/"abcde.jpg"

              As it could consume tons of spaces, it will be on one external
              harddrive with enough spaces.

================================================================================
WORKFLOW~

- {step0}:  |vault| loads |history_log| and creates a in-memory snapshot 'a'.
- {step1a}: |vault| checks whether all checksums (including the
            deleted ones) are in the |pool|.

- {step1b}: In parallel (with |step0| -> |step1a|), |vault| walks the user
            folder and creates a in-memory snapshot 'b'.

- {step2}:  join |step1a| and |step1b|, then go to |step3|.

- {step3}:  |vault| generates diffs between snapshots 'a' and 'b' and
            computes some statistics (how many new files, delete files, etc).
- {step4}:  |vault| promots the user and gets permission to proceed. Depends on
            the answer, will split to two branches |step5a| and |step5b|.

- {step5a}: If no, stop.
- {step5b}: If yes, it will copy all new files into the |pool| and writes the
            diff into |history_log| so 'a_new' and 'b' are same. For file
            saving/swapping, we need to ensure it is atomic (temp file
            followed by rename).

================================================================================
INTERNAL DATA STRUCTURES~

TODO: Needs change to reflect the c code

>
                                     +--------+
                                     | Folder |
                                     +--------+
                                         |
                                         |                            user space
    -------------------------------------+--------------------------------------
                                         |                           vault space
                                      ftWalk
                                         |       +-   ftDiff  --+
             +-->  hlogToFt ----+        v       |              |
 +---------+ |                   |  +----------+ |              v
 |   hlog  |-+                   +->|    ft    |-+        +------------+
 +---------+                        +----------+          | diff: vec_l|
      ^                                                   |       vec_r|
      r                                                   +------------+
      |                                                         |
      +-------------------------- hlogFromDiff <----------------+
<
