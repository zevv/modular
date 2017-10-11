
.include LMV981.MOD

* -5.0..+5.0 to 0..+3.3 converter
*
*                     C1
*                ,----| |----.
*                |           |
*        R1      |     R2    |
*  ,----\/\/\----o---\/\/\---.
*  | in        2 |           |
*  |             | | \       |
*  |             `-|- \      |
*  |               |   >-----o-- out
*  |          3  ,-|+ /
*  |            |  | /
*  |            <
*  |            < R3
*  |            <
*  |            |
*  |          4 |
*  o 5v AC      o 1.24v DC
*  |            |
*  --------------
*       0| GND


vs in 0 dc 0V ac 5V sin(0v 5v 1000Hz 0ms 0)
R1 in 2 10k
R2 2 out 3k3
C1 2 out 4.7nF
R3 3 4 2k48
vref 4 0 dc 1.24v 
X_AMP 3 2 vplus vmin out NSD LMV981
v1 vplus 0 dc 3.3v
v2 vmin 0 dc 0v

.tran 1us 3ms

.control
run
plot v(in), v(out)

ac dec 100 0.1Hz 48Khz

plot vdb(out)
plot 180/PI*cph(v(out))-180 ylimit -90 90

.endc

.end

