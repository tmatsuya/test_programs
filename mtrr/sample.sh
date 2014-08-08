# cat /proc/mtrr			// display MTRR information

# echo "disable=1" >| /proc/mtrr	//  remove reg01 from /proc/mtrr
# echo "disable=2" >| /proc/mtrr	//  remove reg02 from /proc/mtrr

# echo "base=0xe2000000 size=0x2000000 type=write-combining" >| /proc/mtrr
# echo "base=0xfb000000 size=0x1000 type=uncachable" >| /proc/mtrr

# type list linux/Documentation/x86/mtrr.txt
#     "uncachable",               /* 0 */
#     "write-combining",          /* 1 */
#     "?",                        /* 2 */
#     "?",                        /* 3 */
#     "write-through",            /* 4 */
#     "write-protect",            /* 5 */
#     "write-back",               /* 6 */
