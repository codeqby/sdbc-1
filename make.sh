rm */*.o
cd string
gmake
cd ../conf
gmake
cd ../ds
gmake
cd ../crypto
gmake
cd ../socket
gmake
cd ../sccli
gmake
cd ../scsrv
gmake

if [ -d ../pack ]
then
cd ../pack
gmake
fi

if [ -d ../sqli ]
then
cd ../sqli
gmake
fi

if [ -d ../dau ]
then
cd ../dau
gmake
fi

if [ -d ../sdbc ]
then
cd ../sdbc
gmake
fi

