import matplotlib.pyplot as plt
import numpy as np
with open('log.txt','r') as f:
    log_data = f.read()
l = []
for iter,i in enumerate(log_data.split('\n')):
    if i != '':
        print (i)
        l.append(float(i))
    

num_turns = int(l[0])
del l[0]
print (l)
#print (len(l))
mean = np.array(l).astype(np.float32)
mean = mean.reshape([-1,num_turns])
x = np.mean(mean,axis = 0)

print ( x )
plt.plot(x[1:])
plt.show()





    