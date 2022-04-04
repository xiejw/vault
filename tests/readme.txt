# vim: ft=help

This serves as the test data

>
    a
      - b        (normal dir)
        - c      (normal file)
      - h        (normal file)

      - e        (soft link to dir, which has file f/g)
      - d        (soft link to file)
<

We could use find to do the trick
>
    $ find -L a -type f | sort
    a/b/c
    a/d
    a/e/f/g
    a/h
<
