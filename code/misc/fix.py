#script to change from arm to x86 target
arm = open('full.ll')
arm_s = arm.read()

x86 = open('full_x86.ll')
x86_s = x86.read()

arm.close()
x86.close()

arm_split = arm_s.split('\n\n')
x86_split = x86_s.split('\n\n')

#substitute header
arm_split[0] = x86_split[0]

#substitute attrs
arm_split[-3] = x86_split[-3]

#convert to string and save to file
s = ""
for l in arm_split:
    s = s + '\n' + l 
res = open('sost.ll', 'w')
res.write(s)
res.close()


