Add the following line to syscall.h

Add after macro definition

//Global bitmap added by Chau Cao
extern BitMap * memMap;
//End changes by Chau Cao

Add the following line to exception.cc
Add before Exception Handler comment block

//Global Bitmap initialization added by Chau Cao
BitMap * memMap = new BitMap(NumPhysPages);
//Added by Chau Cao
