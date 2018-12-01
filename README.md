Tema 1 SPG
Ciupitu Andrei Valentin
341C4

Detalii de implementare =======================================================

Am realizat tema intr-o scena 2D in planul XY.

Pentru implementarea raului am pornit de la laboratorul 2 si am generat in 
geometry shader 2 curbe bezier. Pentru asezarea curbelor in plan am translatat
fiecare punct din curbele generate de-a lungul normalei in punctul respectiv.
Normalele le-am obtinut calculand tangentele in fiecare punct(derivata formulei
parametrice) si apoi calculand un vector perpendicular pe acestea.

Pentru animarea raului am modificat maparea texturii in functie de timp.

Pentru particule am folosit implementarea din laboratorul 5, schimband
dimensiunea particulelor, precum si parametrii initiali in functie de starea
curenta a raului.

Am implementat 3 efecte de post procesare: Bloom, Blur, Wave pe axa X.

Controls ======================================================================

T, R - modificare latimea raului
Num_Plus, Num_minus - animation speed
SPACE - ciclare efecte de post procesare

 
