CMPT-300 as3

All requirements have been met. Some design considerations have been made and are summarized below:

Considerations:
    1. The absolute paths of the metadata file and input files are to be provided as inputs

    2. The metadata file will contain 1.0 or 1 if it wishes to use default values as described
       here: https://piazza.com/class/lhaqnxludbb4e3/post/241

    3. Carriage returns are not counted as bytes. newlines are. If only newlines are read
       in the k bytes, it is interpreted as a zero. The last \n in the file is not counted as a
       zero. As described here: https://piazza.com/class/lhaqnxludbb4e3/post/404

    4. The nth line of each input file is summed together to form the nth line of the output file.
       My implementation matches the examples in the AS3.docx. I chose to prioritize the structure
       of the input files (the positioning of data in each file has meaning) over the inputted # of
       threads. Therefore, the output is the same no matter how many threads are used, as long as 
       all other inputs remain consistent. I believe this is a good option, as it abstracts the
       notion of the number of threads from the user, so the user gets the same output every time.

       Example where each thread gets two files:
       [lineN, file1, thread1] + [lineN, file2, thread1] + [lineN, file3, thread2] + [lineN, file4, thread2] = [output file line N] 

       From as3.docx: To find the mixed output, add the corresponding samples from all the channels.

    5. My input files are provided, however they use the absolute path i.e. /home/nick/cmpt300/as3/...
       You'll want to change these if you choose to use them
