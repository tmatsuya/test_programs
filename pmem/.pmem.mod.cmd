savedcmd_/home/tmatsuya/test_programs/pmem/pmem.mod := printf '%s\n'   pmem.o | awk '!x[$$0]++ { print("/home/tmatsuya/test_programs/pmem/"$$0) }' > /home/tmatsuya/test_programs/pmem/pmem.mod
