tfname = "syscall_numbers.h.template"
dfname = "syscall_numbers.txt"
ofname = "syscall_numbers.h"

fstring = "#define %s%s \t(%d)"

tsym = "<~syscall-list~>"

tftext = open(tfname).read()
dflist = open(dfname).read().split()
ofile = open(ofname, 'w')

maxlen = max([len(s) for s in dflist])

rtext = '\n'.join([fstring % (s, " "*(maxlen - len(s)), i) for i,s in enumerate(dflist)])

tftext = tftext.replace(tsym, rtext)

ofile.write(tftext)

ofile.close()
