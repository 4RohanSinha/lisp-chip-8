load_I:
	ld V14, 0
	ld V15, 128
	ld I, 0
load_I_LBL_2:
	sne V14, V2
	jp load_I_LBL_3
	add I, V15
	add I, V15
	add V14, 1
	jp load_I_LBL_2
load_I_LBL_3:
	add I, V3
	ret
print__str__:
	call load_I
	prints I
	ret
print__int__:
	printdb V2
	ret	
