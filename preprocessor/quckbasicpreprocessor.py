import sys

prcessed=[]
labels=[]
linenums=[]
filename=sys.argv[1]
f = open(filename, "r")
line="String"#line needs to be initiated to something, it should be over ridden later, if you see String in hte memory, then this didnt work properly
linenumber=0
while line:
	line = f.readline()
	if not line == '':
		linenumber+=1
		#x=line[len(line)-2:len(line)-1]
		#print "-"+x+"-" 
		#raw_input()
		if line[len(line)-2:len(line)-1]==":":	#found label
			prcessed.append(str(linenumber)+" REM "+line)
			labels.append(line[0:len(line)-2])
			linenums.append(linenumber)
		else:	#not a label
			prcessed.append(str(linenumber)+" "+line)
f.close()
print "POST-PROCESS:"
postproc=""
for line in prcessed:
	postproc=postproc+line
print "LABELS:"
for counter in range(0,len(labels)):
	postproc=postproc.replace(labels[counter],str(linenums[counter]))
	postproc=postproc.replace("REM "+str(linenums[counter]),"REM "+labels[counter])
	print labels[counter]+ " " + str(linenums[counter])
print postproc
print "WRITING PROGRAM.BAS"
FILE = open("PROGRAM.BAS","w")
FILE.write(postproc)
FILE.close()
