cmpt300 as4

    This implementation of ls follows the functionality of the ls command for the three flag options
    -i -R -l

    Some notes on the behaviour that I implemented to match the ls command

        1. When only one directory is inputted in the command line, the name of the directory is not printed
           ex: ls temp_folder will just print the files/directory names without printing temp_folder: first.
           When more than one folder is inputted, the directory name will be printed before the files. This
           is also the case if the -R flag is included, regardless of the # of files inputted. This behaviour
           matches the ls command

        2. Symbolic links are handled exactly as standard ls does.
           
           When symbolic link is passed as command line argument:
                if link points to file, there are no special cases

                if link points to a directory:
                    the directory will be followed and traversed/expanded. The only exception is if the -l flag is used
                    in this case, the link will not be followed. This follows the normal ls command:

                    in coreutils 8.x, the -H flag (follow symlinks in arguments) is implied if -l, -L, 
                    and some other flags are not set


            When symbolic links are not passed as arguments
                if link points to file, there are no special cases

                if link points to a directory, it is not followed or traversed/expanded.


            I know this seems like strange behaviour, but this is how ls works

        3. Newlines are printed after each file is listed. This doesn't follow ls, but does follow the AS4.docx provided
