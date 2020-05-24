import math
import Gnuplot

z = []
k = []

'''
x = []
y = []

def scanBre_Linear(tgt, src, dt, ds):
	global x,y
	
	N = dt
	E = 0
	dsi = ds / dt
	dsf = ds % dt
	
	while(N):
		N = N - 1
		y.append(src)
		x.append(tgt)
		#print src,tgt
		
		tgt = tgt + 1
		src = src + dsi 
		E = E + dsf
		
		if E >= dt:
			E = E - dt
			src = src + 1

def scanColorAlongDepth(z0, z1, k0, k1):
	global z,k
	zrange = range(z0,z1+1)	
	for j in zrange:
		#dz = (j-z0)
		#ku = k0  +  (  (dz*dz*(k1-k0)) / ((z1-z0)*(z1-z0))  )
		#z.append(j)
		#k.append(ku)
				
		#dz = (j-z1)
		#kp = k1  -  (  (dz*dz*(k1-k0)) / ((z1-z0)*(z1-z0))  )
		#z.append(j)
		#k.append(kp)

'''
			
def scanColorAlongDepth(z0, z1, k0, k1):
	global z,k
	
	sz2 = (z1-z0)*(z1-z0)
	S2 = int(round(math.log(sz2,2)))
	SDZ2 = int(math.pow(2,S2))
	print SDZ2
	
	zrange = range(z0,z1+1)	
	for j in zrange:
		#dz = (j-z0)
		#ku = k0  +  (  (dz*dz*(k1-k0)) / ((z1-z0)*(z1-z0))  )
		
		dz = (j-z0)
		ku = k0  +  (  (dz*dz*(k1-k0)) / (SDZ2)  )
		
		
		z.append(j)
		k.append(ku)
				
		#dz = (j-z1)
		#kp = k1  -  (  (dz*dz*(k1-k0)) / ((z1-z0)*(z1-z0))  )
		#z.append(j)
		#k.append(kp)

#scanBre_2(0,0,24,20)
#scanDepth(10, 30, 45, 215)

scanColorAlongDepth(10, 30, 45, 215)
print z
print k
gp = Gnuplot.Gnuplot(persist=1)
data = Gnuplot.Data(z, k, title='myPersp')
gp('set style data lines')
gp('set xlabel "depth"')
gp('set ylabel "Color"')
gp.plot(data);
gp.hardcopy(filename="myPersp.png",terminal="png")
